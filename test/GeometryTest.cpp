//------------------------------------------------------------------------------
// GeometryTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2026 John Hughes
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
#include <catch2/catch_test_macros.hpp>
#include "aether.h"

//------------------------------------------------------------------------------
// Geometry test helpers
//------------------------------------------------------------------------------
namespace
{
bool Approx( float a, float b, float epsilon = 0.001f ) { return std::abs( a - b ) < epsilon; }
bool Approx( const ae::Vec3& a, const ae::Vec3& b, float epsilon = 0.001f ) { return Approx( a.x, b.x, epsilon ) && Approx( a.y, b.y, epsilon ) && Approx( a.z, b.z, epsilon ); }
}

//------------------------------------------------------------------------------
// ae::Triangle constructor tests
//------------------------------------------------------------------------------
TEST_CASE( "Triangle constructors", "[ae::Triangle]" )
{
	ae::Triangle t1( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	REQUIRE( t1.vertices[ 0 ] == ae::Vec3( 0, 0, 0 ) );
	REQUIRE( t1.vertices[ 1 ] == ae::Vec3( 1, 0, 0 ) );
	REQUIRE( t1.vertices[ 2 ] == ae::Vec3( 0, 1, 0 ) );

	ae::Vec3 verts[ 3 ] = { ae::Vec3( 1, 2, 3 ), ae::Vec3( 4, 5, 6 ), ae::Vec3( 7, 8, 9 ) };
	ae::Triangle t2( verts );
	REQUIRE( t2.vertices[ 0 ] == ae::Vec3( 1, 2, 3 ) );
	REQUIRE( t2.vertices[ 1 ] == ae::Vec3( 4, 5, 6 ) );
	REQUIRE( t2.vertices[ 2 ] == ae::Vec3( 7, 8, 9 ) );

	const ae::Vec3* ptr = verts;
	ae::Triangle t3( ptr );
	REQUIRE( t3.vertices[ 0 ] == ae::Vec3( 1, 2, 3 ) );
	REQUIRE( t3.vertices[ 1 ] == ae::Vec3( 4, 5, 6 ) );
	REQUIRE( t3.vertices[ 2 ] == ae::Vec3( 7, 8, 9 ) );
}

//------------------------------------------------------------------------------
// ae::Triangle normal tests
//------------------------------------------------------------------------------
TEST_CASE( "Triangle normals", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	ae::Vec3 ccwNormal = t.CounterClockwiseNormal();
	REQUIRE( Approx( ccwNormal, ae::Vec3( 0, 0, 1 ) ) );

	ae::Vec3 cwNormal = t.ClockwiseNormal();
	REQUIRE( Approx( cwNormal, ae::Vec3( 0, 0, -1 ) ) );
}

//------------------------------------------------------------------------------
// ae::Triangle ray intersection tests
//------------------------------------------------------------------------------
TEST_CASE( "Triangle ray intersection - hit center", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	ae::Vec3 hit;
	ae::Vec3 normal;
	float t_val;
	
	bool result = t.IntersectRay( ae::Vec3( 0.25f, 0.25f, 1.0f ), ae::Vec3( 0, 0, -2.0f ), true, false, &hit, &normal, &t_val );
	REQUIRE( result );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( t_val, 0.5f ) );
	REQUIRE( Approx( normal, ae::Vec3( 0, 0, 1 ) ) );
}

TEST_CASE( "Triangle ray intersection - miss", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	bool result = t.IntersectRay( ae::Vec3( 2.0f, 2.0f, 1.0f ), ae::Vec3( 0, 0, -2.0f ), true, true, nullptr, nullptr, nullptr );
	REQUIRE( !result );
}

TEST_CASE( "Triangle ray intersection - backface culling", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	// Ray from below (CCW normal points +Z, so this is backface)
	bool resultCCW = t.IntersectRay( ae::Vec3( 0.25f, 0.25f, -1.0f ), ae::Vec3( 0, 0, 2.0f ), true, false, nullptr, nullptr, nullptr );
	REQUIRE( !resultCCW );
	
	// Same ray should hit with CW culling
	bool resultCW = t.IntersectRay( ae::Vec3( 0.25f, 0.25f, -1.0f ), ae::Vec3( 0, 0, 2.0f ), false, true, nullptr, nullptr, nullptr );
	REQUIRE( resultCW );
}

