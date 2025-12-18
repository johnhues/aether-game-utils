//------------------------------------------------------------------------------
// 26_Gamepad.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_INFO( "Initialize" );
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::DebugLines debug = TAG_EXAMPLE;
	window.Initialize( 640, 640, false, true, true );
	window.SetTitle( "Gamepad" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debug.Initialize( 16384 );
	
	AE_INFO( "Run" );
	auto Update = [&]()
	{
		input.Pump();
		render.Activate();
		render.Clear( ae::Color::AetherDarkGray() );

		auto DrawGamepad = [&]( float ox, float oy, const ae::GamepadState* gamepad )
		{
			const ae::Color inactiveColor = gamepad->connected ? ae::Color::AetherBlack() : ae::Color::AetherGray();
			auto DrawButton = [&]( float x, float y, float radius, bool pressed )
			{
				x -= 0.5f;
				y = ( 1.0f - y ) - 0.5f;
				debug.AddCircle( ae::Vec3( x, y, 0.0f ), ae::Vec3( 0, 0, 1 ), radius, pressed ? ae::Color::AetherGreen() : inactiveColor, 64 );
			};
			auto DrawSquareButton = [&]( float x, float y, ae::Vec2 halfSize, float pressed )
			{
				x -= 0.5f;
				y = ( 1.0f - y ) - 0.5f;
				const float r = ae::Min( halfSize.x, halfSize.y ) * 0.4f;
				const ae::Color color = ( pressed > 0.0f ) ? ae::Color::AetherDarkGray().Lerp( ae::Color::AetherGreen(), pressed ) : inactiveColor;
				debug.AddRect( ae::Vec3( x, y, 0.0f ), ae::Vec3( 0, 1, 0 ), ae::Vec3( 0, 0, 1 ), halfSize, r, 8, color );
			};
			auto DrawJoystick = [&]( float x, float y, ae::Vec2 dir, bool pressed )
			{
				x -= 0.5f;
				y = ( 1.0f - y ) - 0.5f;
				const float length = ae::Clip01( dir.Length() );
				const ae::Color color = inactiveColor.Lerp( ae::Color::AetherGreen(), ae::Max( (float)pressed, length ) );
				debug.AddCircle( ae::Vec3( x, y, 0.0f ), ae::Vec3( 0, 0, 1 ), 0.075f, inactiveColor, 64 );
				debug.AddCircle( ae::Vec3( x + dir.x * 0.025f, y + dir.y * 0.025f, 0.0f ), ae::Vec3( 0, 0, 1 ), 0.05f, color, 64 );
				debug.AddCircle( ae::Vec3( x + dir.x * 0.025f, y + dir.y * 0.025f, 0.0f ), ae::Vec3( 0, 0, 1 ), 0.04f, color, 64 );
			};
			[&]( float x, float y, float radius )
			{
				x -= 0.5f;
				y = ( 1.0f - y ) - 0.5f;
				const float t = ( ae::GetTime() - (uint32_t)ae::GetTime() );
				const ae::Color color = gamepad->connected ? ( ae::Color::AetherDarkGray().Lerp( ae::Color::AetherGreen(), ( 1.0f - t ) ) ) : inactiveColor;
				debug.AddCircle( ae::Vec3( x, y, 0.0f ), ae::Vec3( 0, 0, 1 ), radius, color, 64 );
				if( gamepad->connected )
				{
					debug.AddCircle( ae::Vec3( x, y, 0.0f ), ae::Vec3( 0, 0, 1 ), radius * t, ae::Color::AetherGreen(), 64 );
				}
			}( ox + 0.5f, oy + 0.25f, 0.045f );
			DrawSquareButton( ox + 0.5f, oy + 0.38f, ae::Vec2( 0.4f, 0.34f ), false );
			DrawButton( ox + 0.57f, oy + 0.37f, 0.021f, gamepad->start );
			DrawButton( ox + 0.43f, oy + 0.37f, 0.021f, gamepad->select );
			DrawButton( ox + 0.75f, oy + 0.43f, 0.035f, gamepad->a );
			DrawButton( ox + 0.82f, oy + 0.37f, 0.035f, gamepad->b );
			DrawButton( ox + 0.68f, oy + 0.37f, 0.035f, gamepad->x );
			DrawButton( ox + 0.75f, oy + 0.31f, 0.035f, gamepad->y );
			DrawButton( ox + 0.38f, oy + 0.54f, 0.085f, false );
			DrawSquareButton( ox + 0.38f, oy + 0.48f + 0.01f, ae::Vec2( 0.025f ), ( gamepad->dpad.y == 1 ) ? 1.0f : 0.0f );
			DrawSquareButton( ox + 0.38f, oy + 0.6f - 0.01f, ae::Vec2( 0.025f ), ( gamepad->dpad.y == -1 ) ? 1.0f : 0.0f );
			DrawSquareButton( ox + 0.32f + 0.01f, oy + 0.54f, ae::Vec2( 0.025f ), ( gamepad->dpad.x == -1 ) ? 1.0f : 0.0f );
			DrawSquareButton( ox + 0.44f - 0.01f, oy + 0.54f, ae::Vec2( 0.025f ), ( gamepad->dpad.x == 1 ) ? 1.0f : 0.0f );
			DrawJoystick( ox + 0.25f, oy + 0.35f, gamepad->leftAnalog, gamepad->leftAnalogClick );
			DrawJoystick( ox + 0.62f, oy + 0.52f, gamepad->rightAnalog, gamepad->rightAnalogClick );
			DrawSquareButton( ox + 0.23f + 0.04f, oy + 0.2f, ae::Vec2( 0.07f, 0.025f ), gamepad->leftBumper ? 1.0f : 0.0f );
			DrawSquareButton( ox + 0.77f - 0.04f, oy + 0.2f, ae::Vec2( 0.07f, 0.025f ), gamepad->rightBumper ? 1.0f : 0.0f );
			DrawSquareButton( ox + 0.22f + 0.04f, oy + 0.12f, ae::Vec2( 0.045f, 0.04f ), gamepad->leftTrigger );
			DrawSquareButton( ox + 0.78f - 0.04f, oy + 0.12f, ae::Vec2( 0.045f, 0.04f ), gamepad->rightTrigger );
		};

		DrawGamepad( -0.5f, -0.5f, &input.gamepads[ 0 ] );
		DrawGamepad( 0.5f, -0.5f, &input.gamepads[ 1 ] );
		DrawGamepad( -0.5f, 0.5f, &input.gamepads[ 2 ] );
		DrawGamepad( 0.5f, 0.5f, &input.gamepads[ 3 ] );

		const ae::Vec3 orthoScale = ( render.GetAspectRatio() > 1.0f ) ? ae::Vec3( 1.0f / render.GetAspectRatio(), 1.0f, 1.0f ) : ae::Vec3( 1.0f, render.GetAspectRatio(), 1.0f );
		const ae::Matrix4 worldToProj = ae::Matrix4::Scaling( orthoScale );
		debug.Render( worldToProj );
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while( Update() );
#endif

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();
	return 0;
}
