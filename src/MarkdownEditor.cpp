#include "MarkdownEditor.h"
#include "DesignSystem.h"
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

MarkdownEditor::MarkdownEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setObjectName("markdownEditor");
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(40);

    QFont font("Consolas, Microsoft YaHei, sans-serif", 11);
    setFont(font);

    m_highlighter = new MarkdownHighlighter(document());

    setThemeColors();

    connect(this, &QPlainTextEdit::textChanged, this, &MarkdownEditor::onTextChanged);
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, &MarkdownEditor::setThemeColors);
}

MarkdownEditor::~MarkdownEditor()
{
}

void MarkdownEditor::setThemeColors()
{
    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    QColor bgColor = isDark ? QColor(45, 45, 45) : QColor(255, 255, 255);
    QColor textColor = isDark ? QColor(230, 230, 230) : QColor(30, 30, 30);
    QColor selBg = DesignSystem::instance()->primaryColor().lighter(150);
    QColor lineNumBg = isDark ? QColor(40, 40, 40) : QColor(245, 245, 245);

    setStyleSheet(QString(
        "QPlainTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  padding: 10px;"
        "  selection-background-color: %3;"
        "}"
    ).arg(bgColor.name()).arg(textColor.name()).arg(selBg.name()));

    viewport()->setStyleSheet(QString("background-color: %1;").arg(bgColor.name()));
}

bool MarkdownEditor::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    m_loading = true;
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    setPlainText(stream.readAll());
    file.close();

    m_filePath = filePath;
    document()->setModified(false);
    m_loading = false;

    emit fileLoaded(filePath);
    return true;
}

bool MarkdownEditor::saveFile()
{
    if (m_filePath.isEmpty()) {
        return false;
    }
    return saveFileAs(m_filePath);
}

bool MarkdownEditor::saveFileAs(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << toPlainText();
    file.close();

    m_filePath = filePath;
    document()->setModified(false);
    emit contentModified(false);
    emit fileSaved(filePath);
    return true;
}

bool MarkdownEditor::isModified() const
{
    return document()->isModified();
}

void MarkdownEditor::setModified(bool modified)
{
    document()->setModified(modified);
}

void MarkdownEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void MarkdownEditor::onTextChanged()
{
    if (!m_loading) {
        emit contentModified(document()->isModified());
    }
}