#include "SettingsDialog.h"
#include "DesignSystem.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setMinimumSize(640, 520);
    setMaximumSize(800, 650);
    
    setupUI();
    setupConnections();
    loadSettings();
    updateThemeColors();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 左侧分类导航
    QFrame* leftFrame = new QFrame(this);
    leftFrame->setFixedWidth(160);
    leftFrame->setObjectName("leftFrame");
    
    QVBoxLayout* leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setSpacing(8);
    leftLayout->setContentsMargins(12, 20, 12, 20);
    
    QLabel* navTitle = new QLabel("设置", leftFrame);
    navTitle->setObjectName("navTitle");
    QFont navFont = navTitle->font();
    navFont.setPointSizeF(18);
    navFont.setBold(true);
    navTitle->setFont(navFont);
    leftLayout->addWidget(navTitle);
    leftLayout->addSpacing(20);
    
    m_categoryList = new QListWidget(leftFrame);
    m_categoryList->setObjectName("categoryList");
    m_categoryList->setFrameShape(QFrame::NoFrame);
    m_categoryList->setSpacing(4);
    
    QListWidgetItem* appearanceItem = new QListWidgetItem("外观", m_categoryList);
    appearanceItem->setData(Qt::UserRole, 0);
    
    QListWidgetItem* advancedItem = new QListWidgetItem("高级", m_categoryList);
    advancedItem->setData(Qt::UserRole, 1);
    
    QListWidgetItem* aboutItem = new QListWidgetItem("关于", m_categoryList);
    aboutItem->setData(Qt::UserRole, 2);
    
    m_categoryList->setCurrentRow(0);
    leftLayout->addWidget(m_categoryList);
    leftLayout->addStretch();
    
    mainLayout->addWidget(leftFrame);

    // 右侧内容区域
    QFrame* rightFrame = new QFrame(this);
    rightFrame->setObjectName("rightFrame");
    
    QVBoxLayout* rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* pageTitle = new QLabel("外观设置", rightFrame);
    pageTitle->setObjectName("pageTitle");
    QFont pageFont = pageTitle->font();
    pageFont.setPointSizeF(14);
    pageFont.setBold(true);
    pageTitle->setFont(pageFont);
    pageTitle->setFixedHeight(50);
    rightLayout->addWidget(pageTitle);
    
    QFrame* titleLine = new QFrame(rightFrame);
    titleLine->setFrameShape(QFrame::HLine);
    titleLine->setFixedHeight(1);
    titleLine->setObjectName("separatorLine");
    rightLayout->addWidget(titleLine);
    
    m_stackedWidget = new QStackedWidget(rightFrame);
    m_stackedWidget->addWidget(wrapInScrollArea(createAppearancePage()));
    m_stackedWidget->addWidget(wrapInScrollArea(createAdvancedPage()));
    m_stackedWidget->addWidget(wrapInScrollArea(createAboutPage()));
    
    rightLayout->addWidget(m_stackedWidget);
    
    QFrame* bottomFrame = new QFrame(rightFrame);
    bottomFrame->setFixedHeight(60);
    bottomFrame->setObjectName("bottomFrame");
    
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomFrame);
    bottomLayout->setContentsMargins(20, 0, 20, 0);
    
    QPushButton* confirmBtn = new QPushButton("完成", bottomFrame);
    confirmBtn->setFixedSize(90, 34);
    confirmBtn->setObjectName("confirmButton");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    
    bottomLayout->addStretch();
    bottomLayout->addWidget(confirmBtn);
    
    rightLayout->addWidget(bottomFrame);
    
    mainLayout->addWidget(rightFrame);
    
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &SettingsDialog::onCategoryChanged);
}

