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
#include "aether.h"

const ae::Tag kClientAllocTag = "client";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::Socket conn = kClientAllocTag;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "client" );
	render.Initialize( &window );
	input.Initialize( &window );
	
	ae::TimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	bool wasConnected = false;
	while ( !input.quit )
	{
		input.Pump();

		if ( !conn.IsConnected() && conn.Connect( ae::Socket::Protocol::TCP, "localhost", 7230 ) )
		{
			AE_LOG( "Connected to '#(#):#'",
				conn.GetAddress(),
				conn.GetResolvedAddress(),
				conn.GetPort() );
			wasConnected = true;
		}
		
		uint16_t messageLen = 0;
		uint8_t messageData[ 64 ];
		while ( ( messageLen = conn.ReceiveMsg( messageData, sizeof(messageData) ) ) )
		{
			if ( messageLen > sizeof( messageData ) )
			{
				AE_LOG( "Received unexpected large message. Disconnect." );
				conn.Disconnect();
				break;
			}

			ae::Str32 msg;
			ae::BinaryStream rStream = ae::BinaryStream::Reader( messageData, messageLen );
			rStream.SerializeString( msg );
			AE_INFO( "Received '#'", msg );
		}
		
		if ( wasConnected && !conn.IsConnected() )
		{
			AE_INFO( "Disconnected" );
			wasConnected = false;
		}

		if ( input.Get( ae::Key::Space ) && !input.GetPrev( ae::Key::Space ) )
		{
			ae::Str32 msg = "ping";
			uint8_t messageData[ 64 ];
			ae::BinaryStream wStream = ae::BinaryStream::Writer( messageData, sizeof(messageData) );
			wStream.SerializeString( msg );
			if ( conn.QueueMsg( wStream.GetData(), wStream.GetOffset() ) )
			{
				AE_INFO( "Send '#'", msg );
			}
			else
			{
				AE_INFO( "Not connected" );
			}
		}

		conn.SendAll();
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		render.Present();
		timeStep.Tick();
	}

	AE_LOG( "Terminate" );

	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
