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

struct NetId
{
  NetId() = default;
  NetId( const NetId& ) = default;
  explicit NetId( uint32_t id ) : m_id( id ) {}
  bool operator==( const NetId& o ) const { return o.m_id == m_id; }
  bool operator!=( const NetId& o ) const { return o.m_id != m_id; }
  explicit operator bool () const { return m_id != 0; }
  void Serialize( aeBinaryStream* s ) { s->SerializeUint32( m_id ); }
  void Serialize( aeBinaryStream* s ) const { s->SerializeUint32( m_id ); }
private:
  uint32_t m_id = 0;
};
struct RemoteId
{
  RemoteId() = default;
  RemoteId( const RemoteId& ) = default;
  explicit RemoteId( uint32_t id ) : m_id( id ) {}
  bool operator==( const RemoteId& o ) const { return o.m_id == m_id; }
  bool operator!=( const RemoteId& o ) const { return o.m_id != m_id; }
  explicit operator bool () const { return m_id != 0; }
  void Serialize( aeBinaryStream* s ) { s->SerializeUint32( m_id ); }
  void Serialize( aeBinaryStream* s ) const { s->SerializeUint32( m_id ); }
private:
  uint32_t m_id = 0;
};

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

  NetId _netId;

  //------------------------------------------------------------------------------
  // General
  //------------------------------------------------------------------------------
  bool IsAuthority() const { return m_local; }
	
  //------------------------------------------------------------------------------
  // Server
  // @NOTE: All server data will be sent with the next aeNetReplicaDB::UpdateSendData()
  //------------------------------------------------------------------------------
  // True until SetInitData() is called
  bool IsPendingInit() const;
  // Call once after aeNetReplicaDB::CreateNetData(), will trigger Create event
  // on clients
  void SetInitData( const void* initData, uint32_t initDataLength );
  // Call SetSyncData each frame to update that state of the clients NetData.
  // Only the most recent data is sent. Data is only sent when changed.
  void SetSyncData( const void* data, uint32_t length );
  // Call as many times as necessary each tick
  void SendMessage( const void* data, uint32_t length );

  //------------------------------------------------------------------------------
  // Client
  //------------------------------------------------------------------------------
  // Use GetInitData() after receiving a new NetData from aeNetReplicaClient::
  // PumpCreate() to construct the object
  const uint8_t* GetInitData() const;
  uint32_t InitDataLength() const;

  // Only the latest sync data is ever available, so there's no need to read this
  // data as if it was a stream.
  const uint8_t* GetSyncData() const;
  // Check for new data from server
  uint32_t SyncDataLength() const;
  // (Optional) Call to clear SyncDataLength() until new data is received
  void ClearSyncData();

  // Get messages sent from the server. Call repeatedly until false is returned
  bool PumpMessages( Msg* msgOut );

  // True once the NetData has been deleted on the server.
  // Call aeNetReplicaClient::Destroy() when you're done with it.
  bool IsPendingDestroy() const;

  //------------------------------------------------------------------------------
  // Internal
  //------------------------------------------------------------------------------
private:
  // @TODO: Expose internals in a safer way
  friend class aeNetReplicaClient;
  friend class aeNetReplicaServer;
  friend class aeNetReplicaDB;

  void m_SetLocal() { m_local = true; }
  void m_SetClientData( const uint8_t* data, uint32_t length );
  void m_ReceiveMessages( const uint8_t* data, uint32_t length );
  void m_FlagForDestruction() { m_isPendingDestroy = true; }
  void m_UpdateHash();
  bool m_Changed() const { return m_hash != m_prevHash; }

  bool m_local = false;
  ae::Array< uint8_t > m_initData = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_data = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_messageDataOut = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_messageDataIn = AE_ALLOC_TAG_NET;
  uint32_t m_messageDataInOffset = 0;
  uint32_t m_hash = 0;
  uint32_t m_prevHash = 0;
  bool m_isPendingInit = true;
  bool m_isPendingDestroy = false;
};

//------------------------------------------------------------------------------
// aeNetReplicaClient class
//------------------------------------------------------------------------------
class aeNetReplicaClient
{
public:
  // The following sequence should be performed each frame
  void ReceiveData( const uint8_t* data, uint32_t length ); // 1) Handle raw data from server (call once when new data arrives)
  aeNetData* PumpCreate(); // 2) Get new objects (call this repeatedly until no new NetDatas are returned)
  // 3) Handle new sync data with aeNetData::GetSyncData() and process incoming messages with aeNetData::PumpMessages()
  void Destroy( aeNetData* pendingDestroy ); // 4) Call this on aeNetDatas once aeNetData::IsPendingDestroy() returns true
  
  NetId GetLocalId( RemoteId remoteId ) const { return m_remoteToLocalIdMap.Get( remoteId, {} ); }
  RemoteId GetRemoteId( NetId localId ) const { return m_localToRemoteIdMap.Get( localId, {} ); }

private:
  void m_CreateNetData( aeBinaryStream* rStream );
  ae::Map< NetId, aeNetData* > m_netDatas = AE_ALLOC_TAG_NET;
  ae::Map< RemoteId, NetId > m_remoteToLocalIdMap = AE_ALLOC_TAG_NET;
  ae::Map< NetId, RemoteId > m_localToRemoteIdMap = AE_ALLOC_TAG_NET;
  ae::Array< aeNetData* > m_created = AE_ALLOC_TAG_NET;
  ae::Array< aeNetData* > m_destroyed = AE_ALLOC_TAG_NET;
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
  ae::Array< uint8_t > m_sendData = AE_ALLOC_TAG_NET;
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
  // Create a server authoritative NetData which will be replicated to clients through NetReplicaServer/NetReplicaClient
  aeNetData* CreateNetData();
  void DestroyNetData( aeNetData* netData );

  // Allocate one server per client connection
  aeNetReplicaServer* CreateServer();
  void DestroyServer( aeNetReplicaServer* server );

  // Call each frame before aeNetReplicaServer::GetSendData()
  void UpdateSendData();

private:
  uint32_t m_lastNetId = 0;
  ae::Array< aeNetData* > m_pendingCreate = AE_ALLOC_TAG_NET;
  ae::Map< NetId, aeNetData* > m_netDatas = AE_ALLOC_TAG_NET;
  ae::Array< aeNetReplicaServer* > m_servers = AE_ALLOC_TAG_NET;
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
//   TypeId type;
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
  NetInstId netId = 0;
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