QWidget* SettingsDialog::createAppearancePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(page);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    // 主题设置卡片
    QFrame* themeCard = new QFrame(page);
    themeCard->setObjectName("settingCard");
    QVBoxLayout* themeLayout = new QVBoxLayout(themeCard);
    themeLayout->setSpacing(12);
    themeLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* themeTitle = new QLabel("主题模式", themeCard);
    themeTitle->setObjectName("cardTitle");
    QFont cardFont = themeTitle->font();
    cardFont.setBold(true);
    themeTitle->setFont(cardFont);
    themeLayout->addWidget(themeTitle);
    
    QHBoxLayout* radioLayout = new QHBoxLayout();
    m_themeGroup = new QButtonGroup(this);
    m_lightRadio = new QRadioButton("浅色", themeCard);
    m_darkRadio = new QRadioButton("深色", themeCard);
    m_lightRadio->setObjectName("themeRadio");
    m_darkRadio->setObjectName("themeRadio");
    m_themeGroup->addButton(m_lightRadio, 0);
    m_themeGroup->addButton(m_darkRadio, 1);
    m_lightRadio->setChecked(true);
    
    radioLayout->addWidget(m_lightRadio);
    radioLayout->addWidget(m_darkRadio);
    radioLayout->addStretch();
    themeLayout->addLayout(radioLayout);
    
    contentLayout->addWidget(themeCard);

    // 标题栏设置卡片
    QFrame* titleBarCard = new QFrame(page);
    titleBarCard->setObjectName("settingCard");
    QVBoxLayout* titleBarLayout = new QVBoxLayout(titleBarCard);
    titleBarLayout->setSpacing(12);
    titleBarLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* titleBarTitle = new QLabel("标题栏", titleBarCard);
    titleBarTitle->setObjectName("cardTitle");
    titleBarTitle->setFont(cardFont);
    titleBarLayout->addWidget(titleBarTitle);
    
    QHBoxLayout* titleBarCheckLayout = new QHBoxLayout();
    QLabel* titleBarLabel = new QLabel("显示标题栏", titleBarCard);
    titleBarLabel->setObjectName("settingLabel");
    m_titleBarVisibleCheck = new QCheckBox(titleBarCard);
    m_titleBarVisibleCheck->setChecked(true);
    titleBarCheckLayout->addWidget(titleBarLabel);
    titleBarCheckLayout->addStretch();
    titleBarCheckLayout->addWidget(m_titleBarVisibleCheck);
    titleBarLayout->addLayout(titleBarCheckLayout);
    
    contentLayout->addWidget(titleBarCard);

    // 窗口样式卡片
    QFrame* windowCard = new QFrame(page);
    windowCard->setObjectName("settingCard");
    QVBoxLayout* windowLayout = new QVBoxLayout(windowCard);
    windowLayout->setSpacing(14);
    windowLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* windowTitle = new QLabel("窗口样式", windowCard);
    windowTitle->setObjectName("cardTitle");
    windowTitle->setFont(cardFont);
    windowLayout->addWidget(windowTitle);
    
    QHBoxLayout* roundedCheckLayout = new QHBoxLayout();
    QLabel* roundedLabel = new QLabel("启用圆角", windowCard);
    roundedLabel->setObjectName("settingLabel");
    m_roundedCornersCheck = new QCheckBox(windowCard);
    roundedCheckLayout->addWidget(roundedLabel);
    roundedCheckLayout->addStretch();
    roundedCheckLayout->addWidget(m_roundedCornersCheck);
    windowLayout->addLayout(roundedCheckLayout);
    
    QHBoxLayout* radiusLayout = new QHBoxLayout();
    QLabel* radiusTextLabel = new QLabel("圆角半径", windowCard);
    radiusTextLabel->setObjectName("subLabel");
    m_cornerRadiusSlider = new QSlider(Qt::Horizontal, windowCard);
    m_cornerRadiusSlider->setRange(4, 32);
    m_cornerRadiusSlider->setValue(12);
    m_cornerRadiusSlider->setEnabled(false);
    m_cornerRadiusLabel = new QLabel("12px", windowCard);
    m_cornerRadiusLabel->setObjectName("valueLabel");
    m_cornerRadiusLabel->setMinimumWidth(40);
    m_cornerRadiusLabel->setAlignment(Qt::AlignRight);
    
    radiusLayout->addWidget(radiusTextLabel);
    radiusLayout->addSpacing(12);
    radiusLayout->addWidget(m_cornerRadiusSlider);
    radiusLayout->addSpacing(8);
    radiusLayout->addWidget(m_cornerRadiusLabel);
    windowLayout->addLayout(radiusLayout);
    
    QHBoxLayout* transparencyLayout = new QHBoxLayout();
    QLabel* transparencyLabel = new QLabel("窗口透明度", windowCard);
    transparencyLabel->setObjectName("settingLabel");
    m_windowTransparencySlider = new QSlider(Qt::Horizontal, windowCard);
    m_windowTransparencySlider->setRange(30, 100);
    m_windowTransparencySlider->setValue(100);
    m_windowTransparencyLabel = new QLabel("100%", windowCard);
    m_windowTransparencyLabel->setObjectName("valueLabel");
    m_windowTransparencyLabel->setMinimumWidth(40);
    m_windowTransparencyLabel->setAlignment(Qt::AlignRight);
    
    transparencyLayout->addWidget(transparencyLabel);
    transparencyLayout->addSpacing(12);
    transparencyLayout->addWidget(m_windowTransparencySlider);
    transparencyLayout->addSpacing(8);
    transparencyLayout->addWidget(m_windowTransparencyLabel);
    windowLayout->addLayout(transparencyLayout);
    
    contentLayout->addWidget(windowCard);

    // 交互设置卡片
    QFrame* interactionCard = new QFrame(page);
    interactionCard->setObjectName("settingCard");
    QVBoxLayout* interactionLayout = new QVBoxLayout(interactionCard);
    interactionLayout->setSpacing(14);
    interactionLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* interactionTitle = new QLabel("交互效果", interactionCard);
    interactionTitle->setObjectName("cardTitle");
    interactionTitle->setFont(cardFont);
    interactionLayout->addWidget(interactionTitle);
    
    QHBoxLayout* borderLayout = new QHBoxLayout();
    QLabel* borderLabel = new QLabel("边框宽度", interactionCard);
    borderLabel->setObjectName("settingLabel");
    m_borderWidthSlider = new QSlider(Qt::Horizontal, interactionCard);
    m_borderWidthSlider->setRange(4, 20);
    m_borderWidthSlider->setValue(8);
    m_borderWidthLabel = new QLabel("8px", interactionCard);
    m_borderWidthLabel->setObjectName("valueLabel");
    m_borderWidthLabel->setMinimumWidth(40);
    m_borderWidthLabel->setAlignment(Qt::AlignRight);
    
    borderLayout->addWidget(borderLabel);
    borderLayout->addSpacing(12);
    borderLayout->addWidget(m_borderWidthSlider);
    borderLayout->addSpacing(8);
    borderLayout->addWidget(m_borderWidthLabel);
    interactionLayout->addLayout(borderLayout);
    
    QHBoxLayout* animLayout = new QHBoxLayout();
    QLabel* animLabel = new QLabel("动画速度", interactionCard);
    animLabel->setObjectName("settingLabel");
    m_animationSpeedSlider = new QSlider(Qt::Horizontal, interactionCard);
    m_animationSpeedSlider->setRange(100, 1000);
    m_animationSpeedSlider->setValue(500);
    m_animationSpeedSlider->setSingleStep(50);
    m_animationSpeedLabel = new QLabel("500ms", interactionCard);
    m_animationSpeedLabel->setObjectName("valueLabel");
    m_animationSpeedLabel->setMinimumWidth(40);
    m_animationSpeedLabel->setAlignment(Qt::AlignRight);
    
    animLayout->addWidget(animLabel);
    animLayout->addSpacing(12);
    animLayout->addWidget(m_animationSpeedSlider);
    animLayout->addSpacing(8);
    animLayout->addWidget(m_animationSpeedLabel);
    interactionLayout->addLayout(animLayout);
    
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    QLabel* opacityLabel = new QLabel("内容透明度", interactionCard);
    opacityLabel->setObjectName("settingLabel");
    m_opacitySlider = new QSlider(Qt::Horizontal, interactionCard);
    m_opacitySlider->setRange(50, 100);
    m_opacitySlider->setValue(100);
    m_opacityLabel = new QLabel("100%", interactionCard);
    m_opacityLabel->setObjectName("valueLabel");
    m_opacityLabel->setMinimumWidth(40);
    m_opacityLabel->setAlignment(Qt::AlignRight);
    
    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addSpacing(12);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addSpacing(8);
    opacityLayout->addWidget(m_opacityLabel);
    interactionLayout->addLayout(opacityLayout);
    
    contentLayout->addWidget(interactionCard);
    contentLayout->addStretch();

    return page;
}

