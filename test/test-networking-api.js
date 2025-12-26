// Example of using the new Steam Networking Sockets API
// This example requires Steam to be running and a valid steam_appid.txt file

const greenworks = require('../greenworks');

// Initialize Steam API
if (!greenworks.initAPI()) {
  console.error('Failed to initialize Steam API');
  console.error('Make sure Steam is running and steam_appid.txt exists');
  process.exit(1);
}

console.log('Steam API initialized successfully');
console.log('Steam ID:', greenworks.getSteamId().steamId);

// Example 1: Create a listen socket for incoming connections
function exampleListenSocket() {
  console.log('\n=== Example 1: Listen Socket ===');
  
  const PORT = 27015;
  const listenSocket = greenworks.createListenSocketIP(PORT);
  
  if (listenSocket) {
    console.log('✓ Created listen socket on port', PORT);
    console.log('  Listen socket handle:', listenSocket);
    
    // Clean up
    setTimeout(() => {
      greenworks.closeListenSocket(listenSocket);
      console.log('✓ Closed listen socket');
    }, 1000);
  } else {
    console.log('✗ Failed to create listen socket');
  }
}

// Example 2: Connect to an IP address
function exampleConnectIP() {
  console.log('\n=== Example 2: Connect by IP ===');
  
  // Note: This will fail without a server listening on this address
  const connection = greenworks.connectByIPAddress('127.0.0.1', 27015);
  
  if (connection) {
    console.log('✓ Connection initiated');
    console.log('  Connection handle:', connection);
    
    // Listen for connection status changes
    const handler = (info) => {
      if (info.connection === connection) {
        console.log('  Connection state changed:', info.state);
        console.log('  Description:', info.connectionDescription);
        
        if (info.state === greenworks.NetworkingConnectionState.Connected) {
          console.log('✓ Connected!');
        } else if (info.state === greenworks.NetworkingConnectionState.ProblemDetectedLocally) {
          console.log('✗ Connection failed:', info.endDebug);
          greenworks.removeListener('steam-net-connection-status-changed', handler);
        }
      }
    };
    
    greenworks.on('steam-net-connection-status-changed', handler);
    
    // Clean up after a bit
    setTimeout(() => {
      greenworks.closeConnection(connection);
      console.log('✓ Closed connection');
      greenworks.removeListener('steam-net-connection-status-changed', handler);
    }, 2000);
  } else {
    console.log('✗ Failed to initiate connection');
  }
}

// Example 3: P2P connection to a Steam user
function exampleConnectP2P() {
  console.log('\n=== Example 3: P2P Connection ===');
  
  // Get own Steam ID (in a real scenario, you'd use a friend's Steam ID)
  const mySteamId = greenworks.getSteamId().steamId;
  console.log('Attempting P2P connection to:', mySteamId);
  
  const connection = greenworks.connectP2P(mySteamId, 0);
  
  if (connection) {
    console.log('✓ P2P connection initiated');
    console.log('  Connection handle:', connection);
  } else {
    console.log('✗ Failed to initiate P2P connection');
  }
}

// Example 4: Check available constants
function exampleConstants() {
  console.log('\n=== Example 4: Available Constants ===');
  
  console.log('NetworkingSendFlags:', greenworks.NetworkingSendFlags);
  console.log('NetworkingConnectionState:', greenworks.NetworkingConnectionState);
  console.log('NetworkingDebugOutputType:', greenworks.NetworkingDebugOutputType);
  console.log('NetworkingConfigValue:', greenworks.NetworkingConfigValue);
}

// Example 5: Networking Messages API
function exampleNetworkingMessages() {
  console.log('\n=== Example 5: Networking Messages ===');
  
  // Try to send a message to ourselves (will fail, but demonstrates the API)
  const mySteamId = greenworks.getSteamId().steamId;
  const message = Buffer.from('Hello, world!');
  
  const success = greenworks.sendMessageToUser(
    mySteamId,
    message,
    greenworks.NetworkingSendFlags.Reliable,
    0  // channel
  );
  
  console.log('Send message result:', success ? '✓ Success' : '✗ Failed');
  
  // Try to receive messages
  const messages = greenworks.receiveMessagesOnChannel(0);
  console.log('Received messages:', messages.length);
}

// Example 6: Networking Utils API
function exampleNetworkingUtils() {
  console.log('\n=== Example 6: Networking Utils ===');
  
  // Get local timestamp
  const timestamp = greenworks.getLocalTimestamp();
  console.log('Local timestamp:', timestamp, 'ms');
  
  // Check ping data
  const upToDate = greenworks.checkPingDataUpToDate(300);
  console.log('Ping data up to date:', upToDate);
  
  // Get PoP information
  const popCount = greenworks.getPOPCount();
  console.log('Available PoPs:', popCount);
  
  if (popCount > 0) {
    const pops = greenworks.getPOPList();
    console.log('First few PoPs:', pops.slice(0, 5));
  }
  
  // Set debug output level
  greenworks.setDebugOutputFunction(
    greenworks.NetworkingDebugOutputType.Warning
  );
  console.log('✓ Set debug output level to Warning');
}

// Run examples
console.log('\n======================================');
console.log('Steam Networking API Examples');
console.log('======================================');

exampleConstants();
exampleNetworkingUtils();
exampleListenSocket();

// Wait a bit before trying connections
setTimeout(() => {
  exampleConnectIP();
}, 1500);

setTimeout(() => {
  exampleConnectP2P();
}, 3000);

setTimeout(() => {
  exampleNetworkingMessages();
}, 4000);

// Run networking callbacks periodically
const callbackInterval = setInterval(() => {
  greenworks.runNetworkingCallbacks();
}, 100);

// Clean up and exit after 6 seconds
setTimeout(() => {
  clearInterval(callbackInterval);
  console.log('\n======================================');
  console.log('Examples completed');
  console.log('======================================\n');
  process.exit(0);
}, 6000);
