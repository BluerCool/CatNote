#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QCloseEvent>

class EditorTab : public QWidget
{
    Q_OBJECT
public:
    explicit EditorTab(const QString &filePath, QWidget *parent = nullptr);
    QString filePath() const { return m_filePath; }
    QString fileName() const;
    void setActive(bool active);
    bool isModified() const { return m_modified; }
    void setModified(bool modified);
    void updateStyle();
    bool isActive() const { return m_active; }

signals:
    void clicked(EditorTab *tab);
    void closeRequested(EditorTab *tab);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_filePath;
    bool m_active = false;
    bool m_modified = false;
    bool m_hovered = false;
    QLabel *m_nameLabel = nullptr;
    QPushButton *m_closeBtn = nullptr;
};

class EditorTabBar : public QWidget
{
    Q_OBJECT
public:
    explicit EditorTabBar(QWidget *parent = nullptr);
    void addTab(const QString &filePath);
    void closeTab(const QString &filePath);
    void setActiveTab(const QString &filePath);
    QString currentFilePath() const;
    bool hasTab(const QString &filePath) const;
    void setTabModified(const QString &filePath, bool modified);
    QList<QString> allFilePaths() const;

signals:
    void tabActivated(const QString &filePath);
    void tabClosed(const QString &filePath);

private:
    QHBoxLayout *m_layout = nullptr;
    QList<EditorTab*> m_tabs;
    EditorTab* findTab(const QString &filePath) const;
    void updateLayout();
};