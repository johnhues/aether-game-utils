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
	float distance;
	
	bool result = t.Raycast( ae::Vec3( 0.25f, 0.25f, 1.0f ), ae::Vec3( 0, 0, -2.0f ), true, false, &hit, &normal, &distance );
	REQUIRE( result );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( distance, 1.0f ) );
	REQUIRE( Approx( normal, ae::Vec3( 0, 0, 1 ) ) );
}

TEST_CASE( "Triangle ray intersection - miss", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	bool result = t.Raycast( ae::Vec3( 2.0f, 2.0f, 1.0f ), ae::Vec3( 0, 0, -2.0f ), true, true, nullptr, nullptr, nullptr );
	REQUIRE( !result );
}

TEST_CASE( "Triangle ray intersection - backface culling", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );
	
	// Ray from below (CCW normal points +Z, so this is backface)
	bool resultCCW = t.Raycast( ae::Vec3( 0.25f, 0.25f, -1.0f ), ae::Vec3( 0, 0, 2.0f ), true, false, nullptr, nullptr, nullptr );
	REQUIRE( !resultCCW );
	
	// Same ray should hit with CW culling
	bool resultCW = t.Raycast( ae::Vec3( 0.25f, 0.25f, -1.0f ), ae::Vec3( 0, 0, 2.0f ), false, true, nullptr, nullptr, nullptr );
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
	float distance;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, 2.0f ), ae::Vec3( 0.0f, 0.0f, -4.0f ), 0.5f, true, false, &hit, &normal, &distance ) );
	REQUIRE( Approx( distance, 1.5f ) );
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
	float distance;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, -2.0f ), ae::Vec3( 0.0f, 0.0f, 4.0f ), 0.5f, false, true, &hit, &normal, &distance ) );
	REQUIRE( Approx( distance, 1.5f ) );
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
	float distance;
	REQUIRE( t.SphereCast( ae::Vec3( 1.0f, -3.0f, 0.0f ), ae::Vec3( 0.0f, 6.0f, 0.0f ), 1.0f, true, false, &hit, &normal, &distance ) );
	REQUIRE( Approx( distance, 2.0f ) );
	REQUIRE( Approx( hit, ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, -1.0f, 0.0f ) ) );
}

TEST_CASE( "Triangle sphere cast - vertex hit", "[ae::Triangle]" )
{
	ae::Triangle t( ae::Vec3( 0, 0, 0 ), ae::Vec3( 1, 0, 0 ), ae::Vec3( 0, 1, 0 ) );

	// Sphere path passes directly through vertex A=(0,0,0); rate=0 so no face hit
	// a=72, b=-72, c=17.75; discr=72; t=(72-sqrt(72))/144 = (3-sqrt(0.125))/6
	ae::Vec3 hit, normal;
	float distance;
	REQUIRE( t.SphereCast( ae::Vec3( -3.0f, -3.0f, 0.0f ), ae::Vec3( 6.0f, 6.0f, 0.0f ), 0.5f, true, false, &hit, &normal, &distance ) );
	const float expectedT = ( 3.0f - sqrtf( 0.125f ) ) / 6.0f;
	const float expectedDistance = ae::Vec3( 6.0f, 6.0f, 0.0f ).Length() * expectedT;
	REQUIRE( Approx( distance, expectedDistance ) );
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
	float distance;
	REQUIRE( t.SphereCast( ae::Vec3( 0.25f, 0.25f, -2.0f ), ae::Vec3( 0.0f, 0.0f, 4.0f ), 0.5f, true, true, &hit, &normal, &distance ) );
	REQUIRE( Approx( distance, 1.5f ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.25f, 0.25f, 0.0f ) ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
}

//------------------------------------------------------------------------------
// ae::RaycastResult early out tests
//------------------------------------------------------------------------------
TEST_CASE( "RaycastResult sphere early out uses distance", "[ae::RaycastResult]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, -100.0f );
	params.maxHits = 1;

	ae::RaycastResult result;
	result.hits.Append( {} );
	result.hits[ 0 ].distance = 10.0f;

	const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, -2.0f ), 0.5f );
	REQUIRE( !result.EarlyOut( params, sphere ) );
}

TEST_CASE( "RaycastResult obb early out uses distance", "[ae::RaycastResult]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, -100.0f );
	params.maxHits = 1;

	ae::RaycastResult result;
	result.hits.Append( {} );
	result.hits[ 0 ].distance = 10.0f;

	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, -2.0f ) );
	REQUIRE( !result.EarlyOut( params, obb ) );
}

//------------------------------------------------------------------------------
// Additional geometry helpers
//------------------------------------------------------------------------------
namespace
{
bool Approx( const ae::Vec2& a, const ae::Vec2& b, float epsilon = 0.001f )
{
	return std::abs( a.x - b.x ) < epsilon && std::abs( a.y - b.y ) < epsilon;
}
} // namespace

//------------------------------------------------------------------------------
// ae::Plane tests
//------------------------------------------------------------------------------
TEST_CASE( "Plane basics", "[geometry]" )
{
	const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	REQUIRE( Approx( plane.GetNormal(), ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
	REQUIRE( Approx( plane.GetClosestPointToOrigin(), ae::Vec3( 0.0f, 0.0f, 5.0f ) ) );
	REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 0.0f, 0.0f, 6.0f ) ), 1.0f ) );
	REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 0.0f, 0.0f, 4.0f ) ), -1.0f ) );
	REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 0.0f, 0.0f, 5.0f ) ), 0.0f ) );
	const ae::Vec3 cp = plane.GetClosestPoint( ae::Vec3( 3.0f, 4.0f, 8.0f ) );
	REQUIRE( Approx( cp, ae::Vec3( 3.0f, 4.0f, 5.0f ) ) );
}

TEST_CASE( "Plane Vec4 constructor and cast", "[geometry]" )
{
	const ae::Plane p0( ae::Vec3( 0.0f, 0.0f, 3.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Vec4 v = (ae::Vec4)p0;
	const ae::Plane p1( v );
	REQUIRE( Approx( p1.GetNormal(), ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
	REQUIRE( Approx( p1.GetClosestPointToOrigin(), ae::Vec3( 0.0f, 0.0f, 3.0f ) ) );
}

TEST_CASE( "Plane from 3 points", "[geometry]" )
{
	// Three points in the z=2 plane — normal should be +/-Z
	const ae::Plane plane( ae::Vec3( 1.0f, 0.0f, 2.0f ), ae::Vec3( 0.0f, 1.0f, 2.0f ), ae::Vec3( 0.0f, 0.0f, 2.0f ) );
	REQUIRE( std::abs( plane.GetNormal().z ) > 0.9f );
	REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 5.0f, 5.0f, 2.0f ) ), 0.0f ) );
}

