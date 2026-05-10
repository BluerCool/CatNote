#include "TitleBar.h"
#include "DesignSystem.h"
#include "StyleSheet.h"

#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>

TitleBar::TitleBar(QWidget *parent)
    : QWidget{parent}
{
    m_titleLayout = new QHBoxLayout(this);
    m_titleLayout->setContentsMargins(20, 0, 0, 0);  // 将标题文字向右移动一定距离
    m_titleLayout->setSpacing(titleBarSpacing);

    QFont font;
    font.setPointSizeF(16);
    font.setBold(true);
    m_titleLabel = new QLabel("", this);
    m_titleLabel->setFont(font);
    m_titleLabel->setContentsMargins(10, 0, 0, 0);  // 进一步增加左侧间距

    // 在标题栏里放标题文字 + 三个按钮
    btnMin = new QToolButton(this);
    btnMax = new QToolButton(this);
    btnClose = new QToolButton(this);

    auto *ins = DesignSystem::instance();
    btnMin->setIcon(ins->btnMinIcon());
    btnMax->setIcon(ins->btnMaxIcon());
    btnClose->setIcon(ins->btnCloseIcon());

    btnMin->setStyleSheet(StyleSheet::toolBtnQss());
    btnMax->setStyleSheet(StyleSheet::toolBtnQss());
    btnClose->setStyleSheet(StyleSheet::toolBtnQss(true));

    // btnMin->setFixedSize(14, 14);
    // btnMax->setFixedSize(14, 14);
    // btnClose->setFixedSize(14, 14);

    constexpr size_t buttonSize = 50;  // 增加点击区域
    btnMin->setFixedSize(buttonSize, buttonSize);
    btnMax->setFixedSize(buttonSize, buttonSize);
    btnClose->setFixedSize(buttonSize, buttonSize);
    
    constexpr size_t iconSize = 15;
    btnMin->setIconSize(QSize(iconSize, iconSize));   
    btnMax->setIconSize(QSize(iconSize, iconSize));
    btnClose->setIconSize(QSize(iconSize, iconSize));

    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(btnMin);
    m_titleLayout->addWidget(btnMax);
    m_titleLayout->addWidget(btnClose);

    connect(btnMin, &QToolButton::clicked, this, &TitleBar::minimizeRequested);
    connect(btnMax, &QToolButton::clicked, this, &TitleBar::maximizeRestoreRequested);
    connect(btnClose, &QToolButton::clicked, this, &TitleBar::closeRequested);

    setMouseTracking(true);
}

TitleBar::~TitleBar()
{
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    // 这句话的意思是：用样式表(CSS)定义的背景来绘制我自己
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !isInButtonArea(event->globalPosition().toPoint()))
    {
        emit titleBarDoubleClicked();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}