//------------------------------------------------------------------------------
// 12_Geometry.cpp
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
#include "ae/SpriteRenderer.h"

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
	ae::FileSystem fileSystem;
	ae::DebugLines debug = TAG_EXAMPLE;
	ae::Texture2D fontTexture;
	ae::TextRender text = TAG_EXAMPLE;
	ae::DebugCamera camera = ae::Axis::Z;

	window.Initialize( 800, 600, false, true, true );
	window.SetTitle( "geometry" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "ae", "geometry" );
	debug.Initialize( 2048 );
	{
		const char* fileName = "font.tga";
		uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
		AE_ASSERT_MSG( fileSize, "Could not load #", fileName );
		ae::Scratch< uint8_t > fileBuffer( fileSize );
		fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileSize );
		ae::TargaFile targa = TAG_EXAMPLE;
		targa.Load( fileBuffer.Data(), fileSize );
		fontTexture.Initialize( targa.textureParams );
	}
	text.Initialize( 16, 512, &fontTexture, 8, 1.0f );
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 5.0f, 5.0f, 5.0f ) );
	
	// AABB and OBB test state
	static ae::Vec3 s_translation( 0.0f );
	static ae::Vec3 s_rotation( 0.0f );
	static ae::Vec3 s_scale( 1.0f );
	static float s_pa = 0.0f;
	static float s_pb = 0.0f;

	AE_INFO( "Run" );
	while( !input.quit )
	{
		input.Pump();

		if( input.Get( ae::Key::F ) && !input.GetPrev( ae::Key::F ) )
		{
			camera.Refocus( ae::Vec3( 0.0f ) );
		}
		camera.Update( &input, timeStep.GetTimeStep() );

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( ae::QUARTER_PI, render.GetAspectRatio(), 0.25f, 100.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;

		// UI units in pixels, origin in bottom left
		ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );

		ae::Matrix4 worldToUI = textToNdc.GetInverse() * worldToProj;

		auto DrawText = [&]( ae::Vec3 worldPos, const char* str, ae::Color color )
		{
			text.Add( ae::Vec3( ae::Vec3::ProjectPoint( worldToUI, worldPos ).GetXY() ), ae::Vec2( text.GetFontSize() * 2.0f ), str, color, 0, 0 );
		};
		
		ae::Str256 infoText = "";

		// Edit ray direction
		ae::Vec3 ray;
		ae::Vec3 raySource;
		auto doRay = [&]( bool drawRay )
		{
			infoText.Append( "------------------------\n" );
			infoText.Append( "Refocus: F\n" );
			infoText.Append( "Cast Ray from Camera: G\n" );
			infoText.Append( "Ray Length: 1-2\n" );
			
			static ae::Vec3 s_raySource( 2.0f, 0.6f, 2.0f );
			static ae::Vec3 s_rayDir = -s_raySource.SafeNormalizeCopy();
			static float s_rayLength = 4.0f;
			
			if( input.Get( ae::Key::Num1 ) ) s_rayLength -= 0.016f;
			if( input.Get( ae::Key::Num2 ) ) s_rayLength += 0.016f;
			s_rayLength = ae::Clip( s_rayLength, 0.0f, 16.0f );
			
			if( input.Get( ae::Key::G ) )
			{
				s_raySource = camera.GetPosition();
				s_rayDir = camera.GetForward();
			}
			
			ray = s_rayDir * s_rayLength;
			raySource = s_raySource;
			
			if( drawRay )
			{
				debug.AddLine( raySource, raySource + ray, ae::Color::Red() );
			}
		};

		// Test selection
		static int32_t currentTest = 0;
		if( input.Get( ae::Key::Left ) && !input.GetPrev( ae::Key::Left ) )
		{
			currentTest--;
		}
		if( input.Get( ae::Key::Right ) && !input.GetPrev( ae::Key::Right ) )
		{
			currentTest++;
		}
		currentTest = ae::Mod( currentTest, 10 );

		// Geometry calculations / rendering
		switch( currentTest )
		{
			case 0:
			{
				infoText.Append( "Triangle-Ray\n" );
				doRay( true );
				infoText.Append( "Scale: 3-4\n" );
				
				static float s_triangleScale = 1.0f;
				if( input.Get( ae::Key::Num3 ) ) s_triangleScale -= 0.016f;
				if( input.Get( ae::Key::Num4 ) ) s_triangleScale += 0.016f;
				s_triangleScale = ae::Max( 0.0f, s_triangleScale );
				ae::Vec3 triangle[] =
				{
					ae::Vec3( 0.0f, -1.0f, -1.0f ) * s_triangleScale,
					ae::Vec3( 0.0f, 1.0f, -1.0f ) * s_triangleScale,
					ae::Vec3( 0.0f, 0.0f, 1.0f ) * s_triangleScale,
				};
				ae::Vec3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				ae::Vec3 normal = ( triangle[ 1 ] - triangle[ 0 ] ).Cross( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], ae::Color::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], ae::Color::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], ae::Color::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, ae::Color::Red() );

				static float r = 1.0f;
				if( input.Get( ae::Key::Num3 ) ) r -= 0.016f;
				if( input.Get( ae::Key::Num4 ) ) r += 0.016f;
				r = ae::Clip( r, 0.01f, 8.0f );
				
				bool ccw = true;
				bool cw = false;
				
				ae::Vec3 p;
				ae::Vec3 n;
				float t;
				if( ae::IntersectRayTriangle( raySource, ray, triangle[ 0 ], triangle[ 1 ], triangle[ 2 ], ccw, cw, &p, &n, &t ) )
				{
					debug.AddSphere( p, 0.1f, ae::Color::Green(), 8 );
					debug.AddLine( p, p + n, ae::Color::Green() );
				}
				break;
			}
			case 1:
			{
				infoText.Append( "Sphere-Ray\n" );
				doRay( true );
				
				debug.AddSphere( ae::Vec3( 0.0f ), 1.0f, ae::Color::Blue(), 16 );

				ae::Vec3 e = raySource;
				ae::Vec3 d = ray.SafeNormalizeCopy();

				float b = 2.0f * e.Dot( d );
				float c = e.LengthSquared() - 1.0f;

				float discriminant = b * b - 4.0f * c;
				if( discriminant >= 0.0f )
				{
					discriminant = sqrtf( discriminant );
					float t0 = ( -b + discriminant ) / 2.0f;
					float t1 = ( -b - discriminant ) / 2.0f;

					debug.AddSphere( e + d * t0, 0.05f, ae::Color::Red(), 8 );
					debug.AddSphere( e + d * t1, 0.05f, ae::Color::Green(), 8 );
				}

				break;
			}
			case 2:
			{
				static ae::Vec3 s_spherePos( 0.0f, 0.0f, 0.0f );
				static float s_sphereRadius = 1.0f;
				infoText.Append( "Sphere Distance\n" );
				doRay( true );
				infoText.Append( "Translate: W,A,S,D,E,Q\n" );
				infoText.Append( "Radius: 3-4\n" );
				
				if( input.Get( ae::Key::D ) ) { s_spherePos.x += 0.01f; }
				if( input.Get( ae::Key::A ) ) { s_spherePos.x -= 0.01f; }
				if( input.Get( ae::Key::W ) ) { s_spherePos.y += 0.01f; }
				if( input.Get( ae::Key::S ) ) { s_spherePos.y -= 0.01f; }
				if( input.Get( ae::Key::E ) ) { s_spherePos.z += 0.01f; }
				if( input.Get( ae::Key::Q ) ) { s_spherePos.z -= 0.01f; }
				if( input.Get( ae::Key::Num3 ) ) s_sphereRadius -= timeStep.GetDt();
				if( input.Get( ae::Key::Num4 ) ) s_sphereRadius += timeStep.GetDt();
				s_sphereRadius = ae::Max( 0.01f, s_sphereRadius );
				
				const ae::Vec3 pos = raySource + ray;
				const ae::Sphere sphere( s_spherePos, s_sphereRadius );
				debug.AddSphere( sphere.center, sphere.radius, ae::Color::Blue(), 32 );

				float signedDist = 0.0f;
				const ae::Vec3 closest = sphere.GetNearestPointOnSurface( pos, &signedDist );
				const ae::Color color = ( signedDist > 0.0f ) ? ae::Color::Green() : ae::Color::Red();
				debug.AddLine( pos, closest, color );

				break;
			}
			case 3:
			{
				infoText.Append( "Cylinder-Ray\n" );
				
				doRay( true );
				
				bool hit0 = false;
				bool hit1 = false;

				ae::Vec3 p0( 0.0f, 0.0f, 2.0f );
				ae::Vec3 p1( 0.0f, 0.0f, -2.0f );

				ae::Vec3 e = raySource;
				ae::Vec3 d = ray;
				ae::Vec2 exy = raySource.GetXY();
				ae::Vec2 dxy = ray.GetXY();
				
				float a = dxy.LengthSquared();
				if( a > 0.0f )
				{
					float b = 2.0f * exy.Dot( dxy );
					float c = exy.LengthSquared() - 1.0f;

					float discriminant = b * b - 4.0f * a * c;
					if( discriminant >= 0.0f )
					{
						discriminant = sqrtf( discriminant );
						float t0 = ( -b + discriminant ) / ( 2.0f * a );
						float t1 = ( -b - discriminant ) / ( 2.0f * a );

						ae::Vec3 r0 = e + d * t0;
						ae::Vec3 r1 = e + d * t1;
						debug.AddSphere( r0, 0.05f, ae::Color::Red(), 8 );
						debug.AddSphere( r1, 0.05f, ae::Color::Red(), 8 );
						DrawText( r0, "t0", ae::Color::Red() );
						DrawText( r1, "t1", ae::Color::Red() );

						ae::Vec3 z0( 0.0f, 0.0f, r0.z );
						ae::Vec3 z1( 0.0f, 0.0f, r1.z );
						debug.AddSphere( z0, 0.05f, ae::Color::Blue(), 8 );
						debug.AddSphere( z1, 0.05f, ae::Color::Blue(), 8 );
						DrawText( z0, "z0", ae::Color::Blue() );
						DrawText( z1, "z1", ae::Color::Blue() );
						debug.AddLine( z0, z1, ae::Color::Blue() );

						if( ( p0.z - z0.z ) * ( p0.z - z1.z ) < 0.0f )
						{
							float t2 = ( p0.z - e.z ) / d.z;
							ae::Vec3 c0 = e + d * t2;
							debug.AddSphere( c0, 0.05f, ae::Color::Green(), 8 );
							DrawText( c0, "c0", ae::Color::Green() );
							hit0 = true;
						}

						if( ( p1.z - z0.z ) * ( p1.z - z1.z ) < 0.0f )
						{
							float t2 = ( p1.z - e.z ) / d.z;
							ae::Vec3 c1 = e + d * t2;
							debug.AddSphere( c1, 0.05f, ae::Color::Green(), 8 );
							DrawText( c1, "c1", ae::Color::Green() );
							hit1 = true;
						}
					}
				}

				debug.AddCircle( p0, p0 - p1, 1.0f, hit0 ? ae::Color::Green() : ae::Color::Blue(), 16 );
				debug.AddCircle( p1, p1 - p0, 1.0f, hit1 ? ae::Color::Green() : ae::Color::Blue(), 16 );
				for( uint32_t i = 0; i < 8; i++ )
				{
					ae::Vec2 offset = ae::Vec2::FromAngle( ae::TWO_PI * i / 8.0f );
					debug.AddLine( p0 + ae::Vec3( offset ), p1 + ae::Vec3( offset ), ae::Color::Blue() );
				}

				break;
			}
			case 4:
			{
				infoText.Append( "Sphere-Triangle\n" );
				
				doRay( true );
				
				ae::Vec3 triangle[] =
				{
					ae::Vec3( -1.0f, -1.5f, -1.0f ) * 2.0f,
					ae::Vec3( 1.0f, -0.5f, -1.0f ) * 2.0f,
					ae::Vec3( 0.0f, 1.5f, 1.0f ) * 2.0f,
				};
				ae::Vec3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				ae::Vec3 normal = ( triangle[ 1 ] - triangle[ 0 ] ).Cross( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], ae::Color::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], ae::Color::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], ae::Color::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, ae::Color::Red() );
				
				static float r = 1.0f;
				if( input.Get( ae::Key::Num3 ) ) r -= 0.016f;
				if( input.Get( ae::Key::Num4 ) ) r += 0.016f;
				r = ae::Clip( r, 0.01f, 8.0f );

				ae::Vec3 nearestIntersectionPoint;
				ae::Sphere sphere( raySource + ray, r );
				if( sphere.IntersectTriangle( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ], &nearestIntersectionPoint ) )
				{
					debug.AddSphere( raySource + ray, r, ae::Color::Green(), 16 );
					debug.AddSphere( nearestIntersectionPoint, 0.05f, ae::Color::Red(), 8 );
					
					ae::Vec3 spherePoint = sphere.center + ( nearestIntersectionPoint - sphere.center ).SafeNormalizeCopy() * r;
					debug.AddSphere( spherePoint, 0.05f, ae::Color::Green(), 8 );
					debug.AddLine( nearestIntersectionPoint, spherePoint, ae::Color::Green() );
				}
				else
				{
					debug.AddSphere( raySource + ray, r, ae::Color::Red(), 16 );
				}
				break;
			}
			case 5:
			{
				static bool rayTest = true;
				static float a0 = ae::HalfPi;
				static float a1 = ae::QuarterPi;
				static float d = 1.0f;

				infoText.Append( rayTest ? "Plane-Ray" : "Plane-Line" );
				infoText.Append( " (Toggle: Space)\n" );
				doRay( false );
				infoText.Append( "Rotate Normal XY: 3-4\n" );
				infoText.Append( "Rotate Normal Z: 5-6\n" );
				infoText.Append( "Plane Distance from Origin: 7-8\n" );
				infoText.Append( "Flip Normal: F\n" );
				
				if( input.GetPress( ae::Key::Space ) ) { rayTest = !rayTest; }
				if( input.Get( ae::Key::Num3 ) ) a0 -= 0.016f;
				if( input.Get( ae::Key::Num4 ) ) a0 += 0.016f;
				if( input.Get( ae::Key::Num5 ) ) a1 -= 0.016f;
				if( input.Get( ae::Key::Num6 ) ) a1 += 0.016f;
				if( input.Get( ae::Key::Num7 ) ) d -= 0.016f;
				if( input.Get( ae::Key::Num8 ) ) d += 0.016f;
				bool flipNormal = input.Get( ae::Key::F );
				
				ae::Vec2 xy( ae::Cos( a0 ), ae::Sin( a0 ) );
				ae::Plane plane( ae::Vec4( xy * ae::Cos( a1 ), ae::Sin( a1 ), d ) );
				if( flipNormal )
				{
					plane = ae::Plane( plane.GetClosestPointToOrigin(), -plane.GetNormal() );
				}
				
				ae::Vec3 p = plane.GetClosestPointToOrigin();
				ae::Vec3 pn = plane.GetClosestPointToOrigin() + plane.GetNormal();
				
				// Reference lines
				if( d > 0.0f )
				{
					debug.AddLine( ae::Vec3( 0.0f ), p, ae::Color::Gray() );
				}
				else if( d < -1.0f )
				{
					debug.AddLine( ae::Vec3( 0.0f ), pn, ae::Color::Gray() );
				}
				ae::AABB aabb( ae::Vec3( 0.0f ), p );
				debug.AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), ae::Color::Gray() );
				
				// Axis lines
				debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Color::Red() );
				debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Color::Green() );
				debug.AddLine( ae::Vec3( 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Color::Blue() );
				
				// Plane
				debug.AddLine( p, pn, ae::Color::PicoPink() );
				
				// Ray
				ae::Vec3 rayHit( 0.0f );
				float t = 0.0f;
				debug.AddSphere( raySource, 0.05f, ae::Color::PicoPeach(), 8 );
				if( rayTest && plane.IntersectRay( raySource, ray, &rayHit ) )
				{
					debug.AddSphere( rayHit, 0.05f, ae::Color::PicoPeach(), 8 );
					debug.AddLine( raySource, rayHit, ae::Color::PicoPeach() );
					debug.AddCircle( p, plane.GetNormal(), ( p - rayHit ).Length(), ae::Color::PicoPink(), 32 );
					debug.AddLine( rayHit, raySource + ray, ae::Color::Red() );
				}
				else if( !rayTest && plane.IntersectLine( raySource, ray, nullptr, &t ) )
				{
					rayHit = raySource + ray * t;
					debug.AddSphere( rayHit, 0.05f, ae::Color::PicoPeach(), 8 );
					debug.AddLine( raySource, rayHit, ae::Color::PicoPeach() );
					debug.AddCircle( p, plane.GetNormal(), ( p - rayHit ).Length(), ae::Color::PicoPink(), 32 );
				}
				else
				{
					debug.AddLine( raySource, raySource + ray, ae::Color::PicoPeach() );
				}
				ae::Vec3 closest = plane.GetClosestPoint( raySource );
				debug.AddCircle( p, plane.GetNormal(), ( p - closest ).Length(), ae::Color::PicoPink(), 32 );
				debug.AddLine( p, closest, ae::Color::PicoPink() );
				
				float sd = plane.GetSignedDistance( raySource );
				ae::Color sdColor = sd > 0.0f ? ae::Color::PicoGreen() : ae::Color::PicoRed();
				debug.AddSphere( closest, 0.05f, sdColor, 8 );
				debug.AddLine( closest, closest + plane.GetNormal() * sd, sdColor );
				
				break;
			}
			case 6:
			{
				infoText.Append( "AABB-Ray/Point\n" );
				doRay( false );
				infoText.Append( "Rotate Point: Space\n" );
				infoText.Append( "Translate: W,A,S,D,E,Q\n" );
				infoText.Append( "Scale: I,J,K,L,U,O\n" );
				
				if( input.Get( ae::Key::D ) ) { s_translation.x += 0.01f; }
				if( input.Get( ae::Key::A ) ) { s_translation.x -= 0.01f; }
				if( input.Get( ae::Key::W ) ) { s_translation.y += 0.01f; }
				if( input.Get( ae::Key::S ) ) { s_translation.y -= 0.01f; }
				if( input.Get( ae::Key::E ) ) { s_translation.z += 0.01f; }
				if( input.Get( ae::Key::Q ) ) { s_translation.z -= 0.01f; }
				
				if( input.Get( ae::Key::L ) ) { s_scale.x += 0.07f; }
				if( input.Get( ae::Key::J ) ) { s_scale.x -= 0.07f; }
				if( input.Get( ae::Key::I ) ) { s_scale.y += 0.07f; }
				if( input.Get( ae::Key::K ) ) { s_scale.y -= 0.07f; }
				if( input.Get( ae::Key::O ) ) { s_scale.z += 0.07f; }
				if( input.Get( ae::Key::U ) ) { s_scale.z -= 0.07f; }
				s_scale = ae::Max( ae::Vec3( 0.0f ), s_scale );
				
				ae::Matrix4 transform = ae::Matrix4::Translation( s_translation );
				transform *= ae::Matrix4::Scaling( s_scale );
				ae::Vec4 pMin = transform * ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f );
				ae::Vec4 pMax = transform * ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f );
				ae::AABB aabb( pMin.GetXYZ(), pMax.GetXYZ() );
				debug.AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), ae::Color::Green() );
				
				if( input.Get( ae::Key::Space ) )
				{
					s_pa += 0.007f;
					s_pb += 0.01f;
				}
				
				ae::Vec3 p( 0.0f );
				p += ae::Vec3( cosf( s_pa ), sinf( s_pa ), 0.0f );
				p += ae::Vec3( cosf( s_pb ), 0.0f, sinf( s_pb ) );
				
				float dist = aabb.GetSignedDistanceFromSurface( p );
				debug.AddSphere( p, 0.05f, ae::Color::PicoPink(), 8 );
				
				ae::Color nearestColor = ( dist > 0.0f ) ? ae::Color::PicoGreen() : ae::Color::PicoRed();
				ae::Vec3 surface = aabb.GetClosestPointOnSurface( p );
				debug.AddSphere( surface, 0.05f, nearestColor, 8 );
				ae::Vec3 toSurface = ( surface - p ).NormalizeCopy() * ae::Abs( dist );
				debug.AddLine( p, p + toSurface, nearestColor );
				
				ae::Vec3 rayP, rayN;
				float rayT;
				if( aabb.IntersectRay( raySource, ray, &rayP, &rayN, &rayT ) )
				{
					debug.AddLine( raySource, raySource + ray * rayT, ae::Color::PicoBlue() );
					debug.AddLine( rayP, raySource + ray, ae::Color::Red() );

					debug.AddSphere( rayP, 0.05f, ae::Color::PicoBlue(), 8 );
					debug.AddLine( rayP, rayP + rayN, ae::Color::PicoBlue() );
					
					float t1;
					ae::Vec3 n1;
					AE_ASSERT( aabb.IntersectLine( raySource, ray, nullptr, &t1, nullptr, &n1 ) );
					ae::Vec3 p1 = raySource + ray * t1;
					debug.AddSphere( p1, 0.05f, ae::Color::Red(), 8 );
					debug.AddLine( p1, p1 + n1, ae::Color::Red() );
				}
				else
				{
					debug.AddLine( raySource, raySource + ray, ae::Color::Red() );
					debug.AddSphere( raySource + ray, 0.05f, ae::Color::Red(), 8 );
				}
				break;
			}
			case 7:
			{
				infoText.Append( "OBB-Ray/Point\n" );
				doRay( false );
				infoText.Append( "Rotate Point: Space\n" );
				infoText.Append( "Translate: W,A,S,D,E,Q\n" );
				infoText.Append( "Scale: I,J,K,L,U,O\n" );
				infoText.Append( "Rotate: Z,X,C\n" );
				
				if( input.Get( ae::Key::D ) ) { s_translation.x += 0.01f; }
				if( input.Get( ae::Key::A ) ) { s_translation.x -= 0.01f; }
				if( input.Get( ae::Key::W ) ) { s_translation.y += 0.01f; }
				if( input.Get( ae::Key::S ) ) { s_translation.y -= 0.01f; }
				if( input.Get( ae::Key::E ) ) { s_translation.z += 0.01f; }
				if( input.Get( ae::Key::Q ) ) { s_translation.z -= 0.01f; }
				
				if( input.Get( ae::Key::L ) ) { s_scale.x += 0.07f; }
				if( input.Get( ae::Key::J ) ) { s_scale.x -= 0.07f; }
				if( input.Get( ae::Key::I ) ) { s_scale.y += 0.07f; }
				if( input.Get( ae::Key::K ) ) { s_scale.y -= 0.07f; }
				if( input.Get( ae::Key::O ) ) { s_scale.z += 0.07f; }
				if( input.Get( ae::Key::U ) ) { s_scale.z -= 0.07f; }
				s_scale = ae::Max( ae::Vec3( 0.0f ), s_scale );
				
				if( input.Get( ae::Key::Z ) ) { s_rotation.x += 0.01f; }
				if( input.Get( ae::Key::X ) ) { s_rotation.y += 0.01f; }
				if( input.Get( ae::Key::C ) ) { s_rotation.z += 0.01f; }
				
				ae::Matrix4 transform = ae::Matrix4::Translation( s_translation );
				transform *= ae::Matrix4::RotationX( s_rotation.x );
				transform *= ae::Matrix4::RotationY( s_rotation.y );
				transform *= ae::Matrix4::RotationZ( s_rotation.z );
				transform *= ae::Matrix4::Scaling( s_scale );
				ae::OBB obb( transform );
				debug.AddOBB( obb.GetTransform(), ae::Color::Green() );
				
				if( input.Get( ae::Key::Space ) )
				{
					s_pa += 0.007f;
					s_pb += 0.01f;
				}
				
				ae::Vec3 p( 0.0f );
				p += ae::Vec3( cosf( s_pa ), sinf( s_pa ), 0.0f );
				p += ae::Vec3( cosf( s_pb ), 0.0f, sinf( s_pb ) );
				
				float dist = obb.GetSignedDistanceFromSurface( p );
				debug.AddSphere( p, 0.05f, ae::Color::PicoPink(), 8 );
				
				ae::Color nearestColor = ( dist > 0.0f ) ? ae::Color::PicoGreen() : ae::Color::PicoRed();
				ae::Vec3 surface = obb.GetClosestPointOnSurface( p );
				debug.AddSphere( surface, 0.05f, nearestColor, 8 );
				ae::Vec3 toSurface = ( surface - p ).NormalizeCopy() * ae::Abs( dist );
				debug.AddLine( p, p + toSurface, nearestColor );
				
				ae::Vec3 rayP, rayN;
				float rayT;
				
				struct
				{
					ae::Vec3 from;
					ae::Vec3 to;
					ae::Color color;
				} rays[] = {
					{ ae::Vec3( 10.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f ), ae::Color::PicoRed() },
					{ ae::Vec3( 0.0f, 10.0f, 0.0f ), ae::Vec3( 0.0f ), ae::Color::PicoGreen() },
					{ ae::Vec3( 0.0f, 0.0f, 10.0f ), ae::Vec3( 0.0f ), ae::Color::PicoBlue() },
					{ ae::Vec3( -10.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f ), ae::Color::PicoRed() },
					{ ae::Vec3( 0.0f, -10.0f, 0.0f ), ae::Vec3( 0.0f ), ae::Color::PicoGreen() },
					{ ae::Vec3( 0.0f, 0.0f, -10.0f ), ae::Vec3( 0.0f ), ae::Color::PicoBlue() },
				};
				for( uint32_t i = 0; i < countof(rays); i++ )
				{
					rayP = rays[ i ].to;
					obb.IntersectRay( rays[ i ].from, rays[ i ].to - rays[ i ].from, &rayP );
					debug.AddLine( rays[ i ].from, rayP, rays[ i ].color );
				}
				
				if( obb.IntersectRay( raySource, ray, &rayP, &rayN, &rayT ) )
				{
					debug.AddLine( raySource, raySource + ray * rayT, ae::Color::PicoBlue() );
					debug.AddLine( rayP, raySource + ray, ae::Color::Red() );
					
					debug.AddSphere( rayP, 0.05f, ae::Color::PicoBlue(), 8 );
					debug.AddLine( rayP, rayP + rayN, ae::Color::PicoBlue() );
					
					float t1;
					ae::Vec3 n1;
					AE_ASSERT( obb.IntersectLine( raySource, ray, nullptr, &t1, nullptr, &n1 ) );
					ae::Vec3 p1 = raySource + ray * t1;
					debug.AddSphere( p1, 0.05f, ae::Color::Red(), 8 );
					debug.AddLine( p1, p1 + n1, ae::Color::Red() );
				}
				else
				{
					debug.AddLine( raySource, raySource + ray, ae::Color::Red() );
					debug.AddSphere( raySource + ray, 0.05f, ae::Color::Red(), 8 );
				}
				break;
			}
			case 8:
			{
				infoText.Append( "Angle Lerp\n" );
				infoText.Append( "Rotate Speed: 1-2\n" );
				infoText.Append( "Lerp: Space\n" );

				debug.AddCircle( ae::Vec3( 0.0f ), camera.GetPosition(), 1.0f, ae::Color::PicoPink(), 32 );

				static float s_setAngle = 0.0f;
				ae::Vec3 s = camera.GetRight();
				s = s.RotateCopy( camera.GetPosition(), s_setAngle );
				debug.AddLine( ae::Vec3( 0.0f ), s, ae::Color::Black() );

				static float s_setAnglePrev = 0.0f;
				ae::Vec3 p = camera.GetRight();
				p = p.RotateCopy( camera.GetPosition(), s_setAnglePrev );
				debug.AddLine( ae::Vec3( 0.0f ), p, ae::Color::Black() );

				static float s_dtLerp = 0.0f;
				s_dtLerp = ae::DtLerpAngle( s_dtLerp, 4.0f, timeStep.GetTimeStep(), s_setAngle );
				ae::Vec3 l = camera.GetRight();
				l = l.RotateCopy( camera.GetPosition(), s_dtLerp );
				debug.AddLine( ae::Vec3( 0.0f ), l, ae::Color::Green() );

				static float s_lerpAmt = 0.0f;
				s_lerpAmt += timeStep.GetTimeStep();
				s_lerpAmt = ae::Min( s_lerpAmt, 1.0f );
				float lerpAngle = ae::LerpAngle( s_setAnglePrev, s_setAngle, s_lerpAmt );
				ae::Vec3 l2 = camera.GetRight();
				l2 = l2.RotateCopy( camera.GetPosition(), lerpAngle );
				debug.AddLine( ae::Vec3( 0.0f ), l2, ae::Color::Blue() );

				static float s_turnerAngle = 0.0f;
				static float s_turnerVel = 0.0f;
				if( input.Get( ae::Key::Num1 ) ) { s_turnerVel -= timeStep.GetTimeStep(); }
				if( input.Get( ae::Key::Num2 ) ) { s_turnerVel += timeStep.GetTimeStep(); }
				if( input.Get( ae::Key::Num3 ) ) { s_turnerVel = 0.0f; }
				if( input.Get( ae::Key::Space ) && !input.GetPrev( ae::Key::Space ) )
				{
					s_setAnglePrev = s_setAngle;
					s_setAngle = s_turnerAngle;
					s_lerpAmt = 0.0f;
				}
				s_turnerAngle += s_turnerVel * timeStep.GetTimeStep();
				ae::Vec3 t = camera.GetRight();
				t = t.RotateCopy( camera.GetPosition(), s_turnerAngle );
				debug.AddLine( ae::Vec3( 0.0f ), t, ae::Color::Red() );
				break;
			}
			case 9:
			{
				infoText.Append( "AABB Overlap\n" );
				static bool s_first = true;
				static ae::AABB s_aabb0;
				static ae::AABB s_aabb1;
				if( s_first || input.Get( ae::Key::R ) )
				{
					s_aabb0 = ae::AABB( ae::Vec3( 0.5f ), ae::Vec3( 1.5f ) );
					s_aabb1 = ae::AABB( ae::Vec3( -0.5f ), ae::Vec3( 0.5f ) );
					s_first = false;
				}
				auto TranslateAABB = []( ae::AABB& aabb, const ae::Vec3& translation )
				{
					aabb = ae::AABB( aabb.GetMin() + translation, aabb.GetMax() + translation );
				};
				auto ScaleAABB = []( ae::AABB& aabb, const ae::Vec3& scale )
				{
					const ae::Vec3 halfSize = aabb.GetHalfSize() * scale;
					aabb = ae::AABB( aabb.GetCenter() - halfSize, aabb.GetCenter() + halfSize );
				};
				if( input.Get( ae::Key::W ) ) { TranslateAABB( s_aabb0, ae::Vec3( 0.0f, 0.5f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::S ) ) { TranslateAABB( s_aabb0, ae::Vec3( 0.0f, -0.5f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::A ) ) { TranslateAABB( s_aabb0, ae::Vec3( -0.5f, 0.0f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::D ) ) { TranslateAABB( s_aabb0, ae::Vec3( 0.5f, 0.0f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::Q ) ) { TranslateAABB( s_aabb0, ae::Vec3( 0.0f, 0.0f, -0.5f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::E ) ) { TranslateAABB( s_aabb0, ae::Vec3( 0.0f, 0.0f, 0.5f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::I ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( 0.0f, 0.5f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::K ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( 0.0f, -0.5f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::J ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( -0.5f, 0.0f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::L ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( 0.5f, 0.0f, 0.0f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::U ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( 0.0f, 0.0f, -0.5f ) * timeStep.GetTimeStep() ); }
				if( input.Get( ae::Key::O ) ) { ScaleAABB( s_aabb0, ae::Vec3( 1.0f ) + ae::Vec3( 0.0f, 0.0f, 0.5f ) * timeStep.GetTimeStep() ); }
				const bool overlap = s_aabb0.Intersect( s_aabb1 );
				debug.AddAABB( s_aabb0, overlap ? ae::Color::Red() : ae::Color::Green() );
				debug.AddAABB( s_aabb1, overlap ? ae::Color::Red() : ae::Color::Green() );
				break;
			}
			default:
				break;
		}

		int newlineCount = 0;
		const char* infoStr = infoText.c_str();
		while( *infoStr )
		{
			if( *infoStr == '\n' )
			{
				newlineCount++;
			}
			infoStr++;
		}
		text.Add( ae::Vec3( 10.0f, 10.0f + newlineCount * text.GetFontSize() * 2.0f, 0.0f ), ae::Vec2( text.GetFontSize() * 2.0f ), infoText.c_str(), ae::Color::Red(), 0, 0 );

		debug.Render( worldToProj );
		text.Render( textToNdc );

		render.Present();

		timeStep.Tick();
	}

	AE_INFO( "Terminate" );
	text.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