TEST_CASE( "Plane IntersectLine", "[geometry]" )
{
	const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	ae::Vec3 hit;
	float t;
	// Line p=(0,-2,0) + d=(0,4,0) crosses the plane at t=0.5
	REQUIRE( plane.IntersectLine( ae::Vec3( 0.0f, -2.0f, 0.0f ), ae::Vec3( 0.0f, 4.0f, 0.0f ), &hit, &t ) );
	REQUIRE( Approx( t, 0.5f ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	// Line parallel to the plane — no intersection
	REQUIRE_FALSE( plane.IntersectLine( ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ), nullptr, nullptr ) );
}

TEST_CASE( "Plane Raycast", "[geometry]" )
{
	const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	ae::Vec3 hitOut;
	float dist;
	// Ray from (0,-5,0) heading +Y — hits plane at origin, distance 5
	REQUIRE( plane.Raycast( ae::Vec3( 0.0f, -5.0f, 0.0f ), ae::Vec3( 0.0f, 10.0f, 0.0f ), &hitOut, &dist ) );
	REQUIRE( Approx( hitOut, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( dist, 5.0f ) );
	// Ray starting on the +Y side, moving further +Y — plane is behind the ray
	REQUIRE_FALSE( plane.Raycast( ae::Vec3( 0.0f, 5.0f, 0.0f ), ae::Vec3( 0.0f, 10.0f, 0.0f ), nullptr, nullptr ) );
}

//------------------------------------------------------------------------------
// ae::Line tests
//------------------------------------------------------------------------------
TEST_CASE( "Line GetClosest and GetDistance", "[geometry]" )
{
	const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	float dist;
	const ae::Vec3 closest = line.GetClosest( ae::Vec3( 0.5f, 3.0f, 0.0f ), &dist );
	REQUIRE( Approx( closest, ae::Vec3( 0.5f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( dist, 3.0f ) );
	REQUIRE( Approx( line.GetDistance( ae::Vec3( 0.5f, 3.0f, 0.0f ) ), 3.0f ) );
}

TEST_CASE( "Line extends infinitely past endpoints", "[geometry]" )
{
	const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
	// Point far past the second endpoint — infinite line still projects correctly
	const ae::Vec3 closest = line.GetClosest( ae::Vec3( 5.0f, 2.0f, 0.0f ) );
	REQUIRE( Approx( closest, ae::Vec3( 5.0f, 0.0f, 0.0f ) ) );
	// Point behind the first endpoint
	const ae::Vec3 behind = line.GetClosest( ae::Vec3( -4.0f, 1.0f, 0.0f ) );
	REQUIRE( Approx( behind, ae::Vec3( -4.0f, 0.0f, 0.0f ) ) );
}

//------------------------------------------------------------------------------
// ae::LineSegment tests
//------------------------------------------------------------------------------
TEST_CASE( "LineSegment properties", "[geometry]" )
{
	const ae::LineSegment seg( ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 5.0f, 0.0f, 0.0f ) );
	REQUIRE( Approx( seg.GetStart(), ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( seg.GetEnd(), ae::Vec3( 5.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( seg.GetLength(), 4.0f ) );
}

TEST_CASE( "LineSegment GetClosest clamps to endpoints", "[geometry]" )
{
	const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 4.0f, 0.0f, 0.0f ) );
	// Interior projection
	float dist;
	const ae::Vec3 mid = seg.GetClosest( ae::Vec3( 2.0f, 3.0f, 0.0f ), &dist );
	REQUIRE( Approx( mid, ae::Vec3( 2.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( dist, 3.0f ) );
	// Past the end — clamps to end vertex
	const ae::Vec3 pastEnd = seg.GetClosest( ae::Vec3( 10.0f, 1.0f, 0.0f ) );
	REQUIRE( Approx( pastEnd, ae::Vec3( 4.0f, 0.0f, 0.0f ) ) );
	// Before the start — clamps to start vertex
	const ae::Vec3 beforeStart = seg.GetClosest( ae::Vec3( -5.0f, 1.0f, 0.0f ) );
	REQUIRE( Approx( beforeStart, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "LineSegment GetDistance", "[geometry]" )
{
	const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 4.0f, 0.0f, 0.0f ) );
	REQUIRE( Approx( seg.GetDistance( ae::Vec3( 2.0f, 3.0f, 0.0f ) ), 3.0f ) );
	// Point before start: clamps to (0,0,0), distance = sqrt(9+16) = 5
	REQUIRE( Approx( seg.GetDistance( ae::Vec3( -3.0f, 4.0f, 0.0f ) ), 5.0f ) );
}

//------------------------------------------------------------------------------
// ae::Sphere tests
//------------------------------------------------------------------------------
TEST_CASE( "Sphere constructors", "[geometry]" )
{
	const ae::Sphere s( ae::Vec3( 1.0f, 2.0f, 3.0f ), 2.5f );
	REQUIRE( Approx( s.center, ae::Vec3( 1.0f, 2.0f, 3.0f ) ) );
	REQUIRE( Approx( s.radius, 2.5f ) );
}

TEST_CASE( "Sphere Expand", "[geometry]" )
{
	ae::Sphere s( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
	const ae::Vec3 pt( 0.0f, 0.0f, 5.0f );
	s.Expand( pt );
	// The sphere repositions to contain the point minimally
	const float distToPoint = ( s.center - pt ).Length();
	REQUIRE( distToPoint <= s.radius + 0.001f );
	// Original opposite pole (0,0,-1) should also still be contained
	const float distToOld = ( s.center - ae::Vec3( 0.0f, 0.0f, -1.0f ) ).Length();
	REQUIRE( distToOld <= s.radius + 0.001f );
}

TEST_CASE( "Sphere Raycast", "[geometry]" )
{
	const ae::Sphere s( ae::Vec3( 0.0f, 0.0f, 5.0f ), 1.0f );
	ae::Vec3 hit, normal;
	float dist;
	// Hit from outside — first surface at z=4
	REQUIRE( s.Raycast( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), &hit, &normal, &dist ) );
	REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 4.0f ) ) );
	REQUIRE( Approx( dist, 4.0f ) );
	REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
	// Miss — ray passes to the side
	REQUIRE_FALSE( s.Raycast( ae::Vec3( 5.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), nullptr, nullptr, nullptr ) );
	// Source inside sphere — still hits
	REQUIRE( s.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), nullptr, nullptr, nullptr ) );
}

TEST_CASE( "Sphere IntersectTriangle", "[geometry]" )
{
	// Sphere overlapping a large triangle in the XY plane
	const ae::Sphere s( ae::Vec3( 0.3f, 0.3f, 0.3f ), 1.0f );
	ae::Vec3 hitOut;
	REQUIRE( s.IntersectTriangle( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 2.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 2.0f, 0.0f ), &hitOut ) );
	// Sphere far away — no intersection
	const ae::Sphere sOff( ae::Vec3( 10.0f, 10.0f, 10.0f ), 1.0f );
	REQUIRE_FALSE( sOff.IntersectTriangle( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 2.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 2.0f, 0.0f ), nullptr ) );
}

TEST_CASE( "Sphere GetNearestPointOnSurface", "[geometry]" )
{
	const ae::Sphere s( ae::Vec3( 0.0f, 0.0f, 0.0f ), 3.0f );
	float signedDist;
	// Point outside — nearest surface is along the axis toward the point
	const ae::Vec3 surfPt = s.GetNearestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 6.0f ), &signedDist );
	REQUIRE( Approx( surfPt, ae::Vec3( 0.0f, 0.0f, 3.0f ) ) );
	REQUIRE( Approx( signedDist, 3.0f ) );
	// Point inside — nearest surface still on +Z
	const ae::Vec3 inPt = s.GetNearestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 1.5f ), &signedDist );
	REQUIRE( Approx( inPt, ae::Vec3( 0.0f, 0.0f, 3.0f ) ) );
	REQUIRE( signedDist < 0.0f );
}

//------------------------------------------------------------------------------
// ae::Circle tests
//------------------------------------------------------------------------------
TEST_CASE( "Circle basics", "[geometry]" )
{
	const ae::Circle c( ae::Vec2( 1.0f, 2.0f ), 3.0f );
	REQUIRE( Approx( c.GetCenter().x, 1.0f ) );
	REQUIRE( Approx( c.GetCenter().y, 2.0f ) );
	REQUIRE( Approx( c.GetRadius(), 3.0f ) );
	REQUIRE( Approx( ae::Circle::GetArea( 3.0f ), 3.14159265f * 9.0f ) );
}

TEST_CASE( "Circle Intersect", "[geometry]" )
{
	const ae::Circle a( ae::Vec2( 0.0f, 0.0f ), 2.0f );
	ae::Vec2 out;
	// Overlapping circles — out is midpoint of overlap zone on the line between centers
	const ae::Circle b( ae::Vec2( 3.0f, 0.0f ), 2.0f );
	REQUIRE( a.Intersect( b, &out ) );
	REQUIRE( ( out.x > 0.0f && out.x < 3.0f ) );
	REQUIRE( Approx( out.y, 0.0f ) );
	// Non-overlapping — far apart
	const ae::Circle c( ae::Vec2( 10.0f, 0.0f ), 1.0f );
	REQUIRE_FALSE( a.Intersect( c, nullptr ) );
	// Exactly touching at edge — distance equals sum of radii
	const ae::Circle d( ae::Vec2( 4.0f, 0.0f ), 2.0f );
	REQUIRE( a.Intersect( d, &out ) );
}

TEST_CASE( "Circle GetRandomPoint stays inside", "[geometry]" )
{
	const ae::Circle c( ae::Vec2( 5.0f, 5.0f ), 2.0f );
	uint64_t seed = 42;
	for( uint32_t i = 0; i < 32; i++ )
	{
		const ae::Vec2 pt = c.GetRandomPoint( &seed );
		const float dx = pt.x - 5.0f;
		const float dy = pt.y - 5.0f;
		REQUIRE( std::sqrt( dx * dx + dy * dy ) <= 2.0f + 0.001f );
	}
}

