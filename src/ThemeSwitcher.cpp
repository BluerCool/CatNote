#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include "ThemeSwitcher.h"
#include "DesignSystem.h"
#include "StyleSheet.h"

ThemeSwitcher::ThemeSwitcher(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_TranslucentBackground);
	setFocusPolicy(Qt::NoFocus);

	isDarkTheme = DesignSystem::instance()->themeMode() == DesignSystem::Dark ? true : false;

	anim = new QPropertyAnimation(this, "circleRadius");
	anim->setDuration(800);
	anim->setEasingCurve(QEasingCurve::InOutCubic);
	connect(anim, &QAbstractAnimation::stateChanged, this, [this](QAbstractAnimation::State newState, QAbstractAnimation::State)
		{
			if (newState == QAbstractAnimation::Running && !isDarkTheme)
			{
				// 正向时切换到深色
				DesignSystem::instance()->switchTheme();
				themeBtn->setIcon(DesignSystem::instance()->setThemeIcon());
				setThemeColor();
				emit DesignSystem::instance()->themeChanged();
			}
			else if (newState == QAbstractAnimation::Running && isDarkTheme)
			{
				// 反向时切换到浅色
				DesignSystem::instance()->switchTheme();
				themeBtn->setIcon(DesignSystem::instance()->setThemeIcon());
				setThemeColor();
				emit DesignSystem::instance()->themeChanged();
			}
		});

	connect(anim, &QPropertyAnimation::finished, this, [this]()
		{
			isDarkTheme = !isDarkTheme;
			currentBackground = QPixmap();  // 主动清理截图
		});
}

ThemeSwitcher::~ThemeSwitcher()
{
}

void ThemeSwitcher::setThemeColor()
{
	QPalette palette = QApplication::palette();
	QColor defaultColor = DesignSystem::instance()->themeMode() == DesignSystem::ThemeMode::Light ? Qt::black : Qt::white;
	palette.setColor(QPalette::WindowText, defaultColor);
	palette.setColor(QPalette::Text, defaultColor);
	palette.setColor(QPalette::ButtonText, defaultColor);
	palette.setColor(QPalette::PlaceholderText, DesignSystem::instance()->currentTheme().placeholderColor);
	QApplication::setPalette(palette);
}

void ThemeSwitcher::paintEvent(QPaintEvent* e)
{
	if (!currentBackground.isNull())
	{
		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

		QPainterPath path;
		QPainterPath circlePath;
		circlePath.addEllipse(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2, radius * 2);

		if (anim->direction() == QAbstractAnimation::Forward)
		{
			// 正向：从中心向外扩张 => 挖掉中间圆
			path.addRect(rect());     // 添加整个窗口区域
			path -= circlePath;       // 减去中间圆形区域（形成挖空效果）
		}
		else
		{
			// 反向：从外向中心收缩 => 只保留中间圆
			path = circlePath;
		}

		painter.setClipPath(path);
		painter.drawPixmap(0, 0, currentBackground);
	}
}

void ThemeSwitcher::resizeByMainWindow(int parentW, int parentH)
{
	resize(parentW, parentH);
	// 计算对角线长度作为最大半径
	int maxRadius = static_cast<int>(std::ceil(std::sqrt(parentW * parentW + parentH * parentH)));
	anim->setStartValue(0);
	anim->setEndValue(maxRadius);
}

void ThemeSwitcher::startSwitchTheme(QPixmap grab, QPushButton* btn, QPoint btnGlobalCenter)
{
	if (anim->state() == QAbstractAnimation::Running)
	{
		return;
	}

	if (btn)
	{
		themeBtn = btn;
		currentBackground = grab;

		// 获取圆形的中心点 (全局坐标转为当前控件的局部坐标)
		circleCenter = mapFromGlobal(btnGlobalCenter);

		if (!isDarkTheme)
		{
			anim->setDirection(QAbstractAnimation::Forward);
		}
		else
		{
			anim->setDirection(QAbstractAnimation::Backward);
		}

		anim->start();
	}
}

//=========================ThemeSwitcherButton===========================
// ThemeSwitcherButton::ThemeSwitcherButton(QWidget* parent)
//     : QPushButton(parent)
// {
//     // 设置按钮样式
//     setFixedSize(40, 40);
//     setIcon(DesignSystem::instance()->setThemeIcon());
//     setIconSize(QSize(24, 24));
//     setStyleSheet(themeSwitcherBtnQss());
//     // 获取主窗口（最顶层的窗口）
//     m_mainWindow = window();
    
//     // 创建内部的 ThemeSwitcher（覆盖整个主窗口）
//     if (m_mainWindow) {
//         m_switcher = new ThemeSwitcher(m_mainWindow);
//         m_switcher->resizeByMainWindow(m_mainWindow->width(), m_mainWindow->height());
//         m_switcher->hide();  // 初始隐藏，只在动画时显示
//     }
// }
ThemeSwitcherButton::ThemeSwitcherButton(QWidget* parent)
    : QPushButton(parent)
{
    // 设置固定大小
    setFixedSize(52, 52);
    setIcon(DesignSystem::instance()->setThemeIcon());
    setIconSize(QSize(26, 26));
    
    // 关键：完全透明无边框
    setFlat(true);  // 扁平化，去除默认按钮效果
    setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 26px;
            padding: 0px;
        }
        QPushButton:hover {
            background-color: rgba(128, 128, 128, 30);
        }
        QPushButton:pressed {
            background-color: rgba(128, 128, 128, 50);
        }
    )");
    
    // 去掉默认焦点矩形
    setFocusPolicy(Qt::NoFocus);
    
    m_mainWindow = window();
    
    if (m_mainWindow) {
        m_switcher = new ThemeSwitcher(m_mainWindow);
        m_switcher->resizeByMainWindow(m_mainWindow->width(), m_mainWindow->height());
        m_switcher->hide();
    }
}

ThemeSwitcherButton::~ThemeSwitcherButton()
{
    // m_switcher 会随父窗口自动销毁
}

void ThemeSwitcherButton::mouseReleaseEvent(QMouseEvent* e)
{
    QPushButton::mouseReleaseEvent(e);
    startSwitchAnimation();
}

void ThemeSwitcherButton::startSwitchAnimation()
{
    if (!m_switcher || !m_mainWindow) return;
    
    // 更新主窗口引用（防止窗口变化）
    m_mainWindow = window();
    
    // 截取主窗口内容
    QPixmap screenshot = m_mainWindow->grab();
    
    // 获取按钮全局中心点
    QPoint btnGlobalCenter = mapToGlobal(rect().center());
    
    // 确保 switcher 覆盖整个主窗口
    m_switcher->resizeByMainWindow(m_mainWindow->width(), m_mainWindow->height());
    m_switcher->show();
    m_switcher->raise();  // 提升到最前
    
    // 开始动画
    m_switcher->startSwitchTheme(screenshot, this, btnGlobalCenter);
}
