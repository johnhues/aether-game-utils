//------------------------------------------------------------------------------
// aeNet.h
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
#ifndef AENET_H
#define AENET_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aePlatform.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
typedef uint32_t NetInstId;
const uint32_t kMaxSyncData = 128;

//------------------------------------------------------------------------------
// Aether Common
//------------------------------------------------------------------------------
struct AetherUuid
{
  AetherUuid() {}

  AetherUuid( const char* str )
  {
// #define _aescn8  "%2" SCNx8
//     sscanf( str, _aescn8 _aescn8 _aescn8 _aescn8 "-"
//       _aescn8 _aescn8 "-" _aescn8 _aescn8 "-" _aescn8 _aescn8 "-"
//       _aescn8 _aescn8 _aescn8 _aescn8 _aescn8 _aescn8,
//       &uuid[0], &uuid[1], &uuid[2], &uuid[3],
//       &uuid[4], &uuid[5], &uuid[6], &uuid[7], &uuid[8], &uuid[9],
//       &uuid[10], &uuid[11], &uuid[12], &uuid[13], &uuid[14], &uuid[15] );
// #undef _aescn8
  }
  
  bool operator==( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) == 0; }
  bool operator!=( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) != 0; }

  static AetherUuid Generate();

  static AetherUuid Zero()
  {
    AetherUuid result;
    memset( &result, 0, sizeof(result) );
    return result;
  }

  void ToString( char* str, uint32_t max ) const
  {
    AE_ASSERT( max >= 37 );
    sprintf( str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
      uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
      uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15] );
  }

  uint8_t uuid[ 16 ];
};

const uint32_t kMaxMessageSize = 1024;
typedef uint16_t AetherMsgId;
const AetherMsgId kSysMsgMask = 1 << ( sizeof(AetherMsgId) * 8 - 1 );
const AetherMsgId kSysMsgServerConnect = 1 | kSysMsgMask;
const AetherMsgId kSysMsgServerDisconnect = 2 | kSysMsgMask;
const AetherMsgId kSysMsgPlayerConnect = 3 | kSysMsgMask;
const AetherMsgId kSysMsgPlayerDisconnect = 4 | kSysMsgMask;

struct AetherAddress
{
  char host[ 32 ];
  uint16_t port;
};

struct ReceiveInfo
{
  AetherMsgId msgId;
  uint8_t data[ kMaxMessageSize ];
  int32_t length;
};

struct SendInfo
{
  AetherMsgId msgId;
  uint8_t data[ kMaxMessageSize ];
  int32_t length;
  bool reliable;
};

//------------------------------------------------------------------------------
// Aether Player
//------------------------------------------------------------------------------
struct AetherPlayer
{
  AetherUuid uuid;
  NetInstId netId;
  void* userData;
  int luaRef;
  bool alive;

  aeStr32 pendingLevel;
  aeStr32 pendingLink;
  bool hasPendingLevelChange;
};

//------------------------------------------------------------------------------
// Aether Client
//------------------------------------------------------------------------------
struct AetherClient
{
  AetherPlayer* localPlayer;
  AetherPlayer* allPlayers;
  int32_t playerCount;
  
  AetherAddress serverAddress;
  bool isConnected;

  bool IsConnected() { return isConnected; }
  bool IsConnecting() { return m_isConnecting; }

  bool m_isConnecting;
};

AetherClient* AetherClient_New( AetherUuid uuid, const char* ip, uint16_t port );
void AetherClient_Delete( AetherClient* );

void AetherClient_Connect( AetherClient* _ac );
bool AetherClient_Receive( AetherClient*, ReceiveInfo* infoOut );
void AetherClient_QueueSend( AetherClient*, const SendInfo* infoIn );
void AetherClient_SendAll( AetherClient* );

template<typename T>
void AetherClient_QueueSend( AetherClient* ac, AetherMsgId msgId, bool reliable, const T& msg )
{
  SendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = sizeof(msg);
  memcpy( info.data, &msg, info.length );
  AetherClient_QueueSend( ac, &info );
}

//------------------------------------------------------------------------------
// Aether Server
//------------------------------------------------------------------------------
struct ServerReceiveInfo
{
  AetherMsgId msgId;
  uint8_t data[ kMaxMessageSize ];
  int32_t length;
  AetherPlayer* player;
};

struct ServerSendInfo
{
  AetherMsgId msgId = ~0;
  bool reliable = true;
  
  AetherPlayer* player = nullptr;
  void* group = nullptr;
  AetherPlayer* playerFilter = nullptr;
  void* groupFilter = nullptr;

  int32_t length = 0;
  uint8_t data[ kMaxMessageSize ];
};

struct AetherServer
{
  AetherPlayer** allPlayers;
  int32_t playerCount;
};

AetherServer* AetherServer_New( uint16_t port, uint16_t webPort );
void AetherServer_Delete( AetherServer* );
void AetherServer_Update( AetherServer* );

bool AetherServer_Receive( AetherServer*, ServerReceiveInfo* infoOut );
void AetherServer_QueueSendInfo( AetherServer*, const ServerSendInfo* infoIn );
void AetherServer_SendAll( AetherServer* );

AetherPlayer* AetherServer_GetPlayerByNetInstId( AetherServer*, NetInstId id );
uint32_t AetherServer_GetPlayerByUserData( AetherServer* as, const void* userData, AetherPlayer* (&playersOut)[ 32 ] );

template<typename T>
void AetherServer_QueueBroadcast( AetherServer* as, AetherMsgId msgId, bool reliable, const T& msg )
{
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = sizeof(msg);
  memcpy( info.data, &msg, info.length );
  info.player = nullptr;
  info.group = nullptr;
  AetherServer_QueueSendInfo( as, &info );
}

template<typename T>
void AetherServer_QueueSendToPlayer( AetherServer* as, AetherMsgId msgId, bool reliable, const T& msg, AetherPlayer* player )
{
  AE_ASSERT( player );
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = sizeof(msg);
  memcpy( info.data, &msg, info.length );
  info.player = player;
  info.group = nullptr;
  AetherServer_QueueSendInfo( as, &info );
}

template<typename T>
void AetherServer_QueueSendToGroup( AetherServer* as, AetherMsgId msgId, bool reliable, const T& msg, void* group )
{
  AE_ASSERT( group );
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = sizeof(msg);
  memcpy( info.data, &msg, info.length );
  info.player = nullptr;
  info.group = group;
  AetherServer_QueueSendInfo( as, &info );
}

//------------------------------------------------------------------------------
// Aether Internal
//------------------------------------------------------------------------------
struct AetherServerHeader
{
  AetherMsgId msgId;
};

struct AetherClientHeader
{
  AetherMsgId msgId;
  AetherUuid uuid;
};

struct AetherMsgConnect
{
  AetherUuid uuid;
};

const int32_t kNetChannelReliable = 0;
const int32_t kNetChannelUnreliable = 1;

#endif
