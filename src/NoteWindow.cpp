#include "NoteWindow.h"
#include "DesignSystem.h"
#include "MessageManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QApplication>
#include <QBitmap>
#include <QShortcut>

NoteWindow::NoteWindow(BaseWindow* parent) 
    : BaseWindow(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    connect(this, &BaseWindow::resized, this, &NoteWindow::onWindowResized);

    setupCornerButtons();
    setupEditor();
    
    onWindowResized(width(), height());
    applySettings();
}

NoteWindow::~NoteWindow()
{
    if (m_settingsDialog) {
        delete m_settingsDialog;
    }
}

void NoteWindow::setupCornerButtons()
{
    m_themeBtn = new ThemeSwitcherButton(this);
    m_themeBtn->setFixedSize(40, 40);
    m_themeBtn->setIconSize(QSize(22, 22));

    m_settingsBtn = new Button("", 12, this);
    m_settingsBtn->setSvgIcon(DesignSystem::instance()->btnSettingsIconPath());
    m_settingsBtn->setFixedSize(40, 40);
    
    m_newFileBtn = new Button("", 12, this);
    m_newFileBtn->setSvgIcon(DesignSystem::instance()->newFileIconPath());
    m_newFileBtn->setFixedSize(36, 36);
    m_newFileBtn->setToolTip("新建文件 (Ctrl+N)");
    
    m_openFileBtn = new Button("", 12, this);
    m_openFileBtn->setSvgIcon(DesignSystem::instance()->openFileIconPath());
    m_openFileBtn->setFixedSize(36, 36);
    m_openFileBtn->setToolTip("打开文件 (Ctrl+O)");
    
    updateFileButtonStyles();
    
    connect(m_settingsBtn, &Button::clicked, this, &NoteWindow::onSettingsClicked);
    connect(m_newFileBtn, &Button::clicked, this, &NoteWindow::onNewFileClicked);
    connect(m_openFileBtn, &Button::clicked, this, &NoteWindow::onOpenFileClicked);
    
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, [this]()
        {
            m_settingsBtn->setSvgIcon(DesignSystem::instance()->btnSettingsIconPath());
        });
    connect(DesignSystem::instance(), &DesignSystem::themeChanged,
            this, &NoteWindow::updateFileButtonStyles);
            
    // 快捷键
    QShortcut *newShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(newShortcut, &QShortcut::activated, this, &NoteWindow::onNewFileClicked);
    
    QShortcut *openShortcut = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(openShortcut, &QShortcut::activated, this, &NoteWindow::onOpenFileClicked);
    
    QShortcut *saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(saveShortcut, &QShortcut::activated, this, &NoteWindow::onSaveFileClicked);
}

void NoteWindow::updateFileButtonStyles()
{
    if (m_newFileBtn) {
        m_newFileBtn->setSvgIcon(DesignSystem::instance()->newFileIconPath());
    }
    if (m_openFileBtn) {
        m_openFileBtn->setSvgIcon(DesignSystem::instance()->openFileIconPath());
    }
}

void NoteWindow::setCornerRadius(int radius)
{
    m_cornerRadius = radius;
    if (m_roundedCornersEnabled) {
        updateRoundedMask();
        update();
    }
}

void NoteWindow::enableRoundedCorners(bool enabled)
{
    m_roundedCornersEnabled = enabled;
    updateRoundedMask();
    update();
}

void NoteWindow::updateRoundedMask()
{
    if (!m_roundedCornersEnabled || m_cornerRadius <= 0) {
        clearMask();
        m_roundedPath = QPainterPath();
        return;
    }
    
    QBitmap bmp(size());
    bmp.fill(Qt::color0);
    
    QPainter painter(&bmp);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::color1);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
    painter.end();
    
    setMask(bmp);
    
    m_roundedPath = QPainterPath();
    m_roundedPath.addRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
}

void NoteWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (m_roundedCornersEnabled && m_cornerRadius > 0) {
        QColor bgColor = DesignSystem::instance()->backgroundColor();
        painter.fillPath(m_roundedPath, bgColor);
        
        QPen borderPen(DesignSystem::instance()->borderColor());
        borderPen.setWidth(1);
        painter.setPen(borderPen);
        painter.drawPath(m_roundedPath);
    } else {
        painter.fillRect(rect(), DesignSystem::instance()->backgroundColor());
    }
}

void NoteWindow::resizeEvent(QResizeEvent* event)
{
    BaseWindow::resizeEvent(event);
    if (m_roundedCornersEnabled) {
        updateRoundedMask();
    }
}

