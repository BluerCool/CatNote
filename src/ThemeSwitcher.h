#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QPushButton>

class ThemeSwitcher : public QWidget
{
	Q_OBJECT
		Q_PROPERTY(int circleRadius READ circleRadius WRITE setCircleRadius)
public:
	ThemeSwitcher(QWidget* parent);
	~ThemeSwitcher();

	int circleRadius() const { return radius; }
	void setCircleRadius(int r) { radius = r; update(); }
	void setThemeColor();
protected:
	void paintEvent(QPaintEvent* e) override;
public slots:
	void startSwitchTheme(QPixmap grab, QPushButton* btn, QPoint btnGlobalCenter);
	void resizeByMainWindow(int parentW, int parentH);
private:
	QPixmap currentBackground;
	QColor bgColor;
	int radius = 0;
	bool isDarkTheme;
	QPushButton* themeBtn = nullptr;
	QPropertyAnimation* anim = nullptr;
	QPoint circleCenter = QPoint(width() / 2, height() / 2); // 默认值为窗口中心
};

class ThemeSwitcherButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ThemeSwitcherButton(QWidget* parent = nullptr);
    ~ThemeSwitcherButton();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    void startSwitchAnimation();
    
    ThemeSwitcher* m_switcher = nullptr;  // 原来的 ThemeSwitcher 作为内部成员
    QWidget* m_mainWindow = nullptr;      // 主窗口引用
};
