// Copyright (c) 2024 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <memory>

#include "nan.h"
#include "steam/steam_api.h"
#include "steam/isteamnetworkingutils.h"
#include "v8.h"

#include "greenworks_utils.h"
#include "steam_api_registry.h"

namespace greenworks {
namespace api {
namespace {

// Time

NAN_METHOD(GetLocalTimestamp) {
  Nan::HandleScope scope;
  SteamNetworkingMicroseconds timestamp = 
      SteamNetworkingUtils()->GetLocalTimestamp();
  // Convert to milliseconds for JavaScript
  double timestampMs = static_cast<double>(timestamp) / 1000.0;
  info.GetReturnValue().Set(Nan::New(timestampMs));
}

// Configuration

NAN_METHOD(SetGlobalConfigValueInt32) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsInt32() || !info[1]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: config value type and int32 value required");
  }
  
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  int32 val = info[1]->Int32Value(Nan::GetCurrentContext()).FromJust();
  
  bool result = SteamNetworkingUtils()->SetGlobalConfigValueInt32(eValue, val);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(SetGlobalConfigValueFloat) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsInt32() || !info[1]->IsNumber()) {
    THROW_BAD_ARGS("Bad arguments: config value type and float value required");
  }
  
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  float val = static_cast<float>(
      info[1]->NumberValue(Nan::GetCurrentContext()).FromJust());
  
  bool result = SteamNetworkingUtils()->SetGlobalConfigValueFloat(eValue, val);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(SetGlobalConfigValueString) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsInt32() || !info[1]->IsString()) {
    THROW_BAD_ARGS("Bad arguments: config value type and string value required");
  }
  
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  std::string val_str(*(Nan::Utf8String(info[1])));
  
  bool result = SteamNetworkingUtils()->SetGlobalConfigValueString(
      eValue, val_str.c_str());
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(SetConnectionConfigValueInt32) {
  Nan::HandleScope scope;
  if (info.Length() < 3 || !info[0]->IsUint32() || 
      !info[1]->IsInt32() || !info[2]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: connection handle, config value type, and int32 value required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[1]->Int32Value(Nan::GetCurrentContext()).FromJust());
  int32 val = info[2]->Int32Value(Nan::GetCurrentContext()).FromJust();
  
  bool result = SteamNetworkingUtils()->SetConnectionConfigValueInt32(
      hConn, eValue, val);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(SetConnectionConfigValueFloat) {
  Nan::HandleScope scope;
  if (info.Length() < 3 || !info[0]->IsUint32() || 
      !info[1]->IsInt32() || !info[2]->IsNumber()) {
    THROW_BAD_ARGS("Bad arguments: connection handle, config value type, and float value required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[1]->Int32Value(Nan::GetCurrentContext()).FromJust());
  float val = static_cast<float>(
      info[2]->NumberValue(Nan::GetCurrentContext()).FromJust());
  
  bool result = SteamNetworkingUtils()->SetConnectionConfigValueFloat(
      hConn, eValue, val);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(SetConnectionConfigValueString) {
  Nan::HandleScope scope;
  if (info.Length() < 3 || !info[0]->IsUint32() || 
      !info[1]->IsInt32() || !info[2]->IsString()) {
    THROW_BAD_ARGS("Bad arguments: connection handle, config value type, and string value required");
  }
  
  HSteamNetConnection hConn = static_cast<HSteamNetConnection>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  ESteamNetworkingConfigValue eValue = static_cast<ESteamNetworkingConfigValue>(
      info[1]->Int32Value(Nan::GetCurrentContext()).FromJust());
  std::string val_str(*(Nan::Utf8String(info[2])));
  
  bool result = SteamNetworkingUtils()->SetConnectionConfigValueString(
      hConn, eValue, val_str.c_str());
  info.GetReturnValue().Set(Nan::New(result));
}

// Debug output

NAN_METHOD(SetDebugOutputFunction) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments: debug output type required");
  }
  
  ESteamNetworkingSocketsDebugOutputType eDetailLevel = 
      static_cast<ESteamNetworkingSocketsDebugOutputType>(
          info[0]->Int32Value(Nan::GetCurrentContext()).FromJust());
  
  // For now, use the default debug output function
  SteamNetworkingUtils()->SetDebugOutputFunction(
      eDetailLevel, nullptr);
  
  info.GetReturnValue().Set(Nan::New(true));
}

// Fake IP (for P2P)

NAN_METHOD(IsFakeIPv4) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: IP address (uint32) required");
  }
  
  uint32 nIPv4 = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  bool result = SteamNetworkingUtils()->IsFakeIPv4(nIPv4);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(GetIPv4FakeIPType) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: IP address (uint32) required");
  }
  
  uint32 nIPv4 = info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust();
  ESteamNetworkingFakeIPType result = 
      SteamNetworkingUtils()->GetIPv4FakeIPType(nIPv4);
  info.GetReturnValue().Set(Nan::New(result));
}

