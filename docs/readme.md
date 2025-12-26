# Greenworks Documents

## FAQ

There are questions/issues being asked quite often. Check following pages before
creating an issue.
  * [Troubleshooting](troubleshooting.md)
  * [Gotchas](gotchas.md)

## Guides

* [Quick Start (NW.js)](quick-start-nwjs.md)

## Build Instructions

* [Build Instructions (NW.js)](build-instructions-nwjs.md)
* [Build Instructions (Electron)](build-instructions-electron.md)
* [Build Instructions (Node.js)](build-instructions-nodejs.md)

## Test

* [Mocha Test](mocha-test.md)

## APIs

The `greenworks` module gives you ability to access Steam APIs:

```js
var greenworks = require('./greenworks');

if (greenworks.init())
  console.log('Steam API has been initialized.');
```

## API References

* [Achievement](achievement.md)
* [Authentication](authentication.md)
* [Cloud](cloud.md)
* [DLC](dlc.md)
* [Events](events.md)
* [Friends](friends.md)
* [Setting](setting.md)
* [Stats](stats.md)
* [Utils](utils.md)
* [Workshop](workshop.md)
* [Matchmaking](matchmaking.md)
* [P2P](p2p.md) - **Deprecated: Use Networking APIs instead**

## Networking APIs (New)

The following modern networking APIs replace the deprecated P2P API:

* [Networking Sockets](networking-sockets.md) - Connection-oriented networking with ISteamNetworkingSockets
* [Networking Messages](networking-messages.md) - Connectionless message-oriented networking with ISteamNetworkingMessages
* [Networking Utils](networking-utils.md) - Configuration and utilities with ISteamNetworkingUtils

**Note:** The old P2P API is deprecated in newer Steam SDK versions. Please migrate to the new Networking APIs for future compatibility.
