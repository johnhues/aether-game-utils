//------------------------------------------------------------------------------
// ReplicationClient.cpp
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
#include "aeClock.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeNet.h"
#include "aeRender.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kReplicaInfoMsg = 1;

//------------------------------------------------------------------------------
// Test classes
//------------------------------------------------------------------------------
class Green
{
public:
  void Update( float dt )
  {
    aeFloat3 netPos;
    float netRotation;

    aeBinaryStream binaryStream( aeBinaryStream::ReadBuffer, netData->Get(), netData->Length() );
    binaryStream.Serialize( netPos );
    binaryStream.Serialize( netRotation );

    if ( binaryStream.IsValid() )
    {
      pos = aeMath::DtLerp( pos, 1.0f, dt, netPos );
      rotation = aeMath::DtLerp( rotation, 1.0f, dt, netRotation );
    }
  }

  aeFloat3 pos;
  aeFloat3 size;
  float rotation;

  aeRef< aeNetData > netData;
};

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRenderer renderer;
	aeInput input;
	AetherClient* client;
  aeNetReplicaClient netReplication;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "Replication Client" );
	renderer.Initialize( &window, 400, 300 );
	renderer.SetClearColor( aeColor::Red );
	input.Initialize( &window, &renderer );
	client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

  aeArray< Green > greens;

	while ( !input.GetState()->esc )
	{
		input.Pump();
		if ( !client->IsConnected() && !client->IsConnecting() )
		{
			AE_LOG( "Connecting to server" );
			AetherClient_Connect( client );
		}
		
		ReceiveInfo receiveInfo;
		while ( AetherClient_Receive( client, &receiveInfo ) )
		{
			switch ( receiveInfo.msgId )
			{
				case kSysMsgServerConnect:
					AE_LOG( "Connected to server" );
					break;
				case kSysMsgServerDisconnect:
					AE_LOG( "Disconnected from server" );
					break;
        case kReplicaInfoMsg:
          netReplication.Receive( receiveInfo.data, receiveInfo.length );
				default:
					break;
			}
		}

    aeNetReplicaEvent replicaEvent;
    while ( netReplication.Pump( &replicaEvent ) )
    {
      switch ( replicaEvent.type )
      {
      case aeNetReplicaEventType::Create:
        break;
      case aeNetReplicaEventType::Destroy:
        break;
      case aeNetReplicaEventType::Update:
        break;
      }
    }

		if ( input.GetState()->space && !input.GetPrevState()->space )
		{
			AE_LOG( "Send ping to server" );
			
			char msg[] = "ping";
			AetherClient_QueueSend( client, 5, true, msg );
		}

    for ( uint32_t i = 0; i < greens.Length(); i++ )
    {
      greens[ i ].Update( timeStep.GetTimeStep() );
    }

		AetherClient_SendAll( client );

		renderer.StartFrame();
		renderer.EndFrame();

		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	AetherClient_Delete( client );
	client = nullptr;
	input.Terminate();
	renderer.Terminate();
	window.Terminate();

	return 0;
}
