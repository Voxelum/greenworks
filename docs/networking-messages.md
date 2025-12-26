# Steam Networking Messages API

The Steam Networking Messages API provides a connectionless, message-oriented interface for sending messages between Steam users. It's ideal for game communication where you don't need the overhead of managing explicit connections.

## Overview

Steam Networking Messages provides:
- Connectionless message delivery
- Automatic session management
- P2P connectivity with Steam relay
- Per-channel message delivery
- Simplified API compared to Sockets

## Key Differences from Sockets

- **No explicit connections**: Messages are sent directly to Steam IDs
- **Automatic sessions**: Steam manages sessions in the background
- **Channel-based**: Messages are organized by virtual port/channel
- **Simpler API**: Less setup code required

## Sending Messages

### greenworks.sendMessageToUser(steamId, data, sendFlags, remoteVirtualPort)

Sends a message to a Steam user.

* `steamId` String - Target Steam ID
* `data` Buffer - Message data
* `sendFlags` Integer (optional) - Send flags (default: Reliable)
* `remoteVirtualPort` Integer (optional) - Remote channel (default: 0)
* Returns Boolean - true if successful

```javascript
const targetSteamId = '76561198012345678';
const message = Buffer.from('Hello, friend!');

greenworks.sendMessageToUser(
  targetSteamId, 
  message,
  greenworks.NetworkingSendFlags.Reliable,
  0  // channel
);
```

## Receiving Messages

### greenworks.receiveMessagesOnChannel(localVirtualPort, maxMessages)

Receives pending messages on a channel.

* `localVirtualPort` Integer (optional) - Local channel (default: 0)
* `maxMessages` Integer (optional) - Max messages to receive (default: 32)
* Returns Array - Array of message objects

```javascript
const messages = greenworks.receiveMessagesOnChannel(0);
messages.forEach(msg => {
  console.log('From:', msg.steamIDRemote);
  console.log('Data:', msg.data.toString());
  console.log('Size:', msg.size);
  console.log('Channel:', msg.channel);
});
```

Message object properties:
- `data` Buffer - Message data
- `size` Integer - Message size in bytes
- `channel` Integer - Channel/port number
- `steamIDRemote` String - Sender's Steam ID

## Session Management

### greenworks.acceptSessionWithUser(steamId)

Accepts incoming messages from a user. Call this in response to receiving a message from a new user.

* `steamId` String - Steam ID to accept
* Returns Boolean - true if successful

```javascript
greenworks.acceptSessionWithUser('76561198012345678');
```

### greenworks.closeSessionWithUser(steamId)

Closes the session with a user, rejecting future messages.

* `steamId` String - Steam ID to close
* Returns Boolean - true if successful

```javascript
greenworks.closeSessionWithUser('76561198012345678');
```

### greenworks.closeChannelWithUser(steamId, localVirtualPort)

Closes a specific channel with a user.

* `steamId` String - Steam ID
* `localVirtualPort` Integer - Local channel to close
* Returns Boolean - true if successful

```javascript
greenworks.closeChannelWithUser('76561198012345678', 0);
```

## Session Information

### greenworks.getSessionConnectionInfo(steamId)

Gets connection information for a session with a user.

* `steamId` String - Steam ID
* Returns Object or null - Session info

```javascript
const info = greenworks.getSessionConnectionInfo('76561198012345678');
if (info) {
  console.log('State:', info.state);
  console.log('Ping:', info.quickStatus.ping, 'ms');
  console.log('Quality:', info.quickStatus.connectionQualityLocal);
  console.log('Out rate:', info.quickStatus.outBytesPerSec);
  console.log('In rate:', info.quickStatus.inBytesPerSec);
}
```

## Example: Simple Peer Chat

