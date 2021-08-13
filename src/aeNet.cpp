//------------------------------------------------------------------------------
// aeNet.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "aeNet.h"

//------------------------------------------------------------------------------
// aeNetData member functions
//------------------------------------------------------------------------------
void aeNetData::SetSyncData( const void* data, uint32_t length )
{
  AE_ASSERT_MSG( IsAuthority(), "Cannot set net data from client. The aeNetReplicaServer has exclusive ownership." );
  m_data.Clear();
  m_data.Append( (const uint8_t*)data, length );
}

void aeNetData::SendMessage( const void* data, uint32_t length )
{
  uint16_t lengthU16 = length;
  m_messageDataOut.Reserve( m_messageDataOut.Length() + sizeof( lengthU16 ) + length );
  m_messageDataOut.Append( (uint8_t*)&lengthU16, sizeof( lengthU16 ) );
  m_messageDataOut.Append( (const uint8_t*)data, length );
}

void aeNetData::SetInitData( const void* initData, uint32_t initDataLength )
{
  m_initData.Clear();
  m_initData.Append( (uint8_t*)initData, initDataLength );
  m_isPendingInit = false;
}

const uint8_t* aeNetData::GetInitData() const
{
  return m_initData.Length() ? &m_initData[ 0 ] : nullptr;
}

uint32_t aeNetData::InitDataLength() const
{
  return m_initData.Length();
}

const uint8_t* aeNetData::GetSyncData() const
{
  return m_data.Length() ? &m_data[ 0 ] : nullptr;
}

uint32_t aeNetData::SyncDataLength() const
{
  return m_data.Length();
}

void aeNetData::ClearSyncData()
{
  m_data.Clear();
}

bool aeNetData::PumpMessages( Msg* msgOut )
{
  if ( m_messageDataInOffset >= m_messageDataIn.Length() )
  {
    AE_ASSERT( m_messageDataInOffset == m_messageDataIn.Length() );
    return false;
  }
  else if ( !msgOut )
  {
    // Early out
    return true;
  }

  // Write out incoming message data
  msgOut->length = *(uint16_t*)&m_messageDataIn[ m_messageDataInOffset ];
  m_messageDataInOffset += sizeof( uint16_t );

  msgOut->data = &m_messageDataIn[ m_messageDataInOffset ];
  m_messageDataInOffset += msgOut->length;

  if ( m_messageDataInOffset >= m_messageDataIn.Length() )
  {
    AE_ASSERT( m_messageDataInOffset == m_messageDataIn.Length() );

    // Clear messages once they've all been read
    m_messageDataInOffset = 0;
    m_messageDataIn.Clear();
  }

  return true;
}

bool aeNetData::IsPendingInit() const
{
  return m_isPendingInit;
}

bool aeNetData::IsPendingDestroy() const
{
  return m_isPendingDestroy;
}

void aeNetData::m_SetClientData( const uint8_t* data, uint32_t length )
{
  m_data.Clear();
  m_data.Append( data, length );
}

void aeNetData::m_ReceiveMessages( const uint8_t* data, uint32_t length )
{
  m_messageDataIn.Append( data, length );
}

void aeNetData::m_UpdateHash()
{
  if ( m_data.Length() )
  {
    m_hash = ae::Hash().HashData( &m_data[ 0 ], m_data.Length() ).Get();
  }
  else
  {
    m_hash = 0;
  }
}

//------------------------------------------------------------------------------
// aeNetReplicaClient member functions
//------------------------------------------------------------------------------
void aeNetReplicaClient::ReceiveData( const uint8_t* data, uint32_t length )
{
  aeBinaryStream rStream = aeBinaryStream::Reader( data, length );
  while ( rStream.GetOffset() < rStream.GetLength() )
  {
    aeNetReplicaServer::EventType type;
    rStream.SerializeRaw( type );
    if ( !rStream.IsValid() )
    {
      break;
    }
    switch ( type )
    {
      case aeNetReplicaServer::EventType::Connect:
      {
        uint32_t signature = 0;
        rStream.SerializeUint32( signature );
        AE_ASSERT( signature );

        ae::Map< aeNetData*, int > toDestroy = AE_ALLOC_TAG_NET;
        bool allowResolve = ( m_serverSignature == signature );
        if ( m_serverSignature )
        {
          if ( allowResolve )
          {
            for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
            {
              toDestroy.Set( m_netDatas.GetValue( i ), 0 );
            }
          }
          else
          {
            m_created.Clear(); // Don't call delete, are pointers to m_netDatas
            for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
            {
              m_StartNetDataDestruction( m_netDatas.GetValue( i ) );
            }
            AE_ASSERT( !m_remoteToLocalIdMap.Length() );
            AE_ASSERT( !m_localToRemoteIdMap.Length() );
          }
        }
        
        uint32_t length = 0;
        rStream.SerializeUint32( length );
        for ( uint32_t i = 0; i < length && rStream.IsValid(); i++ )
        {
          aeNetData* created = m_CreateNetData( &rStream, allowResolve );
          toDestroy.Remove( created );
        }
        for ( uint32_t i = 0; i < toDestroy.Length(); i++ )
        {
          aeNetData* netData = toDestroy.GetKey( i );
          m_StartNetDataDestruction( netData );
        }

        m_serverSignature = signature;
        break;
      }
      case aeNetReplicaServer::EventType::Create:
      {
        m_CreateNetData( &rStream, false );
        break;
      }
      case aeNetReplicaServer::EventType::Destroy:
      {
        RemoteId remoteId;
        rStream.SerializeObject( remoteId );
        NetId localId = m_remoteToLocalIdMap.Get( remoteId );
        aeNetData* netData = m_netDatas.Get( localId );
        m_StartNetDataDestruction( netData );
        break;
      }
      case aeNetReplicaServer::EventType::Update:
      {
        uint32_t netDataCount = 0;
        rStream.SerializeUint32( netDataCount );
        for ( uint32_t i = 0; i < netDataCount; i++ )
        {
          RemoteId remoteId;
          uint32_t dataLen = 0;
          rStream.SerializeObject( remoteId );
          rStream.SerializeUint32( dataLen );

          NetId localId;
          aeNetData* netData = nullptr;
          if ( dataLen
            && m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netDatas.TryGet( localId, &netData ) )
          {
            if ( rStream.GetRemaining() >= dataLen )
            {
              netData->m_SetClientData( rStream.PeekData(), dataLen );
            }
            else
            {
              rStream.Invalidate();
            }
          }

          rStream.Discard( dataLen );
        }
        break;
      }
      case aeNetReplicaServer::EventType::Messages:
      {
        uint32_t netDataCount = 0;
        rStream.SerializeUint32( netDataCount );
        for ( uint32_t i = 0; i < netDataCount; i++ )
        {
          RemoteId remoteId;
          uint32_t dataLen = 0;
          rStream.SerializeObject( remoteId );
          rStream.SerializeUint32( dataLen );

          NetId localId;
          aeNetData* netData = nullptr;
          if ( dataLen
            && m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netDatas.TryGet( localId, &netData ) )
          {
            if ( rStream.GetRemaining() >= dataLen )
            {
              netData->m_ReceiveMessages( rStream.PeekData(), dataLen );
            }
            else
            {
              rStream.Invalidate();
            }
          }

          rStream.Discard( dataLen );
        }
        break;
      }
    }
  }
}

