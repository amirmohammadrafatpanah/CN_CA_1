// AudioOutput.h

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QAudioSink>
#include <QByteArray>
#include <QMutex>
#include <opus.h> // Opus library

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = nullptr);
    ~AudioOutput();

    void addData(const QByteArray& encodedData);

private:
    QAudioSink* audioSink;       // Audio output device
    QIODevice* audioDevice;      // Audio writing device
    OpusDecoder* opusDecoder;    // Opus decoder

    QAudioFormat audioFormat;    // Audio format
    QMutex mutex;                // For thread safety

    QByteArray decodeOpus(const QByteArray& encodedData);
};

#endif // AUDIOOUTPUT_H
