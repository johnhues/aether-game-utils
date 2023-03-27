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
#include "aether.h"

//------------------------------------------------------------------------------
// AetherUuid class
//------------------------------------------------------------------------------
struct AetherUuid
{
  AetherUuid() = default;
  AetherUuid( const char* str );
  
  bool operator==( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) == 0; }
  bool operator!=( const AetherUuid& other ) const { return memcmp( uuid, other.uuid, 16 ) != 0; }

  static AetherUuid Generate();
  static AetherUuid Zero();

  void ToString( char* str, uint32_t max ) const;

  uint8_t uuid[ 16 ];
};
namespace ae { template <> inline uint32_t GetHash( AetherUuid e )
{
  return ae::Hash().HashData( e.uuid, sizeof(e.uuid) ).Get();
} }

std::ostream& operator<<( std::ostream& os, const AetherUuid& uuid );

inline void Serialize( ae::BinaryStream* stream, AetherUuid* uuid )
{
	stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

inline void Serialize( ae::BinaryStream* stream, const AetherUuid* uuid )
{
  stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

//------------------------------------------------------------------------------
// Client / server constants
//------------------------------------------------------------------------------
typedef uint32_t NetInstId;
typedef uint16_t AetherMsgId;
const AetherMsgId kInvalidAetherMsgId = 0;
const AetherMsgId kSysMsgMask = 1 << ( sizeof(AetherMsgId) * 8 - 1 );
const AetherMsgId kSysMsgServerConnect = 1 | kSysMsgMask;
const AetherMsgId kSysMsgServerDisconnect = 2 | kSysMsgMask;
const AetherMsgId kSysMsgPlayerConnect = 3 | kSysMsgMask;
const AetherMsgId kSysMsgPlayerDisconnect = 4 | kSysMsgMask;

#if _AE_EMSCRIPTEN_
const uint32_t kEmMaxMessageSize = 1500;
#endif

struct AetherAddress
{
  bool IsLocalhost() const;
  char host[ 32 ];
  uint16_t port;
};

struct ReceiveInfo
{
  AetherMsgId msgId = kInvalidAetherMsgId;
  ae::Array< uint8_t > data = AE_ALLOC_TAG_NET;
};

struct SendInfo
{
  AetherMsgId msgId = kInvalidAetherMsgId;
  const void* data = nullptr;
  int32_t length = 0;
  bool reliable = false;
};

//------------------------------------------------------------------------------
// Aether Player
//------------------------------------------------------------------------------
struct AetherPlayer
{
  AetherUuid uuid = AetherUuid::Zero();
  void* userData = nullptr;
  bool alive = false;

  ae::Str32 pendingLevel = "";
  ae::Str32 pendingLink = "";
  bool hasPendingLevelChange = false;
};

//------------------------------------------------------------------------------
// Aether Client
//------------------------------------------------------------------------------
struct AetherClient
{
  // @TODO: Shouldn't expose players directly, since they can disconnect and become invalid
  AetherPlayer* localPlayer;
  ae::Array< AetherPlayer* > allPlayers = AE_ALLOC_TAG_NET;
  
  AetherAddress serverAddress;
  bool isConnected;

  bool IsConnected() { return isConnected; }
  bool IsConnecting() { return m_isConnecting; }

  bool m_isConnecting;
};

AetherClient* AetherClient_New( AetherUuid uuid, const char* ip, uint16_t port );
void AetherClient_Delete( AetherClient* );

void AetherClient_Connect( AetherClient* _ac );
void AetherClient_Disconnect( AetherClient* _ac );
bool AetherClient_Receive( AetherClient*, ReceiveInfo* infoOut );
void AetherClient_SendAll( AetherClient* );
void AetherClient_QueueSend( AetherClient* ac, AetherMsgId msgId, bool reliable, const void* data, uint32_t length );

//------------------------------------------------------------------------------
// Aether Server
//------------------------------------------------------------------------------
struct ServerReceiveInfo
{
  AetherMsgId msgId = kInvalidAetherMsgId;
  ae::Array< uint8_t > data = AE_ALLOC_TAG_NET;
  AetherPlayer* player = nullptr;
};

struct ServerSendInfo
{
  AetherMsgId msgId = kInvalidAetherMsgId;
  bool reliable = true;
  
  AetherPlayer* player = nullptr;
  void* group = nullptr;
  AetherPlayer* playerFilter = nullptr;
  void* groupFilter = nullptr;

  int32_t length = 0;
  const void* data = nullptr;
};

struct AetherServer
{
  AetherPlayer** allPlayers;
  int32_t playerCount;
};

AetherServer* AetherServer_New( uint16_t port, uint16_t webPort, uint32_t maxPlayers );
void AetherServer_Delete( AetherServer* );
void AetherServer_Update( AetherServer* );

void AetherServer_Disconnect( AetherServer*, AetherPlayer* player );

bool AetherServer_Receive( AetherServer*, ServerReceiveInfo* infoOut );
void AetherServer_SendAll( AetherServer* );

AetherPlayer* AetherServer_GetPlayerByNetInstId( AetherServer*, NetInstId id );
uint32_t AetherServer_GetPlayerByUserData( AetherServer* as, const void* userData, AetherPlayer* (&playersOut)[ 32 ] );

void AetherServer_QueueSend( AetherServer* as, const ServerSendInfo* info );
void AetherServer_QueueBroadcast( AetherServer* as, AetherMsgId msgId, bool reliable, const void* data, uint32_t length );
void AetherServer_QueueSendToPlayer( AetherServer* as, AetherPlayer* player, AetherMsgId msgId, bool reliable, const void* data, uint32_t length );
void AetherServer_QueueSendToGroup( AetherServer* as, void* group, AetherMsgId msgId, bool reliable, const void* data, uint32_t length );

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
