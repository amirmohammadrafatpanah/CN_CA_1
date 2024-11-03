// Import dependencies
const express = require('express');
const http = require('http');
const { Server } = require('socket.io');

// Initialize Express and HTTP server
const app = express();
const server = http.createServer(app);
const io = new Server(server);

const PORT = 3000;

// Serve static files if needed
app.use(express.static('public'));

// Handle socket connections
io.on('connection', (socket) => {
    console.log('A user connected:', socket.id);

    // Handle joining a room
    socket.on('joinRoom', (roomId) => {
        socket.join(roomId);
        console.log(`User ${socket.id} joined room ${roomId}`);
    });

    // Relay the offer SDP
    socket.on('offer', (data) => {
        const { roomId, sdp } = data;
        socket.to(roomId).emit('offer', { sdp, sender: socket.id });
        console.log(`Offer sent from ${socket.id} to room ${roomId}`);
    });

    // Relay the answer SDP
    socket.on('answer', (data) => {
        const { roomId, sdp } = data;
        socket.to(roomId).emit('answer', { sdp, sender: socket.id });
        console.log(`Answer sent from ${socket.id} to room ${roomId}`);
    });

    // Relay ICE candidates
    socket.on('iceCandidate', (data) => {
        const { roomId, candidate } = data;
        socket.to(roomId).emit('iceCandidate', { candidate, sender: socket.id });
        console.log(`ICE candidate sent from ${socket.id} to room ${roomId}`);
    });

    // Handle user disconnection
    socket.on('disconnect', () => {
        console.log('User disconnected:', socket.id);
    });
});

// Start the server
server.listen(PORT, () => {
    console.log(`Signaling server running on http://localhost:${PORT}`);
});
