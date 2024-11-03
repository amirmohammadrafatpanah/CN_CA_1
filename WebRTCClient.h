// #ifndef WEBRTC_CLIENT_H
// #define WEBRTC_CLIENT_H

// #include "sio_client.h"
// #include <rtc.hpp>
// #include <QObject>
// #include <memory>

// class WebRTCClient : public QObject {
//     Q_OBJECT

// public:
//     WebRTCClient(const std::string &signalingServerUrl, QObject *parent = nullptr);
//     ~WebRTCClient();

//     void connectToPeer();
//     void sendAudioData(const QByteArray &audioData);

// private:
//     sio::client signalingClient;
//     std::shared_ptr<rtc::PeerConnection> peerConnection;
//     std::shared_ptr<rtc::Track> audioTrack;
//     void setupPeerConnection();

// signals:
//     void audioReceived(const QByteArray &audioData);

// private slots:
//     void handleOffer(const std::string &sdp);
//     void handleAnswer(const std::string &sdp);
//     void handleIceCandidate(const std::string &candidate);
// };

// #endif // WEBRTC_CLIENT_H
