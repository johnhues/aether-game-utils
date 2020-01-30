//------------------------------------------------------------------------------
// ReplicationServer.cpp
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
// main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRenderer renderer;
	aeInput input;
	AetherServer* server;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "Replication Server" );
	renderer.Initialize( &window, 400, 300 );
	renderer.SetClearColor( aeColor::Red );
	input.Initialize( &window, &renderer );
	server = AetherServer_New( 3500, 0 );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->esc )
	{
		input.Pump();
		AetherServer_Update( server );
		
		ServerReceiveInfo receiveInfo;
		while ( AetherServer_Receive( server, &receiveInfo ) )
		{
			switch ( receiveInfo.msgId )
			{
				case kSysMsgPlayerConnect:
				{
					AE_LOG( "Player # connected", receiveInfo.player->uuid );
					break;
				}
				case kSysMsgPlayerDisconnect:
				{
					AE_LOG( "Player # disconnected", receiveInfo.player->uuid );
					break;
				}
				default:
				{
					char recvData[ kMaxMessageSize + 1 ];
					AE_ASSERT( receiveInfo.length <= sizeof(recvData) );
					memcpy( recvData, receiveInfo.data, receiveInfo.length );
					recvData[ receiveInfo.length ] = 0;
					AE_LOG( "Received (#) '#'", receiveInfo.player->uuid, recvData );
					
					char msg[] = "pong";
					AetherServer_QueueSendToPlayer( server, 5, true, msg, receiveInfo.player );
					
					break;
				}
			}
		}
		
		AetherServer_SendAll( server );

		renderer.StartFrame();
		renderer.EndFrame();

		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	AetherServer_Delete( server );
	server = nullptr;
	input.Terminate();
	renderer.Terminate();
	window.Terminate();

	return 0;
}
