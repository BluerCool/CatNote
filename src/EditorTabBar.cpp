#include "EditorTabBar.h"
#include "DesignSystem.h"
#include <QFileInfo>
#include <QPainter>

EditorTab::EditorTab(const QString &filePath, QWidget *parent)
    : QWidget(parent), m_filePath(filePath)
{
    setFixedHeight(36);
    setCursor(Qt::PointingHandCursor);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(6);

    m_nameLabel = new QLabel(fileName(), this);
    m_nameLabel->setObjectName("tabNameLabel");
    layout->addWidget(m_nameLabel);

    m_closeBtn = new QPushButton("×", this);
    m_closeBtn->setFixedSize(16, 16);
    m_closeBtn->setObjectName("tabCloseBtn");
    m_closeBtn->setCursor(Qt::ArrowCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        emit closeRequested(this);
    });
    layout->addWidget(m_closeBtn);

    updateStyle();
}

QString EditorTab::fileName() const
{
    if (m_filePath.isEmpty()) return "未命名";
    return QFileInfo(m_filePath).fileName();
}

void EditorTab::setActive(bool active)
{
    m_active = active;
    updateStyle();
}

void EditorTab::setModified(bool modified)
{
    m_modified = modified;
    updateStyle();
}

void EditorTab::updateStyle()
{
    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    QString bgColor, textColor, hoverBg;

    if (m_active) {
        bgColor = isDark ? "#3a3a3a" : "#ffffff";
        textColor = isDark ? "#e8e8e8" : "#1f1f1f";
    } else {
        bgColor = isDark ? "#2d2d2d" : "#f0f0f0";
        textColor = isDark ? "#a0a0a0" : "#666666";
        if (m_hovered) {
            bgColor = isDark ? "#353535" : "#e8e8e8";
        }
    }

    if (m_modified) {
        textColor = DesignSystem::instance()->primaryColor().name();
    }

    QString style = QString(
        "background-color: %1;"
        "border: none;"
        "border-bottom: 2px solid %2;"
        "border-radius: 0px;"
    ).arg(bgColor).arg(m_active ? DesignSystem::instance()->primaryColor().name() : "transparent");

    setStyleSheet(style);

    m_nameLabel->setStyleSheet(QString("color: %1; background: transparent; font-size: 13px;").arg(textColor));
    m_closeBtn->setStyleSheet(QString(
        "QPushButton { color: %1; background: transparent; border: none; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { color: #ff4444; background: transparent; }"
    ).arg(textColor));
}

void EditorTab::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(this);
    }
    QWidget::mousePressEvent(event);
}

void EditorTab::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    if (!m_active) updateStyle();
    QWidget::enterEvent(event);
}

void EditorTab::leaveEvent(QEvent *event)
{
    m_hovered = false;
    if (!m_active) updateStyle();
    QWidget::leaveEvent(event);
}

EditorTabBar::EditorTabBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(36);
    setObjectName("editorTabBar");

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addStretch();

    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    setStyleSheet(QString("background-color: %1; border-bottom: 1px solid %2;")
        .arg(isDark ? "#2d2d2d" : "#e0e0e0")
        .arg(isDark ? "#3a3a3a" : "#d0d0d0"));

    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, [this]() {
        bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
        setStyleSheet(QString("background-color: %1; border-bottom: 1px solid %2;")
            .arg(isDark ? "#2d2d2d" : "#e0e0e0")
            .arg(isDark ? "#3a3a3a" : "#d0d0d0"));
        for (auto *tab : m_tabs) {
            tab->updateStyle();
        }
    });
}

void EditorTabBar::addTab(const QString &filePath)
{
    if (hasTab(filePath)) {
        setActiveTab(filePath);
        return;
    }

    EditorTab *tab = new EditorTab(filePath, this);
    connect(tab, &EditorTab::clicked, this, [this, tab]() {
        setActiveTab(tab->filePath());
        emit tabActivated(tab->filePath());
    });
    connect(tab, &EditorTab::closeRequested, this, [this, tab]() {
        emit tabClosed(tab->filePath());
    });

    m_tabs.append(tab);
    m_layout->insertWidget(m_layout->count() - 1, tab);
    setActiveTab(filePath);
}

void EditorTabBar::closeTab(const QString &filePath)
{
    EditorTab *tab = findTab(filePath);
    if (!tab) return;

    m_tabs.removeOne(tab);
    tab->deleteLater();
    m_layout->removeWidget(tab);

    if (m_tabs.isEmpty()) {
        emit tabActivated("");
    } else {
        setActiveTab(m_tabs.last()->filePath());
        emit tabActivated(m_tabs.last()->filePath());
    }
}

void EditorTabBar::setActiveTab(const QString &filePath)
{
    for (EditorTab *tab : m_tabs) {
        tab->setActive(tab->filePath() == filePath);
    }
}

QString EditorTabBar::currentFilePath() const
{
    for (EditorTab *tab : m_tabs) {
        if (tab->isActive()) return tab->filePath();
    }
    return "";
}

bool EditorTabBar::hasTab(const QString &filePath) const
{
    return findTab(const_cast<QString&>(filePath)) != nullptr;
}

void EditorTabBar::setTabModified(const QString &filePath, bool modified)
{
    EditorTab *tab = findTab(filePath);
    if (tab) tab->setModified(modified);
}

QList<QString> EditorTabBar::allFilePaths() const
{
    QList<QString> paths;
    for (EditorTab *tab : m_tabs) {
        paths.append(tab->filePath());
    }
    return paths;
}

EditorTab* EditorTabBar::findTab(const QString &filePath) const
{
    for (EditorTab *tab : m_tabs) {
        if (tab->filePath() == filePath) return tab;
    }
    return nullptr;
}

void EditorTabBar::updateLayout()
{
    // Layout auto-updates
}