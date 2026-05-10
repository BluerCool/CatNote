#include "MarkdownEditor.h"
#include "DesignSystem.h"
#include <QKeyEvent>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QMimeData>
#include <QImage>
#include <QImageReader>
#include <QUrl>
#include <QUuid>
#include <QDir>
#include <QApplication>

MarkdownEditor::MarkdownEditor(QWidget *parent)
    : QTextEdit(parent)
{
    setObjectName("markdownEditor");
    setLineWrapMode(QTextEdit::WidgetWidth);
    setTabStopDistance(40);
    setAcceptRichText(true);  // Enable direct editing of rendered Markdown content

    QFont font("Consolas, Microsoft YaHei, sans-serif", 11);
    setFont(font);

    setThemeColors();

    connect(this, &QTextEdit::textChanged, this, &MarkdownEditor::onTextChanged);
    connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, &MarkdownEditor::setThemeColors);
}

MarkdownEditor::~MarkdownEditor()
{
}

void MarkdownEditor::setThemeColors()
{
    bool isDark = DesignSystem::instance()->themeMode() == DesignSystem::Dark;
    QColor bgColor = DesignSystem::instance()->bodyColor();
    QColor textColor = isDark ? QColor(230, 230, 230) : QColor(30, 30, 30);
    QColor selBg = DesignSystem::instance()->primaryColor().lighter(150);

    setStyleSheet(QString(
        "QTextEdit {"
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
    const QString markdown = stream.readAll();
    document()->setMarkdown(markdown);  // Render Markdown directly for editing
    document()->setBaseUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath() + "/"));  // Support relative image paths
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    stream << document()->toMarkdown();
#else
    stream << toPlainText();
#endif
    file.close();

    m_filePath = filePath;
    document()->setBaseUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath() + "/"));
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
    QTextEdit::keyPressEvent(event);
}

void MarkdownEditor::insertFromMimeData(const QMimeData *source)
{
    QImage image;
    if (source->hasImage()) {
        image = qvariant_cast<QImage>(source->imageData());
    }
    else if (source->hasUrls()) {
        const QList<QUrl> urls = source->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString localPath = url.toLocalFile();
                QImageReader reader(localPath);
                if (reader.canRead()) {
                    image = reader.read();
                    if (!image.isNull()) {
                        break;
                    }
                }
            }
        }
    }

    if (!image.isNull()) {
        QString baseDir;
        if (!m_filePath.isEmpty()) {
            baseDir = QFileInfo(m_filePath).absolutePath();
        } else {
            baseDir = QApplication::applicationDirPath();
        }

        QString imageDir = QDir(baseDir).filePath("images");
        QDir().mkpath(imageDir);
        QString fileName = QUuid::createUuid().toString().remove('{').remove('}').remove('-') + ".png";
        QString fullPath = QDir(imageDir).filePath(fileName);
        if (image.save(fullPath)) {
            QString relativePath = QDir(baseDir).relativeFilePath(fullPath);
            QUrl imageUrl(relativePath);
            QTextCursor cursor = textCursor();
            QTextImageFormat imageFormat;
            imageFormat.setName(imageUrl.toString());
            imageFormat.setWidth(image.width());
            imageFormat.setHeight(image.height());
            document()->addResource(QTextDocument::ImageResource, imageUrl, QVariant(image));
            cursor.insertImage(imageFormat);
            return;
        }
    }

    QTextEdit::insertFromMimeData(source);
}

void MarkdownEditor::onTextChanged()
{
    if (!m_loading) {
        emit contentModified(document()->isModified());
    }
}