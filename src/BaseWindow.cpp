#include "BaseWindow.h"
#include <QWindow>
#include <QApplication>  // Added for QApplication::screens()
#include "DesignSystem.h"
#include "MessageManager.h"

#ifdef Q_OS_WIN

#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>

#pragma comment(lib, "Dwmapi.lib")

#endif // Q_OS_WIN

BaseWindow::BaseWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("BaseWindow");
    setWindowFlags(Qt::FramelessWindowHint);

    QSize miniSize(300, 164);
    setMinimumSize(miniSize);
    // 获取主屏幕尺寸
    int w = 0, h = 0;
    QScreen *screen = QApplication::screens().first();  // Updated from deprecated QGuiApplication::primaryScreen()
    if (screen)
    {
        QRect availGeom = screen->availableGeometry();  // Updated from deprecated availableSize()
        QSize screenSize = availGeom.size(); // 可用屏幕大小，不包括任务栏
        w = int(screenSize.width() * 0.50);         // % 宽度
        h = int(screenSize.height() * 0.55);        // % 高度
        if (w < miniSize.width() && h < miniSize.height())
        {
            w = miniSize.width();
            h = miniSize.height();
        }
        resize(w, h);
    }
    setContentsMargins(0, 0, 0, 0);

    // 主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题栏
    m_titleBar = new TitleBar(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(m_titleBar->m_titleBarHeight);
    mainLayout->addWidget(m_titleBar);

    // 内容区
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("contentArea");
    mainLayout->addWidget(m_contentWidget);

    // 深色遮罩
    m_mask = new MaskWidget(0, 0, this);

    // 安装 Helper
    m_helper = new FramelessHelper(this, this);
    //m_helper->setTitleBar(m_titleBar);
    m_helper->setMask(m_mask);
    m_helper->setBorderWidth(8); // 可调

    // 最小化
    connect(m_titleBar, &TitleBar::minimizeRequested,
            this, &BaseWindow::handleMinimize);

    // 最大化 / 还原
    connect(m_titleBar, &TitleBar::maximizeRestoreRequested,
            this, &BaseWindow::handleMaximizeRestore);
    // 双击标题栏最大化 / 还原
    connect(m_titleBar, &TitleBar::titleBarDoubleClicked,
            this, &BaseWindow::handleMaximizeRestore);

    // 关闭
    connect(m_titleBar, &TitleBar::closeRequested,
            this, &BaseWindow::handleClose);

    m_titleBarVisible = true;            
}

BaseWindow::~BaseWindow()
{
}

void BaseWindow::setTitle(const QString &text)
{
    m_titleBar->setTitle(text);
}

void BaseWindow::handleMinimize()
{
    onBeforeMinimize();

#ifdef Q_OS_WIN
    if (!m_hwnd)
        m_hwnd = reinterpret_cast<HWND>(winId());
    if (m_hwnd)
        ShowWindow(m_hwnd, SW_MINIMIZE);
#else
    showMinimized();
#endif
}

void BaseWindow::handleMaximizeRestore()
{
    if (isMaximized())
    {
        onBeforeRestore();
#ifdef Q_OS_WIN
        if (!m_hwnd)
            m_hwnd = reinterpret_cast<HWND>(winId());
        if (m_hwnd)
            ShowWindow(m_hwnd, SW_RESTORE);
#else
        showNormal();
#endif
    }
    else
    {
        onBeforeMaximize();
#ifdef Q_OS_WIN
        if (!m_hwnd)
            m_hwnd = reinterpret_cast<HWND>(winId());
        if (m_hwnd)
            ShowWindow(m_hwnd, SW_MAXIMIZE);
#else
        showMaximized();
#endif
    }
}

void BaseWindow::handleClose()
{
    onBeforeClose();
    close();
}

bool BaseWindow::isPointOnTitleBar() const
{
    QPoint cursorPos = QCursor::pos();
    if (!m_titleBar || !m_titleBar->isVisible()) 
        return false;
    
    // 获取标题栏在屏幕上的全局几何区域
    QRect titleBarGlobalRect = QRect(
        m_titleBar->mapToGlobal(QPoint(0, 0)),
        m_titleBar->size()
    );
    
    // 判断点是否在标题栏区域内
    return titleBarGlobalRect.contains(cursorPos);
}

void BaseWindow::mousePressEvent(QMouseEvent* event){
    if (event->button() == Qt::LeftButton && isPointOnTitleBar()) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();  // Updated from deprecated globalPos()
        m_dragging = true;  
    }
}

void BaseWindow::mouseMoveEvent(QMouseEvent* event){
    if (m_dragging && isMaximized()){
        handleMaximizeRestore();
    }
    else if(m_dragging && (event->buttons() & Qt::LeftButton)) {
        // 使用保存的偏移量移动窗口
        move(event->globalPosition().toPoint() - m_dragPosition);  // Updated from deprecated globalPos()
    }
}

void BaseWindow::mouseReleaseEvent(QMouseEvent* event){
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
}

void BaseWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    emit resized(width(), height());
    if (m_mask)
    {
        m_mask->setGeometry(0, 0, width(), height());
        m_mask->raise();
    }
}

// void BaseWindow::moveEvent(QMoveEvent *event)
// {
//     QWidget::moveEvent(event);
//     // 发送窗口左上角全局坐标
//     emit windowMoved(this->mapToGlobal(QPoint(0, 0)));
// }

void BaseWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *stateEvent = static_cast<QWindowStateChangeEvent *>(event);

        if (isMaximized())
        {
            m_titleBar->maximizeButton()->setIcon(DesignSystem::instance()->btnRestoreIcon());
            #ifdef Q_OS_WIN
            mainLayout->setContentsMargins(0, 0, 5, 0);
            #else
            mainLayout->setContentsMargins(0, 0, 0, 0);
            #endif
        }
        else if (stateEvent->oldState() & Qt::WindowMaximized)
        {
            m_titleBar->maximizeButton()->setIcon(DesignSystem::instance()->btnMaxIcon());
            mainLayout->setContentsMargins(0, 0, 0, 0);
        }
    }

    QWidget::changeEvent(event);
}

void BaseWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!property("_note_centered").toBool())
    {
        QScreen *screen = nullptr;
        if (windowHandle())
            screen = windowHandle()->screen();
        if (!screen)
            screen = QApplication::screens().first();  // Updated from deprecated QGuiApplication::primaryScreen()
        if (screen)
        {
            const QRect avail = screen->availableGeometry();
            move(avail.center() - rect().center());
            setProperty("_note_centered", true);
        }
    }

#ifdef Q_OS_WIN
    if (m_helper)
        m_helper->applyWindowStyle();
#endif
}

void BaseWindow::setTitleBarVisible(bool visible)
{
    m_titleBarVisible = visible;
    if (m_titleBar) {
        m_titleBar->setVisible(visible);
    }
    // 重新调整内容区域
    if (visible) {
        mainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        mainLayout->setContentsMargins(0, 0, 0, 0);
    }
}