// AudioOutput.cpp

#include "AudioOutput.h"
#include <QMediaDevices>
#include <QDebug>
#include <QMutexLocker>
#include <opus.h> // Ensure opus.h is included

AudioOutput::AudioOutput(QObject* parent)
    : QObject(parent), audioSink(nullptr), audioDevice(nullptr), opusDecoder(nullptr)
{
    // Audio format settings must match those in AudioInput
    audioFormat.setSampleRate(48000);       // 48kHz
    audioFormat.setChannelCount(1);         // Mono
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    // Note: In Qt 6.5, setCodec, setByteOrder, and setSampleType are not available

    // Check if the audio format is supported by the default output device
    QAudioDevice outputDeviceInfo = QMediaDevices::defaultAudioOutput(); // Updated
    if (!outputDeviceInfo.isFormatSupported(audioFormat)) {
        qWarning() << "Audio format not supported by output device!";
        return;
    }

    // Set up QAudioSink for audio output
    audioSink = new QAudioSink(outputDeviceInfo, audioFormat, this);
    audioDevice = audioSink->start();
    if (!audioDevice) {
        qWarning() << "Failed to start QAudioSink!";
        return;
    }

    // Initialize Opus decoder with matching settings
    int opusError;
    opusDecoder = opus_decoder_create(48000, 1, &opusError);  // 48kHz, Mono
    if (opusError != OPUS_OK) {
        qWarning() << "Failed to create Opus decoder:" << opus_strerror(opusError);
        opusDecoder = nullptr;
    }
}

AudioOutput::~AudioOutput()
{
    if (opusDecoder) {
        opus_decoder_destroy(opusDecoder);
    }
    // No need to manually delete audioSink; Qt will handle it automatically
}

void AudioOutput::addData(const QByteArray& encodedData)
{
    QMutexLocker locker(&mutex); // Lock for thread safety

    if (!audioDevice || !opusDecoder) {
        qWarning() << "Audio device or Opus decoder is not initialized!";
        return;
    }

    // Decode Opus data to PCM
    QByteArray pcmData = decodeOpus(encodedData);
    if (pcmData.isEmpty()) {
        qWarning() << "Failed to decode Opus data!";
        return;
    }

    // Write PCM data to the audio device
    qint64 written = audioDevice->write(pcmData);
    if (written != pcmData.size()) {
        qWarning() << "Not all PCM data was written to the audio device!";
    }
    qDebug() << "PCM data written for playback.";
}

QByteArray AudioOutput::decodeOpus(const QByteArray& encodedData)
{
    if (!opusDecoder) {
        return QByteArray(); // Return empty if decoder is not initialized
    }

    // Prepare buffer for decoded PCM data
    opus_int16 pcmData[960 * 1]; // 960 samples for 20ms frame at 48kHz, mono

    // Decode Opus data
    int frameSize = opus_decode(opusDecoder,
                                reinterpret_cast<const unsigned char*>(encodedData.constData()),
                                encodedData.size(),
                                pcmData,
                                960,
                                0);
    if (frameSize < 0) {
        qWarning() << "Opus decoding failed with error:" << opus_strerror(frameSize);
        return QByteArray();
    }

    // Convert PCM data to QByteArray
    return QByteArray(reinterpret_cast<const char*>(pcmData), frameSize * sizeof(opus_int16));
}