// Get real identity associated with a fake IP

NAN_METHOD(GetRealIdentityForFakeIP) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: IP address string and port required");
  }
  
  std::string ip_str(*(Nan::Utf8String(info[0])));
  uint16 port = static_cast<uint16>(
      info[1]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  SteamNetworkingIPAddr fakeIP;
  if (!fakeIP.ParseString(ip_str.c_str())) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  fakeIP.m_port = port;
  
  SteamNetworkingIdentity pOutRealIdentity;
  EResult result = SteamNetworkingUtils()->GetRealIdentityForFakeIP(
      fakeIP, &pOutRealIdentity);
  
  if (result != k_EResultOK) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  CSteamID steamID;
  if (pOutRealIdentity.GetSteamID(&steamID)) {
    info.GetReturnValue().Set(
        Nan::New(std::to_string(steamID.ConvertToUint64())).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

// Ping location / measurement

NAN_METHOD(CheckPingDataUpToDate) {
  Nan::HandleScope scope;
  
  float maxAgeSeconds = 60.0f * 5;  // Default: 5 minutes
  if (info.Length() >= 1 && info[0]->IsNumber()) {
    maxAgeSeconds = static_cast<float>(
        info[0]->NumberValue(Nan::GetCurrentContext()).FromJust());
  }
  
  bool result = SteamNetworkingUtils()->CheckPingDataUpToDate(maxAgeSeconds);
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(GetPingToDataCenter) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: PoP ID required");
  }
  
  SteamNetworkingPOPID popID = static_cast<SteamNetworkingPOPID>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  SteamNetworkingPOPID pViaRelayPoP;
  int ping = SteamNetworkingUtils()->GetPingToDataCenter(popID, &pViaRelayPoP);
  
  if (ping < 0) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  
  v8::Local<v8::Object> resultObj = Nan::New<v8::Object>();
  Nan::Set(resultObj, Nan::New("ping").ToLocalChecked(), Nan::New(ping));
  Nan::Set(resultObj, Nan::New("viaRelayPoP").ToLocalChecked(), 
           Nan::New(static_cast<uint32_t>(pViaRelayPoP)));
  
  info.GetReturnValue().Set(resultObj);
}

NAN_METHOD(GetDirectPingToPOP) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsUint32()) {
    THROW_BAD_ARGS("Bad arguments: PoP ID required");
  }
  
  SteamNetworkingPOPID popID = static_cast<SteamNetworkingPOPID>(
      info[0]->Uint32Value(Nan::GetCurrentContext()).FromJust());
  
  int ping = SteamNetworkingUtils()->GetDirectPingToPOP(popID);
  
  if (ping < 0) {
    info.GetReturnValue().Set(Nan::Null());
  } else {
    info.GetReturnValue().Set(Nan::New(ping));
  }
}

NAN_METHOD(GetPOPCount) {
  Nan::HandleScope scope;
  int count = SteamNetworkingUtils()->GetPOPCount();
  info.GetReturnValue().Set(Nan::New(count));
}

NAN_METHOD(GetPOPList) {
  Nan::HandleScope scope;
  
  int count = SteamNetworkingUtils()->GetPOPCount();
  if (count <= 0) {
    info.GetReturnValue().Set(Nan::New<v8::Array>(0));
    return;
  }
  
  SteamNetworkingPOPID *list = new SteamNetworkingPOPID[count];
  int actualCount = SteamNetworkingUtils()->GetPOPList(list, count);
  
  v8::Local<v8::Array> result = Nan::New<v8::Array>(actualCount);
  for (int i = 0; i < actualCount; i++) {
    Nan::Set(result, i, Nan::New(static_cast<uint32_t>(list[i])));
  }
  
  delete[] list;
  info.GetReturnValue().Set(result);
}