aeNetData* aeNetReplicaClient::PumpCreate()
{
  if ( !m_created.Length() )
  {
    return nullptr;
  }

  aeNetData* created = m_created[ 0 ];
  AE_ASSERT( created );
  m_created.Remove( 0 );
  return created;
}

void aeNetReplicaClient::Destroy( aeNetData* pendingDestroy )
{
  if ( !pendingDestroy )
  {
    return;
  }
  // @TODO: Maybe this should be supported in the case the client is shutting down with an active server connection?
  AE_ASSERT_MSG( pendingDestroy->IsPendingDestroy(), "aeNetData was not pending Destroy()" );
  AE_ASSERT_MSG( !pendingDestroy->PumpMessages( nullptr ), "aeNetData had pending messages when it was Destroy()ed" );
  bool removed = m_netDatas.Remove( pendingDestroy->_netId );
  AE_ASSERT( removed, "NetData can't be deleted. It was registered." );
  ae::Delete( pendingDestroy );
}

aeNetData* aeNetReplicaClient::m_CreateNetData( aeBinaryStream* rStream, bool allowResolve )
{
  AE_ASSERT( rStream->IsReader() );

  RemoteId remoteId;
  rStream->SerializeObject( remoteId );

  aeNetData* netData = nullptr;
  if ( allowResolve )
  {
    NetId localId = m_remoteToLocalIdMap.Get( remoteId, {} );
    if ( localId )
    {
      netData = m_netDatas.Get( localId );
    }
  }

  if ( !netData )
  {
    NetId localId( ++m_lastNetId );
    netData = ae::New< aeNetData >( AE_ALLOC_TAG_NET );
    netData->_netId = localId;

    m_netDatas.Set( localId, netData );
    m_remoteToLocalIdMap.Set( remoteId, localId );
    m_localToRemoteIdMap.Set( localId, remoteId );
    m_created.Append( netData );
  }
  
  rStream->SerializeArray( netData->m_initData );

  return netData;
}

void aeNetReplicaClient::m_StartNetDataDestruction( aeNetData* netData )
{
  AE_ASSERT( netData );
  if ( netData->IsPendingDestroy() )
  {
    return;
  }
  
  RemoteId remoteId;
  bool found = m_localToRemoteIdMap.Remove( netData->_netId, &remoteId );
  AE_ASSERT( found );
  found = m_remoteToLocalIdMap.Remove( remoteId );
  AE_ASSERT( found );
  netData->m_FlagForDestruction();
}

//------------------------------------------------------------------------------
// aeNetReplicaServer member functions
//------------------------------------------------------------------------------
void aeNetReplicaServer::m_UpdateSendData()
{
  AE_ASSERT( m_replicaDB );

  ae::Array< aeNetData* > toSync = AE_ALLOC_TAG_NET;
  uint32_t netDataMessageCount = 0;
  for ( uint32_t i = 0; i < m_replicaDB->GetNetDataCount(); i++ )
  {
    aeNetData* netData = m_replicaDB->GetNetData( i );
    if ( m_first || netData->m_Changed() )
    {
      toSync.Append( netData );
    }

    if ( netData->m_messageDataOut.Length() )
    {
      netDataMessageCount++;
    }
  }

  aeBinaryStream wStream = aeBinaryStream::Writer( &m_sendData );

  if ( toSync.Length() )
  {
    wStream.SerializeRaw( aeNetReplicaServer::EventType::Update );
    wStream.SerializeUint32( toSync.Length() );
    for ( uint32_t i = 0; i < toSync.Length(); i++ )
    {
      aeNetData* netData = toSync[ i ];
      wStream.SerializeObject( netData->_netId );
      wStream.SerializeUint32( netData->SyncDataLength() );
      wStream.SerializeRaw( netData->GetSyncData(), netData->SyncDataLength() );
    }
  }

  if ( netDataMessageCount )
  {
    wStream.SerializeRaw( aeNetReplicaServer::EventType::Messages );
    wStream.SerializeUint32( netDataMessageCount );
    for ( uint32_t i = 0; i < m_replicaDB->GetNetDataCount(); i++ )
    {
      aeNetData* netData = m_replicaDB->GetNetData( i );
      if ( netData->m_messageDataOut.Length() )
      {
        wStream.SerializeObject( netData->_netId );
        wStream.SerializeUint32( netData->m_messageDataOut.Length() );
        wStream.SerializeRaw( &netData->m_messageDataOut[ 0 ], netData->m_messageDataOut.Length() );
      }
    }
  }

  m_pendingClear = true;
  m_first = false;
}