QWidget* SettingsDialog::createAdvancedPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QFrame* placeholderCard = new QFrame(page);
    placeholderCard->setObjectName("settingCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(placeholderCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* placeholder = new QLabel("高级设置", placeholderCard);
    placeholder->setObjectName("cardTitle");
    QFont font = placeholder->font();
    font.setBold(true);
    placeholder->setFont(font);
    cardLayout->addWidget(placeholder);
    
    QLabel* desc = new QLabel("此处预留高级功能设置项", placeholderCard);
    desc->setObjectName("subLabel");
    cardLayout->addWidget(desc);
    
    layout->addWidget(placeholderCard);
    layout->addStretch();
    
    return page;
}

QWidget* SettingsDialog::createAboutPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QFrame* aboutCard = new QFrame(page);
    aboutCard->setObjectName("settingCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(aboutCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* title = new QLabel("关于", aboutCard);
    title->setObjectName("cardTitle");
    QFont font = title->font();
    font.setBold(true);
    title->setFont(font);
    cardLayout->addWidget(title);
    
    QLabel* version = new QLabel("版本 1.0.0", aboutCard);
    version->setObjectName("settingLabel");
    cardLayout->addWidget(version);
    
    QLabel* copyright = new QLabel("2026 Keycat. All rights reserved.", aboutCard);
    copyright->setObjectName("subLabel");
    cardLayout->addWidget(copyright);
    
    layout->addWidget(aboutCard);
    layout->addStretch();
    
    return page;
}