void NoteWindow::setupEditor()
{
    QWidget *content = contentWidget();
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    m_tabBar = new EditorTabBar(content);
    connect(m_tabBar, &EditorTabBar::tabActivated, this, &NoteWindow::onTabActivated);
    connect(m_tabBar, &EditorTabBar::tabClosed, this, &NoteWindow::onTabClosed);
    layout->addWidget(m_tabBar);
    
    m_editorStack = new QStackedWidget(content);
    m_editorStack->setObjectName("editorStack");
    layout->addWidget(m_editorStack, 1);
    
    // 默认创建一个新文件
    createNewFile();
}

void NoteWindow::createNewFile()
{
    MarkdownEditor *editor = new MarkdownEditor(m_editorStack);
    connect(editor, &MarkdownEditor::contentModified, this, &NoteWindow::onEditorModified);
    
    QString tempPath = "未命名-" + QString::number(m_editors.count() + 1);
    m_editors[tempPath] = editor;
    m_editorStack->addWidget(editor);
    m_tabBar->addTab(tempPath);
    
    onTabActivated(tempPath);
}

void NoteWindow::openExistingFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
        "打开 Markdown 文件", 
        QString(), 
        "Markdown 文件 (*.md *.markdown *.txt);;所有文件 (*.*)");
    
    if (filePath.isEmpty()) return;
    
    if (m_editors.contains(filePath)) {
        onTabActivated(filePath);
        return;
    }
    
    MarkdownEditor *editor = new MarkdownEditor(m_editorStack);
    if (!editor->loadFile(filePath)) {
        MessageManager::instance()->showMessage(Message::Error, 
            "无法打开文件: " + filePath);
        delete editor;
        return;
    }
    
    connect(editor, &MarkdownEditor::contentModified, this, &NoteWindow::onEditorModified);
    
    m_editors[filePath] = editor;
    m_editorStack->addWidget(editor);
    m_tabBar->addTab(filePath);
    
    onTabActivated(filePath);
    MessageManager::instance()->showMessage(Message::Success, 
        "已打开: " + QFileInfo(filePath).fileName());
}

bool NoteWindow::saveCurrentFile()
{
    MarkdownEditor *editor = currentEditor();
    if (!editor) return false;
    
    QString path = editor->filePath();
    if (path.isEmpty() || path.startsWith("未命名-")) {
        return saveCurrentFileAs();
    }
    
    if (editor->saveFile()) {
        m_tabBar->setTabModified(path, false);
        MessageManager::instance()->showMessage(Message::Success, 
            "已保存: " + QFileInfo(path).fileName());
        return true;
    }
    
    MessageManager::instance()->showMessage(Message::Error, "保存失败");
    return false;
}

bool NoteWindow::saveCurrentFileAs()
{
    MarkdownEditor *editor = currentEditor();
    if (!editor) return false;
    
    QString filePath = QFileDialog::getSaveFileName(this,
        "保存 Markdown 文件",
        QString(),
        "Markdown 文件 (*.md);;文本文件 (*.txt);;所有文件 (*.*)");
    
    if (filePath.isEmpty()) return false;
    
    if (!filePath.endsWith(".md") && !filePath.endsWith(".txt")) {
        filePath += ".md";
    }
    
    QString oldPath = editor->filePath();
    
    if (editor->saveFileAs(filePath)) {
        // 更新映射
        m_editors.remove(oldPath);
        m_editors[filePath] = editor;
        m_tabBar->closeTab(oldPath);
        m_tabBar->addTab(filePath);
        onTabActivated(filePath);
        MessageManager::instance()->showMessage(Message::Success,
            "已保存: " + QFileInfo(filePath).fileName());
        return true;
    }
    
    MessageManager::instance()->showMessage(Message::Error, "保存失败");
    return false;
}

MarkdownEditor* NoteWindow::currentEditor() const
{
    if (!m_editorStack) return nullptr;
    return qobject_cast<MarkdownEditor*>(m_editorStack->currentWidget());
}

void NoteWindow::onNewFileClicked()
{
    createNewFile();
}

void NoteWindow::onOpenFileClicked()
{
    openExistingFile();
}

void NoteWindow::onSaveFileClicked()
{
    saveCurrentFile();
}

void NoteWindow::onTabActivated(const QString &filePath)
{
    m_currentFilePath = filePath;
    m_tabBar->setActiveTab(filePath);
    
    if (m_editors.contains(filePath)) {
        m_editorStack->setCurrentWidget(m_editors[filePath]);
    }
    
    updateWindowTitle();
}

