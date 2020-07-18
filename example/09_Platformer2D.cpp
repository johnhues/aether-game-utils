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
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeSparseGrid.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// HotSpotObject class
//------------------------------------------------------------------------------
class HotSpotObject
{
public:
};

//------------------------------------------------------------------------------
// HotSpotWorld class
//------------------------------------------------------------------------------
class HotSpotWorld
{
public:
  void Initialize();
  void Update();

  // 1) Tiles - Setup tile properties and enable collision
  void SetTileProperties( uint32_t type, uint32_t mask );
  uint32_t GetTileProperties( uint32_t type );
  void SetCollisionMask( uint32_t mask );

  // 2) Map - Populate map tiles
  void LoadTiles( const uint32_t* tiles, uint32_t width, uint32_t height, bool flipVertical );
  void SetTile( aeInt2 pos, uint32_t type );
  uint32_t GetTile( aeInt2 pos ) const;

  // 3) Objects - Create objects for simulation
  HotSpotObject* CreateObject();

private:
  aeSparseGrid< aeSparseGridZone< uint32_t, 16, 16, 1 > > m_tiles;

  aeMap< uint32_t, uint32_t > m_tileProperties;
  uint32_t m_collisionMask = 0;

  aeArray< HotSpotObject* > m_objects;
};

//------------------------------------------------------------------------------
// HotSpotWorld member functions
//------------------------------------------------------------------------------
void HotSpotWorld::Initialize()
{
}

void HotSpotWorld::Update()
{
}

void HotSpotWorld::SetTileProperties( uint32_t type, uint32_t mask )
{
}

uint32_t HotSpotWorld::GetTileProperties( uint32_t type )
{
  return m_tileProperties.Get( type, 0 );
}

void HotSpotWorld::SetCollisionMask( uint32_t mask )
{
  m_collisionMask = mask;
}

void HotSpotWorld::LoadTiles( const uint32_t* tiles, uint32_t width, uint32_t height, bool flipVertical )
{
  for ( uint32_t y = 0; y < height; y++ )
  {
    for ( uint32_t x = 0; x < width; x++ )
    {
      uint32_t y1 = flipVertical ? height - 1 - y : y;
      m_tiles.Set( aeInt3( x, y1, 0 ), tiles[ y * width + x ] );
    }
  }
}

void HotSpotWorld::SetTile( aeInt2 pos, uint32_t type )
{
  m_tiles.Set( aeInt3( pos, 0 ), type );
}

uint32_t HotSpotWorld::GetTile( aeInt2 pos ) const
{
  const uint32_t* type = m_tiles.TryGet( aeInt3( pos, 0 ) );
  return type ? *type : 0;
}

HotSpotObject* HotSpotWorld::CreateObject()
{
  return nullptr;
}

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kTileMask_Open = 0;
const uint32_t kTileMask_Collision = 1 >> 0;
const uint32_t kTileMask_Kill = 1 >> 1;

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
  window.SetTitle( "Platformer 2D" );  render.InitializeOpenGL( &window, 400, 300 );
  render.SetClearColor( aeColor::Red() );
  input.Initialize( &window, &render );
  spriteRender.Initialize( 512 );
  spriteRender.SetBlending( true );
  spriteRender.SetDepthTest( true );
  spriteRender.SetDepthWrite( true );
  spriteRender.SetSorting( true );

  HotSpotWorld world;
  world.Initialize();
  world.SetTileProperties( 0, kTileMask_Kill ); // By default all tiles will kill player
  world.SetTileProperties( 1, kTileMask_Open );
  world.SetTileProperties( 2, kTileMask_Collision );
  const uint32_t kMapData[] =
  {
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,
    2,1,1,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,2,
    2,1,1,1,1,1,2,2,1,1,2,2,1,1,1,1,1,1,2,2,2,2,2,1,1,1,1,2,
    2,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,
  };
  const uint32_t kMapWidth = 33;
  const uint32_t kMapHeight = 12;
  world.LoadTiles( kMapData, kMapWidth, kMapHeight, true );
  
  aeFixedTimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );

  aeTexture2D tex;
  tex.Initialize( "circle.png", aeTextureFilter::Linear, aeTextureWrap::Repeat );

  float scale = 10.0f;
  aeFloat2 camera( 0.0f );

  while ( !input.GetState()->exit )
  {
    input.Pump();
    render.StartFrame();
    spriteRender.Clear();

    if ( input.GetState()->left ) { camera.x -= 3.0f * timeStep.GetTimeStep(); }
    if ( input.GetState()->right ) { camera.x += 3.0f * timeStep.GetTimeStep(); }
    if ( input.GetState()->up ) { camera.y += 3.0f * timeStep.GetTimeStep(); }
    if ( input.GetState()->down ) { camera.y -= 3.0f * timeStep.GetTimeStep(); }

    aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / scale, render.GetAspectRatio() / scale, 1.0f ) );
    screenTransform.Translate( aeFloat3( -camera.x, -camera.y, 0.0f ) );

    aeFloat4x4 transform;

    transform = aeFloat4x4::Translation( aeFloat3( 0.5f, 0.5f, -0.5f ) );
    transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
    spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Blue() );

    transform = aeFloat4x4::Translation( aeFloat3( -0.5f, -0.5f, 0.5f ) );
    transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
    spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Blue() );

    transform = aeFloat4x4::Scaling(  aeFloat3( 1.0f, 1.0f, 0.5f ) );
    spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::White() );

    for ( uint32_t y = 0; y < kMapHeight; y++ )
    {
      for ( uint32_t x = 0; x < kMapWidth; x++ )
      {
        aeColor color;
        switch ( world.GetTile( aeInt2( x, y ) ) )
        {
          case 0:
            color = aeColor::PicoRed();
            break;
          case 1:
            color = aeColor::PicoPeach();
            break;
          case 2:
            color = aeColor::PicoOrange();
            break;
          default:
            color = aeColor::PicoPink();
            break;
        }
        aeColor::PicoDarkPurple();
        transform = aeFloat4x4::Translation( aeFloat3( x, y, 0.5f ) );
        transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
        spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), color );
      }
    }

    spriteRender.Render( screenTransform );
    render.EndFrame();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