```javascript
const greenworks = require('greenworks');

if (!greenworks.initAPI()) {
  console.error('Failed to initialize Steam API');
  process.exit(1);
}

const CHAT_CHANNEL = 0;
const activeSessions = new Set();

// Send a message to a friend
function sendChatMessage(steamId, text) {
  const message = Buffer.from(text);
  const success = greenworks.sendMessageToUser(
    steamId,
    message,
    greenworks.NetworkingSendFlags.Reliable,
    CHAT_CHANNEL
  );
  
  if (success) {
    console.log('Sent to', steamId, ':', text);
    activeSessions.add(steamId);
  } else {
    console.error('Failed to send message to', steamId);
  }
}

// Receive and process messages
function processMessages() {
  const messages = greenworks.receiveMessagesOnChannel(CHAT_CHANNEL);
  
  messages.forEach(msg => {
    const sender = msg.steamIDRemote;
    const text = msg.data.toString();
    
    console.log('Received from', sender, ':', text);
    
    // Auto-accept sessions from new senders
    if (!activeSessions.has(sender)) {
      console.log('Accepting session from', sender);
      greenworks.acceptSessionWithUser(sender);
      activeSessions.add(sender);
    }
    
    // Echo back
    sendChatMessage(sender, 'You said: ' + text);
  });
}

// Process messages periodically
setInterval(() => {
  processMessages();
}, 50);

// Example: Send a message to a friend
const friendSteamId = '76561198012345678';
setTimeout(() => {
  sendChatMessage(friendSteamId, 'Hello from Steam Networking Messages!');
}, 1000);
```

## Example: Game State Sync

```javascript
const greenworks = require('greenworks');

class GameStateSync {
  constructor(channel = 0) {
    this.channel = channel;
    this.peers = new Map();
    this.lastSyncTime = Date.now();
  }
  
  // Add a peer to sync with
  addPeer(steamId) {
    if (!this.peers.has(steamId)) {
      console.log('Adding peer:', steamId);
      greenworks.acceptSessionWithUser(steamId);
      this.peers.set(steamId, {
        lastReceived: 0,
        lastSent: 0
      });
    }
  }
  
  // Remove a peer
  removePeer(steamId) {
    if (this.peers.has(steamId)) {
      console.log('Removing peer:', steamId);
      greenworks.closeSessionWithUser(steamId);
      this.peers.delete(steamId);
    }
  }
  
  // Broadcast game state to all peers
  broadcastState(stateData) {
    const buffer = Buffer.from(JSON.stringify(stateData));
    
    this.peers.forEach((peerInfo, steamId) => {
      const success = greenworks.sendMessageToUser(
        steamId,
        buffer,
        greenworks.NetworkingSendFlags.Unreliable,
        this.channel
      );
      
      if (success) {
        peerInfo.lastSent = Date.now();
      }
    });
  }
  
  // Receive state updates from peers
  receiveUpdates() {
    const messages = greenworks.receiveMessagesOnChannel(this.channel);
    const updates = [];
    
    messages.forEach(msg => {
      const sender = msg.steamIDRemote;
      
      try {
        const stateData = JSON.parse(msg.data.toString());
        
        // Track peer
        if (!this.peers.has(sender)) {
          this.addPeer(sender);
        }
        
        // Update peer info
        const peerInfo = this.peers.get(sender);
        if (peerInfo) {
          peerInfo.lastReceived = Date.now();
        }
        
        updates.push({
          steamId: sender,
          data: stateData
        });
      } catch (err) {
        console.error('Failed to parse state from', sender, ':', err);
      }
    });
    
    return updates;
  }
  
  // Check connection quality with peers
  checkConnections() {
    this.peers.forEach((peerInfo, steamId) => {
      const info = greenworks.getSessionConnectionInfo(steamId);
      if (info && info.quickStatus) {
        const ping = info.quickStatus.ping;
        const quality = info.quickStatus.connectionQualityLocal;
        
        if (quality < 0.5) {
          console.warn('Poor connection to', steamId, 
                      '- Quality:', quality, 'Ping:', ping);
        }
      }
    });
  }
}

// Usage
if (greenworks.initAPI()) {
  const gameSync = new GameStateSync(0);
  
  // Add peers
  const peer1 = '76561198012345678';
  const peer2 = '76561198087654321';
  gameSync.addPeer(peer1);
  gameSync.addPeer(peer2);
  
  // Game loop
  let playerPosition = { x: 0, y: 0 };
  
  setInterval(() => {
    // Update position
    playerPosition.x += Math.random() * 2 - 1;
    playerPosition.y += Math.random() * 2 - 1;
    
    // Broadcast state
    gameSync.broadcastState({
      type: 'position',
      position: playerPosition,
      timestamp: Date.now()
    });
    
    // Receive updates
    const updates = gameSync.receiveUpdates();
    updates.forEach(update => {
      console.log('Peer', update.steamId, 'at', update.data.position);
    });
    
    // Check connections every 5 seconds
    if (Date.now() % 5000 < 50) {
      gameSync.checkConnections();
    }
  }, 50);
}
```