void SettingsDialog::setupConnections()
{
    connect(m_borderWidthSlider, &QSlider::valueChanged, 
            this, &SettingsDialog::onBorderWidthChanged);
    connect(m_animationSpeedSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onAnimationSpeedChanged);
    connect(m_roundedCornersCheck, &QCheckBox::stateChanged,
            this, &SettingsDialog::onRoundedCornersChanged);
    connect(m_cornerRadiusSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onCornerRadiusChanged);
    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &SettingsDialog::onOpacityChanged);
    connect(m_windowTransparencySlider, &QSlider::valueChanged,
            this, &SettingsDialog::onWindowTransparencyChanged);
    connect(m_themeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &SettingsDialog::onThemeModeChanged);
    
    connect(m_roundedCornersCheck, &QCheckBox::toggled,
            m_cornerRadiusSlider, &QSlider::setEnabled);
    connect(m_roundedCornersCheck, &QCheckBox::toggled,
            m_cornerRadiusLabel, &QLabel::setEnabled);

    connect(m_titleBarVisibleCheck, &QCheckBox::stateChanged,
        this, &SettingsDialog::onTitleBarVisibilityChanged);
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    updateThemeColors();
}

void SettingsDialog::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    Theme currentTheme = DesignSystem::instance()->currentTheme();
    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    
    QPainterPath path;
    path.addRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
    
    painter.fillPath(path, currentTheme.backgroundColor);
    
    if (isDark) {
        QPen borderPen(QColor(255, 255, 255, 30));
        borderPen.setWidth(1);
        painter.setPen(borderPen);
        painter.drawPath(path);
    } else {
        QPen borderPen(currentTheme.borderColor);
        borderPen.setWidth(1);
        painter.setPen(borderPen);
        painter.drawPath(path);
    }
}

