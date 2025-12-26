# P2P API (Deprecated)

> **⚠️ DEPRECATED:** The P2P API is deprecated in newer Steam SDK versions and will be removed in the future. Please migrate to the new [Networking Sockets API](networking-sockets.md) or [Networking Messages API](networking-messages.md) for better performance, reliability, and future compatibility.

## Migration Guide

For migrating from the P2P API to the new networking APIs, see:
- [Networking Sockets API Documentation](networking-sockets.md#migration-from-p2p-api)
- [Networking Messages API Documentation](networking-messages.md#migration-from-p2p-api)

---

### added p2p function:
  - `greenworks.sendP2PPacket(steamId: string, sendType: eP2PSendType, data: Buffer,nChannel:number): boolean`
  - `greenworks.isP2PPacketAvailable(nChannel:number): number`
  - `greenworks.readP2PPacket(size: number,nChannel:number):{data: Buffer,steamIDRemote: string}`
  - `greenworks.acceptP2PSessionWithUser(steamId: string): void`
  - `greenworks.getP2PSessionState(steamIDUser: string): {result:boolean,connectionState:Object}`
  - `greenworks.closeP2PSessionWithUser(steamIDUser: string): boolean`
  - `greenworks.closeP2PChannelWithUser(steamIDUser: string, nChannel: number): boolean`
  - `greenworks.isBehindNAT():boolean`

### added enum `eP2PSendType` on types

### added event `p2p-session-request`,`p2p-session-connect-fail`

[Steam docs](https://partner.steamgames.com/doc/api/ISteamNetworking)