void RegisterAPIs(v8::Local<v8::Object> target) {
  // Time
  SET_FUNCTION("getLocalTimestamp", GetLocalTimestamp);
  
  // Configuration - Global
  SET_FUNCTION("setGlobalConfigValueInt32", SetGlobalConfigValueInt32);
  SET_FUNCTION("setGlobalConfigValueFloat", SetGlobalConfigValueFloat);
  SET_FUNCTION("setGlobalConfigValueString", SetGlobalConfigValueString);
  
  // Configuration - Per Connection
  SET_FUNCTION("setConnectionConfigValueInt32", SetConnectionConfigValueInt32);
  SET_FUNCTION("setConnectionConfigValueFloat", SetConnectionConfigValueFloat);
  SET_FUNCTION("setConnectionConfigValueString", SetConnectionConfigValueString);
  
  // Debug
  SET_FUNCTION("setDebugOutputFunction", SetDebugOutputFunction);
  
  // Fake IP
  SET_FUNCTION("isFakeIPv4", IsFakeIPv4);
  SET_FUNCTION("getIPv4FakeIPType", GetIPv4FakeIPType);
  SET_FUNCTION("getRealIdentityForFakeIP", GetRealIdentityForFakeIP);
  
  // Ping/Location
  SET_FUNCTION("checkPingDataUpToDate", CheckPingDataUpToDate);
  SET_FUNCTION("getPingToDataCenter", GetPingToDataCenter);
  SET_FUNCTION("getDirectPingToPOP", GetDirectPingToPOP);
  SET_FUNCTION("getPOPCount", GetPOPCount);
  SET_FUNCTION("getPOPList", GetPOPList);
  
  // Constants - Debug Output Types
  v8::Local<v8::Object> debugOutputTypes = Nan::New<v8::Object>();
  SET_TYPE(debugOutputTypes, "None", k_ESteamNetworkingSocketsDebugOutputType_None);
  SET_TYPE(debugOutputTypes, "Bug", k_ESteamNetworkingSocketsDebugOutputType_Bug);
  SET_TYPE(debugOutputTypes, "Error", k_ESteamNetworkingSocketsDebugOutputType_Error);
  SET_TYPE(debugOutputTypes, "Important", k_ESteamNetworkingSocketsDebugOutputType_Important);
  SET_TYPE(debugOutputTypes, "Warning", k_ESteamNetworkingSocketsDebugOutputType_Warning);
  SET_TYPE(debugOutputTypes, "Msg", k_ESteamNetworkingSocketsDebugOutputType_Msg);
  SET_TYPE(debugOutputTypes, "Verbose", k_ESteamNetworkingSocketsDebugOutputType_Verbose);
  SET_TYPE(debugOutputTypes, "Debug", k_ESteamNetworkingSocketsDebugOutputType_Debug);
  SET_TYPE(debugOutputTypes, "Everything", k_ESteamNetworkingSocketsDebugOutputType_Everything);
  Nan::Set(target, Nan::New("NetworkingDebugOutputType").ToLocalChecked(), debugOutputTypes);
  
  // Constants - Config Values (commonly used ones)
  v8::Local<v8::Object> configValues = Nan::New<v8::Object>();
  SET_TYPE(configValues, "TimeoutInitial", k_ESteamNetworkingConfig_TimeoutInitial);
  SET_TYPE(configValues, "TimeoutConnected", k_ESteamNetworkingConfig_TimeoutConnected);
  SET_TYPE(configValues, "SendBufferSize", k_ESteamNetworkingConfig_SendBufferSize);
  SET_TYPE(configValues, "SendRateMin", k_ESteamNetworkingConfig_SendRateMin);
  SET_TYPE(configValues, "SendRateMax", k_ESteamNetworkingConfig_SendRateMax);
  SET_TYPE(configValues, "NagleTime", k_ESteamNetworkingConfig_NagleTime);
  SET_TYPE(configValues, "IP_AllowWithoutAuth", k_ESteamNetworkingConfig_IP_AllowWithoutAuth);
  SET_TYPE(configValues, "MTU_PacketSize", k_ESteamNetworkingConfig_MTU_PacketSize);
  SET_TYPE(configValues, "Unencrypted", k_ESteamNetworkingConfig_Unencrypted);
  SET_TYPE(configValues, "SymmetricConnect", k_ESteamNetworkingConfig_SymmetricConnect);
  SET_TYPE(configValues, "LocalVirtualPort", k_ESteamNetworkingConfig_LocalVirtualPort);
  Nan::Set(target, Nan::New("NetworkingConfigValue").ToLocalChecked(), configValues);
}

SteamAPIRegistry::Add X(RegisterAPIs);

}  // namespace
}  // namespace api
}  // namespace greenworks