const uint8_t* aeNetReplicaServer::GetSendData() const
{
  return m_sendData.Length() ? &m_sendData[ 0 ] : nullptr;
}

uint32_t aeNetReplicaServer::GetSendLength() const
{
  return m_sendData.Length();
}

//------------------------------------------------------------------------------
// aeNetReplicaDB member functions
//------------------------------------------------------------------------------
#include <random>
aeNetReplicaDB::aeNetReplicaDB()
{
  std::random_device random_device;
  std::mt19937 random_engine( random_device() );
  std::uniform_int_distribution< uint32_t > dist( 1, ae::MaxValue< uint32_t >() );
  m_signature = dist( random_engine );
}

aeNetData* aeNetReplicaDB::CreateNetData()
{
  aeNetData* netData = ae::New< aeNetData >( AE_ALLOC_TAG_NET );
  netData->m_SetLocal();
  netData->_netId = NetId( ++m_lastNetId );
  m_pendingCreate.Append( netData );
  return netData;
}

void aeNetReplicaDB::DestroyNetData( aeNetData* netData )
{
  if ( !netData )
  {
    return;
  }
  
  int32_t pendingIdx = m_pendingCreate.Find( netData );
  if ( pendingIdx >= 0 )
  {
    // Early out, no need to send Destroy message because Create has not been queued
    m_pendingCreate.Remove( pendingIdx );
    ae::Delete( netData );
    return;
  }

  NetId id = netData->_netId;
  bool removed = m_netDatas.Remove( id );
  AE_ASSERT_MSG( removed, "aeNetData was not found." );

  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    aeNetReplicaServer* server = m_servers[ i ];
    if ( server->m_pendingClear )
    {
      server->m_sendData.Clear();
      server->m_pendingClear = false;
    }

    aeBinaryStream wStream = aeBinaryStream::Writer( &server->m_sendData );
    wStream.SerializeRaw( aeNetReplicaServer::EventType::Destroy );
    wStream.SerializeObject( id );
  }

  ae::Delete( netData );
}

aeNetReplicaServer* aeNetReplicaDB::CreateServer()
{
  aeNetReplicaServer* server = m_servers.Append( ae::New< aeNetReplicaServer >( AE_ALLOC_TAG_NET ) );
  AE_ASSERT( !server->m_pendingClear );
  server->m_replicaDB = this;

  // Send initial net datas
  aeBinaryStream wStream = aeBinaryStream::Writer( &server->m_sendData );
  wStream.SerializeRaw( aeNetReplicaServer::EventType::Connect );
  wStream.SerializeUint32( m_signature );
  wStream.SerializeUint32( m_netDatas.Length() );
  for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
  {
    const aeNetData* netData = m_netDatas.GetValue( i );
    wStream.SerializeObject( netData->_netId );
    wStream.SerializeArray( netData->m_initData );
  }

  return server;
}

void aeNetReplicaDB::DestroyServer( aeNetReplicaServer* server )
{
  if ( !server )
  {
    return;
  }

  int32_t index = m_servers.Find( server );
  if ( index >= 0 )
  {
    m_servers.Remove( index );
    ae::Delete( server );
  }
}

void aeNetReplicaDB::UpdateSendData()
{
  // Clear old send data before writing new
  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    aeNetReplicaServer* server = m_servers[ i ];
    if ( server->m_pendingClear )
    {
      server->m_sendData.Clear();
      server->m_pendingClear = false;
    }
  }
  
  // Send info about new objects (delayed until Update in case objects initData need to reference each other)
  for ( aeNetData* netData : m_pendingCreate )
  {
    if ( !netData->IsPendingInit() )
    {
      // Add net data to list, remove all initialized net datas from m_pendingCreate at once below
      m_netDatas.Set( netData->_netId, netData );
      
      // Send create messages on existing server connections
      for ( uint32_t i = 0; i < m_servers.Length(); i++ )
      {
        aeNetReplicaServer* server = m_servers[ i ];
        aeBinaryStream wStream = aeBinaryStream::Writer( &server->m_sendData );
        wStream.SerializeRaw( aeNetReplicaServer::EventType::Create );
        wStream.SerializeObject( netData->_netId );
        wStream.SerializeArray( netData->m_initData );
      }
    }
  }
  // Remove all pending net datas that were just initialized
  m_pendingCreate.RemoveAllFn( []( const aeNetData* netData ){ return !netData->IsPendingInit(); } );
  
  for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
  {
    m_netDatas.GetValue( i )->m_UpdateHash();
  }

  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    m_servers[ i ]->m_UpdateSendData();
  }

  for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
  {
    aeNetData* netData = m_netDatas.GetValue( i );
    netData->m_prevHash = netData->m_hash;
    netData->m_messageDataOut.Clear();
  }
}

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
//AE_REGISTER_CLASS( aeRpc );

// //------------------------------------------------------------------------------
// // Internal Net Struct
// //------------------------------------------------------------------------------
// const double kSendInterval = 0.1;

// //------------------------------------------------------------------------------
// // Net Private Functions
// //------------------------------------------------------------------------------
// void Net_Register( Net* net, GameObject* go, NetInstId instId )
// {
//   Game* game = go->game;
//   HVN_ASSERT( instId >= game->GetNetId() );

//   NetInstInfo info;
//   info.gameId = go->ID;
//   info.game = go->game;
//   info.netId = instId;
//   info.type = aeMeta::GetType( go )->GetId();
//   info.levelObject = false;
//   net->netInsts[ net->netInstCount++ ] = info;
  
//   go->netInstId = info.netId;
// }

// void Net_AddObject( Net* net, GameObject* go, bool levelObject )
// {
//   Game* game = go->game;
//   HVN_ASSERT( levelObject || game->IsServer() );

