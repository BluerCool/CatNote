#pragma once

#include <QPlainTextEdit>
#include <QFile>
#include "MarkdownHighlighter.h"

class MarkdownEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MarkdownEditor(QWidget *parent = nullptr);
    ~MarkdownEditor();

    bool loadFile(const QString &filePath);
    bool saveFile();
    bool saveFileAs(const QString &filePath);
    QString filePath() const { return m_filePath; }
    bool isModified() const;
    void setModified(bool modified);
    void setThemeColors();

signals:
    void contentModified(bool modified);
    void fileLoaded(const QString &filePath);
    void fileSaved(const QString &filePath);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onTextChanged();

private:
    QString m_filePath;
    MarkdownHighlighter *m_highlighter = nullptr;
    bool m_loading = false;
};