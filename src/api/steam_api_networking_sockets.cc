// Copyright (c) 2024 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <memory>

#include "nan.h"
#include "steam/steam_api.h"
#include "steam/isteamnetworkingsockets.h"
#include "steam/isteamnetworkingutils.h"
#include "v8.h"

#include "greenworks_utils.h"
#include "steam_api_registry.h"
#include "steam_id.h"

namespace greenworks {
namespace api {
namespace {

// Connection management

NAN_METHOD(CreateListenSocketIP) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: port number required");
  }
  
  int port = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  
  SteamNetworkingIPAddr localAddress;
  localAddress.Clear();
  localAddress.m_port = port;
  
  int nOptions = 0;
  const SteamNetworkingConfigValue_t *pOptions = nullptr;
  
  HSteamListenSocket hSocket = SteamNetworkingSockets()->CreateListenSocketIP(
      localAddress, nOptions, pOptions);
  
  if (hSocket == k_HSteamListenSocket_Invalid) {
    info.GetReturnValue().Set(Nan::New(0));
  } else {
    info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(hSocket)));
  }
}

NAN_METHOD(ConnectByIPAddress) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: IP address (string) and port (number) required");
  }
  
  std::string ip_str(*(Nan::Utf8String(info[0])));
  int port = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  
  SteamNetworkingIPAddr address;
  if (!address.ParseString(ip_str.c_str())) {
    THROW_BAD_ARGS("Invalid IP address format");
  }
  address.m_port = port;
  
  int nOptions = 0;
  const SteamNetworkingConfigValue_t *pOptions = nullptr;
  
  HSteamNetConnection hConn = SteamNetworkingSockets()->ConnectByIPAddress(
      address, nOptions, pOptions);
  
  if (hConn == k_HSteamNetConnection_Invalid) {
    info.GetReturnValue().Set(Nan::New(0));
  } else {
    info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(hConn)));
  }
}

NAN_METHOD(ConnectP2P) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsString()) {
    THROW_BAD_ARGS("Bad arguments: Steam ID required");
  }
  
  std::string steam_id_str(*(Nan::Utf8String(info[0])));
  CSteamID steam_id(utils::strToUint64(steam_id_str));
  if (!steam_id.IsValid()) {
    THROW_BAD_ARGS("Steam ID is invalid");
  }
  
  SteamNetworkingIdentity identity;
  identity.SetSteamID(steam_id);
  
  int nVirtualPort = 0;
  if (info.Length() >= 2 && info[1]->IsInt32()) {
    nVirtualPort = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  }
  
  int nOptions = 0;
  const SteamNetworkingConfigValue_t *pOptions = nullptr;
  
  HSteamNetConnection hConn = SteamNetworkingSockets()->ConnectP2P(
      identity, nVirtualPort, nOptions, pOptions);
  
  if (hConn == k_HSteamNetConnection_Invalid) {
    info.GetReturnValue().Set(Nan::New(0));
  } else {
    info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(hConn)));
  }
}

NAN_METHOD(AcceptConnection) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  EResult result = SteamNetworkingSockets()->AcceptConnection(hConn);
  info.GetReturnValue().Set(Nan::New(result == k_EResultOK));
}

NAN_METHOD(CloseConnection) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  int nReason = 0;
  const char *pszDebug = nullptr;
  bool bEnableLinger = false;
  
  if (info.Length() >= 2 && info[1]->IsInt32()) {
    nReason = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  }
  if (info.Length() >= 3 && info[2]->IsString()) {
    Nan::Utf8String debug_str(info[2]);
    pszDebug = *debug_str;
  }
  if (info.Length() >= 4 && info[3]->IsBoolean()) {
    bEnableLinger = info[3]->BooleanValue(Nan::GetCurrentContext()).FromJust();
  }
  
  bool success = SteamNetworkingSockets()->CloseConnection(
      hConn, nReason, pszDebug, bEnableLinger);
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(CloseListenSocket) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: listen socket handle required");
  }
  
  HSteamListenSocket hSocket = static_cast<HSteamListenSocket>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  bool success = SteamNetworkingSockets()->CloseListenSocket(hSocket);
  info.GetReturnValue().Set(Nan::New(success));
}

// Message sending/receiving

NAN_METHOD(SendMessageToConnection) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle and data buffer required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  if (!info[1]->IsObject() || !node::Buffer::HasInstance(info[1])) {
    THROW_BAD_ARGS("Second argument must be a buffer");
  }
  
  v8::Local<v8::Object> bufferObj =
      info[1]->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
  const void *pData = node::Buffer::Data(bufferObj);
  uint32 cbData = static_cast<uint32>(node::Buffer::Length(bufferObj));
  
  int nSendFlags = k_nSteamNetworkingSend_Reliable;
  if (info.Length() >= 3 && info[2]->IsInt32()) {
    nSendFlags = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  }
  
  int64 *pOutMessageNumber = nullptr;
  
  EResult result = SteamNetworkingSockets()->SendMessageToConnection(
      hConn, pData, cbData, nSendFlags, pOutMessageNumber);
  
  info.GetReturnValue().Set(Nan::New(result == k_EResultOK));
}

