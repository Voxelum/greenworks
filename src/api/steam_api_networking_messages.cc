// Copyright (c) 2024 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <memory>

#include "nan.h"
#include "steam/steam_api.h"
#include "steam/isteamnetworkingmessages.h"
#include "v8.h"

#include "greenworks_utils.h"
#include "steam_api_registry.h"
#include "steam_id.h"

namespace greenworks {
namespace api {
namespace {

// Send messages to a remote host

NAN_METHOD(SendMessageToUser) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString()) {
    THROW_BAD_ARGS("Bad arguments: Steam ID and data buffer required");
  }
  
  std::string steam_id_str(*(Nan::Utf8String(info[0])));
  CSteamID steam_id(utils::strToUint64(steam_id_str));
  if (!steam_id.IsValid()) {
    THROW_BAD_ARGS("Steam ID is invalid");
  }
  
  SteamNetworkingIdentity identity;
  identity.SetSteamID(steam_id);
  
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
  
  int nRemoteVirtualPort = 0;
  if (info.Length() >= 4 && info[3]->IsInt32()) {
    nRemoteVirtualPort = info[3]->Int32Value(Nan::GetCurrentContext()).FromJust();
  }
  
  EResult result = SteamNetworkingMessages()->SendMessageToUser(
      identity, pData, cbData, nSendFlags, nRemoteVirtualPort);
  
  info.GetReturnValue().Set(Nan::New(result == k_EResultOK));
}

// Receive messages

NAN_METHOD(ReceiveMessagesOnChannel) {
  Nan::HandleScope scope;
  
  int nLocalVirtualPort = 0;
  if (info.Length() >= 1 && info[0]->IsInt32()) {
    nLocalVirtualPort = info[0]->Int32Value(Nan::GetCurrentContext()).FromJust();
  }
  
  int nMaxMessages = 32;
  if (info.Length() >= 2 && info[1]->IsInt32()) {
    nMaxMessages = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
    if (nMaxMessages < 1 || nMaxMessages > 256) {
      nMaxMessages = 32;
    }
  }
  
  SteamNetworkingMessage_t **ppOutMessages = 
      new SteamNetworkingMessage_t*[nMaxMessages];
  
  int numMessages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(
      nLocalVirtualPort, ppOutMessages, nMaxMessages);
  
  v8::Local<v8::Array> messages = Nan::New<v8::Array>(numMessages);
  
  for (int i = 0; i < numMessages; i++) {
    SteamNetworkingMessage_t *pMsg = ppOutMessages[i];
    
    v8::Local<v8::Object> messageObj = Nan::New<v8::Object>();
    
    Nan::Set(messageObj, Nan::New("data").ToLocalChecked(),
             Nan::CopyBuffer(static_cast<const char*>(pMsg->m_pData), 
                           pMsg->m_cbSize).ToLocalChecked());
    Nan::Set(messageObj, Nan::New("size").ToLocalChecked(),
             Nan::New(pMsg->m_cbSize));
    Nan::Set(messageObj, Nan::New("channel").ToLocalChecked(),
             Nan::New(pMsg->m_nChannel));
    
    // Get sender Steam ID
    CSteamID senderSteamID;
    if (pMsg->m_identityPeer.GetSteamID(&senderSteamID)) {
      Nan::Set(messageObj, Nan::New("steamIDRemote").ToLocalChecked(),
               Nan::New(std::to_string(senderSteamID.ConvertToUint64())).ToLocalChecked());
    }
    
    Nan::Set(messages, i, messageObj);
    
    pMsg->Release();
  }
  
  delete[] ppOutMessages;
  
  info.GetReturnValue().Set(messages);
}

// Accept incoming messages

NAN_METHOD(AcceptSessionWithUser) {
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
  
  bool result = SteamNetworkingMessages()->AcceptSessionWithUser(identity);
  info.GetReturnValue().Set(Nan::New(result));
}

// Close session

NAN_METHOD(CloseSessionWithUser) {
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
  
  bool result = SteamNetworkingMessages()->CloseSessionWithUser(identity);
  info.GetReturnValue().Set(Nan::New(result));
}

// Close all channels to a user

NAN_METHOD(CloseChannelWithUser) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: Steam ID and channel required");
  }
  
  std::string steam_id_str(*(Nan::Utf8String(info[0])));
  CSteamID steam_id(utils::strToUint64(steam_id_str));
  if (!steam_id.IsValid()) {
    THROW_BAD_ARGS("Steam ID is invalid");
  }
  
  SteamNetworkingIdentity identity;
  identity.SetSteamID(steam_id);
  
  int nLocalVirtualPort = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  
  bool result = SteamNetworkingMessages()->CloseChannelWithUser(
      identity, nLocalVirtualPort);
  info.GetReturnValue().Set(Nan::New(result));
}

