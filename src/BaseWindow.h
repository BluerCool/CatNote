#pragma once

#include <QWidget>
#include "FramelessHelper.h"
#include "TitleBar.h"
#include "MaskWidget.h"

class BaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);
    ~BaseWindow();

    void setTitle(const QString &text);
    QWidget *contentWidget() const { return m_contentWidget; }
    void setBorderWidth(int w)
    {
        m_helper->setBorderWidth(w);
    }

    // 获取深色遮罩
    MaskWidget *maskWidget() const { return m_mask; }

    void setTitleBarVisible(bool visible);
    bool isTitleBarVisible() const { return m_titleBarVisible; }
    TitleBar* titleBar() const { return m_titleBar; }

protected:
    void resizeEvent(QResizeEvent *event);
    //void moveEvent(QMoveEvent *event);
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *event);

protected:
    // 子类可重写
    virtual void onBeforeMinimize() {};
    virtual void onBeforeMaximize() {}
    virtual void onBeforeRestore() {};
    virtual void onBeforeClose() {};

    void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void resized(int w, int h);
    void windowMoved(const QPoint &topLeft);

private:
    void handleMinimize();
    void handleMaximizeRestore();
    void handleClose();
    bool isPointOnTitleBar() const;

    bool m_titleBarVisible = true;

protected:
#ifdef Q_OS_WIN
    HWND m_hwnd = nullptr;
#endif
    QVBoxLayout *mainLayout = nullptr;
    TitleBar *m_titleBar = nullptr;
    QWidget *m_contentWidget = nullptr;
    MaskWidget *m_mask = nullptr;

    FramelessHelper *m_helper = nullptr;

    int m_titleBarHeight = 50;
    int m_naviWidth = 62;

    QRect m_normalGeometry;

    // 拖动的相关变量
    bool m_dragging = false;
    QPoint m_dragPosition;
};
