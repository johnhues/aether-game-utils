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
#include "aeBinaryStream.h"
#include "aeMap.h"
#include "aeMeta.h"
#include "aePlatform.h"
#include "aeRef.h"
#include "aeString.h"

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

std::ostream& operator<<( std::ostream& os, const AetherUuid& uuid );

//------------------------------------------------------------------------------
// aeRpc class
//------------------------------------------------------------------------------
class aeRpc : public aeInheritor< aeObject, aeRpc >
{
public:
  aeRpc() : aetherPlayer( nullptr ), userdata( nullptr ) {}
  virtual ~aeRpc() {}

  virtual void Run() {}
  virtual void Serialize( class BinaryStream* stream ) {}

  struct AetherPlayer* aetherPlayer = nullptr;
  void* userdata = nullptr;
};

const uint32_t kMaxNetInst = 256;
const uint32_t kMaxNetMessages = 256;

//------------------------------------------------------------------------------
// aeNetReplication
//------------------------------------------------------------------------------
enum class aeNetReplicaEventType
{
  Create,
  Destroy,
  Update
};

struct aeNetReplicaEvent
{
  aeNetReplicaEventType type;
};

class aeNetData
{
public:
  AE_REFABLE( aeNetData );

  uint8_t* Get() const { return m_data; }
  uint32_t Length() const { return m_length; }

private:
  uint8_t* m_data = nullptr;
  uint32_t m_length = 0;
};

enum class aeNetType
{
  Float,
  Int,
  Uint
};

class aeNetReplicaClient
{
public:
  void Receive( const uint8_t* data, uint32_t length ) {}
  bool Pump( aeNetReplicaEvent* event ) { return false;  }

private:
  aeMap< aeId< aeNetData >, aeNetData* > m_objects;
  aeArray< aeNetReplicaEvent > m_events;
};

class aeNetReplicaServer
{
public:
  void Update();

  const uint8_t* GetSendData() const;
  uint32_t GetSendLength() const;

private:
  aeMap< aeId< aeNetData >, aeNetData* > m_objects;
  aeArray< uint8_t > m_sendData;
};

// //------------------------------------------------------------------------------
// // Net structs
// //------------------------------------------------------------------------------
// struct NetPlayer
// {
//   uint8_t uuid[ 16 ];
// };

// struct NetInstInfo
// {
//   GameObjId gameId;
//   struct Game* game;
//   NetInstId netId;
//   aeMetaTypeId type;
//   bool levelObject;
// };

// struct Net
// {
//   double lastSendTime;
  
//   NetInstInfo netInsts[ kMaxNetInst ];
//   uint32_t netInstCount;

//   MsgObject messages[ kMaxNetMessages ];
//   uint32_t messageCount;
// };

// //------------------------------------------------------------------------------
// // Net Public Functions
// //------------------------------------------------------------------------------
// Net* Net_New();
// void Net_Delete( Net* );
// void Net_UpdateServer( struct Net* net, struct AetherServer* aether, struct Game* games, uint32_t count );
// void Net_UpdateClient( struct Net* net, struct AetherClient* aether, struct Game* game );
// void Net_RemoveObject( GameObject* go );
// GameObject* Net_GetGameObject( Net* net, Game* game, NetInstId id );
// GameObject* Net_GetGameObject( Net* net, Game* game, NetInstInfo info );
// AetherPlayer* Net_GetPlayer( Net* net, AetherServer* aether, NetInstId id );
// bool Net_CanChangeLevel( Net* net, struct AetherServer* aether, struct Player* player, const char* level );
// bool Net_PlayerChangeLevel( Net* net, struct AetherServer* aether, struct Player* player, const char* level, const char* link );

// void Net_SendObjectMessage( struct Net* net, struct AetherServer* aether, void* group, NetInstId id, uint8_t type, const uint8_t* data, uint32_t length );
// uint32_t Net_RecvObjectMessage( struct Net* net, NetInstId id, uint8_t* typeOut, uint8_t* dataOut );

// void Net_AddObject( Net* net, GameObject* go, bool levelObject );

// void Aether_SendRpc( struct AetherClient* aether, class Rpc* rpc );

//------------------------------------------------------------------------------
// Client / server constants
//------------------------------------------------------------------------------
typedef uint32_t NetInstId;
const uint32_t kMaxSyncData = 128;
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
void AetherClient_QueueSend( AetherClient*, const SendInfo* infoIn ); // @TODO: Easy to mess up, should be private? Or SendInfo should have better default values
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

template<typename T>
void AetherClient_QueueSend( AetherClient* ac, AetherMsgId msgId, bool reliable, uint8_t* data, uint32_t length )
{
  SendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = length;
  memcpy( info.data, data, length );
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
void AetherServer_QueueSendInfo( AetherServer*, const ServerSendInfo* infoIn ); // @TODO: Easy to mess up, should be private? Or SendInfo should have better default values
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
