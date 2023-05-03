//------------------------------------------------------------------------------
// 09_Platformer2D.cpp
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
#include "ae/aeHotSpot.h"
#include "ae/SpriteRenderer.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
// Allocations
const ae::Tag TAG_ALL = "all";

// Frame rate
const uint32_t kFramesPerSecond = 60;
const uint32_t kSimulationStepsPerSecond = 240;

// Tiles
const uint32_t kTileMask_Open = 0;
const uint32_t kTileMask_Collision = 1;

const uint32_t kTile_Air = 0;
const uint32_t kTile_Wall = 1;
const uint32_t kTile_Water = 2;

const float kGravity = 10.0f;
const float kAirDensity = 12.5f;
const float kWaterDensity = 1000.0f;

// Player
const float kPlayerMass = 70.0f;
const float kPlayerDensity = 1050.0f;
// Movement
const float kMoveHorizontal = 4.5f;
// Jump
const float kJumpInitial = 5.5f;
const float kJumpHold = 5.5f;
const float kJumpMaxAirTime = 0.35f;
const float kJumpHoldTimeMax = 0.4f;
// Swim
const float kSwimHorizontal = 6.5f;
const float kSwimUp = 14.0f;
const float kSwimDown = 10.0f;

// Map
#define O kTile_Air
#define B kTile_Wall
#define W kTile_Water
const uint32_t kMapData[] =
{
	B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
	B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,B,B,B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,B,B,B,B,B,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,B,B,B,
	B,O,O,O,O,O,O,O,B,B,B,B,B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
	B,O,O,O,O,O,O,O,O,O,B,B,W,W,W,W,W,W,B,B,B,B,O,O,O,O,O,B,
	B,O,O,O,O,O,B,B,O,O,B,B,W,W,W,W,W,W,B,B,B,B,B,W,W,W,W,B,
	B,O,O,O,O,O,O,O,O,O,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,
	B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
};
#undef O
#undef B
#undef W
const uint32_t kMapWidth = 28;
const uint32_t kMapHeight = 12;

//------------------------------------------------------------------------------
// Player class
//------------------------------------------------------------------------------
class Player
{
public:
	void Initialize( HotSpotWorld* world, ae::Vec2 startPos );
	void OnCollision( const HotSpotObject::CollisionInfo* info );
	void Update( HotSpotWorld* world, ae::Input* input, float dt );

	void Render( SpriteRenderer* spriteRender, ae::Texture2D* tex );
	ae::Vec2 GetPosition() const { return m_body->GetPosition(); }
	bool CanJump() const { return m_canJumpTimer > 0.0f; }

private:
	HotSpotObject* m_body = nullptr;
	float m_canJumpTimer = 0.0f;
	float m_jumpHoldTimer = 0.0f;
};

//------------------------------------------------------------------------------
// Player member functions
//------------------------------------------------------------------------------
void Player::Initialize( HotSpotWorld* world, ae::Vec2 startPos )
{
	m_body = world->CreateObject();
	m_body->SetMass( kPlayerMass );
	m_body->SetVolume( kPlayerMass / kPlayerDensity );
	m_body->onCollision.Add( this, &Player::OnCollision );
	m_body->Warp( startPos );
}

void Player::OnCollision( const HotSpotObject::CollisionInfo* info )
{
	if ( info->normal.y > 0 )
	{
		// Reset jump when touching ground
		m_canJumpTimer = kJumpMaxAirTime;
		m_jumpHoldTimer = 0.0f;
	}
}

void Player::Update( HotSpotWorld* world, ae::Input* input, float dt )
{
	uint32_t tile = world->GetTile( HotSpotWorld::_GetTilePos( m_body->GetPosition() ) );
	
	bool up = input->Get( ae::Key::Up ) || input->gamepad.leftAnalog.y > 0.1f || input->gamepad.dpad.y > 0;
	bool down = input->Get( ae::Key::Down ) || input->gamepad.leftAnalog.y < -0.1f || input->gamepad.dpad.y < 0;
	bool left = input->Get( ae::Key::Left ) || input->gamepad.leftAnalog.x < -0.1f || input->gamepad.dpad.x < 0;
	bool right = input->Get( ae::Key::Right ) || input->gamepad.leftAnalog.x > 0.1f || input->gamepad.dpad.x > 0;
	bool jumpButton = ( up || input->Get( ae::Key::Space ) || input->Get( ae::Key::A ) );

	m_canJumpTimer -= dt;

	// Water
	if ( tile == kTile_Water )
	{
		if ( up || jumpButton ) { m_body->AddForce( ae::Vec2( 0.0f, kPlayerMass * kSwimUp ) ); }
		if ( down ) { m_body->AddForce( ae::Vec2( 0.0f, -kPlayerMass * kSwimDown ) ); }

		if ( left ) { m_body->AddForce( ae::Vec2( -kPlayerMass * kSwimHorizontal, 0.0f ) ); }
		if ( right ) { m_body->AddForce( ae::Vec2( kPlayerMass * kSwimHorizontal, 0.0f ) );  }

		// Always reset jump so a jump is possible immediately after leaving water
		m_canJumpTimer = kJumpMaxAirTime;
		m_jumpHoldTimer = 0.0f;
	}
	else // Air / ground
	{
		if ( left ) { m_body->AddForce( ae::Vec2( -kPlayerMass * kMoveHorizontal, 0.0f ) ); }
		if ( right ) { m_body->AddForce( ae::Vec2( kPlayerMass * kMoveHorizontal, 0.0f ) ); }

		if ( CanJump() && jumpButton )
		{
			// Cancel previous downward velocity for last kJumpMaxAirTime seconds
			// to get full jump height
			ae::Vec2 vel = m_body->GetVelocity();
			vel.y = 0.0f;
			m_body->SetVelocity( vel );

			m_canJumpTimer = 0.0f;
			m_jumpHoldTimer = kJumpHoldTimeMax;
			m_body->AddImpulse( ae::Vec2( 0.0f, kPlayerMass * kJumpInitial ) );
		}

		if ( m_jumpHoldTimer > 0.0f && jumpButton )
		{
			m_jumpHoldTimer -= dt;
			m_body->AddForce( ae::Vec2( 0.0f, kPlayerMass * kJumpHold ) );
		}
	}

	m_body->AddGravity( ae::Vec2( 0.0f, -kGravity ) );
}