## Multiple Channels

You can use multiple channels to organize different types of messages:

```javascript
const CHANNEL_CHAT = 0;
const CHANNEL_GAME_STATE = 1;
const CHANNEL_VOICE = 2;

// Send chat message
greenworks.sendMessageToUser(steamId, chatBuffer, flags, CHANNEL_CHAT);

// Send game state
greenworks.sendMessageToUser(steamId, stateBuffer, flags, CHANNEL_GAME_STATE);

// Send voice data
greenworks.sendMessageToUser(steamId, voiceBuffer, flags, CHANNEL_VOICE);

// Receive from specific channels
const chatMessages = greenworks.receiveMessagesOnChannel(CHANNEL_CHAT);
const gameUpdates = greenworks.receiveMessagesOnChannel(CHANNEL_GAME_STATE);
const voiceData = greenworks.receiveMessagesOnChannel(CHANNEL_VOICE);
```

## Best Practices

1. **Accept sessions**: Always call `acceptSessionWithUser()` when receiving messages from a new peer
2. **Use channels**: Organize message types using different channels
3. **Check size**: Be mindful of message sizes; fragment large data
4. **Choose send flags**: Use unreliable for frequent updates, reliable for critical data
5. **Monitor quality**: Use `getSessionConnectionInfo()` to monitor connection health
6. **Clean up**: Call `closeSessionWithUser()` when done with a peer

## Send Flags

Use appropriate flags for your use case:

- **Reliable**: For critical data that must arrive (chat, game events)
- **Unreliable**: For frequent updates where loss is acceptable (position, state)
- **ReliableNoNagle**: For low-latency reliable data
- **UnreliableNoDelay**: For minimal latency unreliable data

```javascript
// High-frequency position updates (can drop some)
greenworks.sendMessageToUser(
  steamId, 
  positionBuffer,
  greenworks.NetworkingSendFlags.UnreliableNoDelay,
  CHANNEL_GAME_STATE
);

// Critical game event (must arrive)
greenworks.sendMessageToUser(
  steamId,
  eventBuffer,
  greenworks.NetworkingSendFlags.Reliable,
  CHANNEL_EVENTS
);
```

## Migration from P2P API

| Old P2P API | New Messages API |
|-------------|------------------|
| `acceptP2PSessionWithUser()` | `acceptSessionWithUser()` |
| `sendP2PPacket()` | `sendMessageToUser()` |
| `readP2PPacket()` | `receiveMessagesOnChannel()` |
| `isP2PPacketAvailable()` | Check array length from `receiveMessagesOnChannel()` |
| `closeP2PSessionWithUser()` | `closeSessionWithUser()` |

## See Also

- [Steam Networking Sockets API](networking-sockets.md)
- [Steam Networking Utils API](networking-utils.md)
- [Official Steam Documentation](https://partner.steamgames.com/doc/api/ISteamNetworkingMessages)