//------------------------------------------------------------------------------
// ae::AABB tests
//------------------------------------------------------------------------------
TEST_CASE( "AABB default empty then single-point expand", "[geometry]" )
{
	ae::AABB box;
	box.Expand( ae::Vec3( 3.0f, 4.0f, 5.0f ) );
	REQUIRE( Approx( box.GetMin(), ae::Vec3( 3.0f, 4.0f, 5.0f ) ) );
	REQUIRE( Approx( box.GetMax(), ae::Vec3( 3.0f, 4.0f, 5.0f ) ) );
	REQUIRE( Approx( box.GetHalfSize(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "AABB constructors and equality", "[geometry]" )
{
	const ae::AABB a( ae::Vec3( -1.0f, -2.0f, -3.0f ), ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	const ae::AABB b( ae::Vec3( -1.0f, -2.0f, -3.0f ), ae::Vec3( 1.0f, 2.0f, 3.0f ) );
	const ae::AABB c( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	REQUIRE( a == b );
	REQUIRE( a != c );
	REQUIRE( Approx( a.GetMin(), ae::Vec3( -1.0f, -2.0f, -3.0f ) ) );
	REQUIRE( Approx( a.GetMax(), ae::Vec3( 1.0f, 2.0f, 3.0f ) ) );
	REQUIRE( Approx( a.GetCenter(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( a.GetHalfSize(), ae::Vec3( 1.0f, 2.0f, 3.0f ) ) );
}

TEST_CASE( "AABB from Sphere", "[geometry]" )
{
	const ae::AABB box( ae::Sphere( ae::Vec3( 1.0f, 2.0f, 3.0f ), 2.0f ) );
	REQUIRE( Approx( box.GetMin(), ae::Vec3( -1.0f, 0.0f, 1.0f ) ) );
	REQUIRE( Approx( box.GetMax(), ae::Vec3( 3.0f, 4.0f, 5.0f ) ) );
}

TEST_CASE( "AABB Expand variants", "[geometry]" )
{
	ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	// Expand(Vec3) — point outside current bounds
	box.Expand( ae::Vec3( 3.0f, 0.0f, 0.0f ) );
	REQUIRE( Approx( box.GetMax().x, 3.0f ) );
	REQUIRE( Approx( box.GetMin().x, -1.0f ) );
	// Expand(AABB) — merge in another box
	box.Expand( ae::AABB( ae::Vec3( -5.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( box.GetMin().x, -5.0f ) );
	// Expand(float) — pad all sides uniformly
	ae::AABB box2( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 2.0f, 2.0f, 2.0f ) );
	box2.Expand( 1.0f );
	REQUIRE( Approx( box2.GetMin(), ae::Vec3( -1.0f, -1.0f, -1.0f ) ) );
	REQUIRE( Approx( box2.GetMax(), ae::Vec3( 3.0f, 3.0f, 3.0f ) ) );
}

TEST_CASE( "AABB GetTransform", "[geometry]" )
{
	// Box (-2,-1,0) to (2,1,4): full size (4,2,4), center (0,0,2)
	const ae::AABB box( ae::Vec3( -2.0f, -1.0f, 0.0f ), ae::Vec3( 2.0f, 1.0f, 4.0f ) );
	const ae::Matrix4 t = box.GetTransform();
	// GetTransform uses Scaling(max-min) then SetAxis(3, center)
	const ae::Vec3 fullSize = box.GetMax() - box.GetMin();
	REQUIRE( Approx( t.columns[ 0 ].x, fullSize.x ) );
	REQUIRE( Approx( t.columns[ 1 ].y, fullSize.y ) );
	REQUIRE( Approx( t.columns[ 2 ].z, fullSize.z ) );
	const ae::Vec3 center = box.GetCenter();
	REQUIRE( Approx( t.columns[ 3 ].x, center.x ) );
	REQUIRE( Approx( t.columns[ 3 ].y, center.y ) );
	REQUIRE( Approx( t.columns[ 3 ].z, center.z ) );
}

TEST_CASE( "AABB Contains", "[geometry]" )
{
	const ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	REQUIRE( box.Contains( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );     // interior
	REQUIRE( box.Contains( ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );     // on surface
	REQUIRE_FALSE( box.Contains( ae::Vec3( 1.1f, 0.0f, 0.0f ) ) ); // exterior
}

TEST_CASE( "AABB Intersect", "[geometry]" )
{
	const ae::AABB a( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	const ae::AABB b( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 2.0f, 2.0f, 2.0f ) );    // overlapping
	const ae::AABB c( ae::Vec3( 3.0f, 3.0f, 3.0f ), ae::Vec3( 5.0f, 5.0f, 5.0f ) );    // non-overlapping
	const ae::AABB d( ae::Vec3( 1.0f, -1.0f, -1.0f ), ae::Vec3( 2.0f, 1.0f, 1.0f ) );  // touching face
	REQUIRE( a.Intersect( b ) );
	REQUIRE_FALSE( a.Intersect( c ) );
	REQUIRE( a.Intersect( d ) );
}

TEST_CASE( "AABB GetSignedDistanceFromSurface", "[geometry]" )
{
	// Unit box: GetHalfSize=(1,1,1), center=(0,0,0)
	const ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	REQUIRE( Approx( box.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ), -1.0f ) ); // center, 1 unit from nearest face
	REQUIRE( Approx( box.GetSignedDistanceFromSurface( ae::Vec3( 2.0f, 0.0f, 0.0f ) ), 1.0f ) );  // 1 unit outside
	REQUIRE( box.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) < 0.0f );
	REQUIRE( box.GetSignedDistanceFromSurface( ae::Vec3( 2.0f, 0.0f, 0.0f ) ) > 0.0f );
}

TEST_CASE( "AABB GetClosestPointOnSurface", "[geometry]" )
{
	const ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	bool contains;
	// Point inside close to +Z face → nearest surface is +Z
	const ae::Vec3 inPt = box.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 0.9f ), &contains );
	REQUIRE( Approx( inPt.z, 1.0f ) );
	REQUIRE( contains );
	// Point outside along +X → nearest surface at x=1
	const ae::Vec3 outPt = box.GetClosestPointOnSurface( ae::Vec3( 3.0f, 0.0f, 0.0f ), &contains );
	REQUIRE( Approx( outPt, ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE_FALSE( contains );
}

TEST_CASE( "AABB IntersectLine", "[geometry]" )
{
	// Unit box: faces at ±1 on each axis
	const ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	float t0, t1;
	// Line from (0,0,-3) in +Z direction: enters at t=2 (z=-1), exits at t=4 (z=1)
	REQUIRE( box.IntersectLine( ae::Vec3( 0.0f, 0.0f, -3.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), &t0, &t1 ) );
	REQUIRE( Approx( t0, 2.0f ) );
	REQUIRE( Approx( t1, 4.0f ) );
	REQUIRE( t0 < t1 );
	// Line that misses entirely
	REQUIRE_FALSE( box.IntersectLine( ae::Vec3( 5.0f, 5.0f, -3.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), &t0, &t1 ) );
}

TEST_CASE( "AABB Raycast", "[geometry]" )
{
	const ae::AABB box( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	ae::Vec3 hitOut, normOut;
	float dist;
	// Hit from outside: source at (0,0,-5), ray (0,0,10) → first hit at z=-1, dist=4
	REQUIRE( box.Raycast( ae::Vec3( 0.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), &hitOut, &normOut, &dist ) );
	REQUIRE( Approx( hitOut, ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
	REQUIRE( Approx( dist, 4.0f ) );
	// Miss
	REQUIRE_FALSE( box.Raycast( ae::Vec3( 5.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), nullptr, nullptr, nullptr ) );
	// Source inside — returns true
	REQUIRE( box.Raycast( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), nullptr, nullptr, nullptr ) );
}

//------------------------------------------------------------------------------
// ae::OBB tests
// OBB::SetTransform stores halfSize = column_length * 0.5, so
// ae::Matrix4::Translation() (unit columns) → halfSize = (0.5, 0.5, 0.5)
//------------------------------------------------------------------------------
TEST_CASE( "OBB from Matrix4", "[geometry]" )
{
	// Translation-only matrix: unit columns → halfSize = (0.5,0.5,0.5), center = translation
	const ae::OBB obb( ae::Matrix4::Translation( 1.0f, 2.0f, 3.0f ) );
	REQUIRE( Approx( obb.GetCenter(), ae::Vec3( 1.0f, 2.0f, 3.0f ) ) );
	REQUIRE( Approx( obb.GetAxis( 0 ), ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
	REQUIRE( Approx( obb.GetAxis( 1 ), ae::Vec3( 0.0f, 1.0f, 0.0f ) ) );
	REQUIRE( Approx( obb.GetAxis( 2 ), ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
	REQUIRE( Approx( obb.GetHalfSize(), ae::Vec3( 0.5f, 0.5f, 0.5f ) ) );
}

TEST_CASE( "OBB equality and SetTransform and GetTransform", "[geometry]" )
{
	const ae::Matrix4 m = ae::Matrix4::Translation( 1.0f, 0.0f, 0.0f );
	const ae::OBB a( m );
	ae::OBB b;
	b.SetTransform( m );
	REQUIRE( a == b );
	const ae::OBB c( ae::Matrix4::Translation( 2.0f, 0.0f, 0.0f ) );
	REQUIRE( a != c );
	// GetTransform round-trips center to the translation column
	const ae::Matrix4 t = a.GetTransform();
	REQUIRE( Approx( t.columns[ 3 ].x, 1.0f ) );
	REQUIRE( Approx( t.columns[ 3 ].y, 0.0f ) );
	REQUIRE( Approx( t.columns[ 3 ].z, 0.0f ) );
}

TEST_CASE( "OBB GetSignedDistanceFromSurface", "[geometry]" )
{
	// OBB at origin with halfSize=(0.5,0.5,0.5)
	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) );
	REQUIRE( Approx( obb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ), -0.5f ) ); // center
	REQUIRE( Approx( obb.GetSignedDistanceFromSurface( ae::Vec3( 3.0f, 0.0f, 0.0f ) ), 2.5f ) );  // outside
	REQUIRE( obb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) < 0.0f );
	REQUIRE( obb.GetSignedDistanceFromSurface( ae::Vec3( 3.0f, 0.0f, 0.0f ) ) > 0.0f );
}

TEST_CASE( "OBB GetClosestPointOnSurface", "[geometry]" )
{
	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) );
	bool contains;
	// Point inside near +Z face → closest on +Z
	const ae::Vec3 inPt = obb.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 0.3f ), &contains );
	REQUIRE( Approx( inPt.z, 0.5f ) );
	REQUIRE( contains );
	// Point outside along +X → closest on +X face
	const ae::Vec3 outPt = obb.GetClosestPointOnSurface( ae::Vec3( 3.0f, 0.0f, 0.0f ), &contains );
	REQUIRE( Approx( outPt, ae::Vec3( 0.5f, 0.0f, 0.0f ) ) );
	REQUIRE_FALSE( contains );
}

TEST_CASE( "OBB IntersectLine", "[geometry]" )
{
	// halfSize=(0.5,0.5,0.5): faces at ±0.5 on each axis
	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) );
	float t0, t1;
	// Line from (0,0,-5) in +Z: enters at t=4.5 (z=-0.5), exits at t=5.5 (z=0.5)
	REQUIRE( obb.IntersectLine( ae::Vec3( 0.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), &t0, &t1 ) );
	REQUIRE( Approx( t0, 4.5f ) );
	REQUIRE( Approx( t1, 5.5f ) );
	// Line that misses entirely
	REQUIRE_FALSE( obb.IntersectLine( ae::Vec3( 5.0f, 5.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), &t0, &t1 ) );
}

TEST_CASE( "OBB Raycast", "[geometry]" )
{
	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) );
	ae::Vec3 hitOut, normOut;
	float dist;
	// Hit from outside: source (0,0,-5), ray (0,0,10) → first hit at z=-0.5, dist=4.5
	REQUIRE( obb.Raycast( ae::Vec3( 0.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), &hitOut, &normOut, &dist ) );
	REQUIRE( Approx( hitOut, ae::Vec3( 0.0f, 0.0f, -0.5f ) ) );
	REQUIRE( Approx( dist, 4.5f ) );
	// Miss
	REQUIRE_FALSE( obb.Raycast( ae::Vec3( 5.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 10.0f ), nullptr, nullptr, nullptr ) );
}

TEST_CASE( "OBB GetAABB", "[geometry]" )
{
	// Axis-aligned OBB from Translation(1,2,3): center=(1,2,3), halfSize=(0.5,0.5,0.5)
	const ae::OBB obb( ae::Matrix4::Translation( 1.0f, 2.0f, 3.0f ) );
	const ae::AABB aabb = obb.GetAABB();
	REQUIRE( Approx( aabb.GetMin(), ae::Vec3( 0.5f, 1.5f, 2.5f ) ) );
	REQUIRE( Approx( aabb.GetMax(), ae::Vec3( 1.5f, 2.5f, 3.5f ) ) );
}

//------------------------------------------------------------------------------
// ae::Frustum tests
// Normals point OUTWARD: Intersects returns false when GetSignedDistance > 0.
// An interior point therefore has GetSignedDistance <= 0 for all 6 planes.
//------------------------------------------------------------------------------
TEST_CASE( "Frustum Intersects point", "[geometry]" )
{
	const ae::Matrix4 view = ae::Matrix4::WorldToView(
		ae::Vec3( 0.0f, 0.0f, 5.0f ),
		ae::Vec3( 0.0f, 0.0f, -1.0f ),
		ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( 1.5707963f, 1.0f, 0.1f, 100.0f );
	const ae::Frustum frustum( proj * view );
	// Camera looks -Z from z=5; near=4.9, far=-95 in world Z
	// (0,0,0) is 5 units ahead of camera — well inside
	REQUIRE( frustum.Intersects( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
	// (0,0,200) is behind the camera — outside
	REQUIRE_FALSE( frustum.Intersects( ae::Vec3( 0.0f, 0.0f, 200.0f ) ) );
}

TEST_CASE( "Frustum Intersects sphere", "[geometry]" )
{
	const ae::Matrix4 view = ae::Matrix4::WorldToView(
		ae::Vec3( 0.0f, 0.0f, 5.0f ),
		ae::Vec3( 0.0f, 0.0f, -1.0f ),
		ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( 1.5707963f, 1.0f, 0.1f, 100.0f );
	const ae::Frustum frustum( proj * view );
	// Sphere fully inside
	REQUIRE( frustum.Intersects( ae::Sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 0.5f ) ) );
	// Sphere far behind camera — fully outside
	REQUIRE_FALSE( frustum.Intersects( ae::Sphere( ae::Vec3( 0.0f, 0.0f, 200.0f ), 0.5f ) ) );
	// Sphere near the near-plane, large enough to straddle it — still intersects
	REQUIRE( frustum.Intersects( ae::Sphere( ae::Vec3( 0.0f, 0.0f, 4.5f ), 1.5f ) ) );
}

TEST_CASE( "Frustum GetPlane sign convention", "[geometry]" )
{
	const ae::Matrix4 view = ae::Matrix4::WorldToView(
		ae::Vec3( 0.0f, 0.0f, 5.0f ),
		ae::Vec3( 0.0f, 0.0f, -1.0f ),
		ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( 1.5707963f, 1.0f, 0.1f, 100.0f );
	const ae::Frustum frustum( proj * view );
	// Interior point — all planes should have GetSignedDistance <= 0 (outward normals)
	const ae::Vec3 inside( 0.0f, 0.0f, 0.0f );
	const ae::Frustum::Plane ids[] = {
		ae::Frustum::Plane::Near, ae::Frustum::Plane::Far,
		ae::Frustum::Plane::Left, ae::Frustum::Plane::Right,
		ae::Frustum::Plane::Top, ae::Frustum::Plane::Bottom
	};
	for( uint32_t i = 0; i < 6; i++ )
	{
		REQUIRE( frustum.GetPlane( ids[ i ] ).GetSignedDistance( inside ) <= 0.001f );
	}
}

//------------------------------------------------------------------------------
// ae::Rect tests
//------------------------------------------------------------------------------
TEST_CASE( "Rect FromCenterAndSize", "[geometry]" )
{
	const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 4.0f, 2.0f ) );
	REQUIRE( Approx( r.GetMin(), ae::Vec2( -2.0f, -1.0f ) ) );
	REQUIRE( Approx( r.GetMax(), ae::Vec2( 2.0f, 1.0f ) ) );
	REQUIRE( Approx( r.GetCenter(), ae::Vec2( 0.0f, 0.0f ) ) );
	REQUIRE( Approx( r.GetSize(), ae::Vec2( 4.0f, 2.0f ) ) );
}

TEST_CASE( "Rect FromPoints", "[geometry]" )
{
	const ae::Rect r = ae::Rect::FromPoints( ae::Vec2( -1.0f, -2.0f ), ae::Vec2( 3.0f, 4.0f ) );
	REQUIRE( Approx( r.GetMin(), ae::Vec2( -1.0f, -2.0f ) ) );
	REQUIRE( Approx( r.GetMax(), ae::Vec2( 3.0f, 4.0f ) ) );
}

TEST_CASE( "Rect Contains", "[geometry]" )
{
	const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
	REQUIRE( r.Contains( ae::Vec2( 0.0f, 0.0f ) ) );       // interior
	REQUIRE( r.Contains( ae::Vec2( 1.0f, 0.0f ) ) );       // on boundary
	REQUIRE_FALSE( r.Contains( ae::Vec2( 1.1f, 0.0f ) ) ); // exterior
}

TEST_CASE( "Rect Clip clamps point inside", "[geometry]" )
{
	const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
	const ae::Vec2 clamped = r.Clip( ae::Vec2( 5.0f, -3.0f ) );
	REQUIRE( Approx( clamped.x, 1.0f ) );
	REQUIRE( Approx( clamped.y, -1.0f ) );
}

TEST_CASE( "Rect ExpandPoint", "[geometry]" )
{
	ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
	r.ExpandPoint( ae::Vec2( 3.0f, 0.0f ) );
	REQUIRE( Approx( r.GetMax().x, 3.0f ) );
	REQUIRE( Approx( r.GetMin().x, -1.0f ) );
}

TEST_CASE( "Rect ExpandEdge", "[geometry]" )
{
	ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
	// Before: min=(-1,-1), max=(1,1)
	r.ExpandEdge( ae::Vec2( 1.0f, 0.5f ) );
	// min -= (1,0.5), max += (1,0.5) → min=(-2,-1.5), max=(2,1.5)
	REQUIRE( Approx( r.GetMin().x, -2.0f ) );
	REQUIRE( Approx( r.GetMax().x, 2.0f ) );
	REQUIRE( Approx( r.GetMin().y, -1.5f ) );
	REQUIRE( Approx( r.GetMax().y, 1.5f ) );
}

TEST_CASE( "Rect GetIntersection", "[geometry]" )
{
	const ae::Rect a = ae::Rect::FromPoints( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 4.0f, 4.0f ) );
	const ae::Rect b = ae::Rect::FromPoints( ae::Vec2( 2.0f, 2.0f ), ae::Vec2( 6.0f, 6.0f ) );
	const ae::Rect c = ae::Rect::FromPoints( ae::Vec2( 10.0f, 10.0f ), ae::Vec2( 12.0f, 12.0f ) );
	ae::Rect intersection;
	REQUIRE( a.GetIntersection( b, &intersection ) );
	REQUIRE( Approx( intersection.GetMin(), ae::Vec2( 2.0f, 2.0f ) ) );
	REQUIRE( Approx( intersection.GetMax(), ae::Vec2( 4.0f, 4.0f ) ) );
	REQUIRE_FALSE( a.GetIntersection( c, &intersection ) );
}

TEST_CASE( "Rect operators", "[geometry]" )
{
	// r: min=(1,1), max=(3,3)
	const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 2.0f, 2.0f ), ae::Vec2( 2.0f, 2.0f ) );
	// operator*(float) scales min and max directly
	const ae::Rect scaled = r * 2.0f;
	REQUIRE( Approx( scaled.GetMin(), ae::Vec2( 2.0f, 2.0f ) ) );
	REQUIRE( Approx( scaled.GetMax(), ae::Vec2( 6.0f, 6.0f ) ) );
	// operator/(float)
	const ae::Rect halved = r / 2.0f;
	REQUIRE( Approx( halved.GetMin(), ae::Vec2( 0.5f, 0.5f ) ) );
	// operator+(Vec2) translates
	const ae::Rect shifted = r + ae::Vec2( 1.0f, 0.0f );
	REQUIRE( Approx( shifted.GetCenter().x, 3.0f ) );
	// operator-(Vec2) translates
	const ae::Rect unshifted = shifted - ae::Vec2( 1.0f, 0.0f );
	REQUIRE( Approx( unshifted.GetCenter().x, 2.0f ) );
	// operator*(Vec2) scales per-axis
	const ae::Rect axisScaled = r * ae::Vec2( 2.0f, 3.0f );
	REQUIRE( Approx( axisScaled.GetMin().x, 2.0f ) );
	REQUIRE( Approx( axisScaled.GetMin().y, 3.0f ) );
	// Compound assign
	ae::Rect rMut = r;
	rMut *= 2.0f;
	REQUIRE( Approx( rMut.GetMin(), ae::Vec2( 2.0f, 2.0f ) ) );
	rMut += ae::Vec2( 0.0f, 1.0f );
	REQUIRE( Approx( rMut.GetMin().y, 3.0f ) );
}

//------------------------------------------------------------------------------
// ae::RectInt tests
//------------------------------------------------------------------------------
TEST_CASE( "RectInt FromPointAndSize", "[geometry]" )
{
	const ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 2, 3 ), ae::Int2( 4, 5 ) );
	REQUIRE( r.GetPos() == ae::Int2( 2, 3 ) );
	REQUIRE( r.GetSize() == ae::Int2( 4, 5 ) );
	// pos is contained when size is non-zero
	REQUIRE( r.Contains( ae::Int2( 2, 3 ) ) );
	// pos + size is NOT contained (non-inclusive upper bound)
	REQUIRE_FALSE( r.Contains( ae::Int2( 6, 8 ) ) );
}