//   NetInstInfo info;
//   info.gameId = go->ID;
//   info.game = go->game;
//   info.netId = game->GenerateNetId();
//   info.type = aeMeta::GetType( go )->GetId();
//   info.levelObject = levelObject;
//   net->netInsts[ net->netInstCount++ ] = info;

//   go->netInstId = info.netId;
  
//   // if ( levelObject ) { HVN_LOG( "Add Net Level Object : %s : %d : %d : %d", game->GetLevelName(), info.netId, info.gameId, type ); }
//   // else { HVN_LOG( "Add Net Object : %s : %d : %d : %d", game->GetLevelName(), info.netId, info.gameId, type ); }
// }

// //------------------------------------------------------------------------------
// // Net Public Functions
// //------------------------------------------------------------------------------
// Net* Net_New()
// {
//   Net* net = new Net();
  
//   net->lastSendTime = 0.0;
//   net->netInstCount = 0;
//   net->messageCount = 0;
  
//   return (Net*)net;
// }

// void Net_Delete( Net* _net )
// {
//   delete _net;
// }

// void Net_RemoveObject( GameObject* go )
// {
//   HVN_ASSERT( go->netInstId != 0 );
  
//   // HVN_LOG( "Remove Net Object : %s : %d : %d", go->game->GetLevelName(), go->netInstId, go->ID );
    
//   Net* net = go->game->net;
//   uint32_t netInstCount = net->netInstCount;
//   for ( uint32_t i = 0; i < netInstCount; i++ )
//   {
//     NetInstInfo info = net->netInsts[ i ];
//     if ( info.game == go->game && info.netId == go->netInstId )
//     {
//       HVN_ASSERT( info.gameId == go->ID );
//       HVN_ASSERT( GetObjectTypeId( go ) == info.type );
//       net->netInsts[ i ] = net->netInsts[ netInstCount - 1 ];
//       net->netInstCount--;
//       if ( go->game->IsServer() )
//       {
//         MsgRpcDestroy msg;
//         msg.instId = go->netInstId;
//         AetherServer* aether = go->game->aetherServer;
//         AetherServer_QueueSendToGroup( aether, kMsgRpcDestroy, true, msg, go->game );
//       }
//       go->netInstId = 0;
//       GameObjectManager_SetObjectDelete( go );
      
//       return;
//     }
//   }
  
//   HVN_FAIL();
// }

// GameObject* Net_GetGameObject( Net* net, Game* game, NetInstId id )
// {
//   for ( uint32_t i = 0; i < net->netInstCount; i++ )
//   {
//     if ( net->netInsts[ i ].game == game && net->netInsts[ i ].netId == id )
//     {
//       return Net_GetGameObject( net, game, net->netInsts[ i ] );
//     }
//   }
//   return nullptr;
// }

// GameObject* Net_GetGameObject( Net* net, Game* game, NetInstInfo info )
// {
//   if ( info.game == game )
//   {
//     return GameObjectManager_GetByID( game->gameObjectManager, info.gameId );
//   }
//   return nullptr;
// }

// AetherPlayer* Net_GetPlayer( Net* net, AetherServer* aether, NetInstId id )
// {
//   HVN_ASSERT( aether );
//   for ( int32_t i = 0; i < aether->playerCount; i++ )
//   {
//     AetherPlayer* player = aether->allPlayers[ i ];
//     if ( player->netId == id )
//     {
//       return player;
//     }
//   }
//   return nullptr;
// }

// void Net_SpawnPlayerAvatar( Net* net, AetherServer* aether, AetherPlayer* player, Game* game, const char* link )
// {
//   v2i pos;
//   if ( !ChangeLevel_GetEmptyPos( game, link, &pos ) )
//   {
//     return;
//   }

//   HVN_LOG( "Spawn player avatar" );

//   game->SetCheckPoint( pos ); // hack
  
//   ae::Dict dict;
//   dict.SetV2i( "position", pos );
//   Player* playerObj = game->gameObjectManager->Create< Player >( dict, true );
//   playerObj->uuid = player->uuid;
  
//   player->netId = playerObj->netInstId;

//   Player_Load( hvn_cast<Player>(playerObj) );

//   HVN_LOG( "HAVEN Spawn player avatar %d %d %d", pos.X, pos.Y, playerObj->ID );
// }

// void Net_UpdateServer( Net* net, AetherServer* aether, Game* games, uint32_t gameCount )
// {
//   AetherServer_Update( aether );
  
//   double currentTime = Time_GetTotalElapsed( games[ 0 ].time ); // HACK
//   if ( net->lastSendTime + kSendInterval >= currentTime )
//   {
//     return;
//   }
//   net->lastSendTime = currentTime;
  
//   ServerReceiveInfo recvInfo;
//   while( AetherServer_Receive( aether, &recvInfo ) )
//   {
//     HVN_ASSERT( recvInfo.player );
//     Game* game = (Game*)recvInfo.player->userData;

//     bool forward = false;
//     switch ( recvInfo.msgId )
//     {
//       case kSysMsgPlayerConnect:
//       {
//         if ( game )
//         {
//           // TODO: Already in game, reject
//           HVN_LOG( "Warning: Player already in game!" );
//         }

//         char uuidStr[ 64 ];
//         recvInfo.player->uuid.ToString( uuidStr, sizeof(uuidStr) );

//         HVN_LOG( "HAVEN Connect %s", uuidStr );
//         for ( uint32_t i = 0; i < gameCount; i++ )
//         {
//           if ( strcmp( games[ i ].GetLevelName(), kStartLevel ) == 0 )
//           {
//             game = &games[ i ];
//           }
//         }
//         HVN_ASSERT( game );
//         recvInfo.player->userData = game;

//         HVN_ASSERT( strcmp( game->GetLevelName(), kStartLevel ) == 0 );
//         recvInfo.player->pendingLevel = kStartLevel;
//         recvInfo.player->pendingLink = "start";
//         recvInfo.player->hasPendingLevelChange = true;

