//------------------------------------------------------------------------------
// 03_Client.cpp
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
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	AetherClient* client;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "client" );
	render.InitializeOpenGL( &window );
	render.SetClearColor( aeColor::Red() );
	input.Initialize( &window );
	client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
	
	aeFixedTimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->exit )
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
				case 777:
					if ( receiveInfo.data.Length() )
					{
						AE_LOG( "Received (#) '#'", client->localPlayer->uuid, (char*)&receiveInfo.data[ 0 ] );
					}
					break;
				default:
					break;
			}
		}

		if ( input.GetState()->space && !input.GetPrevState()->space )
		{
			char msg[] = "ping";
			AE_LOG( "Send (#) '#'", client->localPlayer->uuid, msg );
			AetherClient_QueueSend( client, 666, true, msg, sizeof(msg) );
		}

		AetherClient_SendAll( client );

		render.Activate();
		render.Present();

		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	AetherClient_Delete( client );
	client = nullptr;
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