TEST_CASE( "RectInt FromPoints inclusive", "[geometry]" )
{
	const ae::RectInt r = ae::RectInt::FromPoints( ae::Int2( 1, 2 ), ae::Int2( 5, 7 ) );
	// Both endpoints are inclusive
	REQUIRE( r.Contains( ae::Int2( 1, 2 ) ) );
	REQUIRE( r.Contains( ae::Int2( 5, 7 ) ) );
	REQUIRE_FALSE( r.Contains( ae::Int2( 6, 7 ) ) );
}

TEST_CASE( "RectInt Contains", "[geometry]" )
{
	const ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 5, 5 ) );
	REQUIRE( r.Contains( ae::Int2( 0, 0 ) ) );
	REQUIRE( r.Contains( ae::Int2( 4, 4 ) ) );
	REQUIRE_FALSE( r.Contains( ae::Int2( 5, 5 ) ) ); // exclusive upper bound
	REQUIRE_FALSE( r.Contains( ae::Int2( -1, 0 ) ) );
}

TEST_CASE( "RectInt Intersects", "[geometry]" )
{
	const ae::RectInt a = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 4, 4 ) );
	const ae::RectInt b = ae::RectInt::FromPointAndSize( ae::Int2( 2, 2 ), ae::Int2( 4, 4 ) ); // overlaps
	const ae::RectInt c = ae::RectInt::FromPointAndSize( ae::Int2( 10, 10 ), ae::Int2( 4, 4 ) ); // no overlap
	REQUIRE( a.Intersects( b ) );
	REQUIRE_FALSE( a.Intersects( c ) );
}

