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
#include "ae/ae.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_INFO( "Initialize" );

	aeWindow window;
	aeRender render;
	aeInput input;
	aeFixedTimeStep timeStep;
	aeDebugRender debug;
	aeTextRender text;
	aeEditorCamera camera;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "geometry" );
	render.InitializeOpenGL( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debug.Initialize();
	text.Initialize( "font.png", aeTextureFilter::Nearest, 8 );

	AE_INFO( "Run" );
	while ( !input.GetState()->exit )
	{
		input.Pump();

		if ( !input.GetPrevState()->Get( aeKey::F ) && input.GetState()->Get( aeKey::F ) )
		{
			camera.Refocus( aeFloat3( 0.0f ) );
		}
		camera.Update( &input, timeStep.GetTimeStep() );

		render.Activate();
		render.Clear( aeColor::PicoDarkPurple() );
		
		aeFloat4x4 worldToView = aeFloat4x4::WorldToView( camera.GetPosition(), camera.GetForward(), aeFloat3( 0.0f, 0.0f, 1.0f ) );
		aeFloat4x4 viewToProj = aeFloat4x4::ViewToProjection( 0.6f, render.GetAspectRatio(), 0.25f, 50.0f );
		aeFloat4x4 worldToProj = viewToProj * worldToView;

		// UI units in pixels, origin in bottom left
		aeFloat4x4 textToNdc = aeFloat4x4::Scaling( aeFloat3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
		textToNdc *= aeFloat4x4::Translation( aeFloat3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );

		aeFloat4x4 worldToUI = textToNdc.Inverse() * worldToProj;

		auto DrawText = [&]( aeFloat3 worldPos, const char* str, aeColor color )
		{
			text.Add( aeFloat3( aeFloat3::ProjectPoint( worldToUI, worldPos ).GetXY() ), aeFloat2( text.GetFontSize() * 2.0f ), str, color, 0, 0 );
		};

		// Edit ray direction
		static aeFloat3 s_raySource( 0.0f );
		static aeFloat3 s_rayDir( 0.0f );
		static float s_rayLength = 8.0f;
		if ( input.GetState()->Get( aeKey::Num1 ) ) s_rayLength += 0.016f;
		if ( input.GetState()->Get( aeKey::Num2 ) ) s_rayLength -= 0.016f;
		s_rayLength = aeMath::Clip( s_rayLength, 0.0f, 8.0f );

		if ( input.GetState()->Get( aeKey::G ) )
		{
			s_raySource = camera.GetPosition();
			s_rayDir = camera.GetForward();
			s_rayLength = 8.0f;
		}
		aeFloat3 ray = s_rayDir * s_rayLength;
		aeFloat3 raySource = s_raySource;

		// Actual geometry calculations / rendering
		static int32_t currentTest = 0;
		if ( input.GetState()->left && !input.GetPrevState()->left )
		{
			currentTest--;
		}
		if ( input.GetState()->right && !input.GetPrevState()->right )
		{
			currentTest++;
		}
		currentTest = aeMath::Mod( currentTest, 4 );

		aeStr256 infoText = "";
		switch ( currentTest )
		{
			case 0: // Triangle ray
			{
				infoText.Append( "Triangle-Ray\n" );
				
				aeFloat3 triangle[] =
				{
					aeFloat3( -1.0f, -1.5f, -1.0f ) * 2.0f,
					aeFloat3( 1.0f, -0.5f, -1.0f ) * 2.0f,
					aeFloat3( 0.0f, 1.5f, 1.0f ) * 2.0f,
				};
				aeFloat3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				aeFloat3 normal = ( triangle[ 1 ] - triangle[ 0 ] ) % ( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], aeColor::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], aeColor::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], aeColor::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, aeColor::Red() );

				static float r = 1.0f;
				if ( input.GetState()->Get( aeKey::Num3 ) ) r -= 0.016f;
				if ( input.GetState()->Get( aeKey::Num4 ) ) r += 0.016f;
				r = aeMath::Clip( r, 0.01f, 8.0f );

				float dist = 0.0f;
				aeFloat3 nearestIntersectionPoint;
				aeFloat3 nearestPolygonIntersectionPoint;
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
				
				debug.AddSphere( aeFloat3( 0.0f ), 1.0f, aeColor::Blue(), 16 );

				aeFloat3 e = raySource;
				aeFloat3 d = ray.SafeNormalizeCopy();

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

				aeFloat3 p0( 0.0f, 0.0f, 2.0f );
				aeFloat3 p1( 0.0f, 0.0f, -2.0f );

				aeFloat3 e = raySource;
				aeFloat3 d = ray;
				aeFloat2 exy = raySource.GetXY();
				aeFloat2 dxy = ray.GetXY();
				
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

						aeFloat3 r0 = e + d * t0;
						aeFloat3 r1 = e + d * t1;
						debug.AddSphere( r0, 0.05f, aeColor::Red(), 8 );
						debug.AddSphere( r1, 0.05f, aeColor::Red(), 8 );
						DrawText( r0, "t0", aeColor::Red() );
						DrawText( r1, "t1", aeColor::Red() );

						aeFloat3 z0( 0.0f, 0.0f, r0.z );
						aeFloat3 z1( 0.0f, 0.0f, r1.z );
						debug.AddSphere( z0, 0.05f, aeColor::Blue(), 8 );
						debug.AddSphere( z1, 0.05f, aeColor::Blue(), 8 );
						DrawText( z0, "z0", aeColor::Blue() );
						DrawText( z1, "z1", aeColor::Blue() );
						debug.AddLine( z0, z1, aeColor::Blue() );

						if ( ( p0.z - z0.z ) * ( p0.z - z1.z ) < 0.0f )
						{
							float t2 = ( p0.z - e.z ) / d.z;
							aeFloat3 c0 = e + d * t2;
							debug.AddSphere( c0, 0.05f, aeColor::Green(), 8 );
							DrawText( c0, "c0", aeColor::Green() );
							hit0 = true;
						}

						if ( ( p1.z - z0.z ) * ( p1.z - z1.z ) < 0.0f )
						{
							float t2 = ( p1.z - e.z ) / d.z;
							aeFloat3 c1 = e + d * t2;
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
					aeFloat2 offset = aeFloat2::FromAngle( aeMath::TWO_PI * i / 8.0f );
					debug.AddLine( p0 + aeFloat3( offset ), p1 + aeFloat3( offset ), aeColor::Blue() );
				}

				break;
			}
			case 3:
			{
				infoText.Append( "Sphere-Triangle (static)\n" );
				
				aeFloat3 triangle[] =
				{
					aeFloat3( -1.0f, -1.5f, -1.0f ) * 2.0f,
					aeFloat3( 1.0f, -0.5f, -1.0f ) * 2.0f,
					aeFloat3( 0.0f, 1.5f, 1.0f ) * 2.0f,
				};
				aeFloat3 triangleCenter = ( triangle[ 0 ] + triangle[ 1 ] + triangle[ 2 ] ) / 3.0f;
				aeFloat3 normal = ( triangle[ 1 ] - triangle[ 0 ] ) % ( triangle[ 2 ] - triangle[ 0 ] );
				normal.SafeNormalize();
				debug.AddLine( triangle[ 0 ], triangle[ 1 ], aeColor::Red() );
				debug.AddLine( triangle[ 1 ], triangle[ 2 ], aeColor::Red() );
				debug.AddLine( triangle[ 2 ], triangle[ 0 ], aeColor::Red() );
				debug.AddLine( triangleCenter, triangleCenter + normal, aeColor::Red() );
				
				static float r = 1.0f;
				if ( input.GetState()->Get( aeKey::Num3 ) ) r -= 0.016f;
				if ( input.GetState()->Get( aeKey::Num4 ) ) r += 0.016f;
				r = aeMath::Clip( r, 0.01f, 8.0f );

				aeFloat3 nearestIntersectionPoint;
				aeSphere sphere( raySource + ray, r );
				if ( sphere.IntersectTriangle( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ], &nearestIntersectionPoint ) )
				{
					debug.AddSphere( raySource + ray, r, aeColor::Green(), 16 );
					debug.AddSphere( nearestIntersectionPoint, 0.05f, aeColor::Green(), 8 );
				}
				else
				{
					debug.AddSphere( raySource + ray, r, aeColor::Red(), 16 );
				}
				break;
			}
			default:
				break;
		}

		text.Add( aeFloat3( 50.0f, 50.0f, 0.0f ), aeFloat2( text.GetFontSize() * 2.0f ), infoText.c_str(), aeColor::Red(), 0, 0 );
		debug.AddLine( raySource, raySource + ray, aeColor::Red() );

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
