#include "webrtc.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWebSockets/QWebSocket>
#include <QDebug>

#pragma pack(push, 1)
struct RtpHeader {
    uint8_t first;
    uint8_t marker:1;
    uint8_t payloadType:7;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};
#pragma pack(pop)

// Constructor for WebRTC class
WebRTC::WebRTC(QObject *parent)
    : QObject{parent},
    m_timestamp(0),
    m_ssrc(0),
    m_isOfferer(false)
{
    connect(this, &WebRTC::gatheringComplited, [this] (const QString &peerID) {
        m_localDescription = descriptionToJson(m_peerConnections[peerID]->localDescription().value());

        Q_EMIT localDescriptionGenerated(peerID, m_localDescription);

        if (m_isOfferer) {
            Q_EMIT this->offerIsReady(peerID, m_localDescription);
        } else {
            Q_EMIT this->answerIsReady(peerID, m_localDescription);
        }
    });
}

// Destructor
WebRTC::~WebRTC() {}

/**
 * Initialize WebRTC with signaling client and configuration.
 */
void WebRTC::init(bool isOfferer)
{
    m_localId = "User1";  // Use a unique identifier, like "User1" or "User2"
    m_isOfferer = isOfferer;

    // Configure ICE servers for STUN
    rtc::Configuration config;
    config.iceServers.push_back(rtc::IceServer("stun:stun.l.google.com:19302"));
    m_config = config;

    // RTP settings
    m_sequenceNumber = 0;
    m_timestamp = 0;
    setBitRate(48000);
    setPayloadType(111);
    setSsrc(2);

    // Initialize WebSocket signaling client
    // m_signalingClient = new SignalingClient("ws://localhost:3000", this);

    // connect(this, &WebRTC::offerIsReady, m_signalingClient, &SignalingClient::sendSdp);
    // connect(this, &WebRTC::localCandidateGenerated, m_signalingClient, &SignalingClient::sendIceCandidate);
    // connect(m_signalingClient, &SignalingClient::sdpReceived, this, &WebRTC::setRemoteDescription);
    // connect(m_signalingClient, &SignalingClient::iceCandidateReceived, this, &WebRTC::setRemoteCandidate);
}

/**
 * Add a new peer connection.
 */
void WebRTC::addPeer(const QString &peerId)
{
    // Create a new peer connection
    auto newPeer = std::make_shared<rtc::PeerConnection>(m_config);
    m_peerConnections[peerId] = newPeer;

    // Callback for local SDP generation
    newPeer->onLocalDescription([this, peerId](const rtc::Description &description) {
        QString sdp = descriptionToJson(description);
        if (m_isOfferer) {
            Q_EMIT offerIsReady(peerId, sdp);
        } else {
            Q_EMIT answerIsReady(peerId, sdp);
        }
    });

    // Callback for local ICE candidate generation
    newPeer->onLocalCandidate([this, peerId](rtc::Candidate candidate) {
        Q_EMIT localCandidateGenerated(peerId,
                                       QString::fromStdString(candidate.candidate()),
                                       QString::fromStdString(candidate.mid()));
    });

    // Callback for peer connection state changes
    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        if (state == rtc::PeerConnection::State::Connected) {
            Q_EMIT connected(peerId);
        } else if (state == rtc::PeerConnection::State::Disconnected) {
            Q_EMIT disconnected(peerId);
        }
    });

    // Callback for gathering state changes
    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            Q_EMIT gatheringComplited(peerId);
        }
    });

    // Callback for incoming track messages (assume audio track)
    newPeer->onTrack([this, peerId](std::shared_ptr<rtc::Track> track) {
        track->onMessage([this, peerId](rtc::message_variant data) {
            QByteArray audioData = readVariant(data);
            Q_EMIT incommingPacket(peerId, audioData, audioData.size());
        });
    });

    // Add an audio track
    addAudioTrack(peerId, "audio_track");
}

/**
 * Generate an SDP offer.
 */
void WebRTC::generateOfferSDP(const QString &peerId)
{
    if (m_peerConnections.contains(peerId)) {
        m_peerConnections[peerId]->setLocalDescription(rtc::Description::Type::Offer);
    }
}

/**
 * Generate an SDP answer.
 */
void WebRTC::generateAnswerSDP(const QString &peerId)
{
    if (m_peerConnections.contains(peerId)) {
        m_peerConnections[peerId]->setLocalDescription(rtc::Description::Type::Answer);
    }
}

/**
 * Add an audio track to the peer connection.
 */