void SettingsDialog::updateThemeColors()
{
    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    Theme currentTheme = DesignSystem::instance()->currentTheme();
    
    setAttribute(Qt::WA_TranslucentBackground);
    
    QString textColor = isDark ? "#E8E8E8" : "#1F1F1F";
    QString subTextColor = isDark ? "#A0A0A0" : "#666666";
    QString cardBg = isDark ? "rgba(255,255,255,0.05)" : "rgba(0,0,0,0.03)";
    QString sliderGroove = isDark ? "rgba(255,255,255,0.1)" : "rgba(0,0,0,0.08)";
    QString themeColor = currentTheme.primaryColor.name();
    QString navBg = isDark ? "rgba(255,255,255,0.03)" : "rgba(0,0,0,0.02)";
    
    QString styleSheet = QString(R"(
        QFrame#leftFrame {
            background-color: %12;
            border-radius: 10px;
        }
        QFrame#rightFrame {
            background-color: transparent;
        }
        QLabel#navTitle {
            color: %1;
            background-color: transparent;
            padding-left: 4px;
        }
        QLabel#pageTitle {
            color: %1;
            background-color: transparent;
            padding-left: 4px;
        }
        QListWidget#categoryList {
            background-color: transparent;
            border: none;
            outline: none;
            color: %2;
        }
        QListWidget#categoryList::item {
            height: 36px;
            border-radius: 8px;
            padding-left: 12px;
            color: %2;
        }
        QListWidget#categoryList::item:selected {
            background-color: %4;
            color: white;
        }
        QListWidget#categoryList::item:hover:!selected {
            background-color: %13;
        }
        QLabel#cardTitle {
            color: %1;
            font-size: 14px;
            background-color: transparent;
        }
        QLabel#settingLabel {
            color: %1;
            font-size: 13px;
            background-color: transparent;
        }
        QLabel#subLabel {
            color: %3;
            font-size: 12px;
            background-color: transparent;
        }
        QLabel#valueLabel {
            color: %4;
            font-size: 12px;
            font-weight: bold;
            background-color: transparent;
        }
        QFrame#separatorLine {
            background-color: %5;
        }
        QFrame#settingCard {
            background-color: %6;
            border-radius: 12px;
            border: none;
        }
        QFrame#bottomFrame {
            background-color: transparent;
            border-top: 1px solid %5;
        }
        QCheckBox {
            color: %1;
            font-size: 13px;
            spacing: 8px;
            background-color: transparent;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 5px;
            border: 2px solid %7;
            background-color: transparent;
        }
        QCheckBox::indicator:checked {
            background-color: %4;
            border-color: %4;
        }
        QCheckBox::indicator:hover {
            border-color: %4;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background: %8;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: %4;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            width: 14px;
            height: 14px;
            margin: -5px 0;
            border-radius: 7px;
            background: %4;
            border: 2px solid %9;
        }
        QSlider::handle:horizontal:hover {
            background: %10;
        }
        QPushButton#confirmButton {
            background-color: %4;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#confirmButton:hover {
            background-color: %10;
        }
        QPushButton#confirmButton:pressed {
            background-color: %11;
        }
        QRadioButton {
            color: %1;
            font-size: 13px;
            background-color: transparent;
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            border-radius: 8px;
            border: 2px solid %7;
        }
        QRadioButton::indicator:checked {
            background-color: %4;
            border-color: %4;
        }
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            width: 6px;
            background: transparent;
            border: none;
        }
        QScrollBar::handle:vertical {
            background: %8;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: %7;
        }
        QWidget {
            background-color: transparent;
        }
                QScrollArea#settingsScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollArea#settingsScrollArea > QWidget > QWidget {
            background-color: transparent;
        }
    )").arg(textColor)
        .arg(subTextColor)
        .arg(subTextColor)
        .arg(themeColor)
        .arg(isDark ? "rgba(255,255,255,0.06)" : "rgba(0,0,0,0.06)")
        .arg(cardBg)
        .arg(isDark ? "rgba(255,255,255,0.15)" : "rgba(0,0,0,0.15)")
        .arg(sliderGroove)
        .arg(isDark ? "rgba(255,255,255,0.2)" : "rgba(0,0,0,0.1)")
        .arg(currentTheme.primaryHoverColor.name())
        .arg(currentTheme.primaryColor.darker(120).name())
        .arg(navBg)
        .arg(isDark ? "rgba(255,255,255,0.06)" : "rgba(0,0,0,0.04)");
    
    setStyleSheet(styleSheet);
    
    if (m_roundedCornersCheck->isChecked()) {
        m_cornerRadius = m_cornerRadiusSlider->value();
    } else {
        m_cornerRadius = 0;
    }
    
    update();
}

void SettingsDialog::onCategoryChanged(int index)
{
    m_stackedWidget->setCurrentIndex(index);
    
    QStringList titles = {"外观设置", "高级设置", "关于"};
    if (index >= 0 && index < titles.size()) {
        QLabel* pageTitle = findChild<QLabel*>("pageTitle");
        if (pageTitle) {
            pageTitle->setText(titles[index]);
        }
    }
}

void SettingsDialog::onBorderWidthChanged(int value)
{
    m_borderWidthLabel->setText(QString("%1px").arg(value));
    emit borderWidthChanged(value);
    saveSettings();
}

