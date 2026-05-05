#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>
#include <QStackedWidget>
#include <QListWidget>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

signals:
    void borderWidthChanged(int width);
    void animationSpeedChanged(int speed);
    void roundedCornersChanged(bool enabled);
    void cornerRadiusChanged(int radius);
    void opacityChanged(int opacity);
    void windowTransparencyChanged(int transparency);
    void titleBarVisibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onCategoryChanged(int index);
    void onBorderWidthChanged(int value);
    void onAnimationSpeedChanged(int value);
    void onRoundedCornersChanged(int state);
    void onCornerRadiusChanged(int value);
    void onOpacityChanged(int value);
    void onWindowTransparencyChanged(int value);
    void onThemeModeChanged(int id);
    void loadSettings();
    void saveSettings();
    void updateThemeColors();
    void onTitleBarVisibilityChanged(int state);

private:
    void setupUI();
    void setupConnections();
    QWidget* createAppearancePage();
    QWidget* createAdvancedPage();
    QWidget* createAboutPage();

    QListWidget* m_categoryList;
    QStackedWidget* m_stackedWidget;
    
    QSlider* m_borderWidthSlider;
    QSlider* m_animationSpeedSlider;
    QSlider* m_cornerRadiusSlider;
    QSlider* m_opacitySlider;
    QSlider* m_windowTransparencySlider;
    QCheckBox* m_roundedCornersCheck;
    QLabel* m_borderWidthLabel;
    QLabel* m_animationSpeedLabel;
    QLabel* m_cornerRadiusLabel;
    QLabel* m_opacityLabel;
    QLabel* m_windowTransparencyLabel;
    
    QButtonGroup* m_themeGroup;
    QRadioButton* m_lightRadio;
    QRadioButton* m_darkRadio;
    
    int m_cornerRadius = 12;
    bool m_roundedCornersEnabled = false;
    QCheckBox* m_titleBarVisibleCheck = nullptr;

    QWidget* wrapInScrollArea(QWidget* page);
};