void NoteWindow::onTabClosed(const QString &filePath)
{
    if (!m_editors.contains(filePath)) return;
    
    MarkdownEditor *editor = m_editors[filePath];
    if (editor->isModified()) {
        // 可以在这里添加确认对话框
        int ret = QMessageBox::question(this, "未保存的更改",
            "文件有未保存的更改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            if (!saveCurrentFile()) return;
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    m_editors.remove(filePath);
    m_editorStack->removeWidget(editor);
    delete editor;
    m_tabBar->closeTab(filePath);
}

void NoteWindow::onEditorModified(bool modified)
{
    MarkdownEditor *editor = qobject_cast<MarkdownEditor*>(sender());
    if (!editor) return;
    
    QString path = editor->filePath();
    if (path.isEmpty()) {
        // 查找对应的key
        for (auto it = m_editors.begin(); it != m_editors.end(); ++it) {
            if (it.value() == editor) {
                path = it.key();
                break;
            }
        }
    }
    
    m_tabBar->setTabModified(path, modified);
    updateWindowTitle();
}

void NoteWindow::updateWindowTitle()
{
    MarkdownEditor *editor = currentEditor();
    if (!editor) {
        setTitle("Markdown Editor");
        return;
    }
    
    QString name = QFileInfo(editor->filePath()).fileName();
    if (name.isEmpty()) name = "未命名";
    if (editor->isModified()) name += " ●";
    
    setTitle(name + " - Markdown Editor");
}

void NoteWindow::onSettingsClicked()
{
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(this);
        
        connect(m_settingsDialog, &SettingsDialog::borderWidthChanged,
                this, &NoteWindow::onBorderWidthChanged);
        connect(m_settingsDialog, &SettingsDialog::animationSpeedChanged,
                this, &NoteWindow::onAnimationSpeedChanged);
        connect(m_settingsDialog, &SettingsDialog::roundedCornersChanged,
                this, &NoteWindow::onRoundedCornersChanged);
        connect(m_settingsDialog, &SettingsDialog::cornerRadiusChanged,
                this, &NoteWindow::onCornerRadiusChanged);
        connect(m_settingsDialog, &SettingsDialog::opacityChanged,
                this, &NoteWindow::onOpacityChanged);
        connect(m_settingsDialog, &SettingsDialog::windowTransparencyChanged,
                this, &NoteWindow::onWindowTransparencyChanged);
        connect(m_settingsDialog, &SettingsDialog::titleBarVisibilityChanged,
                this, &NoteWindow::onTitleBarVisibilityChanged);
    }
    
    m_settingsDialog->exec();
}

void NoteWindow::applySettings()
{
    QString configPath = QApplication::applicationDirPath() + "/settings.json";
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly)) return;
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    
    if (obj.contains("borderWidth")) {
        setBorderWidth(obj["borderWidth"].toInt());
    }
    if (obj.contains("animationSpeed")) {
        DesignSystem::instance()->setAnimationDuration(obj["animationSpeed"].toInt());
    }
    if (obj.contains("roundedCorners")) {
        enableRoundedCorners(obj["roundedCorners"].toBool());
    }
    if (obj.contains("cornerRadius")) {
        setCornerRadius(obj["cornerRadius"].toInt());
    }
    if (obj.contains("opacity")) {
        setWindowOpacity(obj["opacity"].toInt() / 100.0);
    }
    if (obj.contains("windowTransparency")) {
        setWindowOpacity(obj["windowTransparency"].toInt() / 100.0);
    }
    if (obj.contains("titleBarVisible")) {
        onTitleBarVisibilityChanged(obj["titleBarVisible"].toBool());
    }
}

void NoteWindow::onBorderWidthChanged(int width) { setBorderWidth(width); }
void NoteWindow::onAnimationSpeedChanged(int speed) { DesignSystem::instance()->setAnimationDuration(speed); }
void NoteWindow::onRoundedCornersChanged(bool enabled) { enableRoundedCorners(enabled); }
void NoteWindow::onCornerRadiusChanged(int radius) { setCornerRadius(radius); }
void NoteWindow::onOpacityChanged(int opacity) { setWindowOpacity(opacity / 100.0); }
void NoteWindow::onWindowTransparencyChanged(int transparency) { setWindowOpacity(transparency / 100.0); }

void NoteWindow::onTitleBarVisibilityChanged(bool visible)
{
    setTitleBarVisible(visible);
}

void NoteWindow::onWindowResized(int w, int h)
{
    if (m_themeBtn) {
        m_themeBtn->move(16, h - m_themeBtn->height() - 16);
    }
    
    if (m_settingsBtn) {
        m_settingsBtn->move(w - m_settingsBtn->width() - 16, 
                            h - m_settingsBtn->height() - 16);
    }
    
    if (m_openFileBtn) {
        m_openFileBtn->move(18, h - m_openFileBtn->height() - 64 - m_newFileBtn->height() - 8);
    }
    
    if (m_newFileBtn) {
        m_newFileBtn->move(18, h - m_newFileBtn->height() - 64);
    }
}