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
#include "aether.h"

const ae::Tag kServerAllocTag = "client";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::ListenerSocket listener = kServerAllocTag;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "server" );
	render.Initialize( &window );
	input.Initialize( &window );
	
	ae::TimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.quit )
	{
		input.Pump();

		if ( !listener.IsListening() )
		{
			if ( listener.Listen( ae::Socket::Protocol::TCP, false, 7230, 2 ) )
			{
				AE_LOG( "Listening for connections on port '#'", listener.GetPort() );
			}
		}
		while ( ae::Socket* newConn = listener.Accept() )
		{
			AE_LOG( "New connection established from #:#", newConn->GetAddress(), newConn->GetPort() );
		}
		
		for ( uint32_t i = 0; i < listener.GetConnectionCount(); i++ )
		{
			ae::Socket* conn = listener.GetConnection( i );

			// Handle messages
			uint16_t messageLen = 0;
			uint8_t messageData[ 64 ];
			while ( ( messageLen = conn->ReceiveMsg( messageData, sizeof(messageData) ) ) )
			{
				if ( messageLen > sizeof( messageData ) )
				{
					AE_LOG( "Received # unexpected large message. Disconnect.", 0 );
					conn->Disconnect();
					break;
				}
				
				// Receive message data
				ae::Str32 recvMsg;
				ae::BinaryStream rStream = ae::BinaryStream::Reader( messageData, messageLen );
				rStream.SerializeString( recvMsg );
				
				// Send response
				ae::Str32 sendMsg = "pong";
				ae::BinaryStream wStream = ae::BinaryStream::Writer( messageData, sizeof(messageData) );
				wStream.SerializeString( sendMsg );
				if ( conn->QueueMsg( wStream.GetData(), wStream.GetOffset() ) )
				{
					AE_INFO( "Received '#'. Send '#'.", recvMsg, sendMsg );
				}
			}
			conn->SendAll();
			
			if ( !conn->IsConnected() )
			{
				AE_LOG( "Connection  from #:# terminated", conn->GetAddress(), conn->GetPort() );
				listener.Destroy( conn );
			}
		}

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		render.Present();

		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	listener.DestroyAll();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
