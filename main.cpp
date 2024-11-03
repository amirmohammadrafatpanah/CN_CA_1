#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "AudioApp.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Create an instance of the AudioApp class
    AudioApp audioApp;
    audioApp.startRecording();

    // Load the main.qml file
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Check for errors in loading QML
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