void SettingsDialog::onAnimationSpeedChanged(int value)
{
    m_animationSpeedLabel->setText(QString("%1ms").arg(value));
    emit animationSpeedChanged(value);
    saveSettings();
}

void SettingsDialog::onRoundedCornersChanged(int state)
{
    bool enabled = (state == Qt::Checked);
    emit roundedCornersChanged(enabled);
    if (enabled) {
        emit cornerRadiusChanged(m_cornerRadiusSlider->value());
    }
    saveSettings();
    updateThemeColors();
}

void SettingsDialog::onCornerRadiusChanged(int value)
{
    m_cornerRadiusLabel->setText(QString("%1px").arg(value));
    emit cornerRadiusChanged(value);
    saveSettings();
    updateThemeColors();
}

void SettingsDialog::onOpacityChanged(int value)
{
    m_opacityLabel->setText(QString("%1%").arg(value));
    emit opacityChanged(value);
    saveSettings();
}

void SettingsDialog::onWindowTransparencyChanged(int value)
{
    m_windowTransparencyLabel->setText(QString("%1%").arg(value));
    emit windowTransparencyChanged(value);
    saveSettings();
}

void SettingsDialog::onThemeModeChanged(int id)
{
    if (id == 0) {
        DesignSystem::instance()->setThemeMode(DesignSystem::Light);
    } else {
        DesignSystem::instance()->setThemeMode(DesignSystem::Dark);
    }
    
    DesignSystem::instance()->saveThemeToJson();
    emit DesignSystem::instance()->themeChanged();
    
    updateThemeColors();
}

void SettingsDialog::loadSettings()
{
    QString configPath = QApplication::applicationDirPath() + "/settings.json";
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    
    if (obj.contains("borderWidth")) {
        m_borderWidthSlider->setValue(obj["borderWidth"].toInt());
    }
    if (obj.contains("animationSpeed")) {
        m_animationSpeedSlider->setValue(obj["animationSpeed"].toInt());
    }
    if (obj.contains("roundedCorners")) {
        m_roundedCornersCheck->setChecked(obj["roundedCorners"].toBool());
    }
    if (obj.contains("cornerRadius")) {
        m_cornerRadiusSlider->setValue(obj["cornerRadius"].toInt());
    }
    if (obj.contains("opacity")) {
        m_opacitySlider->setValue(obj["opacity"].toInt());
    }
    if (obj.contains("windowTransparency")) {
        m_windowTransparencySlider->setValue(obj["windowTransparency"].toInt());
    }
    if (obj.contains("themeMode")) {
        QString themeStr = obj["themeMode"].toString();
        if (themeStr == "dark") {
            m_darkRadio->setChecked(true);
        } else {
            m_lightRadio->setChecked(true);
        }
    }
    if (obj.contains("titleBarVisible")) {
        m_titleBarVisibleCheck->setChecked(obj["titleBarVisible"].toBool());
    }
}

void SettingsDialog::saveSettings()
{
    QString configPath = QApplication::applicationDirPath() + "/settings.json";
    QFile file(configPath);
    
    QJsonObject obj;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            obj = doc.object();
        }
        file.close();
    }
    
    obj["borderWidth"] = m_borderWidthSlider->value();
    obj["animationSpeed"] = m_animationSpeedSlider->value();
    obj["roundedCorners"] = m_roundedCornersCheck->isChecked();
    obj["cornerRadius"] = m_cornerRadiusSlider->value();
    obj["opacity"] = m_opacitySlider->value();
    obj["windowTransparency"] = m_windowTransparencySlider->value();
    obj["themeMode"] = (DesignSystem::instance()->themeMode() == DesignSystem::Dark) ? "dark" : "light";
    
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(obj);
        file.write(doc.toJson());
    }

    obj["titleBarVisible"] = m_titleBarVisibleCheck->isChecked();
}

void SettingsDialog::onTitleBarVisibilityChanged(int state)
{
    bool visible = (state == Qt::Checked);
    emit titleBarVisibilityChanged(visible);
    saveSettings();
}

QWidget* SettingsDialog::wrapInScrollArea(QWidget* page)
{
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidget(page);
    scrollArea->setObjectName("settingsScrollArea");
    return scrollArea;
}