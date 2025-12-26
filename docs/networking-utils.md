# Steam Networking Utils API

The Steam Networking Utils API provides utility functions for configuration, debugging, and network information. It's used alongside the Sockets and Messages APIs.

## Overview

Steam Networking Utils provides:
- Configuration management (global and per-connection)
- Debug output control
- Time synchronization
- Ping and latency information
- Point of Presence (PoP) data
- Fake IP utilities for P2P

## Time Functions

### greenworks.getLocalTimestamp()

Gets a high-precision local timestamp in milliseconds.

* Returns Number - Timestamp in milliseconds

```javascript
const timestamp = greenworks.getLocalTimestamp();
console.log('Current time:', timestamp);
```

This is useful for:
- Timing events
- Measuring latency
- Synchronizing between peers

## Global Configuration

Configuration values control various aspects of networking behavior.

### greenworks.setGlobalConfigValueInt32(configValue, value)

Sets a global integer configuration value.

* `configValue` Integer - The config value enum
* `value` Integer - The value to set
* Returns Boolean - true if successful

```javascript
// Set initial connection timeout to 30 seconds (30000ms)
greenworks.setGlobalConfigValueInt32(
  greenworks.NetworkingConfigValue.TimeoutInitial,
  30000
);
```

### greenworks.setGlobalConfigValueFloat(configValue, value)

Sets a global float configuration value.

* `configValue` Integer - The config value enum
* `value` Number - The value to set
* Returns Boolean - true if successful

```javascript
// Set max send rate to 1MB/s
greenworks.setGlobalConfigValueFloat(
  greenworks.NetworkingConfigValue.SendRateMax,
  1000000
);
```

### greenworks.setGlobalConfigValueString(configValue, value)

Sets a global string configuration value.

* `configValue` Integer - The config value enum
* `value` String - The value to set
* Returns Boolean - true if successful

## Per-Connection Configuration

You can also configure individual connections.

### greenworks.setConnectionConfigValueInt32(connectionHandle, configValue, value)

Sets an integer configuration value for a specific connection.

* `connectionHandle` Integer - The connection handle
* `configValue` Integer - The config value enum
* `value` Integer - The value to set
* Returns Boolean - true if successful

```javascript
// Set connected timeout for this connection to 60 seconds
greenworks.setConnectionConfigValueInt32(
  connection,
  greenworks.NetworkingConfigValue.TimeoutConnected,
  60000
);
```

### greenworks.setConnectionConfigValueFloat(connectionHandle, configValue, value)

Sets a float configuration value for a specific connection.

* `connectionHandle` Integer - The connection handle
* `configValue` Integer - The config value enum
* `value` Number - The value to set
* Returns Boolean - true if successful

### greenworks.setConnectionConfigValueString(connectionHandle, configValue, value)

Sets a string configuration value for a specific connection.

* `connectionHandle` Integer - The connection handle
* `configValue` Integer - The config value enum
* `value` String - The value to set
* Returns Boolean - true if successful

## Common Configuration Values

The `greenworks.NetworkingConfigValue` object contains these commonly used values:

- `TimeoutInitial` - Initial connection timeout (milliseconds)
- `TimeoutConnected` - Timeout for established connections (milliseconds)
- `SendBufferSize` - Send buffer size (bytes)
- `SendRateMin` - Minimum send rate (bytes/second)
- `SendRateMax` - Maximum send rate (bytes/second)
- `NagleTime` - Nagle algorithm time (microseconds)
- `IP_AllowWithoutAuth` - Allow IP connections without Steam auth
- `MTU_PacketSize` - MTU packet size
- `Unencrypted` - Disable encryption (for debugging only)
- `SymmetricConnect` - Enable symmetric connection mode
- `LocalVirtualPort` - Local virtual port number

## Debug Output

### greenworks.setDebugOutputFunction(detailLevel)

Sets the debug output level for networking.

* `detailLevel` Integer - Debug output level
* Returns Boolean - true if successful

```javascript
// Set debug level to show important messages
greenworks.setDebugOutputFunction(
  greenworks.NetworkingDebugOutputType.Important
);
```

### Debug Output Levels

From `greenworks.NetworkingDebugOutputType`:

- `None` - No debug output
- `Bug` - Only critical bugs
- `Error` - Errors and bugs
- `Important` - Important information
- `Warning` - Warnings and above
- `Msg` - General messages
- `Verbose` - Verbose output
- `Debug` - Debug information
- `Everything` - All output

