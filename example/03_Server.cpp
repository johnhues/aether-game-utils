//------------------------------------------------------------------------------
// 03_Server.cpp
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
#include "ae/aetherEXT.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	AetherServer* server;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "server" );
	render.InitializeOpenGL( &window );
	input.Initialize( &window );
	server = AetherServer_New( 3500, 0, 1 );
	
	ae::TimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.GetState()->exit )
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
				case 666:
				{
					if ( receiveInfo.data.Length() )
					{
						AE_LOG( "Received (#) '#'", receiveInfo.player->uuid, &receiveInfo.data[ 0 ] );

						char msg[] = "pong";
						AE_LOG( "Send (#) '#'", receiveInfo.player->uuid, msg );
						AetherServer_QueueSendToPlayer( server, receiveInfo.player, 777, true, msg, sizeof(msg) );
					}
					break;
				}
				default:
					break;
			}
		}
		
		AetherServer_SendAll( server );

		render.Activate();
		render.Clear( aeColor::PicoDarkPurple() );
		render.Present();

		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	AetherServer_Delete( server );
	server = nullptr;
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