//         HVN_ASSERT( recvInfo.player->netId == 0 );

//         game->TryCreateLuaPlayer( false, uuidStr );

//         MsgLuaPlayer msg;
//         msg.id = recvInfo.player->uuid;
//         msg.alive = true;
        
//         ServerSendInfo sendInfo;
//         sendInfo.msgId = kMsgLuaPlayer;
//         sendInfo.group = game;
//         sendInfo.length = sizeof(msg);
//         memcpy( sendInfo.data, &msg, sizeof(msg) );
//         AetherServer_QueueSendInfo( aether, &sendInfo );
        
//         break;
//       }
//       case kSysMsgPlayerDisconnect:
//       {
//         char uuidStr[ 64 ];
//         recvInfo.player->uuid.ToString( uuidStr, sizeof(uuidStr) );

//         HVN_LOG( "HAVEN Disconnect %s", uuidStr );
//         GameObject* go = Net_GetGameObject( net, game, recvInfo.player->netId );
//         if ( go )
//         {
//           // HVN_LOG( "destroy %s %d", aeMeta::GetType( go )->GetName(), go->ID );
//           GameObjectManager_SetObjectDelete( go );
//         }
//         recvInfo.player->netId = 0;

//         game->TryDestroyLuaPlayer( uuidStr );

//         MsgLuaPlayer msg;
//         msg.id = recvInfo.player->uuid;
//         msg.alive = false;
        
//         ServerSendInfo sendInfo;
//         sendInfo.msgId = kMsgLuaPlayer;
//         sendInfo.group = game;
//         sendInfo.length = sizeof(msg);
//         memcpy( sendInfo.data, &msg, sizeof(msg) );
//         AetherServer_QueueSendInfo( aether, &sendInfo );

//         break;
//       }
//       case kMsgMove:
//       {
//         MsgMove msg;
//         HVN_ASSERT( sizeof(msg) == recvInfo.length );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
        
//         GameObject* go = Net_GetGameObject( net, game, msg.netId );
//         if ( go )
//         {
//           ((Pawn*)go)->net.target = msg.target;
//           ((Pawn*)go)->fd = (FacingDirection)msg.facing;

//           forward = true;
//         }
//         break;
//       }
//       case kMsgAttack:
//       {
//         MsgAttack msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );
        
//         Pawn* pawn = hvn_cast<Pawn>( Net_GetGameObject( net, game, msg.netId ) );
//         if ( pawn )
//         {
//           if ( pawn->net.pendingAttackCount < kMaxPendingAttacks )
//           {
//             pawn->net.pendingAttacks[ pawn->net.pendingAttackCount ] = (AttackType)msg.type;
//             // pawn->net.pendingAttackTargets[ pawn->net.pendingAttackCount ] = 0;
//             pawn->net.pendingAttackCount++;
//           }
//           pawn->fd = (FacingDirection)msg.dir;
//           forward = true;
//         }
        
//         break;
//       }
//       case kMsgDive:
//       {
//         MsgDive msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );

//         Pawn* pawn = hvn_cast<Pawn>( Net_GetGameObject( net, game, msg.netId ) );
//         if ( pawn )
//         {
//           Pawn_Dive( pawn, msg.dive, true );
//           forward = true;
//         }

//         break;
//       }
//       case kMsgChatMessage:
//       {
//         MsgChatMessage msg;
//         HVN_ASSERT( recvInfo.length <= sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );

//         HVN_ASSERT( msg.length < sizeof(msg.message) );

//         // Forward to player in same game
//         ServerSendInfo complete;
//         complete.msgId = kMsgChatMessage;
//         complete.group = game;
//         complete.playerFilter = recvInfo.player;
//         complete.length = recvInfo.length;
//         memcpy( complete.data, recvInfo.data, recvInfo.length );
//         AetherServer_QueueSendInfo( aether, &complete );

//         // Send distorted message to other players
//         MsgChatMessage msgDistorted;
//         memcpy( &msgDistorted, recvInfo.data, recvInfo.length );
//         for ( uint32_t i = 0; i < msgDistorted.length; i++ )
//         {
//           msgDistorted.message[ i ] = PRNG_i( 0, 2 ) != 0 ? msgDistorted.message[ i ] : '#';
//         }
//         ServerSendInfo distorted;
//         distorted.msgId = kMsgChatMessage;
//         distorted.groupFilter = game;
//         distorted.length = sizeof(msgDistorted);
//         memcpy( distorted.data, msgDistorted.message, sizeof(msgDistorted) );
//         AetherServer_QueueSendInfo( aether, &distorted );

//         break;
//       }
//       case kMsgLua:
//       {
//         char uuidStr[ 64 ];
//         recvInfo.player->uuid.ToString( uuidStr, sizeof(uuidStr) );
//         game->ReceiveLuaMessage( uuidStr, recvInfo.data, recvInfo.length );
//         break;
//       }
//       case kMsgRpc:
//       {
//         MsgRpc msg;
//         HVN_ASSERT( recvInfo.length <= sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );

//         const ae::Type* metaType = aeMeta::GetType( msg.typeId );
//         if ( metaType && metaType->GetParentType() == aeMeta::GetType< Rpc >() )
//         {
//           Rpc* rpc = ae::Cast< Rpc >( metaType->New() );
//           rpc->game = game;
//           rpc->aetherPlayer = recvInfo.player;

//           BinaryStream rStream( BinaryStream::ReadBuffer, msg.data, msg.length );
//           rpc->Serialize( &rStream );
          
//           rpc->Run();
//           delete rpc;
//         }

//         break;
//       }
//       default:
//       {
//         HVN_LOG( "MsgId %d", recvInfo.msgId );
//         HVN_FAIL();
//         break;
//       }
//     }
    
//     if ( forward )
//     {
//       HVN_ASSERT( game );