## Ping and Network Information

### greenworks.checkPingDataUpToDate(maxAgeSeconds)

Checks if ping data is up-to-date.

* `maxAgeSeconds` Number (optional) - Maximum age in seconds (default: 300)
* Returns Boolean - true if data is fresh

```javascript
const isUpToDate = greenworks.checkPingDataUpToDate(60);
if (!isUpToDate) {
  console.log('Ping data is stale');
}
```

### greenworks.getPingToDataCenter(popId)

Gets the ping to a specific data center (Point of Presence).

* `popId` Integer - PoP ID
* Returns Object or null - Ping info

```javascript
const pingInfo = greenworks.getPingToDataCenter(popId);
if (pingInfo) {
  console.log('Ping:', pingInfo.ping, 'ms');
  console.log('Via relay PoP:', pingInfo.viaRelayPoP);
}
```

### greenworks.getDirectPingToPOP(popId)

Gets the direct ping to a PoP (without relay).

* `popId` Integer - PoP ID
* Returns Integer or null - Ping in milliseconds

```javascript
const directPing = greenworks.getDirectPingToPOP(popId);
if (directPing !== null) {
  console.log('Direct ping:', directPing, 'ms');
}
```

### greenworks.getPOPCount()

Gets the number of available Points of Presence.

* Returns Integer - Number of PoPs

```javascript
const popCount = greenworks.getPOPCount();
console.log('Available PoPs:', popCount);
```

### greenworks.getPOPList()

Gets the list of available PoP IDs.

* Returns Array - Array of PoP IDs

```javascript
const pops = greenworks.getPOPList();
console.log('Available PoPs:', pops);

// Get ping to each PoP
pops.forEach(popId => {
  const ping = greenworks.getDirectPingToPOP(popId);
  console.log('PoP', popId, '- Ping:', ping, 'ms');
});
```

## Fake IP Functions

Fake IPs are used for P2P connections to provide a consistent IP-like interface.

### greenworks.isFakeIPv4(ipAddress)

Checks if an IP address is a fake IP.

* `ipAddress` Integer - IP address as uint32
* Returns Boolean - true if fake IP

### greenworks.getIPv4FakeIPType(ipAddress)

Gets the type of a fake IP address.

* `ipAddress` Integer - IP address as uint32
* Returns Integer - Fake IP type

### greenworks.getRealIdentityForFakeIP(ipAddress, port)

Gets the real Steam identity for a fake IP.

* `ipAddress` String - IP address string
* `port` Integer - Port number
* Returns String or null - Steam ID

```javascript
const steamId = greenworks.getRealIdentityForFakeIP('10.0.0.1', 27015);
if (steamId) {
  console.log('Real identity:', steamId);
}
```

## Examples

### Example: Configure for Low-Latency Gaming

```javascript
const greenworks = require('greenworks');

if (!greenworks.initAPI()) {
  console.error('Failed to initialize Steam API');
  process.exit(1);
}

// Configure for low-latency, high-frequency updates
function configureLowLatency() {
  // Reduce Nagle time for faster small packet sending
  greenworks.setGlobalConfigValueInt32(
    greenworks.NetworkingConfigValue.NagleTime,
    0  // Disable Nagle
  );
  
  // Set higher send rate for more responsive gameplay
  greenworks.setGlobalConfigValueInt32(
    greenworks.NetworkingConfigValue.SendRateMax,
    2000000  // 2 MB/s
  );
  
  // Enable verbose debug output during development
  greenworks.setDebugOutputFunction(
    greenworks.NetworkingDebugOutputType.Warning
  );
  
  console.log('Configured for low-latency gaming');
}

configureLowLatency();
```

### Example: Monitor Network Quality

