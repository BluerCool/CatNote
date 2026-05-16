#pragma once

#include "BaseWindow.h"
#include "ThemeSwitcher.h"
#include "Button.h"
#include "SettingsDialog.h"
#include "EditorTabBar.h"
#include "MarkdownEditor.h"
#include <QPainter>
#include <QPainterPath>
#include <QStackedWidget>
#include <QFileDialog>
#include <QMessageBox>

class NoteWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit NoteWindow(BaseWindow* parent = nullptr);
    ~NoteWindow();

    void setCornerRadius(int radius);
    int cornerRadius() const { return m_cornerRadius; }
    void enableRoundedCorners(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onWindowResized(int w, int h);
    void onSettingsClicked();
    void applySettings();
    void onBorderWidthChanged(int width);
    void onAnimationSpeedChanged(int speed);
    void onRoundedCornersChanged(bool enabled);
    void onCornerRadiusChanged(int radius);
    void onOpacityChanged(int opacity);
    void onWindowTransparencyChanged(int transparency);
    void onTitleBarVisibilityChanged(bool visible);
    
    void onNewFileClicked();
    void onOpenFileClicked();
    void onSaveFileClicked();
    void onTabActivated(const QString &filePath);
    void onTabClosed(const QString &filePath);
    void onEditorModified(bool modified);

private:
    void setupCornerButtons();
    void updateFileButtonStyles();
    void updateRoundedMask();
    void setupEditor();
    void createNewFile();
    void openExistingFile();
    bool saveCurrentFile();
    bool saveCurrentFileAs();
    void updateWindowTitle();

    ThemeSwitcherButton* m_themeBtn = nullptr;
    Button* m_settingsBtn = nullptr;
    Button* m_newFileBtn = nullptr;
    Button* m_openFileBtn = nullptr;
    SettingsDialog* m_settingsDialog = nullptr;
    
    EditorTabBar* m_tabBar = nullptr;
    QStackedWidget* m_editorStack = nullptr;
    QMap<QString, MarkdownEditor*> m_editors;
    QString m_currentFilePath;
    
    int m_cornerRadius = 12;
    bool m_roundedCornersEnabled = false;
    QPainterPath m_roundedPath;
    MarkdownEditor* currentEditor() const;
};