void Player::Render( SpriteRenderer* spriteRender, ae::Texture2D* tex )
{
	ae::Matrix4 transform = ae::Matrix4::Translation( ae::Vec3( GetPosition(), -0.5f ) );
	spriteRender->AddSprite( transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), CanJump() ? ae::Color::PicoRed() : ae::Color::PicoBlue() );
}

//------------------------------------------------------------------------------
// Game
//------------------------------------------------------------------------------
struct Game
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	SpriteRenderer spriteRender = TAG_ALL;
	ae::TimeStep timeStep;
	HotSpotWorld world;
	ae::Texture2D tex;
	Player player;

	void Initialize()
	{
		AE_LOG( "Initialize" );
		window.Initialize( 800, 600, false, true );
		window.SetTitle( "Platformer 2D" );
		render.Initialize( &window );
		input.Initialize( &window );
		spriteRender.Initialize( 512 );
		timeStep.SetTimeStep( 1.0f / kFramesPerSecond );

		world.Initialize( 1.0f / kSimulationStepsPerSecond );
		world.SetCollisionMask( kTileMask_Collision );
		world.SetTileProperties( kTile_Air, kTileMask_Open );
		world.SetTileFluidDensity( kTile_Air, kAirDensity );
		world.SetTileProperties( kTile_Wall, kTileMask_Collision );
		world.SetTileProperties( kTile_Water, kTileMask_Open );
		world.SetTileFluidDensity( kTile_Water, kWaterDensity );
		AE_STATIC_ASSERT( countof(kMapData) == kMapWidth * kMapHeight );
		world.LoadTiles( kMapData, kMapWidth, kMapHeight, true );
		
		uint8_t texData[] = { 255, 255, 255 };
		tex.Initialize( texData, 1, 1, ae::Texture::Format::RGB8, ae::Texture::Type::Uint8, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Clamp, false );

		player.Initialize( &world, ae::Vec2( 2.0f, 2.0f ) );
	}


	bool Tick()
	{
		input.Pump();
		player.Update( &world, &input, timeStep.GetDt() );
		world.Update( timeStep.GetDt() );
		
		render.Activate();
		render.Clear( ae::Color::PicoDarkBlue() );
		spriteRender.Clear();

		for ( uint32_t y = 0; y < world.GetHeight(); y++ )
		for ( uint32_t x = 0; x < world.GetWidth(); x++ )
		{
			ae::Color color;
			switch ( world.GetTile( ae::Int2( x, y ) ) )
			{
				case kTile_Air: color = ae::Color::PicoPeach(); break;
				case kTile_Water: color = ae::Color::PicoPink(); break;
				default: color = ae::Color::PicoOrange(); break;
			}
			ae::Matrix4 transform = ae::Matrix4::Translation( ae::Vec3( x, y, 0.0f ) );
			spriteRender.AddSprite( transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), color );
		}
		
		player.Render( &spriteRender, &tex );

		ae::Vec2 camera = player.GetPosition();
		ae::Matrix4 screenTransform = ae::Matrix4::Scaling( ae::Vec3( 1.0f / ( 5.0f * render.GetAspectRatio() ), 1.0f / 5.0f, 1.0f ) );
		screenTransform *= ae::Matrix4::Translation( ae::Vec3( -camera.x, -1.5f - camera.y, 0.0f ) );
		spriteRender.Render( screenTransform, &tex );

		render.Present();
		timeStep.Tick();

		return !input.quit;
	}

	void Terminate()
	{
		AE_LOG( "Terminate" );
		input.Terminate();
		render.Terminate();
		window.Terminate();
	}
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	Game game;
	game.Initialize();
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* game ) { ((Game*)game)->Tick(); }, &game, 0, 1 );
#else
	while ( game.Tick() ) {}
#endif
	game.Terminate();
	return 0;
}