```javascript
class NetworkMonitor {
  constructor() {
    this.connections = new Map();
  }
  
  addConnection(handle, name) {
    this.connections.set(handle, {
      name: name,
      samples: [],
      maxSamples: 100
    });
  }
  
  removeConnection(handle) {
    this.connections.delete(handle);
  }
  
  update() {
    const now = greenworks.getLocalTimestamp();
    
    this.connections.forEach((info, handle) => {
      const status = greenworks.getQuickConnectionStatus(handle);
      
      if (status) {
        const sample = {
          time: now,
          ping: status.ping,
          quality: status.connectionQualityLocal,
          inRate: status.inBytesPerSec,
          outRate: status.outBytesPerSec
        };
        
        info.samples.push(sample);
        
        // Keep only recent samples
        if (info.samples.length > info.maxSamples) {
          info.samples.shift();
        }
        
        // Check for issues
        if (status.ping > 200) {
          console.warn(`High ping on ${info.name}: ${status.ping}ms`);
        }
        
        if (status.connectionQualityLocal < 0.5) {
          console.warn(`Poor quality on ${info.name}: ${status.connectionQualityLocal}`);
        }
      }
    });
  }
  
  getAveragePing(handle) {
    const info = this.connections.get(handle);
    if (!info || info.samples.length === 0) return null;
    
    const sum = info.samples.reduce((acc, s) => acc + s.ping, 0);
    return sum / info.samples.length;
  }
  
  getStats(handle) {
    const info = this.connections.get(handle);
    if (!info || info.samples.length === 0) return null;
    
    const avgPing = this.getAveragePing(handle);
    const avgQuality = info.samples.reduce((acc, s) => acc + s.quality, 0) / info.samples.length;
    const avgInRate = info.samples.reduce((acc, s) => acc + s.inRate, 0) / info.samples.length;
    const avgOutRate = info.samples.reduce((acc, s) => acc + s.outRate, 0) / info.samples.length;
    
    return {
      name: info.name,
      avgPing: avgPing.toFixed(1),
      avgQuality: avgQuality.toFixed(2),
      avgInRate: (avgInRate / 1024).toFixed(1),  // KB/s
      avgOutRate: (avgOutRate / 1024).toFixed(1)  // KB/s
    };
  }
  
  printReport() {
    console.log('\n=== Network Quality Report ===');
    this.connections.forEach((info, handle) => {
      const stats = this.getStats(handle);
      if (stats) {
        console.log(`${stats.name}:`);
        console.log(`  Avg Ping: ${stats.avgPing}ms`);
        console.log(`  Avg Quality: ${stats.avgQuality}`);
        console.log(`  Avg In: ${stats.avgInRate} KB/s`);
        console.log(`  Avg Out: ${stats.avgOutRate} KB/s`);
      }
    });
    console.log('==============================\n');
  }
}

// Usage
const monitor = new NetworkMonitor();

const conn1 = greenworks.connectByIPAddress('192.168.1.100', 27015);
monitor.addConnection(conn1, 'Server 1');

const conn2 = greenworks.connectP2P('76561198012345678', 0);
monitor.addConnection(conn2, 'Player 2');

// Update every second
setInterval(() => {
  monitor.update();
}, 1000);

// Print report every 10 seconds
setInterval(() => {
  monitor.printReport();
}, 10000);
```

### Example: Find Best Data Center

```javascript
function findBestDataCenter() {
  // Make sure ping data is up to date
  if (!greenworks.checkPingDataUpToDate(60)) {
    console.log('Updating ping data...');
    // Data will be updated automatically by Steam
    return null;
  }
  
  const pops = greenworks.getPOPList();
  if (pops.length === 0) {
    console.log('No PoPs available');
    return null;
  }
  
  let bestPop = null;
  let bestPing = Infinity;
  
  console.log('Checking', pops.length, 'data centers...');
  
  pops.forEach(popId => {
    const directPing = greenworks.getDirectPingToPOP(popId);
    
    if (directPing !== null && directPing < bestPing) {
      bestPing = directPing;
      bestPop = popId;
    }
    
    console.log(`PoP ${popId}: ${directPing}ms`);
  });
  
  if (bestPop !== null) {
    console.log(`\nBest data center: PoP ${bestPop} with ${bestPing}ms ping`);
    return { popId: bestPop, ping: bestPing };
  }
  
  return null;
}

// Usage
if (greenworks.initAPI()) {
  setTimeout(() => {
    const best = findBestDataCenter();
    if (best) {
      console.log('Use data center:', best.popId);
    }
  }, 2000);  // Wait for ping data to be available
}
```

## Best Practices

1. **Set config early**: Configure networking settings before creating connections
2. **Use debug output**: Enable debug output during development
3. **Monitor quality**: Regularly check connection status and quality
4. **Update ping data**: Ensure ping data is current before making decisions
5. **Per-connection config**: Fine-tune settings for specific connections when needed
6. **Test configurations**: Test different configurations to find optimal settings

## See Also

- [Steam Networking Sockets API](networking-sockets.md)
- [Steam Networking Messages API](networking-messages.md)
- [Official Steam Documentation](https://partner.steamgames.com/doc/api/ISteamNetworkingUtils)
