# CN_CA_1

# Audio Transmission Application

This project is a real-time audio transmission application developed using `Qt` and `WebRTC`, with audio encoding and decoding managed through the `Opus` codec. It captures audio input, processes it, and transmits it to another peer in real-time. Below, you’ll find an overview of each file, class structure, key functionalities, and the main challenges encountered and solutions implemented.

---

## Files and Structure

### File: `AudioApp.h` and `AudioApp.cpp`

This class manages the flow of audio data between `AudioInput` and `AudioOutput`, encapsulating both recording and playback.

#### Class Members
- **audioInput** (`AudioInput*`): Manages audio capture from the input device.
- **audioOutput** (`AudioOutput*`): Manages audio playback.

#### Constructor
- **AudioApp(QObject *parent = nullptr)**: Initializes `audioInput` and `audioOutput`, and connects `encodedAudioReady` from `audioInput` to `handleEncodedAudio` for seamless data flow.

#### Destructor
- **~AudioApp()**: Releases memory by deleting `audioInput` and `audioOutput`.

#### Key Functions
1. **startRecording()**: Initiates audio capture.
2. **stopRecording()**: Stops audio capture.
3. **handleEncodedAudio(const QByteArray& encodedData)**: Forwards encoded audio data to `audioOutput` for playback.

---

### File: `AudioInput.h` and `AudioInput.cpp`

Handles capturing and encoding audio data.

#### Class Members
- **buffer**: Stores raw audio data before encoding.
- **opusEncoder**: Encodes audio in Opus format.
- **audioSource**: Represents the audio input source.
- **inputDevice**: Interface with the audio data stream.
- **sampleRate**, **channels**, **bitrate**: Defines audio quality and format.
- **mutex**: Ensures thread safety.

#### Key Functions
1. **startAudioCapture()**: Starts capturing and encoding audio data.
2. **stopAudioCapture()**: Stops audio capture.
3. **writeData(const char *data, qint64 len)**: Encodes data and signals `encodedAudioReady` with processed data.

---

### File: `AudioOutput.h` and `AudioOutput.cpp`

Handles audio playback and decoding.

#### Class Members
- **audioSink**: Manages the output device.
- **audioDevice**: Writes decoded data to the output.
- **opusDecoder**: Decodes Opus data into PCM format.
- **audioFormat**: Matches settings with `AudioInput`.
- **mutex**: Ensures thread safety.

#### Key Functions
1. **addData(const QByteArray& encodedData)**: Decodes and writes data to the output device.
2. **decodeOpus(const QByteArray& encodedData)**: Converts Opus data to PCM for playback.

---

### File: `webrtc.h` and `webRTC.cpp`

Handles WebRTC connections and manages peer-to-peer communication.

#### Class Members
- **m_sequenceNumber**: Tracks RTP sequence numbers.
- **m_gatheringCompleted**: Indicates ICE candidate gathering status.
- **m_bitRate**, **m_payloadType**: Defines audio settings for WebRTC.
- **m_audio**, **m_ssrc**: Audio stream details.
- **m_peerConnections**: Stores multiple peer connections.

#### Key Functions
1. **init(bool isOfferer = false)**: Configures WebRTC.
2. **addPeer(const QString &peerId)**: Creates a peer connection.
3. **generateOfferSDP(const QString &peerId)**: Generates an SDP offer.
4. **sendTrack(const QString &peerId, const QByteArray &buffer)**: Sends audio data via RTP.
5. **setRemoteDescription(...)**: Sets the peer’s SDP.

---

### File: `main.cpp`

Initializes the application, loads the QML interface, and starts recording.

#### Key Components
- **QGuiApplication app(argc, argv);**: Manages resources for the application.
- **AudioApp audioApp;**: Starts audio capture.
- **QQmlApplicationEngine engine;**: Loads and displays `main.qml`.

---

### File: `main.qml`

Defines the UI, including input fields and call controls.

#### Key Components
- **Window**: Main application window.
- **ColumnLayout**: Displays IP, ICE Candidate, and Caller ID.
- **TextField**: User input for phone numbers.
- **Button**: Starts or ends a call with color toggling.

---



Certainly! Here’s a more concise project report focusing on the signaling server component.

---

### Signaling Server for WebRTC Communication

#### Overview
This part of the project involves building a signaling server using **Node.js**, **Express**, and **Socket.IO** to facilitate WebRTC connections. The signaling server acts as a communication bridge, helping peers exchange the necessary connection details (SDP offers, answers, and ICE candidates) to establish a direct connection for real-time audio or video streaming.