//------------------------------------------------------------------------------
// ae::Triangle closest point tests
//------------------------------------------------------------------------------
TEST_CASE( "Triangle closest point - on face", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	ae::Vec3 closest = t.ClosestPoint( ae::Vec3( 0.25f, 0.25f, 1.0f ) );
	REQUIRE( Approx( closest, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
}

TEST_CASE( "Triangle closest point - vertex regions", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	ae::Vec3 closest0 = t.ClosestPoint( ae::Vec3( -1.0f, -1.0f, 0.0f ) );
	REQUIRE( Approx( closest0, ae::Vec3( 0, 0, 0 ) ) );
	
	ae::Vec3 closest1 = t.ClosestPoint( ae::Vec3( 2.0f, -1.0f, 0.0f ) );
	REQUIRE( Approx( closest1, ae::Vec3( 1, 0, 0 ) ) );
	
	ae::Vec3 closest2 = t.ClosestPoint( ae::Vec3( -1.0f, 2.0f, 0.0f ) );
	REQUIRE( Approx( closest2, ae::Vec3( 0, 1, 0 ) ) );
}

TEST_CASE( "Triangle closest point - edge regions", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	ae::Vec3 closestAB = t.ClosestPoint( ae::Vec3( 0.5f, -0.5f, 0.0f ) );
	REQUIRE( Approx( closestAB, ae::Vec3( 0.5f, 0.0f, 0.0f ) ) );
	
	ae::Vec3 closestAC = t.ClosestPoint( ae::Vec3( -0.5f, 0.5f, 0.0f ) );
	REQUIRE( Approx( closestAC, ae::Vec3( 0.0f, 0.5f, 0.0f ) ) );
	
	ae::Vec3 closestBC = t.ClosestPoint( ae::Vec3( 0.75f, 0.75f, 0.0f ) );
	REQUIRE( Approx( closestBC, ae::Vec3( 0.5f, 0.5f, 0.0f ) ) );
}

//------------------------------------------------------------------------------
// ae::Triangle SphereCast tests
// Triangle: (0,0,0),(1,0,0),(0,1,0) in XY plane, CCW normal = (0,0,1)
//------------------------------------------------------------------------------
TEST_CASE( "Triangle sphere cast - face hit CCW", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Sphere above the triangle (+Z = CCW side), moving straight down
	// dist=2, rate=-4, t=(0.5-2)/-4 = 0.375; planeHit=(0.25,0.25,0)
	ae::Vec3 hit, normal;
	float tVal;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, 2.0f ), ae::Vec3( 0.0f, 0.0f, -4.0f ), 0.5f, true, false, &hit, &normal, &tVal ) );
	REQUIRE( Approx( tVal, 0.375f ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "Triangle sphere cast - face miss", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Sphere aimed far outside the triangle bounds, small radius
	REQUIRE_FALSE( t.SphereCast( ae::Vec3( 5.0f, 5.0f, 2.0f ), ae::Vec3( 0.0f, 0.0f, -4.0f ), 0.1f, true, false ) );
}

TEST_CASE( "Triangle sphere cast - backface culled", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Sphere below the triangle (CW side), moving up; CCW-only should miss
	REQUIRE_FALSE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, -2.0f ), ae::Vec3( 0.0f, 0.0f, 4.0f ), 0.5f, true, false ) );

	// CW-only should hit the back face
	// dist=-2, rate=4, t=(-0.5-(-2))/4 = 0.375; planeHit=(0.25,0.25,0); normal=(0,0,-1)
	ae::Vec3 hit, normal;
	float tVal;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, -2.0f ), ae::Vec3( 0.0f, 0.0f, 4.0f ), 0.5f, false, true, &hit, &normal, &tVal ) );
	REQUIRE( Approx( tVal, 0.375f ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
}

TEST_CASE( "Triangle sphere cast - edge hit", "[ae::Triangle]" )
{
	// Larger triangle so the sphere hits the AB edge before any vertex
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 2, 0, 0 ), ae::Vec3( 0, 2, 0 ) );

	// Sphere moving in +Y in the z=0 plane toward edge AB; rate=0 so no face hit
	// Edge AB: a=36, b=-36, c=8; discr=144; t=(36-12)/72 = 1/3
	ae::Vec3 hit, normal;
	float tVal;
	REQUIRE( t.SphereCast( ae::Vec3( 1.0f, -3.0f, 0.0f ), ae::Vec3( 0.0f, 6.0f, 0.0f ), 1.0f, true, false, &hit, &normal, &tVal ) );
	REQUIRE( Approx( tVal, 1.0f / 3.0f ) );
	REQUIRE( Approx( hit, ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, -1.0f, 0.0f ) ) );
}

TEST_CASE( "Triangle sphere cast - vertex hit", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Sphere path passes directly through vertex A=(0,0,0); rate=0 so no face hit
	// a=72, b=-72, c=17.75; discr=72; t=(72-sqrt(72))/144 = (3-sqrt(0.125))/6
	ae::Vec3 hit, normal;
	float tVal;
	REQUIRE( t.SphereCast( ae::Vec3( -3.0f, -3.0f, 0.0f ), ae::Vec3( 6.0f, 6.0f, 0.0f ), 0.5f, true, false, &hit, &normal, &tVal ) );
	const float expectedT = ( 3.0f - sqrtf( 0.125f ) ) / 6.0f;
	REQUIRE( Approx( tVal, expectedT ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	const ae::Vec3 expectedNormal( -1.0f / sqrtf( 2.0f ), -1.0f / sqrtf( 2.0f ), 0.0f );
	REQUIRE( Approx( normal, expectedNormal ) );
}

TEST_CASE( "Triangle sphere cast - no flags returns false", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	REQUIRE_FALSE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, 2.0f ), ae::Vec3( 0.0f, 0.0f, -4.0f ), 0.5f, false, false ) );
}

TEST_CASE( "Triangle sphere cast - both sides hit", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Both sides enabled; sphere from CW side, moving up → hits back face
	ae::Vec3 hit, normal;
	float tVal;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, -2.0f ), ae::Vec3( 0.0f, 0.0f, 4.0f ), 0.5f, true, true, &hit, &normal, &tVal ) );
	REQUIRE( Approx( tVal, 0.375f ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
}
