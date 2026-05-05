#include <QApplication>
#include "NoteWindow.h"
#include "DesignSystem.h"
#include "MessageManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

void saveSettingsOnExit()
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
    
    obj["themeMode"] = (DesignSystem::instance()->themeMode() == DesignSystem::Dark) ? "dark" : "light";
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonDocument doc(obj);
        file.write(doc.toJson());
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    DesignSystem::instance()->loadThemeFromJson();
    
    NoteWindow window{nullptr};
    window.show();
    
    DesignSystem::instance()->setMainWindow(&window);
    MessageManager::instance()->setParent(&window);
    
    QObject::connect(&app, &QApplication::aboutToQuit, saveSettingsOnExit);
    
    return app.exec();
}