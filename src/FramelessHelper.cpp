#include "FramelessHelper.h"
#include "TitleBar.h"

#include <QMouseEvent>
#include <QApplication>
#include <QWindow>
#include <QScreen>
#include <QChildEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#pragma comment(lib, "Dwmapi.lib")
#endif

namespace {
bool isInButtonAreaSafe(const TitleBar *titleBar, const QPoint &globalPos)
{
    return titleBar && titleBar->isInButtonArea(globalPos);
}

Qt::Edges hitTestEdges(const QPoint &pos, const QRect &rect, int borderWidth)
{
    const bool left = pos.x() < borderWidth;
    const bool right = pos.x() > rect.width() - borderWidth;
    const bool top = pos.y() < borderWidth;
    const bool bottom = pos.y() > rect.height() - borderWidth;

    if (left && top)
        return Qt::TopEdge | Qt::LeftEdge;
    if (right && top)
        return Qt::TopEdge | Qt::RightEdge;
    if (left && bottom)
        return Qt::BottomEdge | Qt::LeftEdge;
    if (right && bottom)
        return Qt::BottomEdge | Qt::RightEdge;
    if (left)
        return Qt::LeftEdge;
    if (right)
        return Qt::RightEdge;
    if (top)
        return Qt::TopEdge;
    if (bottom)
        return Qt::BottomEdge;
    return Qt::Edges();
}

Qt::CursorShape cursorForEdges(Qt::Edges edge)
{
    if (edge == (Qt::TopEdge | Qt::LeftEdge) || edge == (Qt::BottomEdge | Qt::RightEdge))
        return Qt::SizeFDiagCursor;
    if (edge == (Qt::TopEdge | Qt::RightEdge) || edge == (Qt::BottomEdge | Qt::LeftEdge))
        return Qt::SizeBDiagCursor;
    if (edge == Qt::LeftEdge || edge == Qt::RightEdge)
        return Qt::SizeHorCursor;
    if (edge == Qt::TopEdge || edge == Qt::BottomEdge)
        return Qt::SizeVerCursor;
    return Qt::ArrowCursor;
}

void enableMouseTrackingRecursive(QWidget *widget)
{
    if (!widget)
        return;
    widget->setMouseTracking(true);
    widget->setAttribute(Qt::WA_Hover, true);
    const auto children = widget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (auto *child : children)
        enableMouseTrackingRecursive(child);
}
} // namespace

FramelessHelper::FramelessHelper(QWidget *window, QObject *parent)
    : QObject(parent),
      m_window(window)
{
#ifndef Q_OS_WIN
    // 非 Windows（Linux/麒麟）：监听 Qt 事件
    if (window)
    {
        qApp->installEventFilter(this);
        enableMouseTrackingRecursive(window);
    }
#endif
}

void FramelessHelper::setBorderWidth(int w)
{
    m_borderWidth = w;
}

#ifdef Q_OS_WIN

bool FramelessHelper::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);

    if (!m_window)
        return false;

    MSG *msg = static_cast<MSG *>(message);

    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
        // 去掉非客户区，让客户区占满整个窗口（无边框）
        if (msg->wParam)
        {
            *result = 0;
            return true;
        }
        break;
    }

    case WM_NCLBUTTONDBLCLK:
    {
        // 如果存在遮罩则响应遮罩，禁止系统事件（双击最大化）
        if (m_mask && m_mask->isVisible())
        {
            *result = 0;
            return true;
        }
        break;
    }

    case WM_NCHITTEST:
    {
        // 遮罩判断
        if (m_mask && m_mask->isVisible())
        {
            *result = HTCLIENT;
            return true;
        }

        const LONG x = GET_X_LPARAM(msg->lParam);
        const LONG y = GET_Y_LPARAM(msg->lParam);

        RECT winRect;
        GetWindowRect(HWND(m_window->winId()), &winRect);

        // 边缘缩放
        const bool left = x < winRect.left + m_borderWidth;
        const bool right = x > winRect.right - m_borderWidth;
        const bool top = y < winRect.top + m_borderWidth;
        const bool bottom = y > winRect.bottom - m_borderWidth;

        if (left && top)
        {
            *result = HTTOPLEFT;
            return true;
        }
        if (right && top)
        {
            *result = HTTOPRIGHT;
            return true;
        }
        if (left && bottom)
        {
            *result = HTBOTTOMLEFT;
            return true;
        }
        if (right && bottom)
        {
            *result = HTBOTTOMRIGHT;
            return true;
        }
        if (left)
        {
            *result = HTLEFT;
            return true;
        }
        if (right)
        {
            *result = HTRIGHT;
            return true;
        }
        if (top)
        {
            *result = HTTOP;
            return true;
        }
        if (bottom)
        {
            *result = HTBOTTOM;
            return true;
        }

        // 标题栏区域拖动
        if (m_titleBar)
        {
            const int rightBoundary = winRect.right - m_widgetTotalWidthPhysicalPixels;
            if (x > winRect.left + m_titleLeftTotalWidthPhysicalPixels &&
                x < rightBoundary &&
                y > winRect.top &&
                y < winRect.top + m_titleBarHeightPhysicalPixels)
            {
                *result = HTCAPTION;
                return true;
            }
        }

        break;
    }

    default:
        break;
    }

    return false;
}