//       ServerSendInfo sendInfo;
//       sendInfo.msgId = recvInfo.msgId;
//       sendInfo.length = recvInfo.length;
//       memcpy( sendInfo.data, recvInfo.data, recvInfo.length );
//       sendInfo.reliable = true;
//       sendInfo.group = nullptr;
      
//       for ( uint32_t i = 0; i < aether->playerCount; i++ )
//       {
//         if ( recvInfo.player && aether->allPlayers[ i ] != recvInfo.player && recvInfo.player->userData == game )
//         {
//           sendInfo.player = aether->allPlayers[ i ];
//           AetherServer_QueueSendInfo( aether, &sendInfo );      
//         }
//       }
//     }
//   }

//   // Handle pending level changes
//   for ( uint32_t i = 0; i < aether->playerCount; i++ )
//   {
//     AetherPlayer* player = aether->allPlayers[ i ];
//     if ( player->hasPendingLevelChange )
//     {
//       player->hasPendingLevelChange = false;
      
//       Game* currentGame = (Game*)player->userData;
//       if ( currentGame && player->pendingLevel == currentGame->gameState->GetCurrent() )
//       {
//         HVN_LOG( "Warning: Tried to change to current level: %s", player->pendingLevel.cstr() );
//         continue;
//       }

//       Game* nextGame = nullptr;
//       for ( i = 0; i < gameCount; i++ )
//       {
//         if ( player->pendingLevel == games[ i ].gameState->GetCurrent() )
//         {
//           nextGame = &games[ i ];
//           break;
//         }
//       }

//       if ( nextGame )
//       {
//         HVN_LOG( "change to game: %s %p", player->pendingLevel.cstr(), nextGame );
//         HVN_ASSERT( player->userData );
//         HVN_ASSERT( player->alive );

//         GameObject* go = currentGame ? Net_GetGameObject( net, currentGame, player->netId ) : nullptr;
//         if ( go )
//         {
//             GameObjectManager_SetObjectDelete( go );
//         }
//         player->netId = 0;

//         MsgChangeLevel msg;
//         HVN_ASSERT( player->pendingLevel.Length() < sizeof(msg.level) );
//         HVN_ASSERT( player->pendingLink.Length() < sizeof(msg.link) );
//         strcpy( msg.level, player->pendingLevel.cstr() );
//         strcpy( msg.link, player->pendingLink.cstr() );
//         AetherServer_QueueSendToPlayer( aether, kMsgChangeLevel, true, msg, player );

//         player->userData = nextGame;

//         uint32_t netInstCount = net->netInstCount;
//         for ( uint32_t i = 0; i < netInstCount; i++ )
//         {
//           NetInstInfo info = net->netInsts[ i ];
//           GameObject* go = Net_GetGameObject( net, nextGame, info );
//           if ( go && go->alive && !info.levelObject )
//           {
//             HVN_ASSERT( go->game == nextGame );

//             Actor* actor = hvn_cast<Actor>(go);
//             HVN_ASSERT( actor );

//             MsgRpcCreate msg;
//             msg.type = aeMeta::GetType( go )->GetId();
//             msg.instId = info.netId;
//             msg.x = actor->position.X;
//             msg.y = actor->position.Y;
//             BinaryStream wStream( BinaryStream::WriteBuffer, msg.data, sizeof(msg.data) );
//             go->Serialize( &wStream );
//             msg.length = wStream.GetOffset();
            
//             // Send all net object create messages as soon as players connect (before any net objects are
//             // created this frame) otherwise it's possible to send duplicate create messages.
//             AetherServer_QueueSendToPlayer( aether, kMsgRpcCreate, true, msg, player );
//           }
//         }
//       }
//       else
//       {
//         HVN_LOG( "Warning: Change to invalid level '%s'", player->pendingLevel.cstr() );
//       }
//     }
//   }

//   // Spawn unspawned players
//   for ( uint32_t i = 0; i < aether->playerCount; i++ )
//   {
//     AetherPlayer* player = aether->allPlayers[ i ];
//     if ( player->alive )
//     {
//       HVN_ASSERT( player->userData );
//       Game* game = (Game*)player->userData;
//       if ( player->netId == 0 )
//       {
//         HVN_ASSERT( player->pendingLevel == game->GetLevelName() );
//         Net_SpawnPlayerAvatar( net, aether, player, game, player->pendingLink.cstr() );
//       }
//     }
//   }

//   // Sync adventurer state with owners
//   for ( uint32_t i = 0; i < aether->playerCount; i++ )
//   {
//     AetherPlayer* player = aether->allPlayers[ i ];
//     Game* game = (Game*)player->userData;
//     HVN_ASSERT( game );

//     GameObject* go = Net_GetGameObject( net, game, player->netId );
//     if ( go )
//     {
//       MsgAdventurerState advMsg;
//       advMsg.localAdventurer = player->netId;
//       advMsg.health = hvn_cast<Creature>(go)->currentHP;
//       advMsg.energy = hvn_cast<Player>(go)->currentEnergy;
//       advMsg.level = 0;
//       advMsg.exp = 0;
//       advMsg.air = hvn_cast<Player>(go)->air;
//       AetherServer_QueueSendToPlayer( aether, kMsgAdventurerState, true, advMsg, player );
//     }
//   }

//   for ( uint32_t i = 0; i < net->netInstCount; i++ )
//   {
//     NetInstInfo* info = &net->netInsts[ i ];
//     GameObject* go = Net_GetGameObject( net, info->game, info->netId );
//     if ( !go )
//     {
//       continue;
//     }

//     MsgObjectSync msg;
//     memset( &msg, 0, sizeof(msg) );
//     msg.netId = info->netId;
//     BinaryStream wStream( BinaryStream::WriteBuffer, msg.data, sizeof(msg.data) );
//     go->Serialize( &wStream );
//     msg.length = wStream.GetOffset();
//     if ( msg.length )
//     {
//       AetherServer_QueueSendToGroup( aether, kMsgObjectSync, true, msg, go->game );
//     }
//   }

