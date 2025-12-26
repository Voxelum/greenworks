# Steam Networking Sockets API

The Steam Networking Sockets API provides a modern, reliable, and efficient way to communicate between peers. It replaces the deprecated P2P API with improved performance and features.

## Overview

Steam Networking Sockets provides:
- Connection-oriented reliable transport
- Unreliable messages with automatic fragmentation and reassembly
- P2P connectivity using Steam relay servers
- Direct IP connections
- Quality of service (QoS) metrics
- Automatic NAT traversal

## Connection Management

### greenworks.createListenSocketIP(port)

Creates a listen socket on the specified port for accepting incoming IP connections.

* `port` Integer - The port number to listen on
* Returns Integer - The listen socket handle, or 0 on failure

```javascript
const listenSocket = greenworks.createListenSocketIP(27015);
if (listenSocket) {
  console.log('Listening on port 27015');
}
```

### greenworks.connectByIPAddress(ipAddress, port)

Initiates a connection to a remote host by IP address.

* `ipAddress` String - IP address (e.g., "192.168.1.100")
* `port` Integer - Port number
* Returns Integer - The connection handle, or 0 on failure

```javascript
const connection = greenworks.connectByIPAddress('192.168.1.100', 27015);
if (connection) {
  console.log('Connecting to server...');
}
```

### greenworks.connectP2P(steamId, virtualPort)

Initiates a P2P connection to a Steam user.

* `steamId` String - Steam ID of the remote user
* `virtualPort` Integer (optional) - Virtual port number (default: 0)
* Returns Integer - The connection handle, or 0 on failure

```javascript
const connection = greenworks.connectP2P('76561198012345678', 0);
if (connection) {
  console.log('Connecting to peer...');
}
```

### greenworks.acceptConnection(connectionHandle)

Accepts an incoming connection.

* `connectionHandle` Integer - The connection handle from the connection event
* Returns Boolean - true if successful

```javascript
greenworks.on('steam-net-connection-status-changed', (info) => {
  if (info.state === greenworks.NetworkingConnectionState.Connecting) {
    greenworks.acceptConnection(info.connection);
  }
});
```

### greenworks.closeConnection(connectionHandle, reason, debugString, enableLinger)

Closes a connection.

* `connectionHandle` Integer - The connection to close
* `reason` Integer (optional) - Reason code (default: 0)
* `debugString` String (optional) - Debug message
* `enableLinger` Boolean (optional) - Wait for pending messages (default: false)
* Returns Boolean - true if successful

```javascript
greenworks.closeConnection(connection, 0, 'Disconnecting', false);
```

### greenworks.closeListenSocket(listenSocketHandle)

Closes a listen socket.

* `listenSocketHandle` Integer - The listen socket to close
* Returns Boolean - true if successful

```javascript
greenworks.closeListenSocket(listenSocket);
```

## Message Sending and Receiving

### greenworks.sendMessageToConnection(connectionHandle, data, sendFlags)

Sends a message to a connection.

* `connectionHandle` Integer - The connection to send to
* `data` Buffer - The message data
* `sendFlags` Integer (optional) - Send flags (default: Reliable)
* Returns Boolean - true if successful

```javascript
const message = Buffer.from('Hello, world!');
greenworks.sendMessageToConnection(connection, message, 
  greenworks.NetworkingSendFlags.Reliable);
```

### greenworks.receiveMessagesOnConnection(connectionHandle, maxMessages)

Receives pending messages from a connection.

* `connectionHandle` Integer - The connection to receive from
* `maxMessages` Integer (optional) - Max messages to receive (default: 32)
* Returns Array - Array of message objects

```javascript
const messages = greenworks.receiveMessagesOnConnection(connection);
messages.forEach(msg => {
  console.log('Received:', msg.data.toString());
  console.log('Size:', msg.size);
  console.log('Channel:', msg.channel);
});
```

### greenworks.flushMessagesOnConnection(connectionHandle)

Flushes any pending messages on a connection.

* `connectionHandle` Integer - The connection to flush
* Returns Boolean - true if successful

```javascript
greenworks.flushMessagesOnConnection(connection);
```

## Connection Information

### greenworks.getConnectionInfo(connectionHandle)

Gets detailed information about a connection.

* `connectionHandle` Integer - The connection
* Returns Object or null - Connection info object

```javascript
const info = greenworks.getConnectionInfo(connection);
if (info) {
  console.log('State:', info.state);
  console.log('End reason:', info.endReason);
  console.log('Remote Steam ID:', info.steamIDRemote);
  console.log('Description:', info.connectionDescription);
}
```

### greenworks.getQuickConnectionStatus(connectionHandle)

Gets quick connection status and statistics.

* `connectionHandle` Integer - The connection
* Returns Object or null - Status object

```javascript
const status = greenworks.getQuickConnectionStatus(connection);
if (status) {
  console.log('Ping:', status.ping, 'ms');
  console.log('Out rate:', status.outBytesPerSec, 'bytes/sec');
  console.log('In rate:', status.inBytesPerSec, 'bytes/sec');
  console.log('Connection quality:', status.connectionQualityLocal);
}
```

### greenworks.getDetailedConnectionStatus(connectionHandle)

Gets a detailed human-readable connection status string.

* `connectionHandle` Integer - The connection
* Returns String or null - Status string

```javascript
const status = greenworks.getDetailedConnectionStatus(connection);
console.log(status);
```