void WebRTC::addAudioTrack(const QString &peerId, const QString &trackName)
{
    auto track = m_peerConnections[peerId]->addTrack(trackName.toStdString());

    track->onMessage([this, peerId](rtc::message_variant data) {
        QByteArray audioData = readVariant(data);
        Q_EMIT incommingPacket(peerId, audioData, audioData.size());
    });
}

/**
 * Send an RTP packet over the audio track.
 */
void WebRTC::sendTrack(const QString &peerId, const QByteArray &buffer)
{
    RtpHeader rtpHeader;
    rtpHeader.first = 0x80;
    rtpHeader.marker = 1;
    rtpHeader.payloadType = payloadType();
    rtpHeader.sequenceNumber = qToBigEndian(m_sequenceNumber++);
    rtpHeader.timestamp = qToBigEndian(m_timestamp += 160);
    rtpHeader.ssrc = qToBigEndian(ssrc());

    // Build RTP packet by combining header and audio data
    QByteArray packet(reinterpret_cast<char*>(&rtpHeader), sizeof(RtpHeader));
    packet.append(buffer);

    try {
        if (m_peerTracks.contains(peerId)) {
            auto &track = m_peerTracks[peerId];
            track->send(reinterpret_cast<const std::byte*>(packet.data()), packet.size());
        } else {
            qWarning() << "Audio track not found for peer:" << peerId;
        }
    } catch (const std::exception &e) {
        qWarning() << "Failed to send RTP packet over audio track:" << e.what();
    }
}

/**
 * Set the remote SDP description.
 */
void WebRTC::setRemoteDescription(const QString &peerID, const QString &sdp)
{
    if (m_peerConnections.contains(peerID)) {
        rtc::Description description(sdp.toStdString(), m_isOfferer ? rtc::Description::Type::Answer : rtc::Description::Type::Offer);
        m_peerConnections[peerID]->setRemoteDescription(description);
    }
}

/**
 * Add a remote ICE candidate.
 */
void WebRTC::setRemoteCandidate(const QString &peerID, const QString &candidate, const QString &sdpMid)
{
    if (m_peerConnections.contains(peerID)) {
        rtc::Candidate iceCandidate(candidate.toStdString(), sdpMid.toStdString());
        m_peerConnections[peerID]->addRemoteCandidate(iceCandidate);
    }
}

/**
 * Convert rtc::message_variant data to QByteArray.
 */
QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{
    if (auto binaryData = std::get_if<rtc::binary>(&data)) {
        return QByteArray(reinterpret_cast<const char*>(binaryData->data()), binaryData->size());
    }
    return QByteArray();
}

/**
 * Convert rtc::Description to JSON format.
 */
QString WebRTC::descriptionToJson(const rtc::Description &description)
{
    QJsonObject jsonObject;
    jsonObject.insert("type", QString::fromStdString(description.typeString()));
    jsonObject.insert("sdp", QString::fromStdString(description.generateSdp()));
    QJsonDocument doc(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * Get the bit rate.
 */
int WebRTC::bitRate() const
{
    return m_bitRate;
}

/**
 * Set the bit rate.
 */
void WebRTC::setBitRate(int newBitRate)
{
    m_bitRate = newBitRate;
    Q_EMIT bitRateChanged(newBitRate);
}

/**
 * Reset bit rate to default.
 */
void WebRTC::resetBitRate()
{
    setBitRate(48000);
}

/**
 * Get the payload type.
 */
int WebRTC::payloadType() const
{
    return m_payloadType;
}

/**
 * Set the payload type.
 */
void WebRTC::setPayloadType(int newPayloadType)
{
    m_payloadType = newPayloadType;
    Q_EMIT payloadTypeChanged(newPayloadType);
}

/**
 * Reset payload type to default.
 */
void WebRTC::resetPayloadType()
{
    setPayloadType(111);
}

/**
 * Get SSRC.
 */
rtc::SSRC WebRTC::ssrc() const
{
    return m_ssrc;
}

/**
 * Set SSRC.
 */
void WebRTC::setSsrc(rtc::SSRC newSsrc)
{
    m_ssrc = newSsrc;
    Q_EMIT ssrcChanged(newSsrc);
}

/**
 * Reset SSRC to default.
 */
void WebRTC::resetSsrc()
{
    setSsrc(2);
}

/**
 * Check if the current instance is an offerer.
 */
bool WebRTC::isOfferer() const
{
    return m_isOfferer;
}

/**
 * Set the offerer state.
 */
void WebRTC::setIsOfferer(bool newIsOfferer)
{
    m_isOfferer = newIsOfferer;
    Q_EMIT isOffererChanged();
}

/**
 * Reset offerer state to default (false).
 */
void WebRTC::resetIsOfferer()
{
    setIsOfferer(false);
}
