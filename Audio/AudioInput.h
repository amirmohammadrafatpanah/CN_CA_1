// AudioInput.h

#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QIODevice>
#include <QAudioSource>
#include <QByteArray>
#include <QMutex>
#include <opus.h> // Opus library

class AudioInput : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = nullptr);
    ~AudioInput();

    bool startAudioCapture();
    void stopAudioCapture();

signals:
    void encodedAudioReady(const QByteArray& encodedData);

protected:
    qint64 readData(char *data, qint64 maxlen) override { Q_UNUSED(data); Q_UNUSED(maxlen); return -1; }
    qint64 writeData(const char *data, qint64 len) override;

private:
    QByteArray buffer;          // Internal buffer to collect data
    OpusEncoder* opusEncoder;   // Opus encoder
    QAudioSource* audioSource;  // Audio source
    QIODevice* inputDevice;     // Audio input device

    const int sampleRate = 48000; // Sample rate of 48kHz
    const int channels = 1;       // Mono
    const int bitrate = 64000;    // Bitrate of 64kbps

    QMutex mutex; // For thread safety
};

#endif // AUDIOINPUT_H