NAN_METHOD(ReceiveMessagesOnConnection) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  int nMaxMessages = 32;
  if (info.Length() >= 2 && info[1]->IsInt32()) {
    nMaxMessages = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
    if (nMaxMessages < 1 || nMaxMessages > 256) {
      nMaxMessages = 32;
    }
  }
  
  SteamNetworkingMessage_t **ppOutMessages = 
      new SteamNetworkingMessage_t*[nMaxMessages];
  
  int numMessages = SteamNetworkingSockets()->ReceiveMessagesOnConnection(
      hConn, ppOutMessages, nMaxMessages);
  
  v8::Local<v8::Array> messages = Nan::New<v8::Array>(numMessages);
  
  for (int i = 0; i < numMessages; i++) {
    SteamNetworkingMessage_t *pMsg = ppOutMessages[i];
    
    v8::Local<v8::Object> messageObj = Nan::New<v8::Object>();
    
    Nan::Set(messageObj, Nan::New("data").ToLocalChecked(),
             Nan::CopyBuffer(static_cast<const char*>(pMsg->m_pData), 
                           pMsg->m_cbSize).ToLocalChecked());
    Nan::Set(messageObj, Nan::New("size").ToLocalChecked(),
             Nan::New(pMsg->m_cbSize));
    Nan::Set(messageObj, Nan::New("connection").ToLocalChecked(),
             Nan::New(static_cast<uint32_t>(pMsg->m_conn)));
    Nan::Set(messageObj, Nan::New("channel").ToLocalChecked(),
             Nan::New(pMsg->m_nChannel));
    
    Nan::Set(messages, i, messageObj);
    
    pMsg->Release();
  }
  
  delete[] ppOutMessages;
  
  info.GetReturnValue().Set(messages);
}

NAN_METHOD(FlushMessagesOnConnection) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  EResult result = SteamNetworkingSockets()->FlushMessagesOnConnection(hConn);
  info.GetReturnValue().Set(Nan::New(result == k_EResultOK));
}

// Connection info and status

NAN_METHOD(GetConnectionInfo) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  SteamNetConnectionInfo_t connectionInfo;
  if (!SteamNetworkingSockets()->GetConnectionInfo(hConn, &connectionInfo)) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  v8::Local<v8::Object> infoObj = Nan::New<v8::Object>();
  
  Nan::Set(infoObj, Nan::New("state").ToLocalChecked(),
           Nan::New(connectionInfo.m_eState));
  Nan::Set(infoObj, Nan::New("endReason").ToLocalChecked(),
           Nan::New(connectionInfo.m_eEndReason));
  
  // Get Steam ID if available
  CSteamID steamID;
  if (connectionInfo.m_identityRemote.GetSteamID(&steamID)) {
    Nan::Set(infoObj, Nan::New("steamIDRemote").ToLocalChecked(),
             Nan::New(std::to_string(steamID.ConvertToUint64())).ToLocalChecked());
  }
  
  Nan::Set(infoObj, Nan::New("connectionDescription").ToLocalChecked(),
           Nan::New(connectionInfo.m_szConnectionDescription).ToLocalChecked());
  
  info.GetReturnValue().Set(infoObj);
}

NAN_METHOD(GetQuickConnectionStatus) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  SteamNetworkingQuickConnectionStatus status;
  if (!SteamNetworkingSockets()->GetQuickConnectionStatus(hConn, &status)) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  v8::Local<v8::Object> statusObj = Nan::New<v8::Object>();
  
  Nan::Set(statusObj, Nan::New("state").ToLocalChecked(),
           Nan::New(status.m_eState));
  Nan::Set(statusObj, Nan::New("ping").ToLocalChecked(),
           Nan::New(status.m_nPing));
  Nan::Set(statusObj, Nan::New("connectionQualityLocal").ToLocalChecked(),
           Nan::New(status.m_flConnectionQualityLocal));
  Nan::Set(statusObj, Nan::New("connectionQualityRemote").ToLocalChecked(),
           Nan::New(status.m_flConnectionQualityRemote));
  Nan::Set(statusObj, Nan::New("outPacketsPerSec").ToLocalChecked(),
           Nan::New(status.m_flOutPacketsPerSec));
  Nan::Set(statusObj, Nan::New("outBytesPerSec").ToLocalChecked(),
           Nan::New(status.m_flOutBytesPerSec));
  Nan::Set(statusObj, Nan::New("inPacketsPerSec").ToLocalChecked(),
           Nan::New(status.m_flInPacketsPerSec));
  Nan::Set(statusObj, Nan::New("inBytesPerSec").ToLocalChecked(),
           Nan::New(status.m_flInBytesPerSec));
  Nan::Set(statusObj, Nan::New("sendRateBytesPerSecond").ToLocalChecked(),
           Nan::New(status.m_nSendRateBytesPerSecond));
  Nan::Set(statusObj, Nan::New("pendingUnreliable").ToLocalChecked(),
           Nan::New(status.m_cbPendingUnreliable));
  Nan::Set(statusObj, Nan::New("pendingReliable").ToLocalChecked(),
           Nan::New(status.m_cbPendingReliable));
  Nan::Set(statusObj, Nan::New("sentUnackedReliable").ToLocalChecked(),
           Nan::New(status.m_cbSentUnackedReliable));
  
  info.GetReturnValue().Set(statusObj);
}

