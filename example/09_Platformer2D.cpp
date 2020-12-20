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
#include "aeClock.h"
#include "aeHotSpot.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Player class
//------------------------------------------------------------------------------
class Player
{
public:
  Player( HotSpotWorld* world, aeFloat2 startPos );
  void OnCollision( const HotSpotObject::CollisionInfo* info );
  void Update( HotSpotWorld* world, aeInput* input, float dt );

  void Render( aeSpriteRender* spriteRender, aeTexture2D* tex );
  aeFloat2 GetPosition() const { return m_body->GetPosition(); }
  bool CanJump() const { return m_canJumpTimer > 0.0f; }

private:
  HotSpotObject* m_body = nullptr;
  float m_canJumpTimer = 0.0f;
  float m_jumpHoldTimer = 0.0f;
};

//------------------------------------------------------------------------------
// Player member functions
//------------------------------------------------------------------------------
Player::Player( HotSpotWorld* world, aeFloat2 startPos )
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

void Player::Update( HotSpotWorld* world, aeInput* input, float dt )
{
  uint32_t tile = world->GetTile( HotSpotWorld::_GetTilePos( m_body->GetPosition() ) );
  
  const InputState* inputState = input->GetState();
  bool up = inputState->up || inputState->leftAnalog.y > 0.1f || inputState->dpad.y > 0;
  bool down = inputState->down || inputState->leftAnalog.y < -0.1f || inputState->dpad.y < 0;
  bool left = inputState->left || inputState->leftAnalog.x < -0.1f || inputState->dpad.x < 0;
  bool right = inputState->right || inputState->leftAnalog.x > 0.1f || inputState->dpad.x > 0;
  bool jumpButton = ( inputState->space || inputState->up || inputState->a );

  m_canJumpTimer -= dt;

  // Water
  if ( tile == kTile_Water )
  {
    if ( up || jumpButton ) { m_body->AddForce( aeFloat2( 0.0f, kPlayerMass * kSwimUp ) ); }
    if ( down ) { m_body->AddForce( aeFloat2( 0.0f, -kPlayerMass * kSwimDown ) ); }

    if ( left ) { m_body->AddForce( aeFloat2( -kPlayerMass * kSwimHorizontal, 0.0f ) ); }
    if ( right ) { m_body->AddForce( aeFloat2( kPlayerMass * kSwimHorizontal, 0.0f ) );  }

    // Always reset jump so a jump is possible immediately after leaving water
    m_canJumpTimer = kJumpMaxAirTime;
    m_jumpHoldTimer = 0.0f;
  }
  else // Air / ground
  {
    if ( left ) { m_body->AddForce( aeFloat2( -kPlayerMass * kMoveHorizontal, 0.0f ) ); }
    if ( right ) { m_body->AddForce( aeFloat2( kPlayerMass * kMoveHorizontal, 0.0f ) ); }

    if ( CanJump() && jumpButton )
    {
      // Cancel previous downward velocity for last kJumpMaxAirTime seconds
      // to get full jump height
      aeFloat2 vel = m_body->GetVelocity();
      vel.y = 0.0f;
      m_body->SetVelocity( vel );

      m_canJumpTimer = 0.0f;
      m_jumpHoldTimer = kJumpHoldTimeMax;
      m_body->AddImpulse( aeFloat2( 0.0f, kPlayerMass * kJumpInitial ) );
    }

    if ( m_jumpHoldTimer > 0.0f && jumpButton )
    {
      m_jumpHoldTimer -= dt;
      m_body->AddForce( aeFloat2( 0.0f, kPlayerMass * kJumpHold ) );
    }
  }

  m_body->AddGravity( aeFloat2( 0.0f, -kGravity ) );
}

void Player::Render( aeSpriteRender* spriteRender, aeTexture2D* tex )
{
  aeFloat4x4 transform = aeFloat4x4::Translation( aeFloat3( GetPosition(), -0.5f ) );
  transform.Scale( aeFloat3( 1.0f, 1.0f, 1.0f ) );
  spriteRender->AddSprite( tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), CanJump() ? aeColor::PicoRed() : aeColor::PicoBlue() );
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  aeWindow window;
  aeRender render;
  aeInput input;
  aeSpriteRender spriteRender;
  
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "Platformer 2D" );  render.InitializeOpenGL( &window );
  input.Initialize( &window );
  spriteRender.Initialize( 512 );
  spriteRender.SetBlending( true );
  spriteRender.SetDepthTest( true );
  spriteRender.SetDepthWrite( true );
  spriteRender.SetSorting( true );

  aeFixedTimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / kFramesPerSecond );

  //------------------------------------------------------------------------------
  // Tile map
  //------------------------------------------------------------------------------
  HotSpotWorld world;
  world.Initialize( 1.0f / kSimulationStepsPerSecond );
  world.SetCollisionMask( kTileMask_Collision );
  world.SetTileProperties( kTile_Air, kTileMask_Open );
  world.SetTileFluidDensity( kTile_Air, kAirDensity );
  world.SetTileProperties( kTile_Wall, kTileMask_Collision );
  world.SetTileProperties( kTile_Water, kTileMask_Open );
  world.SetTileFluidDensity( kTile_Water, kWaterDensity );
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
  AE_STATIC_ASSERT( countof(kMapData) == kMapWidth * kMapHeight );
  world.LoadTiles( kMapData, kMapWidth, kMapHeight, true );
  
  //------------------------------------------------------------------------------
  // Textures
  //------------------------------------------------------------------------------
  aeTexture2D tex;
  uint8_t texData[] = { 255, 255, 255 };
  tex.Initialize( texData, 1, 1, aeTextureFormat::RGB8, aeTextureType::Uint8, aeTextureFilter::Nearest, aeTextureWrap::Clamp );

  //------------------------------------------------------------------------------
  // Game loop
  //------------------------------------------------------------------------------
  Player player( &world, aeFloat2( 2.0f, 2.0f ) );

  while ( !input.GetState()->exit )
  {
    //------------------------------------------------------------------------------
    // Update
    //------------------------------------------------------------------------------
    input.Pump();
    player.Update( &world, &input, timeStep.GetTimeStep() );
    world.Update( timeStep.GetTimeStep() );
    
    //------------------------------------------------------------------------------
    // Render
    //------------------------------------------------------------------------------
    render.Activate();
    render.Clear( aeColor::PicoDarkBlue() );
    spriteRender.Clear();

    player.Render( &spriteRender, &tex );

    for ( uint32_t y = 0; y < kMapHeight; y++ )
    {
      for ( uint32_t x = 0; x < kMapWidth; x++ )
      {
        aeColor color;
        switch ( world.GetTile( aeInt2( x, y ) ) )
        {
          case kTile_Air: color = aeColor::PicoPeach(); break;
          case kTile_Water: color = aeColor::PicoPink(); break;
          default: color = aeColor::PicoOrange(); break;
        }
        aeFloat4x4 transform = aeFloat4x4::Translation( aeFloat3( x, y, 0.0f ) );
        transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
        spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), color );
      }
    }

    aeFloat2 camera = player.GetPosition();
    aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / 10.0f, render.GetAspectRatio() / 10.0f, 1.0f ) );
    screenTransform.Translate( aeFloat3( -camera.x, -camera.y, 0.0f ) );
    spriteRender.Render( screenTransform );

    render.Present();
    timeStep.Wait();
  }

  //------------------------------------------------------------------------------
  // Cleanup
  //------------------------------------------------------------------------------
  AE_LOG( "Terminate" );

  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
