// AudioApp.cpp

#include "AudioApp.h"

// Constructor
AudioApp::AudioApp(QObject *parent) : QObject(parent), audioInput(nullptr), audioOutput(nullptr) {
    audioInput = new AudioInput(this);
    audioOutput = new AudioOutput(this);

    // Connect AudioInput's encoded signal to AudioOutput's addData function
    connect(audioInput, &AudioInput::encodedAudioReady, this, &AudioApp::handleEncodedAudio);
}

// Destructor
AudioApp::~AudioApp() {
    delete audioInput;
    delete audioOutput;
}

// Start recording
void AudioApp::startRecording() {
    audioInput->startAudioCapture();
}

// Stop recording
void AudioApp::stopRecording() {
    audioInput->stopAudioCapture();
}

// Slot to handle encoded audio and send to AudioOutput for playback
void AudioApp::handleEncodedAudio(const QByteArray& encodedData) {
    audioOutput->addData(encodedData);
}
