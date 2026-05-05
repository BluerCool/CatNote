#pragma once

#include <QObject>
#include <QWidget>
#include <QAbstractNativeEventFilter>

#include "MaskWidget.h"

class TitleBar;

class FramelessHelper : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit FramelessHelper(QWidget *window, QObject *parent = nullptr);

    void setBorderWidth(int w);

    // 必须存在：Win 真处理；Linux/麒麟空实现 return false
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

#ifdef Q_OS_LINUX
    bool eventFilter(QObject *watched, QEvent *event) override;
#endif

#ifdef Q_OS_WIN
    // 只在 Win 才需要的接口
    void applyWindowStyle();
#endif

#ifdef Q_OS_LINUX
    void setTitleBar(TitleBar *titleBar);
#endif

    void setMask(MaskWidget *mask) { m_mask = mask; }

signals:
    void titleBarDoubleClicked();

private:
    QWidget *m_window = nullptr;
    TitleBar *m_titleBar = nullptr;
    MaskWidget *m_mask = nullptr;
    int m_borderWidth = 8;
    bool m_styleApplied = false;
    int titleBarSpacing = 6;
    int rightMargin = 15;
    int m_widgetTotalWidthPhysicalPixels = 0;
    int m_titleLeftTotalWidthPhysicalPixels = 0;
    int m_titleBarHeightPhysicalPixels = 0;
};
