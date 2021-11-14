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
#include "ae/aether.h"
#include "Common.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_LOG( "Initialize" );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::Socket conn;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "client" );
	render.Initialize( &window );
	input.Initialize( &window );
	
	ae::TimeStep timeStep;
	timeStep.SetTimeStep( 1.0f / 60.0f );

	while ( !input.quit )
	{
		input.Pump();
		if ( !conn.IsConnected() && !conn.IsConnecting() )
		{
			AE_LOG( "Connecting to server" );
			conn.Connect( ae::Socket::Protocol::TCP, "localhost", "3500" );
		}
		
		uint16_t messageLen = 0;
		uint8_t messageData[ 64 ];
		while ( messageLen = conn.ReceiveMsg( messageData, sizeof(messageData) ) )
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

		if ( input.Get( ae::Key::Space ) && !input.GetPrev( ae::Key::Space ) )
		{
			ae::Str32 msg = "ping";
			uint8_t messageData[ 64 ];
			ae::BinaryStream wStream = ae::BinaryStream::Writer( messageData, messageLen );
			wStream.SerializeString( msg );
			conn.SendMsg( wStream.GetData(), wStream.GetLength() );
			AE_INFO( "Send '#'", msg );
		}

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		render.Present();
		timeStep.Wait();
	}

	AE_LOG( "Terminate" );

	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
