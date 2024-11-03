// AudioInput.cpp

#include "AudioInput.h"
#include <QDebug>
#include <QAudioFormat>
#include <QMediaDevices>
#include <opus.h> // Ensure opus.h is included

AudioInput::AudioInput(QObject *parent)
    : QIODevice(parent), buffer(), opusEncoder(nullptr), audioSource(nullptr), inputDevice(nullptr)
{
    // Create Opus encoder
    int error;
    opusEncoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        qDebug() << "Failed to create Opus encoder:" << opus_strerror(error);
        return;
    }
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(bitrate));

    // Configure audio format
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleFormat(QAudioFormat::Int16);
    // Note: In Qt 6.5, setCodec, setByteOrder, and setSampleType are not available

    // Check if the audio format is supported by the default input device
    QAudioDevice inputDeviceInfo = QMediaDevices::defaultAudioInput(); // Updated
    if (!inputDeviceInfo.isFormatSupported(format)) {
        qWarning() << "Audio format not supported by input device!";
        return;
    }

    // Create audio source
    audioSource = new QAudioSource(format, this);
    open(QIODevice::WriteOnly); // Open QIODevice for writing
}

AudioInput::~AudioInput()
{
    stopAudioCapture();
    if (opusEncoder) {
        opus_encoder_destroy(opusEncoder);
    }
}

bool AudioInput::startAudioCapture()
{
    if (!audioSource)
        return false;

    inputDevice = audioSource->start();
    if (!inputDevice) {
        qWarning() << "Failed to start QAudioSource!";
        return false;
    }

    connect(inputDevice, &QIODevice::readyRead, this, [this]() {
        QByteArray audioData = inputDevice->readAll();
        writeData(audioData.data(), audioData.size());
    });

    return true;
}

void AudioInput::stopAudioCapture()
{
    if (audioSource) {
        audioSource->stop();
    }
}

qint64 AudioInput::writeData(const char *data, qint64 len)
{
    QMutexLocker locker(&mutex); // Lock for thread safety

    buffer.append(data, len); // Add data to internal buffer

    // Each frame requires 960 samples for 20 ms at a 48kHz sample rate
    const int frameSize = 960 * channels; // 960 samples
    const int bytesPerSample = sizeof(opus_int16); // 2 bytes per sample
    const int bytesPerFrame = frameSize * bytesPerSample; // 1920 bytes

    while (buffer.size() >= bytesPerFrame) {
        QByteArray frameData = buffer.left(bytesPerFrame);
        buffer.remove(0, bytesPerFrame);

        QByteArray encodedData;
        // Use opus_encode_bound to reserve sufficient space
        // If opus_encode_bound is unavailable, use an approximate value
        // int bound = opus_encode_bound(opusEncoder, frameSize); // if available
        int bound = frameSize * bytesPerSample; // Approximate value
        encodedData.resize(bound);

        int encodedBytes = opus_encode(opusEncoder,
                                       reinterpret_cast<const opus_int16*>(frameData.constData()),
                                       frameSize,
                                       reinterpret_cast<unsigned char*>(encodedData.data()),
                                       bound);

        if (encodedBytes < 0) {
            qDebug() << "Encoding failed with error:" << opus_strerror(encodedBytes);
            continue;
        }

        encodedData.resize(encodedBytes);
        emit encodedAudioReady(encodedData);
    }

    return len;
}