//   AetherServer_SendAll( aether );
// }

// void Net_UpdateClient( Net* net, AetherClient* aether, Game* game )
// {
//   // HACK! All objects that can receive messages should check for them each frame, but it's
//   // possible that an object will be destroyed without collecting all of its messages.
//   if ( net->messageCount != 0 )
//   {
//     HVN_LOG( "Warning: Object messages were discarded." );
//     net->messageCount = 0;
//   }

//   if ( !aether->IsConnected() && !aether->IsConnecting() )
//   {
//     HVN_LOG( "HAVEN Start connect" );
//     AetherClient_Connect( aether );
//   }
  
//   ReceiveInfo recvInfo;
//   while( AetherClient_Receive( aether, &recvInfo ) )
//   {
//     switch ( recvInfo.msgId )
//     {
//       case kSysMsgServerConnect:
//       {
//         HVN_LOG( "HAVEN Connect" );
//         break;
//       }
//       case kSysMsgServerDisconnect:
//       {
//         HVN_LOG( "HAVEN Disconnect" );
//         break;
//       }
//       case kMsgChangeLevel:
//       {
//         MsgChangeLevel msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
//         HVN_LOG( "change level: %s %s", msg.level, msg.link );

//         game->gameState->SetNext( msg.level );
//         game->gameState->Update( game ); // HACK

//         break;
//       }
//       case kMsgRpcCreate:
//       {
//         MsgRpcCreate createMsg;
//         HVN_ASSERT( recvInfo.length == sizeof(MsgRpcCreate) );
//         memcpy( &createMsg, recvInfo.data, recvInfo.length );
        
//         const ae::Type* metaType = aeMeta::GetType( createMsg.type );
//         HVN_ASSERT_MSG( metaType, "No type %d", createMsg.type );
//         HVN_LOG( "Net obj create t:%s x:%d y:%d", metaType->GetName(), createMsg.x, createMsg.y );

//         ae::Dict dict;
//         dict.SetV2i( "position", v2i_From_i( createMsg.x, createMsg.y ) );
//         GameObject* go = game->gameObjectManager->Create( metaType, dict );
//         BinaryStream rStream( BinaryStream::ReadBuffer, createMsg.data, createMsg.length );
//         go->Serialize( &rStream );
        
//         // TODO: Net objects should be registered automatically on the client
//         Net_Register( net, go, createMsg.instId );

//         break;
//       }
//       case kMsgRpcDestroy:
//       {
//         MsgRpcDestroy msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
//         GameObject* go = Net_GetGameObject( net, game, msg.instId );
//         if ( go )
//         {
//           Net_RemoveObject( go );
//         }
//         break;
//       }
//       case kMsgResetPos:
//       {
//         MsgResetPos msg;
//         HVN_ASSERT( sizeof(msg) == recvInfo.length );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
        
//         GameObject* go = Net_GetGameObject( net, game, msg.netId );
//         if ( go )
//         {
//           ((Pawn*)go)->net.target = v2i_from_v2f(msg.position);
//           Actor_SetPositionX( (Actor*)go, msg.position.X );
//           Actor_SetPositionY( (Actor*)go, msg.position.Y );
//         }
//         break;
//       }
//       case kMsgAdventurerState:
//       {
//         MsgAdventurerState msg;
//         HVN_ASSERT( sizeof(msg) == recvInfo.length );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
        
//         GameObject* local = Net_GetGameObject( net, game, msg.localAdventurer );
//         HVN_ASSERT( local );
//         HVN_ASSERT( hvn_cast< Player >( local ) );
//         local->netLocal = true;
//         hvn_cast<Creature>(local)->currentHP =  msg.health;
//         hvn_cast<Player>(local)->currentEnergy =  msg.energy;
//         // msg.level;
//         // msg.exp;
//         hvn_cast<Player>(local)->air = msg.air;
        
//         break;
//       }
//       case kMsgMove:
//       {
//         MsgMove msg;
//         HVN_ASSERT( sizeof(msg) == recvInfo.length );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
        
//         GameObject* go = Net_GetGameObject( net, game, msg.netId );
//         if ( go )
//         {
//           ((Pawn*)go)->net.target = msg.target;
//           ((Pawn*)go)->fd = (FacingDirection)msg.facing;
//         }
//         break;
//       }
//       case kMsgAttack:
//       {
//         MsgAttack msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );
//         HVN_ASSERT( msg.netId != 0 );
        
//         Pawn* pawn = (Pawn*)Net_GetGameObject( net, game, msg.netId );
//         if ( pawn )
//         {
//           if ( pawn->net.pendingAttackCount < kMaxPendingAttacks )
//           {
//             pawn->net.pendingAttacks[ pawn->net.pendingAttackCount ] = (AttackType)msg.type;
//             // pawn->net.pendingAttackTargets[ pawn->net.pendingAttackCount ] = msg.netId;
//             pawn->net.pendingAttackCount++;
//           }
//           pawn->fd = (FacingDirection)msg.dir;
//         }
        
//         break;
//       }
//       case kMsgHit:
//       {
//         MsgHit msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );
        
//         Creature* defender = (Creature*)Net_GetGameObject( net, game, msg.defenderNetId );
//         Creature_AttackRemote( msg.attackerType, defender, msg.defenderType, msg.success );

//         break;
//       }
//       case kMsgKill:
//       {
//         MsgKill msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );

//         Pawn* target = (Pawn*)Net_GetGameObject( net, game, msg.netId );
//         if ( target )
//         {
//           Pawn_KillRemote( target );
//         }

//         break;
//       }
//       case kMsgUse:
//       {
//         MsgUse msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );

//         Prop* item = hvn_cast< Prop >( Net_GetGameObject( net, game, msg.item ) );
//         Creature* creature = hvn_cast< Creature >( Net_GetGameObject( net, game, msg.target ) );
//         if ( item && creature )
//         {
//           item->Use( creature );
//         }
//         break;
//       }
//       case kMsgObject:
//       {
//         HVN_LOG( "kMsgObject recv" );
//         HVN_ASSERT( net->messageCount < kMaxNetMessages );
//         memcpy( &net->messages[ net->messageCount ], recvInfo.data, sizeof(*net->messages) );
//         net->messageCount++;
//         break;
//       }
//       case kMsgDive:
//       {
//         MsgDive msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );

//         Pawn* pawn = hvn_cast<Pawn>( Net_GetGameObject( net, game, msg.netId ) );
//         if ( pawn )
//         {
//           Pawn_Dive( pawn, msg.dive, true );
//         }

//         break;
//       }
//       case kMsgObjectSync:
//       {
//         MsgObjectSync msg;
//         HVN_ASSERT( recvInfo.length == sizeof(msg) );
//         memcpy( &msg, recvInfo.data, sizeof(msg) );

//         GameObject* go = Net_GetGameObject( net, game, msg.netId );
//         if ( go )
//         {
//           BinaryStream rStream( BinaryStream::ReadBuffer, msg.data, msg.length );
//           go->Serialize( &rStream );
//         }

//         break;
//       }
//       case kMsgChatMessage:
//       {
//         MsgChatMessage msg;
//         HVN_ASSERT( recvInfo.length <= sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );

//         HVN_ASSERT( msg.length < sizeof(msg.message) );
//         msg.message[ msg.length ] = 0;

//         game->chat->AddMessage( game->time, kChatType_RemotePlayer, "other", msg.message );

//         break;
//       }
//       case kMsgLua:
//       {
//         game->ReceiveLuaMessage( "", recvInfo.data, recvInfo.length );
//         break;
//       }
//       case kMsgLuaPlayer:
//       {
//         MsgLuaPlayer msg;
//         HVN_ASSERT( recvInfo.length <= sizeof(msg) );
//         memcpy( &msg, recvInfo.data, recvInfo.length );
        
//         char uuidStr[ 64 ];
//         msg.id.ToString( uuidStr, sizeof(uuidStr) );
//         if ( msg.alive )
//         {
//           bool isLocal = ( aether->localPlayer->uuid == msg.id );
//           game->TryCreateLuaPlayer( isLocal, uuidStr );
//         }
//         else
//         {
//           game->TryDestroyLuaPlayer( uuidStr );
//         }
        
//         break;
//       }
//       default:
//       {
//         HVN_FAIL();
//         break;
//       }
//     }
//   }

//   double currentTime = Time_GetTotalElapsed( game->time );
//   if ( aether->isConnected && net->lastSendTime + kSendInterval >= currentTime )
//   {
//     return;
//   }
//   net->lastSendTime = currentTime;
  
//   AetherClient_SendAll( aether );
// }

// bool Net_CanChangeLevel( Net* net, AetherServer* aether, Player* player, const char* level )
// {
//   if ( player->changingLevels )
//   {
//     return false;
//   }

//   if ( strcmp( level, player->game->gameState->GetCurrent() ) == 0 )
//   {
//     return false;
//   }

//   if ( !Net_GetPlayer( net, aether, player->netInstId ) )
//   {
//     return false;
//   }

//   return true;
// }

// bool Net_PlayerChangeLevel( Net* net, AetherServer* aether, Player* player, const char* level, const char* link )
// {
//   if ( !Net_CanChangeLevel( net, aether, player, level ) )
//   {
//     return false;
//   }

//   Player_Save( player );

//   AetherPlayer* netPlayer = Net_GetPlayer( net, aether, player->netInstId );
//   netPlayer->hasPendingLevelChange = true;
//   netPlayer->pendingLevel = level;
//   netPlayer->pendingLink = link;

//   player->changingLevels = true;

//   return true;
// }

// void Net_SendObjectMessage( Net* net, struct AetherServer* aether, void* group, NetInstId id, uint8_t type, const uint8_t* data, uint32_t length )
// {
//   HVN_ASSERT_MSG( id, "Object must be networked to send message!" );
//   if ( length == 0 )
//   {
//     return;
//   }

//   MsgObject msg;
//   msg.instId = id;
//   msg.type = type;
//   memcpy( msg.data, data, length );
//   msg.length = length;

//   AetherServer_QueueSendToGroup( aether, kMsgObject, true, msg, group );
// }

// uint32_t Net_RecvObjectMessage( Net* net, NetInstId id, uint8_t* typeOut, uint8_t* dataOut )
// {
//   MsgObject* msg = nullptr;
//   for ( uint32_t i = 0; i < net->messageCount; i++ )
//   {
//     if ( net->messages[ i ].instId == id )
//     {
//       msg = &net->messages[ i ];
//       break;
//     }
//   }

//   if ( msg )
//   {
//     uint32_t length = msg->length;
//     *typeOut = msg->type;
//     memcpy( dataOut, msg->data, length );

//     uint32_t msgIdx = msg - net->messages;
//     uint32_t moveBytes = ( net->messageCount - msgIdx - 1 ) * sizeof(MsgObject);
//     memmove( &net->messages[ msgIdx ], &net->messages[ msgIdx + 1 ], moveBytes );

//     net->messageCount--;

//     return length;
//   }

//   return 0;
// }

// void Aether_SendRpc( AetherClient* aether, Rpc* rpc )
// {
//   MsgRpc msg;

//   const ae::Type* metaType = aeMeta::GetType( rpc );
//   HVN_ASSERT( metaType );
//   msg.typeId = metaType->GetId();

//   BinaryStream wStream( BinaryStream::WriteBuffer, msg.data, sizeof(msg.data) );
//   rpc->Serialize( &wStream );
//   msg.length = wStream.GetOffset();

//   AetherClient_QueueSend( aether, kMsgRpc, true, msg );
// }