#### Key Components

1. **Express Server Setup**
   - We initialize an HTTP server using Express, which can serve static files if needed. The server listens on port `3000` and can easily be scaled to serve other endpoints in the future.

2. **Socket.IO for Real-Time Communication**
   - **Socket.IO** is used to handle real-time, event-based communication between clients and the server.
   - Upon connection, each client is assigned a unique `socket.id`, which allows us to manage individual clients and rooms.

3. **Room-Based Communication**
   - Users can join specific rooms by emitting a `joinRoom` event with a `roomId`. This allows for isolated communication channels, ensuring only users within the same room can exchange signaling data.

4. **Signaling Events**
   - **SDP Offer/Answer**: The server relays `offer` and `answer` messages between users to negotiate WebRTC connections.
   - **ICE Candidate Exchange**: The server also relays ICE candidates, which are essential for NAT traversal and establishing stable P2P connections.

   Example event handling for an SDP offer:
   ```javascript
   socket.on('offer', (data) => {
       const { roomId, sdp } = data;
       socket.to(roomId).emit('offer', { sdp, sender: socket.id });
       console.log(`Offer sent from ${socket.id} to room ${roomId}`);
   });
   ```

5. **Disconnection Handling**
   - The server logs when users disconnect, which can be extended to notify other users in the room if needed.

#### Key Benefits and Challenges

- **Benefits**:
  - **Efficient Room-Based Signaling**: By using rooms, the server can handle multiple separate sessions simultaneously.
  - **Low Latency**: Socket.IO provides low-latency communication, allowing quick setup of WebRTC connections.

- **Challenges**:
  - **NAT Traversal**: Without a TURN server, some connections might fail in strict network environments.
  - **Security**: This basic setup lacks authentication and encryption; for production, adding secure WebSocket (wss) and authentication is recommended.

#### Conclusion
This signaling server successfully handles the necessary WebRTC signaling for P2P connections, enabling efficient room-based signaling with minimal latency. Future improvements include adding TURN server support for better NAT traversal and enhancing security with authentication and SSL.

--- 



---
## Challenges and Solutions

During development, several technical challenges required dedicated solutions. Below are the primary issues encountered and how they were addressed.

### 1. Entire Audio Output Contains Noise

   **Symptom**: Persistent noise or distortion throughout playback, obscuring audio clarity.

   **Root Cause**:
   - **Sample Rate Mismatch**: Encoding sample rate (`48000` Hz) differed from playback expectations.
   - **Incorrect Bit Depth**: Bit depth mismatch (e.g., `32-bit float` vs. `16-bit int`) causing noise.
   - **Improper Buffering**: Misalignment in data written to Opus encoder.
   - **Low Encoding Bitrate**: Compression artifacts due to insufficient bitrate.

   **Solution**:
   1. **Verify and Match Sample Rate**: Ensured both encoding and playback rates were `48000` Hz.
   2. **Confirm Channel Count Consistency**: Set channel count to mono (`1`) across both input and output.
   3. **Adjust Bitrate**: Increased bitrate to reduce compression artifacts, balancing quality and performance.

---

### 2. Library Installation and Configuration

   Installing Opus and WebRTC posed challenges due to dependencies and compatibility.

   **Problem**: Missing dependencies and version conflicts during installation.

   **Solution**: Followed documentation closely and used version managers to maintain compatibility. Compiled manually when pre-built binaries weren’t available, ensuring consistent configurations.

---

### 3. Synchronizing Input and Output in WebRTC

   Ensuring audio format consistency between input and WebRTC output was crucial.

   **Problem**: Format mismatches led to distortion and timing issues.

   **Solution**: Standardized audio settings across input and output, including sample rate, channels, and bit depth. Applied format conversion where necessary to align buffer sizes and maintain smooth streaming.

---

### 4. Handling ICE Candidates in WebRTC

   Managing ICE candidates was essential for establishing peer-to-peer connections.

   **Problem**: Network configurations sometimes delayed or blocked ICE candidates, leading to connection issues.

   **Solution**: Logged each candidate for tracking and processed candidates asynchronously to reduce delays. Testing on varied network setups helped improve connectivity reliability.

---

## Summary

This project successfully achieves real-time audio transmission with clear and reliable audio playback. By addressing challenges related to library setup, audio format consistency, and network connectivity, the application provides stable WebRTC connections and high-quality audio streaming.

--- 





