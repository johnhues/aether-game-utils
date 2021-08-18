//------------------------------------------------------------------------------
// 12_Geometry.cpp
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
#include "Common.h"

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
	ae::DebugLines debug;
  ae::Texture2D fontTexture;
	aeTextRender text;
	aeEditorCamera camera;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "geometry" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debug.Initialize( 512 );
  LoadPng( &fontTexture, "font.png", ae::Texture::Filter::Linear, ae::Texture::Wrap::Repeat, false, true );
	text.Initialize( &fontTexture, 8 );

	AE_INFO( "Run" );
	while ( !input.quit )
	{
		input.Pump();

		if ( input.Get( ae::Key::F ) && !input.GetPrev( ae::Key::F ) )
		{
			camera.Refocus( ae::Vec3( 0.0f ) );
		}
		camera.Update( &input, timeStep.GetTimeStep() );

		render.Activate();
		render.Clear( aeColor::PicoDarkPurple() );
		
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( ae::QUARTER_PI, render.GetAspectRatio(), 0.25f, 50.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;

		// UI units in pixels, origin in bottom left
		ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );

		ae::Matrix4 worldToUI = textToNdc.GetInverse() * worldToProj;

		auto DrawText = [&]( ae::Vec3 worldPos, const char* str, aeColor color )
		{
			text.Add( ae::Vec3( ae::Vec3::ProjectPoint( worldToUI, worldPos ).GetXY() ), ae::Vec2( text.GetFontSize() * 2.0f ), str, color, 0, 0 );
		};

		// Edit ray direction
		static ae::Vec3 s_raySource( 0.0f );
		static ae::Vec3 s_rayDir( 0.0f );
		static float s_rayLength = 8.0f;
		if ( input.Get( ae::Key::Num1 ) ) s_rayLength -= 0.016f;
		if ( input.Get( ae::Key::Num2 ) ) s_rayLength += 0.016f;
		s_rayLength = aeMath::Clip( s_rayLength, 0.0f, 8.0f );

		if ( input.Get( ae::Key::G ) )
		{
			s_raySource = camera.GetPosition();
			s_rayDir = camera.GetForward();
			s_rayLength = 8.0f;
		}
		ae::Vec3 ray = s_rayDir * s_rayLength;
		ae::Vec3 raySource = s_raySource;

		// Actual geometry calculations / rendering
		static int32_t currentTest = 0;
		if ( input.Get( ae::Key::Left ) && !input.GetPrev( ae::Key::Left ) )
		{
			currentTest--;
		}
		if ( input.Get( ae::Key::Right ) && !input.GetPrev( ae::Key::Right ) )
		{
			currentTest++;
		}
		currentTest = aeMath::Mod( currentTest, 5 );

		aeStr256 infoText = "";
		switch ( currentTest )
		{
			case 0: // Triangle ray
			{
				infoText.Append( "Triangle-Ray\n" );
				
				ae::Vec3 triangle[] =
				{
					ae::Vec3( -1.0f, -1.5f, -1.0f ) * 2.0f,
					ae::Vec3( 1.0f, -0.5f, -1.0f ) * 2.0f,
					ae::Vec3( 0.0f, 1.5f, 1.0f ) * 2.0f,
				};
				ae::Vec3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				ae::Vec3 normal = ( triangle[ 1 ] - triangle[ 0 ] ).Cross( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], aeColor::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], aeColor::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], aeColor::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, aeColor::Red() );

				static float r = 1.0f;
				if ( input.Get( ae::Key::Num3 ) ) r -= 0.016f;
				if ( input.Get( ae::Key::Num4 ) ) r += 0.016f;
				r = aeMath::Clip( r, 0.01f, 8.0f );

				float dist = 0.0f;
				ae::Vec3 nearestIntersectionPoint;
				ae::Vec3 nearestPolygonIntersectionPoint;
				aeSphere sphere( triangleCenter - ray, r );
				debug.AddLine( sphere.center, sphere.center + ray, aeColor::Red() );
				debug.AddSphere( sphere.center, sphere.radius, aeColor::Green(), 16 );
				if ( sphere.SweepTriangle( ray, triangle, normal, &dist, &nearestIntersectionPoint, &nearestPolygonIntersectionPoint, &debug ) )
				{
					debug.AddSphere( sphere.center + ray.SafeNormalizeCopy() * dist, sphere.radius, aeColor::Red(), 16 );

					//debug.AddSphere( nearestIntersectionPoint, 0.05f, aeColor::Green(), 8 );
					//debug.AddSphere( nearestPolygonIntersectionPoint, 0.05f, aeColor::Blue(), 8 );
				}
				break;
			}
			case 1: // Sphere ray
			{
				infoText.Append( "Sphere-Ray\n" );
				
				debug.AddSphere( ae::Vec3( 0.0f ), 1.0f, aeColor::Blue(), 16 );

				ae::Vec3 e = raySource;
				ae::Vec3 d = ray.SafeNormalizeCopy();

				float b = 2.0f * e.Dot( d );
				float c = e.LengthSquared() - 1.0f;

				float discriminant = b * b - 4.0f * c;
				if ( discriminant >= 0.0f )
				{
					discriminant = sqrtf( discriminant );
					float t0 = ( -b + discriminant ) / 2.0f;
					float t1 = ( -b - discriminant ) / 2.0f;

					debug.AddSphere( e + d * t0, 0.05f, aeColor::Red(), 8 );
					debug.AddSphere( e + d * t1, 0.05f, aeColor::Green(), 8 );
				}

				break;
			}
			case 2: // Cylinder ray
			{
				infoText.Append( "Cylinder-Ray\n" );
				
				bool hit0 = false;
				bool hit1 = false;

				ae::Vec3 p0( 0.0f, 0.0f, 2.0f );
				ae::Vec3 p1( 0.0f, 0.0f, -2.0f );

				ae::Vec3 e = raySource;
				ae::Vec3 d = ray;
				ae::Vec2 exy = raySource.GetXY();
				ae::Vec2 dxy = ray.GetXY();
				
				float a = dxy.LengthSquared();
				if ( a > 0.0f )
				{
					float b = 2.0f * exy.Dot( dxy );
					float c = exy.LengthSquared() - 1.0f;

					float discriminant = b * b - 4.0f * a * c;
					if ( discriminant >= 0.0f )
					{
						discriminant = sqrtf( discriminant );
						float t0 = ( -b + discriminant ) / ( 2.0f * a );
						float t1 = ( -b - discriminant ) / ( 2.0f * a );

						ae::Vec3 r0 = e + d * t0;
						ae::Vec3 r1 = e + d * t1;
						debug.AddSphere( r0, 0.05f, aeColor::Red(), 8 );
						debug.AddSphere( r1, 0.05f, aeColor::Red(), 8 );
						DrawText( r0, "t0", aeColor::Red() );
						DrawText( r1, "t1", aeColor::Red() );

						ae::Vec3 z0( 0.0f, 0.0f, r0.z );
						ae::Vec3 z1( 0.0f, 0.0f, r1.z );
						debug.AddSphere( z0, 0.05f, aeColor::Blue(), 8 );
						debug.AddSphere( z1, 0.05f, aeColor::Blue(), 8 );
						DrawText( z0, "z0", aeColor::Blue() );
						DrawText( z1, "z1", aeColor::Blue() );
						debug.AddLine( z0, z1, aeColor::Blue() );

						if ( ( p0.z - z0.z ) * ( p0.z - z1.z ) < 0.0f )
						{
							float t2 = ( p0.z - e.z ) / d.z;
							ae::Vec3 c0 = e + d * t2;
							debug.AddSphere( c0, 0.05f, aeColor::Green(), 8 );
							DrawText( c0, "c0", aeColor::Green() );
							hit0 = true;
						}

						if ( ( p1.z - z0.z ) * ( p1.z - z1.z ) < 0.0f )
						{
							float t2 = ( p1.z - e.z ) / d.z;
							ae::Vec3 c1 = e + d * t2;
							debug.AddSphere( c1, 0.05f, aeColor::Green(), 8 );
							DrawText( c1, "c1", aeColor::Green() );
							hit1 = true;
						}
					}
				}

				debug.AddCircle( p0, p0 - p1, 1.0f, hit0 ? aeColor::Green() : aeColor::Blue(), 16 );
				debug.AddCircle( p1, p1 - p0, 1.0f, hit1 ? aeColor::Green() : aeColor::Blue(), 16 );
				for ( uint32_t i = 0; i < 8; i++ )
				{
					ae::Vec2 offset = ae::Vec2::FromAngle( aeMath::TWO_PI * i / 8.0f );
					debug.AddLine( p0 + ae::Vec3( offset ), p1 + ae::Vec3( offset ), aeColor::Blue() );
				}

				break;
			}
			case 3:
			{
				infoText.Append( "Sphere-Triangle (static)\n" );
				
				ae::Vec3 triangle[] =
				{
					ae::Vec3( -1.0f, -1.5f, -1.0f ) * 2.0f,
					ae::Vec3( 1.0f, -0.5f, -1.0f ) * 2.0f,
					ae::Vec3( 0.0f, 1.5f, 1.0f ) * 2.0f,
				};
				ae::Vec3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				ae::Vec3 normal = ( triangle[ 1 ] - triangle[ 0 ] ).Cross( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], aeColor::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], aeColor::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], aeColor::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, aeColor::Red() );
				
				static float r = 1.0f;
				if ( input.Get( ae::Key::Num3 ) ) r -= 0.016f;
				if ( input.Get( ae::Key::Num4 ) ) r += 0.016f;
				r = aeMath::Clip( r, 0.01f, 8.0f );

				ae::Vec3 nearestIntersectionPoint;
				aeSphere sphere( raySource + ray, r );
				if ( sphere.IntersectTriangle( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ], &nearestIntersectionPoint ) )
				{
					debug.AddSphere( raySource + ray, r, aeColor::Green(), 16 );
					debug.AddSphere( nearestIntersectionPoint, 0.05f, aeColor::Red(), 8 );
					
					ae::Vec3 spherePoint = sphere.center + ( nearestIntersectionPoint - sphere.center ).SafeNormalizeCopy() * r;
					debug.AddSphere( spherePoint, 0.05f, aeColor::Green(), 8 );
					debug.AddLine( nearestIntersectionPoint, spherePoint, aeColor::Green() );
				}
				else
				{
					debug.AddSphere( raySource + ray, r, aeColor::Red(), 16 );
				}
				break;
			}
			case 4:
			{
				infoText.Append( "Plane Ray\n" );
        infoText.Append( "Rotate Normal XY: 1-2\n" );
        infoText.Append( "Rotate Normal Z: 3-4\n" );
        infoText.Append( "Plane Distance from Origin: 5-6\n" );
        infoText.Append( "Cast Ray from Camera: G\n" );
        
        static float a0 = 0.0f;
        static float a1 = ae::QUARTER_PI;
        static float d = 1.0f;
        static ae::Vec3 rayP( 3.0f, 0.0f, 0.0f );
        static ae::Vec3 rayD( -1.0f, 0.0f, 0.0f );
        
        if ( input.Get( ae::Key::Num1 ) ) a0 -= 0.016f;
        if ( input.Get( ae::Key::Num2 ) ) a0 += 0.016f;
        if ( input.Get( ae::Key::Num3 ) ) a1 -= 0.016f;
        if ( input.Get( ae::Key::Num4 ) ) a1 += 0.016f;
        if ( input.Get( ae::Key::Num5 ) ) d -= 0.016f;
        if ( input.Get( ae::Key::Num6 ) ) d += 0.016f;
        if ( input.Get( ae::Key::G ) )
        {
          rayP = camera.GetPosition();
          rayD = camera.GetForward();
        }
				
        ae::Vec2 xy( ae::Cos( a0 ), ae::Sin( a0 ) );
				aePlane plane( ae::Vec4( xy * ae::Cos( a1 ), ae::Sin( a1 ), d ) );
        
        ae::Vec3 p = plane.GetClosestPointToOrigin();
        ae::Vec3 pn = plane.GetClosestPointToOrigin() + plane.GetNormal();
				
        // Reference lines
        if ( d > 0.0f )
        {
          debug.AddLine( ae::Vec3( 0.0f ), p, aeColor::Gray() );
        }
        else if ( d < -1.0f )
        {
          debug.AddLine( ae::Vec3( 0.0f ), pn, aeColor::Gray() );
        }
        aeAABB aabb( ae::Vec3( 0.0f ), p );
        debug.AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), aeColor::Gray() );
        
        // Axis lines
        debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ), aeColor::Red() );
        debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), aeColor::Green() );
        debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), aeColor::Blue() );
        
        // Plane
        debug.AddCircle( p, plane.GetNormal(), 0.25f, aeColor::PicoPink(), 16 );
        debug.AddLine( p, pn, aeColor::PicoPink() );
        
        // Ray
        ae::Vec3 rayHit( 0.0f );
        debug.AddSphere( rayP, 0.05f, ae::Color::PicoPeach(), 8 );
        if ( plane.IntersectRay( rayP, rayD, nullptr, &rayHit ) )
        {
          debug.AddSphere( rayHit, 0.05f, ae::Color::PicoPeach(), 8 );
          debug.AddLine( rayP, rayHit, ae::Color::PicoPeach() );
          debug.AddCircle( p, plane.GetNormal(), ( p - rayHit ).Length(), aeColor::PicoPink(), 32 );
        }
        else
        {
          debug.AddLine( rayP, rayP + rayD, ae::Color::PicoPeach() );
        }
        ae::Vec3 closest = plane.GetClosestPoint( rayP );
        debug.AddCircle( p, plane.GetNormal(), ( p - closest ).Length(), aeColor::PicoPink(), 32 );
        debug.AddLine( p, closest, ae::Color::PicoPink() );
        
        float sd = plane.GetSignedDistance( rayP );
        ae::Color sdColor = sd > 0.0f ? ae::Color::PicoRed() : ae::Color::PicoGreen();
        debug.AddSphere( closest, 0.05f, sdColor, 8 );
        debug.AddLine( closest, closest + plane.GetNormal() * sd, sdColor );
				
				break;
			}
			default:
				break;
		}

    int newlineCount = 0;
    const char* infoStr = infoText.c_str();
    while ( *infoStr )
    {
      if ( *infoStr == '\n' )
      {
        newlineCount++;
      }
      infoStr++;
    }
		text.Add( ae::Vec3( 50.0f, 50.0f + newlineCount * text.GetFontSize(), 0.0f ), ae::Vec2( text.GetFontSize() * 2.0f ), infoText.c_str(), aeColor::Red(), 0, 0 );
    if ( currentTest != 4 )
    {
      debug.AddLine( raySource, raySource + ray, aeColor::Red() );
    }

		debug.Render( worldToProj );
		text.Render( textToNdc );

		render.Present();

		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	text.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
