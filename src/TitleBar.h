#pragma once

#include <QWidget>
#include <QLabel>
#include <QHoverEvent>
#include <QHBoxLayout>
#include <QToolButton>

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void setTitle(const QString &text)
    {
        m_titleLabel->setText(text);
    }

    QToolButton *minimizeButton() const
    {
        return btnMin;
    }
    QToolButton *maximizeButton() const
    {
        return btnMax;
    }
    QToolButton *closeButton() const
    {
        return btnClose;
    }

    bool isInButtonArea(const QPoint &globalPos) const
    {
        // globalPos: 全局坐标
        const QPoint p = mapFromGlobal(globalPos);

        // 如果按钮不存在直接认为不在按钮区域
        if (!btnMin || !btnMax || !btnClose)
            return false;

        // 注意：QToolButton 的 geometry() 是相对 titlebar 的坐标系
        if (btnMin->isVisible() && btnMin->geometry().contains(p))
            return true;
        if (btnMax->isVisible() && btnMax->geometry().contains(p))
            return true;
        if (btnClose->isVisible() && btnClose->geometry().contains(p))
            return true;

        return false;
    }

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
signals:
    void minimizeRequested();
    void maximizeRestoreRequested();
    void closeRequested();
    void titleBarDoubleClicked();

public:
    int m_titleBarHeight = 50;
    QLabel *m_titleLabel = nullptr;

protected:
    // 间距
    int titleBarSpacing = 0;
    int rightMargin = 0;

    // 标题栏相关控件
    QHBoxLayout *m_titleLayout = nullptr;
    QToolButton *btnMin = nullptr;
    QToolButton *btnMax = nullptr;
    QToolButton *btnClose = nullptr;
};