// Get session state

NAN_METHOD(GetSessionConnectionInfo) {
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
  
  SteamNetConnectionInfo_t connectionInfo;
  SteamNetworkingQuickConnectionStatus quickStatus;
  
  ESteamNetworkingConnectionState state = 
      SteamNetworkingMessages()->GetSessionConnectionInfo(
          identity, &connectionInfo, &quickStatus);
  
  if (state == k_ESteamNetworkingConnectionState_None) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  v8::Local<v8::Object> resultObj = Nan::New<v8::Object>();
  
  Nan::Set(resultObj, Nan::New("state").ToLocalChecked(),
           Nan::New(state));
  
  v8::Local<v8::Object> infoObj = Nan::New<v8::Object>();
  Nan::Set(infoObj, Nan::New("endReason").ToLocalChecked(),
           Nan::New(connectionInfo.m_eEndReason));
  Nan::Set(infoObj, Nan::New("connectionDescription").ToLocalChecked(),
           Nan::New(connectionInfo.m_szConnectionDescription).ToLocalChecked());
  Nan::Set(resultObj, Nan::New("connectionInfo").ToLocalChecked(), infoObj);
  
  v8::Local<v8::Object> statusObj = Nan::New<v8::Object>();
  Nan::Set(statusObj, Nan::New("ping").ToLocalChecked(),
           Nan::New(quickStatus.m_nPing));
  Nan::Set(statusObj, Nan::New("connectionQualityLocal").ToLocalChecked(),
           Nan::New(quickStatus.m_flConnectionQualityLocal));
  Nan::Set(statusObj, Nan::New("connectionQualityRemote").ToLocalChecked(),
           Nan::New(quickStatus.m_flConnectionQualityRemote));
  Nan::Set(statusObj, Nan::New("outPacketsPerSec").ToLocalChecked(),
           Nan::New(quickStatus.m_flOutPacketsPerSec));
  Nan::Set(statusObj, Nan::New("outBytesPerSec").ToLocalChecked(),
           Nan::New(quickStatus.m_flOutBytesPerSec));
  Nan::Set(statusObj, Nan::New("inPacketsPerSec").ToLocalChecked(),
           Nan::New(quickStatus.m_flInPacketsPerSec));
  Nan::Set(statusObj, Nan::New("inBytesPerSec").ToLocalChecked(),
           Nan::New(quickStatus.m_flInBytesPerSec));
  Nan::Set(resultObj, Nan::New("quickStatus").ToLocalChecked(), statusObj);
  
  info.GetReturnValue().Set(resultObj);
}

void RegisterAPIs(v8::Local<v8::Object> target) {
  SET_FUNCTION("sendMessageToUser", SendMessageToUser);
  SET_FUNCTION("receiveMessagesOnChannel", ReceiveMessagesOnChannel);
  SET_FUNCTION("acceptSessionWithUser", AcceptSessionWithUser);
  SET_FUNCTION("closeSessionWithUser", CloseSessionWithUser);
  SET_FUNCTION("closeChannelWithUser", CloseChannelWithUser);
  SET_FUNCTION("getSessionConnectionInfo", GetSessionConnectionInfo);
}

SteamAPIRegistry::Add X(RegisterAPIs);

}  // namespace
}  // namespace api
}  // namespace greenworks