NAN_METHOD(GetDetailedConnectionStatus) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  char szBuf[2048];
  int result = SteamNetworkingSockets()->GetDetailedConnectionStatus(
      hConn, szBuf, sizeof(szBuf));
  
  if (result < 0) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  info.GetReturnValue().Set(Nan::New(szBuf).ToLocalChecked());
}

// Poll for state changes

NAN_METHOD(RunCallbacks) {
  Nan::HandleScope scope;
  SteamNetworkingSockets()->RunCallbacks();
  info.GetReturnValue().Set(Nan::Undefined());
}

void RegisterAPIs(v8::Local<v8::Object> target) {
  // Connection management
  SET_FUNCTION("createListenSocketIP", CreateListenSocketIP);
  SET_FUNCTION("connectByIPAddress", ConnectByIPAddress);
  SET_FUNCTION("connectP2P", ConnectP2P);
  SET_FUNCTION("acceptConnection", AcceptConnection);
  SET_FUNCTION("closeConnection", CloseConnection);
  SET_FUNCTION("closeListenSocket", CloseListenSocket);
  
  // Message sending/receiving
  SET_FUNCTION("sendMessageToConnection", SendMessageToConnection);
  SET_FUNCTION("receiveMessagesOnConnection", ReceiveMessagesOnConnection);
  SET_FUNCTION("flushMessagesOnConnection", FlushMessagesOnConnection);
  
  // Connection info
  SET_FUNCTION("getConnectionInfo", GetConnectionInfo);
  SET_FUNCTION("getQuickConnectionStatus", GetQuickConnectionStatus);
  SET_FUNCTION("getDetailedConnectionStatus", GetDetailedConnectionStatus);
  
  // Callbacks
  SET_FUNCTION("runNetworkingCallbacks", RunCallbacks);
  
  // Constants
  v8::Local<v8::Object> sendFlags = Nan::New<v8::Object>();
  SET_TYPE(sendFlags, "Unreliable", k_nSteamNetworkingSend_Unreliable);
  SET_TYPE(sendFlags, "NoNagle", k_nSteamNetworkingSend_NoNagle);
  SET_TYPE(sendFlags, "UnreliableNoNagle", k_nSteamNetworkingSend_UnreliableNoNagle);
  SET_TYPE(sendFlags, "NoDelay", k_nSteamNetworkingSend_NoDelay);
  SET_TYPE(sendFlags, "UnreliableNoDelay", k_nSteamNetworkingSend_UnreliableNoDelay);
  SET_TYPE(sendFlags, "Reliable", k_nSteamNetworkingSend_Reliable);
  SET_TYPE(sendFlags, "ReliableNoNagle", k_nSteamNetworkingSend_ReliableNoNagle);
  Nan::Set(target, Nan::New("NetworkingSendFlags").ToLocalChecked(), sendFlags);
  
  v8::Local<v8::Object> connectionState = Nan::New<v8::Object>();
  SET_TYPE(connectionState, "None", k_ESteamNetworkingConnectionState_None);
  SET_TYPE(connectionState, "Connecting", k_ESteamNetworkingConnectionState_Connecting);
  SET_TYPE(connectionState, "FindingRoute", k_ESteamNetworkingConnectionState_FindingRoute);
  SET_TYPE(connectionState, "Connected", k_ESteamNetworkingConnectionState_Connected);
  SET_TYPE(connectionState, "ClosedByPeer", k_ESteamNetworkingConnectionState_ClosedByPeer);
  SET_TYPE(connectionState, "ProblemDetectedLocally", k_ESteamNetworkingConnectionState_ProblemDetectedLocally);
  Nan::Set(target, Nan::New("NetworkingConnectionState").ToLocalChecked(), connectionState);
}

SteamAPIRegistry::Add X(RegisterAPIs);

}  // namespace
}  // namespace api
}  // namespace greenworks
