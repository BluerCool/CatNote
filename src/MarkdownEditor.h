#pragma once

#include <QTextEdit>
#include <QFile>
#include <QEvent>

class MarkdownEditor : public QTextEdit
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
    void insertFromMimeData(const QMimeData *source) override;

private slots:
    void onTextChanged();

private:
    QString m_filePath;
    bool m_loading = false;
};