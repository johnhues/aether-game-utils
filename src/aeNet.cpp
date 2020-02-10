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
const uint8_t* aeNetData::GetInitData() const
{
  return m_initData.Length() ? &m_initData[ 0 ] : nullptr;
}

uint32_t aeNetData::InitDataLength() const
{
  return m_initData.Length();
}

void aeNetData::Set( const uint8_t* data, uint32_t length )
{
  AE_ASSERT_MSG( IsAuthority(), "Cannot set net data from client. The aeNetReplicaServer has exclusive ownership." );
  m_data.Clear();
  m_data.Append( data, length );
}

const uint8_t* aeNetData::Get() const
{
  return m_data.Length() ? &m_data[ 0 ] : nullptr;
}

uint32_t aeNetData::Length() const
{
  return m_data.Length();
}

void aeNetData::Clear()
{
  m_data.Clear();
}

void aeNetData::m_SetClientData( const uint8_t* data, uint32_t length )
{
  m_data.Clear();
  m_data.Append( data, length );
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
        // @TODO: Reconcile or destroy all current net datas first

        uint32_t length = 0;
        rStream.SerializeUint32( length );
        for ( uint32_t i = 0; i < length && rStream.IsValid(); i++ )
        {
          m_CreateNetData( &rStream );
        }
        break;
      }
      case aeNetReplicaServer::EventType::Create:
      {
        m_CreateNetData( &rStream );
        break;
      }
      case aeNetReplicaServer::EventType::Destroy:
      {
        uint32_t remoteId = 0;
        rStream.SerializeUint32( remoteId );

        aeId< aeNetData > localId;
        if ( m_remoteToLocalIdMap.TryGet( remoteId, &localId ) )
        {
          m_remoteToLocalIdMap.Remove( remoteId );

          int32_t createdIndex = m_created.FindFn( [localId]( aeRef< aeNetData >& ref ){ return ref.GetId() == localId; } );
          if ( createdIndex >= 0 )
          {
            // @TODO: Removing this from the created list will prevent the client from ever knowing about the object.
            //        How should objects that are created and destroyed within a single network frame be handled?
            //        Maybe deletion should be queued and happen after one network frame has passed.
            m_created.Remove( createdIndex );
          }

          aeNetData* netData = nullptr;
          if ( m_netDatas.TryGet( localId, &netData ) )
          {
            AE_ASSERT( netData );
            m_netDatas.Remove( localId );
            aeAlloc::Release( netData );
          }
        }

        break;
      }
      case aeNetReplicaServer::EventType::Update:
      {
        uint32_t netDataCount = 0;
        rStream.SerializeUint32( netDataCount );
        for ( uint32_t i = 0; i < netDataCount; i++ )
        {
          uint32_t remoteId = 0;
          uint32_t dataLen = 0;
          rStream.SerializeUint32( remoteId );
          rStream.SerializeUint32( dataLen );

          aeId< aeNetData > localId;
          aeNetData* netData = nullptr;
          if ( dataLen
            && m_remoteToLocalIdMap.TryGet( remoteId, &localId )
            && m_netDatas.TryGet( localId, &netData ) )
          {
            if ( rStream.GetRemaining() >= dataLen )
            {
              netData->m_SetClientData( rStream.PeakData(), dataLen );
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

aeRef< aeNetData > aeNetReplicaClient::PumpCreated()
{
  if ( !m_created.Length() )
  {
    return aeRef< aeNetData >();
  }

  aeRef< aeNetData > created = m_created[ 0 ];
  AE_ASSERT( created );
  m_created.Remove( 0 );
  return created;
}

void aeNetReplicaClient::m_CreateNetData( aeBinaryStream* rStream )
{
  AE_ASSERT( rStream->IsReader() );

  uint32_t netDataId = 0;
  uint32_t type = 0;
  rStream->SerializeUint32( netDataId );
  rStream->SerializeUint32( type );

  aeNetData* netData = aeAlloc::Allocate< aeNetData >( type );
  m_netDatas.Set( netData->GetId(), netData );
  m_remoteToLocalIdMap.Set( netDataId, netData->GetId() );

  rStream->SerializeArray( netData->m_initData );

  m_created.Append( netData );
}

//------------------------------------------------------------------------------
// aeNetReplicaServer member functions
//------------------------------------------------------------------------------
void aeNetReplicaServer::UpdateSendData()
{
  AE_ASSERT( m_owner );
  if ( m_pendingClear )
  {
    m_sendData.Clear();
    m_pendingClear = false;
  }

  aeBinaryStream wStream = aeBinaryStream::Writer( &m_sendData );
  wStream.SerializeRaw( aeNetReplicaServer::EventType::Update );
  wStream.SerializeUint32( m_owner->GetNetDataCount() );
  for ( uint32_t i = 0; i < m_owner->GetNetDataCount(); i++ )
  {
    aeNetData* netData = m_owner->GetNetData( i );
    wStream.SerializeUint32( netData->GetId().GetInternalId() );
    wStream.SerializeUint32( netData->Length() );
    wStream.SerializeRaw( netData->Get(), netData->Length() );
  }

  m_pendingClear = true;
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
aeNetData* aeNetReplicaDB::CreateNetData( uint32_t type, const uint8_t* initData, uint32_t initDataLength )
{
  aeNetData* netData = aeAlloc::Allocate< aeNetData >( type );
  netData->m_SetLocal();
  netData->m_initData.Append( initData, initDataLength );
  m_netDatas.Set( netData->GetId(), netData );

  for ( uint32_t i = 0; i < m_servers.Length(); i++ )
  {
    aeNetReplicaServer* server = m_servers[ i ];
    if ( server->m_pendingClear )
    {
      server->m_sendData.Clear();
      server->m_pendingClear = false;
    }

    aeBinaryStream wStream = aeBinaryStream::Writer( &server->m_sendData );
    wStream.SerializeRaw( aeNetReplicaServer::EventType::Create );
    wStream.SerializeUint32( netData->GetId().GetInternalId() );
    wStream.SerializeUint32( type );
    wStream.SerializeArray( netData->m_initData );
  }

  return netData;
}

void aeNetReplicaDB::DestroyNetData( aeNetData* netData )
{
  if ( !netData )
  {
    return;
  }

  aeId< aeNetData > id = netData->GetId();
  AE_ASSERT_MSG( m_netDatas.Remove( id ), "aeNetData was not found." );

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
    wStream.SerializeUint32( id.GetInternalId() );
  }

  aeAlloc::Release( netData );
}

aeNetReplicaServer* aeNetReplicaDB::CreateServer()
{
  aeNetReplicaServer* server = m_servers.Append( aeAlloc::Allocate< aeNetReplicaServer >() );
  AE_ASSERT( !server->m_pendingClear );
  server->m_owner = this;

  // Send initial net datas
  aeBinaryStream wStream = aeBinaryStream::Writer( &server->m_sendData );
  wStream.SerializeRaw( aeNetReplicaServer::EventType::Connect );
  wStream.SerializeUint32( m_netDatas.Length() );
  for ( uint32_t i = 0; i < m_netDatas.Length(); i++ )
  {
    const aeNetData* netData = m_netDatas.GetValue( i );
    wStream.SerializeUint32( netData->GetId().GetInternalId() );
    wStream.SerializeUint32( netData->GetType() );
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
    aeAlloc::Release( server );
  }
}

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
//AE_META_CLASS( aeRpc );

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
//       HVN_ASSERT( aeMetaGetObjectTypeId( go ) == info.type );
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
  
//   aeDict dict;
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

//         const aeMeta::Type* metaType = aeMeta::GetType( msg.typeId );
//         if ( metaType && metaType->GetBaseType() == aeMeta::GetType< Rpc >() )
//         {
//           Rpc* rpc = aeCast< Rpc >( metaType->New() );
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
        
//         const aeMeta::Type* metaType = aeMeta::GetType( createMsg.type );
//         HVN_ASSERT_MSG( metaType, "No type %d", createMsg.type );
//         HVN_LOG( "Net obj create t:%s x:%d y:%d", metaType->GetName(), createMsg.x, createMsg.y );

//         aeDict dict;
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

//   const aeMeta::Type* metaType = aeMeta::GetType( rpc );
//   HVN_ASSERT( metaType );
//   msg.typeId = metaType->GetId();

//   BinaryStream wStream( BinaryStream::WriteBuffer, msg.data, sizeof(msg.data) );
//   rpc->Serialize( &wStream );
//   msg.length = wStream.GetOffset();

//   AetherClient_QueueSend( aether, kMsgRpc, true, msg );
// }