TEST_CASE( "RectInt Expand", "[geometry]" )
{
	ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 2, 2 ) );
	r.Expand( ae::Int2( 5, 5 ) );
	REQUIRE( r.Contains( ae::Int2( 5, 5 ) ) );
	REQUIRE( r.Contains( ae::Int2( 0, 0 ) ) );
}

//------------------------------------------------------------------------------
// ae::Raycast free function tests (chaining / RaycastResult accumulator)
//------------------------------------------------------------------------------
TEST_CASE( "Raycast free function - Sphere hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, 0.0f, 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 10.0f );
	params.maxHits = 1;
	const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 5.0f ), 1.0f );
	const ae::RaycastResult result = ae::Raycast( sphere, params );
	REQUIRE( result.hits.Length() > 0 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 4.0f ) );
}

TEST_CASE( "Raycast free function - Sphere miss", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 5.0f, 0.0f, 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 10.0f );
	params.maxHits = 1;
	const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 5.0f ), 1.0f );
	const ae::RaycastResult result = ae::Raycast( sphere, params );
	REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast free function - Plane hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, -5.0f, 0.0f );
	params.ray = ae::Vec3( 0.0f, 10.0f, 0.0f );
	params.maxHits = 1;
	const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	const ae::RaycastResult result = ae::Raycast( plane, params );
	REQUIRE( result.hits.Length() > 0 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 5.0f ) );
}

TEST_CASE( "Raycast free function - AABB hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, 0.0f, -5.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 10.0f );
	params.maxHits = 1;
	const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
	const ae::RaycastResult result = ae::Raycast( aabb, params );
	REQUIRE( result.hits.Length() > 0 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 4.0f ) );
}

TEST_CASE( "Raycast free function - OBB hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, 0.0f, -5.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 10.0f );
	params.maxHits = 1;
	// OBB from Translation(0,0,0): halfSize=(0.5,0.5,0.5), first hit at z=-0.5, dist=4.5
	const ae::OBB obb( ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) );
	const ae::RaycastResult result = ae::Raycast( obb, params );
	REQUIRE( result.hits.Length() > 0 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 4.5f ) );
}

TEST_CASE( "Raycast free function - Triangle hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.25f, 0.25f, 5.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
	params.maxHits = 1;
	const ae::Triangle tri(
		ae::Vec3( 0.0f, 0.0f, 0.0f ),
		ae::Vec3( 1.0f, 0.0f, 0.0f ),
		ae::Vec3( 0.0f, 1.0f, 0.0f ) );
	ae::TriangleRaycastParams triParams;
	triParams.hitCounterclockwise = true;
	triParams.hitClockwise = false;
	const ae::RaycastResult result = ae::Raycast( tri, params, triParams );
	REQUIRE( result.hits.Length() > 0 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 5.0f ) );
}

TEST_CASE( "RaycastResult Accumulate chain keeps nearest hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, 0.0f, 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 20.0f );
	params.maxHits = 1;
	// Near sphere at z=3 (dist≈2), far sphere at z=8 (dist≈7)
	const ae::Sphere nearSphere( ae::Vec3( 0.0f, 0.0f, 3.0f ), 0.5f );
	const ae::Sphere farSphere( ae::Vec3( 0.0f, 0.0f, 8.0f ), 0.5f );
	// Chain: cast far first, then near as prevResult — result keeps the nearest
	const ae::RaycastResult r1 = ae::Raycast( farSphere, params );
	const ae::RaycastResult r2 = ae::Raycast( nearSphere, params, r1 );
	REQUIRE( r2.hits.Length() > 0 );
	REQUIRE( r2.hits[ 0 ].distance < 4.0f ); // near sphere hit at ~2.5
}

TEST_CASE( "RaycastResult Accumulate member adds hit", "[geometry]" )
{
	ae::RaycastParams params;
	params.source = ae::Vec3( 0.0f, 0.0f, 0.0f );
	params.ray = ae::Vec3( 0.0f, 0.0f, 10.0f );
	params.maxHits = 2;
	ae::RaycastResult result;
	ae::RaycastResult::Hit h;
	h.distance = 3.0f;
	h.position = ae::Vec3( 0.0f, 0.0f, 3.0f );
	h.normal = ae::Vec3( 0.0f, 0.0f, -1.0f );
	result.Accumulate( params, h );
	REQUIRE( result.hits.Length() == 1 );
	REQUIRE( Approx( result.hits[ 0 ].distance, 3.0f ) );
}

//------------------------------------------------------------------------------
// ae::Plane tests
//------------------------------------------------------------------------------
TEST_CASE( "Plane point-normal constructor", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE( Approx( plane.GetNormal(), ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "Plane GetClosestPointToOrigin", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 3.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE( Approx( plane.GetClosestPointToOrigin(), ae::Vec3( 0.0f, 0.0f, 3.0f ) ) );
}

TEST_CASE( "Plane GetSignedDistance", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 0.0f, 0.0f, 5.0f ) ), 5.0f ) );
REQUIRE( Approx( plane.GetSignedDistance( ae::Vec3( 0.0f, 0.0f, -3.0f ) ), -3.0f ) );
}

TEST_CASE( "Plane GetClosestPoint", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE( Approx( plane.GetClosestPoint( ae::Vec3( 5.0f, 5.0f, 5.0f ) ), ae::Vec3( 5.0f, 5.0f, 0.0f ) ) );
}

TEST_CASE( "Plane Raycast hit", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
ae::Vec3 hit;
float dist;
REQUIRE( plane.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -5.0f ), &hit, &dist ) );
REQUIRE( Approx( dist, 5.0f ) );
REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Plane Raycast ray too short", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE_FALSE( plane.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -4.0f ) ) );
}

TEST_CASE( "Plane Raycast parallel ray", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE_FALSE( plane.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) ) );
}

TEST_CASE( "Plane Raycast behind", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE_FALSE( plane.Raycast( ae::Vec3( 0.0f, 0.0f, -1.0f ), ae::Vec3( 0.0f, 0.0f, -2.0f ) ) );
}

TEST_CASE( "Plane IntersectLine returns tOut=5", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
ae::Vec3 hit;
float t;
REQUIRE( plane.IntersectLine( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -1.0f ), &hit, &t ) );
REQUIRE( Approx( t, 5.0f ) );
REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Plane IntersectLine parallel", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
REQUIRE_FALSE( plane.IntersectLine( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ) ) );
}

TEST_CASE( "Plane three-point constructor", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 0.0f ) );
const ae::Vec3 n = plane.GetNormal();
REQUIRE( Approx( n.x, 0.0f ) );
REQUIRE( Approx( n.y, 0.0f ) );
REQUIRE( Approx( std::abs( n.z ), 1.0f ) );
}

TEST_CASE( "Plane operator Vec4", "[ae::Plane]" )
{
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 3.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
const ae::Vec4 v = (ae::Vec4)plane;
REQUIRE( Approx( v.w, 3.0f ) );
}

