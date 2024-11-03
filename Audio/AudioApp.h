// AudioApp.h

#ifndef AUDIOAPP_H
#define AUDIOAPP_H

#include <QObject>
#include "AudioInput.h"
#include "AudioOutput.h"

class AudioApp : public QObject
{
    Q_OBJECT
public:
    explicit AudioApp(QObject *parent = nullptr);
    ~AudioApp();

    void startRecording();
    void stopRecording();

private slots:
    void handleEncodedAudio(const QByteArray& encodedData);

private:
    AudioInput* audioInput;
    AudioOutput* audioOutput;
};

#endif // AUDIOAPP_H