void FramelessHelper::applyWindowStyle()
{
    if (m_styleApplied)
        return;

    if (!m_window)
        return;

    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    if (!hwnd)
        return;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    // 非客户区被隐藏了，但仍保持标准窗口特性（动画、最大化最小化等）
    style |= WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLong(hwnd, GWL_STYLE, style);

    DWORD cornerPref = 2;
    DwmSetWindowAttribute(hwnd, 33, &cornerPref, sizeof(cornerPref));

    qApp->installNativeEventFilter(this);

    m_styleApplied = true;
}

#else // ================= Linux / 麒麟 =================

void FramelessHelper::setTitleBar(TitleBar *titleBar)
{
    m_titleBar = titleBar;
    if (!m_titleBar)
        return;

    // 标题栏右侧所有控件的长宽转为物理像素后在 native 事件中限制标题栏的范围
    const qreal dpiScale = QApplication::screens().first()->devicePixelRatio();  // Updated from deprecated primaryScreen()

    // 计算控件总宽度（逻辑像素） 右侧3个控件 + 2个间隔 + 右边距
    const int widgetTotalWidth =
        rightMargin +
        m_titleBar->minimizeButton()->width() +
        m_titleBar->maximizeButton()->width() +
        m_titleBar->closeButton()->width() +
        2 * titleBarSpacing;

    // 转换为物理像素
    m_widgetTotalWidthPhysicalPixels = static_cast<int>(widgetTotalWidth * dpiScale);

    // 同理转换标题栏高度（你的 titleBar 里如果是 public 成员就保留；否则建议改 getter）
    m_titleBarHeightPhysicalPixels = static_cast<int>(m_titleBar->m_titleBarHeight * dpiScale);
}

// 关键：必须提供一个空实现，否则会链接 undefined reference
bool FramelessHelper::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
}

bool FramelessHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_window)
        return false;

    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget)
        return false;

    if (widget->window() != m_window)
        return false;

    if (m_mask && m_mask->isVisible())
    {
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::MouseMove)
        {
            // 遮罩显示时不拦截，让遮罩/对话框正常接收事件
            return false;
        }
    }

    switch (event->type())
    {
    case QEvent::ChildAdded:
    {
        auto *ce = static_cast<QChildEvent *>(event);
        if (auto *child = qobject_cast<QWidget *>(ce->child()))
            enableMouseTrackingRecursive(child);
        break;
    }

    case QEvent::MouseButtonPress:
    {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton)
        {
            const QPoint globalPos = me->globalPosition().toPoint();

            if (isInButtonAreaSafe(m_titleBar, globalPos))
                return false;

            if (!m_window->isMaximized())
            {
                const QPoint pos = m_window->mapFromGlobal(globalPos);
                const QRect r = m_window->rect();
                const Qt::Edges edge = hitTestEdges(pos, r, m_borderWidth);
                if (edge != Qt::Edges())
                {
                    if (m_window->windowHandle())
                        m_window->windowHandle()->startSystemResize(edge);
                    return true;
                }
            }

            if (m_titleBar)
            {
                const QPoint localPos = m_titleBar->mapFromGlobal(globalPos);
                if (m_titleBar->rect().contains(localPos))
                {
                    if (m_window->windowHandle())
                        m_window->windowHandle()->startSystemMove();
                    return true;
                }
            }
        }
        break;
    }

    case QEvent::MouseMove:
    {
        auto *me = static_cast<QMouseEvent *>(event);

        const QPoint globalPos = me->globalPosition().toPoint();
        const bool inButtonArea = isInButtonAreaSafe(m_titleBar, globalPos);

        if (m_window->isMaximized())
        {
            m_window->unsetCursor();
            return false;
        }

        const QPoint pos = m_window->mapFromGlobal(globalPos);
        const QRect r = m_window->rect();
        Qt::Edges edge = hitTestEdges(pos, r, m_borderWidth);
        if (inButtonArea)
            edge = Qt::Edges();

        if (edge != Qt::Edges())
            m_window->setCursor(cursorForEdges(edge));
        else
            m_window->unsetCursor();
        break;
    }

    case QEvent::Leave:
    {
        m_window->unsetCursor();
        break;
    }

    default:
        break;
    }

    return false;
}

#endif // Q_OS_WIN