//------------------------------------------------------------------------------
// ae::Line tests
//------------------------------------------------------------------------------
TEST_CASE( "Line GetClosest off-axis point", "[ae::Line]" )
{
const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( line.GetClosest( ae::Vec3( 3.0f, 2.0f, 0.0f ) ), ae::Vec3( 3.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Line GetClosest perpendicular", "[ae::Line]" )
{
const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( line.GetClosest( ae::Vec3( 0.0f, 0.0f, 5.0f ) ), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Line GetDistance", "[ae::Line]" )
{
const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( line.GetDistance( ae::Vec3( 3.0f, 2.0f, 0.0f ) ), 2.0f ) );
}

TEST_CASE( "Line GetDistance point on line", "[ae::Line]" )
{
const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( line.GetDistance( ae::Vec3( 3.0f, 0.0f, 0.0f ) ), 0.0f ) );
}

TEST_CASE( "Line GetClosest returns distance via out param", "[ae::Line]" )
{
const ae::Line line( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
float dist;
line.GetClosest( ae::Vec3( 3.0f, 2.0f, 0.0f ), &dist );
REQUIRE( Approx( dist, 2.0f ) );
}

//------------------------------------------------------------------------------
// ae::LineSegment tests
//------------------------------------------------------------------------------
TEST_CASE( "LineSegment accessors", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetStart(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
REQUIRE( Approx( seg.GetEnd(), ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
REQUIRE( Approx( seg.GetLength(), 1.0f ) );
}

TEST_CASE( "LineSegment GetClosest inside segment", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetClosest( ae::Vec3( 0.5f, 1.0f, 0.0f ) ), ae::Vec3( 0.5f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "LineSegment GetClosest clamped to start", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetClosest( ae::Vec3( -2.0f, 0.0f, 0.0f ) ), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "LineSegment GetClosest clamped to end", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetClosest( ae::Vec3( 3.0f, 0.0f, 0.0f ) ), ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "LineSegment GetDistance inside", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetDistance( ae::Vec3( 0.5f, 1.0f, 0.0f ) ), 1.0f ) );
}

TEST_CASE( "LineSegment GetDistance clamped to start", "[ae::LineSegment]" )
{
const ae::LineSegment seg( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( seg.GetDistance( ae::Vec3( -2.0f, 0.0f, 0.0f ) ), 2.0f ) );
}

//------------------------------------------------------------------------------
// ae::Sphere tests
//------------------------------------------------------------------------------
TEST_CASE( "Sphere Raycast hit from outside", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
ae::Vec3 hit, normal;
float dist;
REQUIRE( sphere.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -6.0f ), &hit, &normal, &dist ) );
REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
REQUIRE( Approx( normal, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
REQUIRE( Approx( dist, 4.0f ) );
}

TEST_CASE( "Sphere Raycast miss", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
REQUIRE_FALSE( sphere.Raycast( ae::Vec3( 0.0f, 5.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -10.0f ) ) );
}

TEST_CASE( "Sphere Raycast from inside clamps to source", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
ae::Vec3 hit;
float dist;
const ae::Vec3 source( 0.0f, 0.0f, 0.5f );
REQUIRE( sphere.Raycast( source, ae::Vec3( 0.0f, 0.0f, 1.0f ), &hit, nullptr, &dist ) );
REQUIRE( Approx( dist, 0.0f ) );
REQUIRE( Approx( hit, source ) );
}

TEST_CASE( "Sphere IntersectTriangle hit", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
REQUIRE( sphere.IntersectTriangle(
ae::Vec3( 0.5f, 0.0f, 0.0f ),
ae::Vec3( 1.5f, 0.0f, 0.0f ),
ae::Vec3( 0.0f, 1.5f, 0.0f ) ) );
}

TEST_CASE( "Sphere IntersectTriangle miss", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 10.0f, 10.0f, 10.0f ), 1.0f );
REQUIRE_FALSE( sphere.IntersectTriangle(
ae::Vec3( 0.0f, 0.0f, 0.0f ),
ae::Vec3( 1.0f, 0.0f, 0.0f ),
ae::Vec3( 0.0f, 1.0f, 0.0f ) ) );
}

TEST_CASE( "Sphere GetNearestPointOnSurface outside", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
float signedDist;
const ae::Vec3 nearest = sphere.GetNearestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 5.0f ), &signedDist );
REQUIRE( Approx( nearest, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
REQUIRE( Approx( signedDist, 4.0f ) );
}

TEST_CASE( "Sphere GetNearestPointOnSurface inside", "[ae::Sphere]" )
{
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
float signedDist;
sphere.GetNearestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 0.5f ), &signedDist );
REQUIRE( Approx( signedDist, -0.5f ) );
}

TEST_CASE( "Sphere Expand", "[ae::Sphere]" )
{
	ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
	sphere.Expand( ae::Vec3( 0.0f, 0.0f, 3.0f ) );
	// Min-enclosing: far edge at (0,0,-1), new diameter covers (0,0,-1) to (0,0,3)
	// center=(0,0,1), radius=2 — just verify expanded point is now inside the sphere
	const ae::Vec3 expanded( 0.0f, 0.0f, 3.0f );
	REQUIRE( ( expanded - sphere.center ).Length() <= sphere.radius + 0.001f );
}

//------------------------------------------------------------------------------
// ae::Circle tests
//------------------------------------------------------------------------------
TEST_CASE( "Circle GetArea", "[ae::Circle]" )
{
REQUIRE( Approx( ae::Circle::GetArea( 1.0f ), ae::PI, 0.001f ) );
REQUIRE( Approx( ae::Circle::GetArea( 2.0f ), 4.0f * ae::PI, 0.001f ) );
}

TEST_CASE( "Circle Intersect overlapping", "[ae::Circle]" )
{
const ae::Circle a( ae::Vec2( 0.0f, 0.0f ), 1.0f );
const ae::Circle b( ae::Vec2( 0.5f, 0.0f ), 1.0f );
REQUIRE( a.Intersect( b, nullptr ) );
}

TEST_CASE( "Circle Intersect separated", "[ae::Circle]" )
{
const ae::Circle a( ae::Vec2( 0.0f, 0.0f ), 1.0f );
const ae::Circle b( ae::Vec2( 3.0f, 0.0f ), 1.0f );
REQUIRE_FALSE( a.Intersect( b, nullptr ) );
}

TEST_CASE( "Circle Intersect touching", "[ae::Circle]" )
{
const ae::Circle a( ae::Vec2( 0.0f, 0.0f ), 1.0f );
const ae::Circle b( ae::Vec2( 2.0f, 0.0f ), 1.0f );
REQUIRE( a.Intersect( b, nullptr ) );
}

TEST_CASE( "Circle Intersect out param", "[ae::Circle]" )
{
const ae::Circle a( ae::Vec2( 0.0f, 0.0f ), 1.0f );
const ae::Circle b( ae::Vec2( 1.0f, 0.0f ), 1.0f );
ae::Vec2 out;
REQUIRE( a.Intersect( b, &out ) );
REQUIRE( Approx( out, ae::Vec2( 0.5f, 0.0f ) ) );
}

TEST_CASE( "Circle accessors", "[ae::Circle]" )
{
const ae::Circle c( ae::Vec2( 3.0f, 4.0f ), 2.5f );
REQUIRE( Approx( c.GetCenter(), ae::Vec2( 3.0f, 4.0f ) ) );
REQUIRE( Approx( c.GetRadius(), 2.5f ) );
}

//------------------------------------------------------------------------------
// ae::AABB tests
//------------------------------------------------------------------------------
TEST_CASE( "AABB accessors", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE( Approx( aabb.GetMin(), ae::Vec3( -1.0f, -1.0f, -1.0f ) ) );
REQUIRE( Approx( aabb.GetMax(), ae::Vec3( 1.0f, 1.0f, 1.0f ) ) );
REQUIRE( Approx( aabb.GetCenter(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
REQUIRE( Approx( aabb.GetHalfSize(), ae::Vec3( 1.0f, 1.0f, 1.0f ) ) );
}

TEST_CASE( "AABB Contains", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE( aabb.Contains( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
REQUIRE_FALSE( aabb.Contains( ae::Vec3( 2.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "AABB Intersect overlapping", "[ae::AABB]" )
{
const ae::AABB a( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
const ae::AABB b( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 2.0f, 2.0f, 2.0f ) );
REQUIRE( a.Intersect( b ) );
}

TEST_CASE( "AABB Intersect separated", "[ae::AABB]" )
{
const ae::AABB a( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 0.0f, 0.0f, 0.0f ) );
const ae::AABB b( ae::Vec3( 1.0f, 1.0f, 1.0f ), ae::Vec3( 2.0f, 2.0f, 2.0f ) );
REQUIRE_FALSE( a.Intersect( b ) );
}

TEST_CASE( "AABB Expand point", "[ae::AABB]" )
{
ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
aabb.Expand( ae::Vec3( 2.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( aabb.GetMax().x, 2.0f ) );
}

TEST_CASE( "AABB Expand AABB", "[ae::AABB]" )
{
ae::AABB a( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
const ae::AABB b( ae::Vec3( 2.0f, 2.0f, 2.0f ), ae::Vec3( 3.0f, 3.0f, 3.0f ) );
a.Expand( b );
REQUIRE( Approx( a.GetMax(), ae::Vec3( 3.0f, 3.0f, 3.0f ) ) );
REQUIRE( Approx( a.GetMin(), ae::Vec3( -1.0f, -1.0f, -1.0f ) ) );
}

TEST_CASE( "AABB Expand float", "[ae::AABB]" )
{
ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
aabb.Expand( 0.5f );
REQUIRE( Approx( aabb.GetMax(), ae::Vec3( 1.5f, 1.5f, 1.5f ) ) );
REQUIRE( Approx( aabb.GetMin(), ae::Vec3( -1.5f, -1.5f, -1.5f ) ) );
}

TEST_CASE( "AABB GetSignedDistanceFromSurface inside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE( Approx( aabb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ), -1.0f ) );
}

TEST_CASE( "AABB GetSignedDistanceFromSurface outside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE( Approx( aabb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 1.5f ) ), 0.5f ) );
}

TEST_CASE( "AABB GetClosestPointOnSurface outside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE( Approx( aabb.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 5.0f ) ), ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "AABB GetClosestPointOnSurface containsOut false for outside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
bool contains;
aabb.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 5.0f ), &contains );
REQUIRE_FALSE( contains );
}

TEST_CASE( "AABB GetClosestPointOnSurface containsOut true for inside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
bool contains;
aabb.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ), &contains );
REQUIRE( contains );
}

TEST_CASE( "AABB Raycast hit from outside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
ae::Vec3 hit, norm;
float dist;
REQUIRE( aabb.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -8.0f ), &hit, &norm, &dist ) );
REQUIRE( Approx( dist, 4.0f ) );
REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
REQUIRE( Approx( norm, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "AABB Raycast from inside returns dist=0", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
ae::Vec3 hit;
float dist;
const ae::Vec3 source( 0.0f, 0.0f, 0.0f );
REQUIRE( aabb.Raycast( source, ae::Vec3( 0.0f, 0.0f, 1.0f ), &hit, nullptr, &dist ) );
REQUIRE( Approx( dist, 0.0f ) );
REQUIRE( Approx( hit, source ) );
}

TEST_CASE( "AABB Raycast miss", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE_FALSE( aabb.Raycast( ae::Vec3( 0.0f, 5.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -10.0f ) ) );
}

TEST_CASE( "AABB IntersectLine through box", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
float t0, t1;
REQUIRE( aabb.IntersectLine( ae::Vec3( 0.0f, 0.0f, -5.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), &t0, &t1 ) );
REQUIRE( t0 < t1 );
REQUIRE( t1 > 0.0f );
}

TEST_CASE( "AABB IntersectLine parallel outside", "[ae::AABB]" )
{
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
REQUIRE_FALSE( aabb.IntersectLine( ae::Vec3( 0.0f, 5.0f, 0.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ) ) );
}

//------------------------------------------------------------------------------
// ae::OBB tests
//------------------------------------------------------------------------------
TEST_CASE( "OBB from identity matrix", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
REQUIRE( Approx( obb.GetCenter(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
REQUIRE( Approx( obb.GetHalfSize(), ae::Vec3( 0.5f, 0.5f, 0.5f ) ) );
}

TEST_CASE( "OBB from translation matrix", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Translation( 2.0f, 0.0f, 0.0f ) );
REQUIRE( Approx( obb.GetCenter(), ae::Vec3( 2.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "OBB GetAABB from identity", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
const ae::AABB aabb = obb.GetAABB();
REQUIRE( Approx( aabb.GetMin(), ae::Vec3( -0.5f, -0.5f, -0.5f ) ) );
REQUIRE( Approx( aabb.GetMax(), ae::Vec3( 0.5f, 0.5f, 0.5f ) ) );
}

TEST_CASE( "OBB GetClosestPointOnSurface outside", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
REQUIRE( Approx( obb.GetClosestPointOnSurface( ae::Vec3( 0.0f, 0.0f, 5.0f ) ), ae::Vec3( 0.0f, 0.0f, 0.5f ) ) );
}

TEST_CASE( "OBB GetSignedDistanceFromSurface inside", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
REQUIRE( obb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 0.0f ) ) < 0.0f );
}

TEST_CASE( "OBB GetSignedDistanceFromSurface outside", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
REQUIRE( obb.GetSignedDistanceFromSurface( ae::Vec3( 0.0f, 0.0f, 1.0f ) ) > 0.0f );
}

TEST_CASE( "OBB Raycast hit", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
ae::Vec3 hit, norm;
float dist;
REQUIRE( obb.Raycast( ae::Vec3( 0.0f, 0.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -10.0f ), &hit, &norm, &dist ) );
REQUIRE( Approx( dist, 4.5f ) );
REQUIRE( Approx( hit, ae::Vec3( 0.0f, 0.0f, 0.5f ) ) );
REQUIRE( Approx( norm, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "OBB Raycast miss", "[ae::OBB]" )
{
const ae::OBB obb( ae::Matrix4::Identity() );
REQUIRE_FALSE( obb.Raycast( ae::Vec3( 0.0f, 5.0f, 5.0f ), ae::Vec3( 0.0f, 0.0f, -10.0f ) ) );
}

TEST_CASE( "OBB SetTransform GetTransform round-trip", "[ae::OBB]" )
{
ae::OBB obb;
obb.SetTransform( ae::Matrix4::Identity() );
const ae::Matrix4 t = obb.GetTransform();
REQUIRE( Approx( t.GetTranslation(), ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
REQUIRE( Approx( t.GetAxis( 0 ).Length(), 1.0f ) );
REQUIRE( Approx( t.GetAxis( 1 ).Length(), 1.0f ) );
REQUIRE( Approx( t.GetAxis( 2 ).Length(), 1.0f ) );
}

//------------------------------------------------------------------------------
// ae::Frustum tests
//------------------------------------------------------------------------------
TEST_CASE( "Frustum Intersects point inside", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE( frustum.Intersects( ae::Vec3( 0.0f, 0.0f, -1.0f ) ) );
}

TEST_CASE( "Frustum Intersects point behind far", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE_FALSE( frustum.Intersects( ae::Vec3( 0.0f, 0.0f, -200.0f ) ) );
}

TEST_CASE( "Frustum Intersects point far to the side", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE_FALSE( frustum.Intersects( ae::Vec3( 1000.0f, 0.0f, -1.0f ) ) );
}

TEST_CASE( "Frustum Intersects point behind camera", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE_FALSE( frustum.Intersects( ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "Frustum Intersects sphere inside", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE( frustum.Intersects( ae::Sphere( ae::Vec3( 0.0f, 0.0f, -5.0f ), 0.1f ) ) );
}

TEST_CASE( "Frustum Intersects sphere behind far", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
REQUIRE_FALSE( frustum.Intersects( ae::Sphere( ae::Vec3( 0.0f, 0.0f, -200.0f ), 0.5f ) ) );
}

TEST_CASE( "Frustum GetPlane Near normal outward", "[ae::Frustum]" )
{
const ae::Matrix4 proj = ae::Matrix4::ViewToProjection( ae::HalfPi, 1.0f, 0.1f, 100.0f );
const ae::Frustum frustum( proj );
const ae::Plane near = frustum.GetPlane( ae::Frustum::Plane::Near );
REQUIRE( near.GetNormal().z > 0.0f );
}

//------------------------------------------------------------------------------
// ae::Rect tests
//------------------------------------------------------------------------------
TEST_CASE( "Rect FromCenterAndSize", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
REQUIRE( Approx( r.GetMin(), ae::Vec2( -1.0f, -1.0f ) ) );
REQUIRE( Approx( r.GetMax(), ae::Vec2( 1.0f, 1.0f ) ) );
}

TEST_CASE( "Rect FromPoints", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromPoints( ae::Vec2( -1.0f, -1.0f ), ae::Vec2( 1.0f, 1.0f ) );
REQUIRE( Approx( r.GetMin(), ae::Vec2( -1.0f, -1.0f ) ) );
REQUIRE( Approx( r.GetMax(), ae::Vec2( 1.0f, 1.0f ) ) );
}

TEST_CASE( "Rect accessors", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 4.0f ) );
REQUIRE( Approx( r.GetWidth(), 2.0f ) );
REQUIRE( Approx( r.GetHeight(), 4.0f ) );
REQUIRE( Approx( r.GetCenter(), ae::Vec2( 0.0f, 0.0f ) ) );
REQUIRE( Approx( r.GetSize(), ae::Vec2( 2.0f, 4.0f ) ) );
}

TEST_CASE( "Rect Contains", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
REQUIRE( r.Contains( ae::Vec2( 0.0f, 0.0f ) ) );
REQUIRE_FALSE( r.Contains( ae::Vec2( 2.0f, 0.0f ) ) );
}

TEST_CASE( "Rect Clip outside", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
REQUIRE( Approx( r.Clip( ae::Vec2( 5.0f, 0.0f ) ), ae::Vec2( 1.0f, 0.0f ) ) );
}

TEST_CASE( "Rect Clip inside", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
REQUIRE( Approx( r.Clip( ae::Vec2( 0.5f, 0.5f ) ), ae::Vec2( 0.5f, 0.5f ) ) );
}

TEST_CASE( "Rect ExpandPoint", "[ae::Rect]" )
{
ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
r.ExpandPoint( ae::Vec2( 3.0f, 3.0f ) );
REQUIRE( Approx( r.GetMax(), ae::Vec2( 3.0f, 3.0f ) ) );
}

TEST_CASE( "Rect ExpandEdge", "[ae::Rect]" )
{
ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
r.ExpandEdge( ae::Vec2( 1.0f, 1.0f ) );
REQUIRE( Approx( r.GetMin(), ae::Vec2( -2.0f, -2.0f ) ) );
REQUIRE( Approx( r.GetMax(), ae::Vec2( 2.0f, 2.0f ) ) );
}

TEST_CASE( "Rect GetIntersection overlapping", "[ae::Rect]" )
{
const ae::Rect a = ae::Rect::FromPoints( ae::Vec2( -1.0f, -1.0f ), ae::Vec2( 1.0f, 1.0f ) );
const ae::Rect b = ae::Rect::FromPoints( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
ae::Rect intersection;
REQUIRE( a.GetIntersection( b, &intersection ) );
REQUIRE( Approx( intersection.GetMin(), ae::Vec2( 0.0f, 0.0f ) ) );
REQUIRE( Approx( intersection.GetMax(), ae::Vec2( 1.0f, 1.0f ) ) );
}

TEST_CASE( "Rect GetIntersection non-overlapping", "[ae::Rect]" )
{
const ae::Rect a = ae::Rect::FromPoints( ae::Vec2( -2.0f, -2.0f ), ae::Vec2( -1.0f, -1.0f ) );
const ae::Rect b = ae::Rect::FromPoints( ae::Vec2( 1.0f, 1.0f ), ae::Vec2( 2.0f, 2.0f ) );
REQUIRE_FALSE( a.GetIntersection( b, nullptr ) );
}

TEST_CASE( "Rect operator*", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
const ae::Rect s = r * 2.0f;
REQUIRE( Approx( s.GetMin(), ae::Vec2( -2.0f, -2.0f ) ) );
REQUIRE( Approx( s.GetMax(), ae::Vec2( 2.0f, 2.0f ) ) );
}

TEST_CASE( "Rect operator+", "[ae::Rect]" )
{
const ae::Rect r = ae::Rect::FromCenterAndSize( ae::Vec2( 0.0f, 0.0f ), ae::Vec2( 2.0f, 2.0f ) );
const ae::Rect s = r + ae::Vec2( 1.0f, 0.0f );
REQUIRE( Approx( s.GetMin(), ae::Vec2( 0.0f, -1.0f ) ) );
REQUIRE( Approx( s.GetMax(), ae::Vec2( 2.0f, 1.0f ) ) );
}

//------------------------------------------------------------------------------
// ae::RectInt tests
//------------------------------------------------------------------------------
TEST_CASE( "RectInt FromPointAndSize contains", "[ae::RectInt]" )
{
const ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 3, 3 ) );
REQUIRE( r.Contains( ae::Int2( 2, 2 ) ) );
REQUIRE_FALSE( r.Contains( ae::Int2( 3, 3 ) ) );
}

TEST_CASE( "RectInt FromPoints contains", "[ae::RectInt]" )
{
const ae::RectInt r = ae::RectInt::FromPoints( 0, 0, 2, 2 );
REQUIRE( r.Contains( ae::Int2( 2, 2 ) ) );
REQUIRE( r.GetWidth() == 3 );
}

TEST_CASE( "RectInt accessors", "[ae::RectInt]" )
{
const ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 1, 2 ), ae::Int2( 4, 5 ) );
REQUIRE( r.GetPos() == ae::Int2( 1, 2 ) );
REQUIRE( r.GetSize() == ae::Int2( 4, 5 ) );
REQUIRE( r.GetWidth() == 4 );
REQUIRE( r.GetHeight() == 5 );
}

TEST_CASE( "RectInt Intersects overlapping", "[ae::RectInt]" )
{
const ae::RectInt a = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 3, 3 ) );
const ae::RectInt b = ae::RectInt::FromPointAndSize( ae::Int2( 1, 1 ), ae::Int2( 3, 3 ) );
REQUIRE( a.Intersects( b ) );
}

TEST_CASE( "RectInt Intersects separated", "[ae::RectInt]" )
{
const ae::RectInt a = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 3, 3 ) );
const ae::RectInt b = ae::RectInt::FromPointAndSize( ae::Int2( 4, 4 ), ae::Int2( 3, 3 ) );
REQUIRE_FALSE( a.Intersects( b ) );
}

TEST_CASE( "RectInt Intersects adjacent", "[ae::RectInt]" )
{
const ae::RectInt a = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 3, 3 ) );
const ae::RectInt b = ae::RectInt::FromPointAndSize( ae::Int2( 3, 0 ), ae::Int2( 3, 3 ) );
REQUIRE_FALSE( a.Intersects( b ) );
}

TEST_CASE( "RectInt Expand", "[ae::RectInt]" )
{
ae::RectInt r = ae::RectInt::FromPointAndSize( ae::Int2( 0, 0 ), ae::Int2( 3, 3 ) );
r.Expand( ae::Int2( 5, 5 ) );
REQUIRE( r.Contains( ae::Int2( 5, 5 ) ) );
}

//------------------------------------------------------------------------------
// ae::Raycast free functions tests
//------------------------------------------------------------------------------
TEST_CASE( "Raycast Plane basic hit", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 0.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
const ae::RaycastResult result = ae::Raycast( plane, params );
REQUIRE( result.hits.Length() == 1 );
REQUIRE( Approx( result.hits[ 0 ].distance, 5.0f ) );
REQUIRE( Approx( result.hits[ 0 ].position, ae::Vec3( 0.0f, 0.0f, 0.0f ) ) );
}

TEST_CASE( "Raycast Plane miss", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 0.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, 1.0f );
params.maxHits = 8;
const ae::Plane plane( ae::Vec3( 0.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
const ae::RaycastResult result = ae::Raycast( plane, params );
REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast Sphere basic hit", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 0.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
const ae::RaycastResult result = ae::Raycast( sphere, params );
REQUIRE( result.hits.Length() == 1 );
REQUIRE( Approx( result.hits[ 0 ].distance, 4.0f ) );
REQUIRE( Approx( result.hits[ 0 ].position, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "Raycast Sphere miss", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 5.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::Sphere sphere( ae::Vec3( 0.0f, 0.0f, 0.0f ), 1.0f );
const ae::RaycastResult result = ae::Raycast( sphere, params );
REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast AABB basic hit", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 0.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -8.0f );
params.maxHits = 8;
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
const ae::RaycastResult result = ae::Raycast( aabb, params );
REQUIRE( result.hits.Length() == 1 );
REQUIRE( Approx( result.hits[ 0 ].distance, 4.0f ) );
REQUIRE( Approx( result.hits[ 0 ].position, ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
}

TEST_CASE( "Raycast AABB miss", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 5.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::AABB aabb( ae::Vec3( -1.0f, -1.0f, -1.0f ), ae::Vec3( 1.0f, 1.0f, 1.0f ) );
const ae::RaycastResult result = ae::Raycast( aabb, params );
REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast OBB basic hit", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 0.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::OBB obb( ae::Matrix4::Identity() );
const ae::RaycastResult result = ae::Raycast( obb, params );
REQUIRE( result.hits.Length() == 1 );
REQUIRE( Approx( result.hits[ 0 ].distance, 4.5f ) );
REQUIRE( Approx( result.hits[ 0 ].position, ae::Vec3( 0.0f, 0.0f, 0.5f ) ) );
}

TEST_CASE( "Raycast OBB miss", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.0f, 5.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::OBB obb( ae::Matrix4::Identity() );
const ae::RaycastResult result = ae::Raycast( obb, params );
REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast Triangle basic hit", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 0.25f, 0.25f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::Triangle tri(
ae::Vec3( 0.0f, 0.0f, 0.0f ),
ae::Vec3( 1.0f, 0.0f, 0.0f ),
ae::Vec3( 0.0f, 1.0f, 0.0f ) );
const ae::RaycastResult result = ae::Raycast( tri, params );
REQUIRE( result.hits.Length() == 1 );
REQUIRE( Approx( result.hits[ 0 ].distance, 5.0f ) );
}

TEST_CASE( "Raycast Triangle miss", "[ae::Raycast]" )
{
ae::RaycastParams params;
params.source = ae::Vec3( 5.0f, 5.0f, 5.0f );
params.ray = ae::Vec3( 0.0f, 0.0f, -10.0f );
params.maxHits = 8;
const ae::Triangle tri(
ae::Vec3( 0.0f, 0.0f, 0.0f ),
ae::Vec3( 1.0f, 0.0f, 0.0f ),
ae::Vec3( 0.0f, 1.0f, 0.0f ) );
const ae::RaycastResult result = ae::Raycast( tri, params );
REQUIRE( result.hits.Length() == 0 );
}

TEST_CASE( "Raycast chain Plane then Sphere", "[ae::Raycast]" )
{
ae::RaycastParams chainParams;
chainParams.source = ae::Vec3( 0.0f, 0.0f, 10.0f );
chainParams.ray = ae::Vec3( 0.0f, 0.0f, -20.0f );
chainParams.maxHits = 8;
const ae::Plane chainPlane( ae::Vec3( 0.0f, 0.0f, 2.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
const ae::Sphere chainSphere( ae::Vec3( 0.0f, 0.0f, -2.0f ), 1.0f );
const ae::RaycastResult r1 = ae::Raycast( chainPlane, chainParams );
const ae::RaycastResult r2 = ae::Raycast( chainSphere, chainParams, r1 );
REQUIRE( r2.hits.Length() == 2 );
REQUIRE( Approx( r2.hits[ 0 ].distance, 8.0f ) );
REQUIRE( Approx( r2.hits[ 1 ].distance, 11.0f ) );
}

TEST_CASE( "Raycast Accumulate respects maxHits", "[ae::Raycast]" )
{
ae::RaycastParams accParams;
accParams.maxHits = 2;
ae::RaycastResult accResult;
ae::RaycastResult::Hit h1, h2, h3;
h1.distance = 5.0f;
h2.distance = 3.0f;
h3.distance = 7.0f;
accResult.Accumulate( accParams, h1 );
REQUIRE( accResult.hits.Length() == 1 );
accResult.Accumulate( accParams, h2 );
REQUIRE( accResult.hits.Length() == 2 );
accResult.Accumulate( accParams, h3 );
REQUIRE( accResult.hits.Length() == 2 );
REQUIRE( Approx( accResult.hits[ 0 ].distance, 3.0f ) );
REQUIRE( Approx( accResult.hits[ 1 ].distance, 5.0f ) );
}