## Callbacks

### greenworks.runNetworkingCallbacks()

Processes pending networking callbacks. Call this regularly (e.g., in your game loop).

```javascript
setInterval(() => {
  greenworks.runNetworkingCallbacks();
}, 100);
```

## Events

### Event: 'steam-net-connection-status-changed'

Emitted when a connection's status changes.

```javascript
greenworks.on('steam-net-connection-status-changed', (info) => {
  console.log('Connection:', info.connection);
  console.log('Old state:', info.oldState);
  console.log('New state:', info.state);
  console.log('End reason:', info.endReason);
  
  if (info.state === greenworks.NetworkingConnectionState.Connected) {
    console.log('Connected!');
  } else if (info.state === greenworks.NetworkingConnectionState.ClosedByPeer) {
    console.log('Connection closed by peer');
  }
});
```

## Constants

### greenworks.NetworkingSendFlags

Send flags for controlling message delivery:

* `Unreliable` - Unreliable, lowest latency
* `NoNagle` - Disable Nagle algorithm
* `UnreliableNoNagle` - Unreliable without Nagle
* `NoDelay` - Send immediately
* `UnreliableNoDelay` - Unreliable, send immediately
* `Reliable` - Reliable delivery (default)
* `ReliableNoNagle` - Reliable without Nagle

### greenworks.NetworkingConnectionState

Connection states:

* `None` - Not connected
* `Connecting` - Connection in progress
* `FindingRoute` - Finding optimal route
* `Connected` - Connected and ready
* `ClosedByPeer` - Peer closed connection
* `ProblemDetectedLocally` - Local problem detected

## Example: Simple Server

```javascript
const greenworks = require('greenworks');

if (!greenworks.initAPI()) {
  console.error('Failed to initialize Steam API');
  process.exit(1);
}

const PORT = 27015;
const listenSocket = greenworks.createListenSocketIP(PORT);

if (!listenSocket) {
  console.error('Failed to create listen socket');
  process.exit(1);
}

console.log('Server listening on port', PORT);

const connections = new Map();

greenworks.on('steam-net-connection-status-changed', (info) => {
  const state = info.state;
  
  if (state === greenworks.NetworkingConnectionState.Connecting) {
    console.log('Accepting connection from', info.steamIDRemote);
    greenworks.acceptConnection(info.connection);
    connections.set(info.connection, { steamId: info.steamIDRemote });
  } else if (state === greenworks.NetworkingConnectionState.Connected) {
    console.log('Client connected:', info.connection);
  } else if (state === greenworks.NetworkingConnectionState.ClosedByPeer ||
             state === greenworks.NetworkingConnectionState.ProblemDetectedLocally) {
    console.log('Connection closed:', info.connection);
    connections.delete(info.connection);
  }
});

// Process messages
setInterval(() => {
  greenworks.runNetworkingCallbacks();
  
  connections.forEach((clientInfo, connection) => {
    const messages = greenworks.receiveMessagesOnConnection(connection);
    messages.forEach(msg => {
      console.log('Received from', connection, ':', msg.data.toString());
      
      // Echo back
      greenworks.sendMessageToConnection(connection, msg.data);
    });
  });
}, 50);
```

## Example: Simple Client

```javascript
const greenworks = require('greenworks');

if (!greenworks.initAPI()) {
  console.error('Failed to initialize Steam API');
  process.exit(1);
}

const connection = greenworks.connectByIPAddress('127.0.0.1', 27015);

if (!connection) {
  console.error('Failed to connect');
  process.exit(1);
}

console.log('Connecting to server...');

let isConnected = false;

greenworks.on('steam-net-connection-status-changed', (info) => {
  if (info.connection !== connection) return;
  
  const state = info.state;
  
  if (state === greenworks.NetworkingConnectionState.Connected) {
    console.log('Connected to server!');
    isConnected = true;
    
    // Send a message
    const message = Buffer.from('Hello, server!');
    greenworks.sendMessageToConnection(connection, message);
  } else if (state === greenworks.NetworkingConnectionState.ClosedByPeer ||
             state === greenworks.NetworkingConnectionState.ProblemDetectedLocally) {
    console.log('Disconnected:', info.endDebug);
    isConnected = false;
  }
});

// Process messages
setInterval(() => {
  greenworks.runNetworkingCallbacks();
  
  if (isConnected) {
    const messages = greenworks.receiveMessagesOnConnection(connection);
    messages.forEach(msg => {
      console.log('Received:', msg.data.toString());
    });
  }
}, 50);
```

## Migration from P2P API

The old P2P API is deprecated. Here's how to migrate:

| Old P2P API | New Networking API |
|-------------|-------------------|
| `acceptP2PSessionWithUser()` | `connectP2P()` + listen for connection event + `acceptConnection()` |
| `sendP2PPacket()` | `sendMessageToConnection()` |
| `readP2PPacket()` | `receiveMessagesOnConnection()` |
| `isP2PPacketAvailable()` | Check array length from `receiveMessagesOnConnection()` |
| `closeP2PSessionWithUser()` | `closeConnection()` |
| `getP2PSessionState()` | `getQuickConnectionStatus()` or `getConnectionInfo()` |

## See Also

- [Steam Networking Messages API](networking-messages.md)
- [Steam Networking Utils API](networking-utils.md)
- [Official Steam Documentation](https://partner.steamgames.com/doc/api/ISteamNetworkingSockets)
