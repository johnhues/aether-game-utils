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

inline void Serialize( aeBinaryStream* stream, AetherUuid* uuid )
{
	stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

inline void Serialize( aeBinaryStream* stream, const AetherUuid* uuid )
{
  stream->SerializeRaw( uuid->uuid, sizeof( uuid->uuid ) );
}

//------------------------------------------------------------------------------
// aeNetData class
//------------------------------------------------------------------------------
class aeNetData
{
public:
  struct Msg
  {
    const uint8_t* data;
    uint32_t length;
  };

  // General
  bool IsAuthority() const { return m_local; }
	
  // Server
  void SetSyncData( const void* data, uint32_t length );
  void SendMessage( const void* data, uint32_t length );

  // Client
  const uint8_t* GetInitData() const;
  uint32_t InitDataLength() const;

  const uint8_t* GetSyncData() const;
  uint32_t SyncDataLength() const;
  void ClearSyncData(); // Call to clear local received data. Check ( Length() > 0 ) for new data.

  bool PumpMessages( Msg* msgOut );

  bool IsPendingDelete() const;

private:
  // @TODO: Expose internals in a safer way
  friend class aeNetReplicaClient;
  friend class aeNetReplicaServer;
  friend class aeNetReplicaDB;

  void m_SetLocal() { m_local = true; }
  void m_SetClientData( const uint8_t* data, uint32_t length );
  void m_ReceiveMessages( const uint8_t* data, uint32_t length );
  void FlagForDeletion() { m_isPendingDelete = true; }
  
  void m_UpdateHash();
  bool m_Changed() const { return m_hash != m_prevHash; }

  bool m_local = false;
  aeArray< uint8_t > m_initData;
  aeArray< uint8_t > m_data;

  aeArray< uint8_t > m_messageDataOut;
  aeArray< uint8_t > m_messageDataIn;
  uint32_t m_messageDataInOffset = 0;

  uint32_t m_hash = 0;
  uint32_t m_prevHash = 0;
  bool m_isPendingDelete = false;
public:
  // Internal
  AE_REFABLE( aeNetData );
};

//------------------------------------------------------------------------------
// aeNetReplicaClient class
//------------------------------------------------------------------------------
class aeNetReplicaClient
{
public:
  // The following sequence should be performed each frame
  void ReceiveData( const uint8_t* data, uint32_t length ); // 1) Handle raw data from server (call once when new data arrives)
  aeRef< aeNetData > PumpCreated(); // 2) Get new objects (call this repeatedly until no new NetDatas are returned)
  // 3) Handle new sync data with aeNetData::GetSyncData() and process incoming messages with aeNetData::PumpMessages()
  void DestroyPending(); // 4) Destroy all objects flagged for destruction (call once)

private:
  void m_CreateNetData( aeBinaryStream* rStream );
  aeMap< aeId< aeNetData >, aeNetData* > m_netDatas;
  aeMap< uint32_t, aeId< aeNetData > > m_remoteToLocalIdMap;
  aeArray< aeRef< aeNetData > > m_created;
  aeArray< aeRef< aeNetData > > m_destroyed;
};

//------------------------------------------------------------------------------
// aeNetReplicaServer class
//------------------------------------------------------------------------------
class aeNetReplicaServer
{
public:
  const uint8_t* GetSendData() const; // Call aeNetReplicaDB::UpdateSendData() first
  uint32_t GetSendLength() const; // Call aeNetReplicaDB::UpdateSendData() first

public:
  void m_UpdateSendData();

  bool m_first = true;
  class aeNetReplicaDB* m_replicaDB = nullptr;
  bool m_pendingClear = false;
  aeArray< uint8_t > m_sendData;
  // Internal
  enum class EventType : uint8_t
  {
    Connect,
    Create,
    Destroy,
    Update,
    Messages
  };
};

//------------------------------------------------------------------------------
// aeNetReplicaDB class
//------------------------------------------------------------------------------
class aeNetReplicaDB
{
public:
  aeNetData* CreateNetData( const uint8_t* initData, uint32_t initDataLength );
  void DestroyNetData( aeNetData* netData );

  aeNetReplicaServer* CreateServer();
  void DestroyServer( aeNetReplicaServer* server );

  void UpdateSendData(); // Call each frame before aeNetReplicaServer::GetSendData()

private:
  aeMap< aeId< aeNetData >, aeNetData* > m_netDatas;
  aeArray< aeNetReplicaServer* > m_servers;
public:
  // Internal
  aeNetData* GetNetData( uint32_t index ) { return m_netDatas.GetValue( index ); }
  uint32_t GetNetDataCount() const { return m_netDatas.Length(); }
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
typedef uint16_t AetherMsgId;
const AetherMsgId kInvalidAetherMsgId = 0;
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
  AetherMsgId msgId = kInvalidAetherMsgId;
  aeArray< uint8_t > data;
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
  NetInstId netId = 0;
  void* userData = nullptr;
  bool alive = false;

  aeStr32 pendingLevel = "";
  aeStr32 pendingLink = "";
  bool hasPendingLevelChange = false;
};

//------------------------------------------------------------------------------
// Aether Client
//------------------------------------------------------------------------------
struct AetherClient
{
  AetherPlayer* localPlayer;
  aeArray< AetherPlayer* > allPlayers; // @TODO: Shouldn't expose players directly, since they can disconnect and become invalid
  
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
void AetherClient_SendAll( AetherClient* );
void AetherClient_QueueSend( AetherClient* ac, AetherMsgId msgId, bool reliable, const void* data, uint32_t length );

//------------------------------------------------------------------------------
// Aether Server
//------------------------------------------------------------------------------
struct ServerReceiveInfo
{
  AetherMsgId msgId = kInvalidAetherMsgId;
  aeArray< uint8_t > data;
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
