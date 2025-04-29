//------------------------------------------------------------------------------
// 27_SDFToMesh.cpp
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
#define _AE_DEBUG_ 0
#include "aether.h"

#ifndef AE_TERRAIN_SIMD
	#if _AE_LINUX_ || _AE_EMSCRIPTEN_
		#define AE_TERRAIN_SIMD 0
	#else
		#define AE_TERRAIN_SIMD 1
	#endif
#endif
#if AE_TERRAIN_SIMD && __aarch64__ && _AE_APPLE_
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
	#include "ae/sse2neon.h"
	#pragma clang diagnostic pop
#endif

#define AE_TERRAIN_ITERATE_OCTREE_LEAVES 0
#define AE_TERRAIN_USE_VERTEX_MAP 1
#define AE_TERRAIN_USE_CACHE 0

//------------------------------------------------------------------------------
// Types / constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ISOSURFACE = "isosurface";

struct IsosurfaceVertex
{
	ae::Vec4 position;
	ae::Vec3 normal;
};
using IsosurfaceFn = float( * )( ae::Vec3, const void* );
struct IsosurfaceParams
{
	ae::DebugLines* debug = nullptr;
	float normalSampleOffset = 0.1f;
	float smoothingAmount = 0.05f; // @TODO: This does nothing yet
	IsosurfaceFn fn = nullptr;
	const void* userData = nullptr;
	ae::Vec3 center = ae::Vec3( 0.0f );
	float halfSize = 0.0f;
};

typedef uint32_t IsosurfaceIndex;
const IsosurfaceIndex kInvalidIsosurfaceIndex = ~0;
const uint32_t kChunkSize = 200;
const int32_t kTempChunkSize = kChunkSize + 2; // Include a 1 voxel border
const int32_t kTempChunkSize3 = kTempChunkSize * kTempChunkSize * kTempChunkSize; // Temp voxel count

inline uint32_t GetInt3Hash( uint32_t x, uint32_t y, uint32_t z )
{
	const uint32_t gridSize = 1625; // UINT32_MAX ^ (1/3)
	return ( x + gridSize * ( y + z * gridSize ) );
}

static constexpr float kMinOctantHalfSize = 1.05f; // A little more than a voxel on each side of the surface
struct SDFOctant
{
	inline ae::AABB GetAABB() const { const ae::Vec3 halfSize3( m_halfSize ); return ae::AABB( m_center - halfSize3, m_center + halfSize3 ); }
	int32_t GetChildOctreeIndex( uint32_t index ) const { return ( m_childrenBaseIndex >= 0 ) ? m_childrenBaseIndex + index : -1; }
	uint32_t GetChildCount() const { return ( m_childrenBaseIndex >= 0 ) ? 8 : 0; }
	bool IntersectsZero() const { return m_intersects; }
private:
	friend class SDFOctree;
	ae::Vec3 m_center;
	float m_halfSize;
	float m_signedSurfaceDistance;
	int32_t m_childrenBaseIndex; // If non-negative, this octant has 8 children starting at this index
	bool m_intersects;
};
class SDFOctree
{
public:
	SDFOctree()
	{
		m_octant.m_center = ae::Vec3( 0.0f );
		m_octant.m_halfSize = 0.0f;
		m_octant.m_signedSurfaceDistance = NAN;
		m_octant.m_childrenBaseIndex = -1;
	}
	void Build( IsosurfaceFn fn, const void* userData, ae::Vec3 center, float halfSize )
	{
		m_fn = fn;
		m_userData = userData;
		m_octants.Clear();
		m_aabb = ae::AABB();
		m_Build( -1, center, halfSize );
	}
	//! Returns an approximate signed distance to the surface of the SDF at the
	//! given position, or NAN if the lookup position is outside the octree. If
	//! If this returns NAN, the original SDF function should be sampled.
	float GetValue( ae::Vec3 position ) const
	{
		if( m_octant.GetAABB().Contains( position ) )
		{
			return m_GetValue( &m_octant, position );
		}
		AE_FAIL(); // @TODO: This should actually fall through but this helps debugging
		return m_fn( position, m_userData );;
	}
	// @TODO: ae::Array< float, N > GetValues( ae::Array< ae::Vec3, N > pos, ae::Array< float, N >& resultsOut ) const;
	
	inline const ae::AABB& GetAABB() const { return m_aabb; }
	inline uint32_t GetOctantCount() const { return m_octants.Length() + 1; } // +1 for root m_octant
	inline const SDFOctant* GetOctant( uint32_t index ) const { return ( index == 0 ) ? &m_octant : &m_octants[ index - 1 ]; }

private:
	void m_Build( int32_t index, ae::Vec3 center, float halfSize )
	{
		// Use index instead of pointer to handle m_octants array expansion
		SDFOctant* octant = ( index == -1 ) ? &m_octant : &m_octants[ index ];
		octant->m_center = center;
		octant->m_halfSize = halfSize;
		octant->m_signedSurfaceDistance = m_fn( center, m_userData );
		const float diagonal = ae::Sqrt( halfSize * halfSize * 3.0f );
		const float nextHalfSize = halfSize * 0.5f;
		octant->m_intersects = ( diagonal > ae::Abs( octant->m_signedSurfaceDistance ) );
		if( ( nextHalfSize > kMinOctantHalfSize ) && octant->m_intersects ) // Only split if next octant is large enough
		{
			const uint32_t baseIndex = m_octants.Length();
			octant->m_childrenBaseIndex = baseIndex;
			// This can reallocate the octant array, so it's not safe to
			// dereference 'octant' after this Append()
			m_octants.Append( {}, 8 );
			for( uint32_t i = 0; i < 8; i++ )
			{
				m_Build(
					baseIndex + i,
					center + kChildOffsets[ i ] * nextHalfSize,
					nextHalfSize
				);
			}
		}
		else
		{
			if( octant->m_intersects )
			{
				const ae::Vec3 halfSize3( halfSize );
				m_aabb.Expand( ae::AABB( center - halfSize3, center + halfSize3 ) );
			}
			octant->m_childrenBaseIndex = -1;
		}
	}
	float m_GetValue( const SDFOctant* octant, ae::Vec3 position ) const
	{
		if( octant->m_childrenBaseIndex >= 0 )
		{
			const SDFOctant* children = &m_octants[ octant->m_childrenBaseIndex ];
			for( uint32_t i = 0; i < 8; i++ )
			{
				const SDFOctant* child = &children[ i ];
				if( child->GetAABB().Contains( position ) )
				{
					return m_GetValue( child, position );
				}
			}
			AE_FAIL();
			return m_fn( position, m_userData );
		}
		else if( octant->m_intersects ) // Leaf nodes only, because of child check above
		{
			return m_fn( position, m_userData );
		}
		else
		{
			return octant->m_signedSurfaceDistance;
		}
	}
	const ae::Vec3 kChildOffsets[ 8 ] =
	{
		{ -1, -1, -1 },
		{ 1, -1, -1 },
		{ -1, 1, -1 },
		{ 1, 1, -1 },
		{ -1, -1, 1 },
		{ 1, -1, 1 },
		{ -1, 1, 1 },
		{ 1, 1, 1 }
	};
	SDFOctant m_octant;
	ae::Array< SDFOctant > m_octants = TAG_ISOSURFACE;
	IsosurfaceFn m_fn = nullptr;
	const void* m_userData = nullptr;
	ae::AABB m_aabb;
};

//------------------------------------------------------------------------------
// IsosurfaceExtractorCache class
//------------------------------------------------------------------------------
class IsosurfaceExtractorCache
{
public:
	void Generate( const IsosurfaceParams& params );
	ae::Vec3 GetCenter() const { return m_params.center; }
	float GetHalfSize() const { return m_params.halfSize; }
	
	float GetValue( ae::Vec3 pos ) const;
	ae::Vec3 GetDerivative( ae::Vec3 p ) const;

	void DrawOctree( ae::DebugLines* debugLines );
	const SDFOctree* GetOctree() const { return &m_octree; }

private:
	IsosurfaceParams m_params;
	SDFOctree m_octree; // @TODO: Should replace IsosurfaceExtractorCache completely with this
};

//------------------------------------------------------------------------------
// IsosurfaceExtractor class
//------------------------------------------------------------------------------
struct IsosurfaceExtractor
{
	IsosurfaceExtractor( ae::Tag tag );
	void Generate( const IsosurfaceExtractorCache* sdf, uint32_t maxVerts, uint32_t maxIndices, ae::Array< ae::Vec3 >* errors );
	
	ae::Array< IsosurfaceVertex > vertices;
	ae::Array< IsosurfaceIndex > indices;

	struct TempEdges
	{
		uint16_t b;
		// 3 planes whose intersections are used to position vertices within voxel
		// EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
		ae::Vec3 p[ 3 ];
		ae::Vec3 n[ 3 ];
	};
	TempEdges m_tempEdges[ kTempChunkSize3 ];
#if AE_TERRAIN_USE_VERTEX_MAP
	ae::Map< uint32_t, IsosurfaceIndex > m_voxelToVertex;
#else
	IsosurfaceIndex m_i[ kChunkSize ][ kChunkSize ][ kChunkSize ];
#endif
};

//------------------------------------------------------------------------------
// ae::SDFBox
// Valid range is 0-1
//------------------------------------------------------------------------------
inline float SDFBox( ae::Vec3 p, ae::Vec3 halfSize, float cornerRadius = 0.0f )
{
	const ae::Vec3 q = ae::Abs( p ) - ( halfSize - ae::Vec3( cornerRadius ) );
	return ( ae::Max( q, ae::Vec3( 0.0f ) ) ).Length() + ae::Min( ae::Max( q.x, ae::Max( q.y, q.z ) ), 0.0f ) - cornerRadius;
}

//------------------------------------------------------------------------------
// ae::SDFCylinder
// @TODO: Valid range
//------------------------------------------------------------------------------
inline float SDFCylinder( ae::Vec3 p, ae::Vec3 halfSize, float top = 1.0f, float bottom = 1.0f )
{
	float scale;
	if( halfSize.x > halfSize.y )
	{
		scale = halfSize.x;
		p.y *= halfSize.x / halfSize.y;
	}
	else
	{
		scale = halfSize.y;
		p.x *= halfSize.y / halfSize.x;
	}

	const float r1 = ae::Clip01( bottom ) * scale;
	const float r2 = ae::Clip01( top ) * scale;
	const float h = halfSize.z;

	const ae::Vec2 q( p.GetXY().Length(), p.z );
	const ae::Vec2 k1(r2,h);
	const ae::Vec2 k2(r2-r1,2.0*h);
	const ae::Vec2 ca(q.x-ae::Min(q.x,(q.y<0.0)?r1:r2), ae::Abs(q.y)-h);
	const ae::Vec2 cb = q - k1 + k2*ae::Clip01( (k1-q).Dot(k2)/k2.Dot(k2) );
	const float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
	return s*sqrt( ae::Min(ca.Dot(ca),cb.Dot(cb)) );
}

//------------------------------------------------------------------------------
// SDF helpers
//------------------------------------------------------------------------------
inline float SDFUnion( float d1, float d2 )
{
	return ae::Min( d1, d2 );
}

inline float SDFSubtraction( float d1, float d2 )
{
	return ae::Max( -d1, d2 );
}

inline float SDFIntersection( float d1, float d2 )
{
	return ae::Max( d1, d2 );
}

inline float SDFSmoothUnion( float d1, float d2, float k )
{
	const float h = ae::Clip01( 0.5f + 0.5f * ( d2 - d1 ) / k );
	return ae::Lerp( d2, d1, h ) - k * h * ( 1.0f - h );
}

inline float SDFSmoothSubtraction( float d1, float d2, float k )
{
	const float h = ae::Clip01( 0.5f - 0.5f * ( d2 + d1 ) / k );
	return ae::Lerp( d2, -d1, h ) + k * h * ( 1.0f - h );
}

const char* kVertShader = R"(
AE_UNIFORM_HIGHP mat4 u_worldToProj;
AE_IN_HIGHP vec4 a_position;
AE_IN_HIGHP vec3 a_normal;
AE_OUT_HIGHP vec3 v_normal;
void main()
{
	v_normal = a_normal;
	gl_Position = u_worldToProj * a_position;
}
)";
const char* kFragShader = R"(
AE_UNIFORM vec3 u_color;
AE_IN_HIGHP vec3 v_normal;
void main()
{
	vec3 light = normalize( vec3( -1.0, -1.0, -1.0 ) );
	vec3 ambient = vec3( 0.1, 0.1, 0.1 );
	float diffuse = max( 0.0, -dot( v_normal, light ) );
	vec3 color = u_color * ( ambient + vec3( 1.0, 1.0, 1.0 ) * diffuse );
	AE_COLOR = vec4( color, 0.5 );
}
)";

int main()
{
	AE_LOG( "Initialize (debug #)", (int)_AE_DEBUG_ );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::DebugLines debugLines = TAG_ISOSURFACE;
	ae::DebugCamera camera = ae::Axis::Z;
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 20.0f ) );
	if( ae::IsDebuggerAttached() )
	{
		ae::Int2 windowPos( 0, 0 );
		ae::Int2 windowSize( 1280, 720 );
		const auto screens = ae::GetScreens();
		if( screens.Length() )
		{
			const ae::Screen* targetScreen = [&]() -> const ae::Screen*
			{
				const ae::Screen* result = nullptr;
				for( const ae::Screen& screen : screens )
				{
					// Smallest screen
					if( !result || screen.size.x * screen.scaleFactor <= result->size.x * result->scaleFactor )
					{
						result = &screen;
					}
				}
				return result;
			}();
			windowPos = targetScreen->position + targetScreen->size / 2;
			windowPos -= windowSize / 2;
			if( screens.Length() >= 2 )
			{
				windowSize = targetScreen->size;
			}
		}
		window.Initialize( windowPos, windowSize.x, windowSize.y, true, false );
		if( screens.Length() == 1 )
		{
			window.SetAlwaysOnTop( true );
		}
	}
	else
	{
		window.Initialize( 1280, 720, false, true, true );
	}
	window.SetTitle( "SDF to Mesh" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 32768 );
	debugLines.SetXRayEnabled( false );

	ae::Shader shader;
	shader.Initialize( kVertShader, kFragShader );

	// Opacity setting
	bool opaque = false;
	auto ToggleOpacity = [&]()
	{
		opaque = !opaque;
		if( opaque )
		{
			shader.SetCulling( ae::Culling::CounterclockwiseFront );
			shader.SetBlending( false );
			shader.SetDepthWrite( true );
			shader.SetDepthTest( true );
		}
		else
		{
			shader.SetCulling( ae::Culling::None );
			shader.SetBlending( true );
			shader.SetDepthWrite( false );
			shader.SetDepthTest( false );
		}
	};
	ToggleOpacity();
	
	// Wireframe setting
	bool wireframe = true;
	auto ToggleWireframe = [&]()
	{
		wireframe = !wireframe;
		shader.SetWireframe( wireframe );
	};
	ToggleWireframe();

	// Octree visualization settings
	bool showOctreeSurface = false;
	auto ToggleOctreeSurface = [&]()
	{
		showOctreeSurface = !showOctreeSurface;
	};

	class SDFToMesh
	{
	public:
		ae::Str32 name;
		ae::Color color;
		SDFToMesh( const char* name, ae::Color color ) :
			name( name ),
			color( color )
		{}

		ae::Vec3 center = ae::Vec3( 0.0f );
		IsosurfaceExtractor* extractor = ae::New< IsosurfaceExtractor >( TAG_ISOSURFACE, TAG_ISOSURFACE );
		IsosurfaceExtractorCache* cache = ae::New< IsosurfaceExtractorCache >( TAG_ISOSURFACE );
		ae::VertexBuffer sdfVertexBuffer;
		ae::Array< ae::Vec3 > errors = TAG_ISOSURFACE;
		void Run( ae::Matrix4 transform )
		{
			// Cache
			AE_INFO( "[#] Caching SDF...", name );
			const double cacheStart = ae::GetTime();
			const ae::Vec3 scale = transform.GetScale();
			transform = transform.GetScaleRemoved();
			const ae::Matrix4 inverseTransform = transform.GetInverse();
			const auto surfaceFn = [&inverseTransform, &scale]( ae::Vec3 _p )
			{
				const float smooth = 0.0f; // World space because scale is applied separately from transform
				const ae::Vec3 p = inverseTransform.TransformPoint3x4( _p );
				float r = SDFBox( p, ae::Vec3( 0.5f, 0.25f, 0.25f ) * scale, smooth );
				r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.5f, 0.25f ) * scale, smooth ), smooth );
				r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.25f, 0.5f ) * scale, smooth ), smooth );
				return r;
				// return ( ae::Vec3( scale.z ) - p ).Length();
			};
			cache->Generate( {
				.fn=[]( ae::Vec3 position, const void* userData ) { return (*(decltype(surfaceFn)*)userData)( position ); },
				.userData=&surfaceFn,
				.center=center,
				.halfSize=( kChunkSize * 0.5f + 5 ) } // @TODO: The sdf extraction needs a little bit of padding, but this plus 5 should be cleaned up
			);
			const double cacheEnd = ae::GetTime();
			AE_INFO( "[#] SDF cache complete. sec:#", name, cacheEnd - cacheStart );
			
			// Mesh
			errors.Clear();
			AE_INFO( "[#] Start mesh generation...", name );
			const double isosurfaceStart = ae::GetTime();
			extractor->Generate( cache, 0, 0, &errors );
			const double isosurfaceEnd = ae::GetTime();
			AE_INFO( "[#] Mesh generation complete. sec:# verts:# indices:# [#]",
				name,
				isosurfaceEnd - isosurfaceStart,
				extractor->vertices.Length(),
				extractor->indices.Length(),
				"@TODO:aabb" );
			if( !extractor->vertices.Length() )
			{
				AE_INFO( "[#] No mesh generated", name );
				return;
			}
			// (Re)Initialize ae::VertexArray here only when needed
			if( !sdfVertexBuffer.GetMaxVertexCount() // Not initialized
				|| sdfVertexBuffer.GetMaxVertexCount() < extractor->vertices.Length() // Too little storage for verts
				|| sdfVertexBuffer.GetMaxIndexCount() < extractor->indices.Length() ) // Too little storage for t_chunkIndices
			{
				sdfVertexBuffer.Initialize(
					sizeof( IsosurfaceVertex ),
					sizeof( IsosurfaceIndex ),
					extractor->vertices.Length(),
					extractor->indices.Length(),
					ae::Vertex::Primitive::Triangle,
					ae::Vertex::Usage::Dynamic,
					ae::Vertex::Usage::Dynamic
				 );
				sdfVertexBuffer.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( IsosurfaceVertex, position ) );
				sdfVertexBuffer.AddAttribute( "a_normal", 3, ae::Vertex::Type::Float, offsetof( IsosurfaceVertex, normal ) );
			}

			// Set vertices
			sdfVertexBuffer.UploadVertices( 0, extractor->vertices.Data(), extractor->vertices.Length() );
			sdfVertexBuffer.UploadIndices( 0, extractor->indices.Data(), extractor->indices.Length() );
		}

		bool show = true;
		void Draw( ae::Shader* shader, ae::UniformList& uniforms, ae::DebugLines* debugLines )
		{
			if( !show || !extractor->indices.Length() )
			{
				return;
			}
			uniforms.Set( "u_color", color.GetLinearRGB() );
			sdfVertexBuffer.Bind( shader, uniforms );
			sdfVertexBuffer.Draw( 0, extractor->indices.Length() / 3 );
			for( ae::Vec3 error : errors )
			{
				debugLines->AddSphere( error, 0.5f, ae::Color::AetherRed(), 4 );
			}
		}

		~SDFToMesh()
		{
			ae::Delete( cache );
			ae::Delete( extractor );
		}
	};
	// SDFToMesh sdfToMesh[] =
	// {
	// 	{ "Mesh0", ae::Color::HSV(0.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh1", ae::Color::HSV(1.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh2", ae::Color::HSV(2.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh3", ae::Color::HSV(3.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh4", ae::Color::HSV(4.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh5", ae::Color::HSV(5.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh6", ae::Color::HSV(6.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh7", ae::Color::HSV(7.0f / 8.0f, 0.7f, 0.5f ) },
	// };
	// sdfToMesh[ 0 ].center = ae::Vec3( -1, -1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 1 ].center = ae::Vec3( -1, -1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 2 ].center = ae::Vec3( -1, 1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 3 ].center = ae::Vec3( -1, 1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 4 ].center = ae::Vec3( 1, -1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 5 ].center = ae::Vec3( 1, -1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 6 ].center = ae::Vec3( 1, 1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 7 ].center = ae::Vec3( 1, 1, 1 ) * ( kChunkSize * 0.5f );
	SDFToMesh sdfToMesh[] =
	{
		{ "Mesh0", ae::Color::HSV(0.0f / 8.0f, 0.7f, 0.5f ) },
	};
	sdfToMesh[ 0 ].center = ae::Vec3( 0, 0, 0 ) * kChunkSize;

	ae::Vec3 translation;
	ae::Vec3 rotation;
	ae::Vec3 scale;
	ae::Matrix4 prevTransform = ae::Matrix4::Scaling( 0.0f ); // Different than transform on frame 1
	auto ResetTransform = [&]()
	{
		translation = ae::Vec3( 0.0f );
		rotation = ae::Vec3( 0.0f );
		scale = ae::Vec3( 150.0f );
	};
	ResetTransform();

	auto Update = [&]() -> bool
	{
		const float dt = ae::Min( 0.1f, timeStep.GetDt() );
		input.Pump();
		camera.Update( &input, dt );
		for( uint32_t i = 0; i < countof(sdfToMesh); i++ )
		{
			if( input.GetPress( ae::Key( (uint32_t)ae::Key::Num1 + i ) ) )
			{
				sdfToMesh[ i ].show = !sdfToMesh[ i ].show;
			}
		}
		if( input.GetPress( ae::Key::Num0 ) )
		{
			for( uint32_t i = 0; i < countof(sdfToMesh); i++ )
			{
				sdfToMesh[ i ].show = false;
			}
		}
		// Rendering
		if( input.GetPress( ae::Key::LeftBracket ) ) { ToggleOpacity(); }
		if( input.GetPress( ae::Key::RightBracket ) ) { ToggleWireframe(); }
		if( input.GetPress( ae::Key::P ) ) { ToggleOctreeSurface(); }
		// Reset
		if( input.GetPress( ae::Key::R ) ) { ResetTransform(); }
		// Translation
		const float transformSpeed = 2.0f * ( scale.x + scale.y + scale.z ) / 3.0f;
		if( input.Get( ae::Key::D ) ) { translation.x += transformSpeed * dt; }
		if( input.Get( ae::Key::A ) ) { translation.x -= transformSpeed * dt; }
		if( input.Get( ae::Key::W ) ) { translation.y += transformSpeed * dt; }
		if( input.Get( ae::Key::S ) ) { translation.y -= transformSpeed * dt; }
		if( input.Get( ae::Key::E ) ) { translation.z += transformSpeed * dt; }
		if( input.Get( ae::Key::Q ) ) { translation.z -= transformSpeed * dt; }
		// Rotation
		if( input.Get( ae::Key::Z ) ) { rotation.x += 1.0f * dt; }
		if( input.Get( ae::Key::X ) ) { rotation.y += 1.0f * dt; }
		if( input.Get( ae::Key::C ) ) { rotation.z += 1.0f * dt; }
		// Scale
		if( input.Get( ae::Key::L ) ) { scale.x += transformSpeed * dt; }
		if( input.Get( ae::Key::J ) ) { scale.x -= transformSpeed * dt; }
		if( input.Get( ae::Key::I ) ) { scale.y += transformSpeed * dt; }
		if( input.Get( ae::Key::K ) ) { scale.y -= transformSpeed * dt; }
		if( input.Get( ae::Key::O ) ) { scale.z += transformSpeed * dt; }
		if( input.Get( ae::Key::U ) ) { scale.z -= transformSpeed * dt; }
		scale = ae::Max( ae::Vec3( 0.01f ), scale );
		// ZYX rotation
		const ae::Quaternion orientation =
			ae::Quaternion( ae::Vec3( 1, 0, 0 ), rotation.x ) *
			ae::Quaternion( ae::Vec3( 0, 1, 0 ), rotation.y ) *
			ae::Quaternion( ae::Vec3( 0, 0, 1 ), rotation.z );
		// Generate SDF when transform changes
		const ae::Matrix4 transform = ae::Matrix4::LocalToWorld( translation, orientation, scale );
		if( prevTransform != transform )
		{
			for( SDFToMesh& sdf : sdfToMesh )
			{
				sdf.Run( transform );
			}
			prevTransform = transform;
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		debugLines.AddAABB( ae::Vec3( 0.0f ), ae::Vec3( kChunkSize * 0.5f ), ae::Color::AetherOrange() );
		debugLines.AddLine( ae::Vec3( -1, 0, 0 ) * 1000.0f, ae::Vec3( 1, 0, 0 ) * 1000.0f, ae::Color::AetherRed() );
		debugLines.AddLine( ae::Vec3( 0, -1, 0 ) * 1000.0f, ae::Vec3( 0, 1, 0 ) * 1000.0f, ae::Color::AetherGreen() );
		debugLines.AddLine( ae::Vec3( 0, 0, -1 ) * 1000.0f, ae::Vec3( 0, 0, 1 ) * 1000.0f, ae::Color::AetherBlue() );
		debugLines.AddOBB( transform * ae::Matrix4::Scaling( 1.0f ), ae::Color::AetherPurple() );
		
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.1f, 1000.0f );
		const ae::Matrix4 worldToProj = viewToProj * worldToView;
		
		ae::UniformList uniforms;
		uniforms.Set( "u_worldToProj", worldToProj );
		for( auto& sdf : sdfToMesh )
		{
			sdf.Draw( &shader, uniforms, &debugLines );
			if( showOctreeSurface )
			{
				sdf.cache->DrawOctree( &debugLines );
			}
		}
		debugLines.Render( worldToProj );
		
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while ( Update() ) {}
#endif

	AE_LOG( "Terminate" );
	return 0;
}

//------------------------------------------------------------------------------
// Private / template implementation
//------------------------------------------------------------------------------
void IsosurfaceExtractorCache::Generate( const IsosurfaceParams& params )
{
	if( params.halfSize > 0.0f )
	{
		m_params = params;
		// Ignore AE_TERRAIN_USE_CACHE here, just so the AABB generates
		m_octree.Build( m_params.fn, m_params.userData, m_params.center, m_params.halfSize );
	}
}

//------------------------------------------------------------------------------
// IsosurfaceExtractorCache member functions
//------------------------------------------------------------------------------
float IsosurfaceExtractorCache::GetValue( ae::Vec3 pos ) const
{
#if AE_TERRAIN_USE_CACHE
	return m_octree.GetValue( pos );
#else
	return m_params.fn( pos, m_params.userData );
#endif
}

ae::Vec3 IsosurfaceExtractorCache::GetDerivative( ae::Vec3 p ) const
{
	// @TODO: m_octree.GetValues( ae::Array< ae::Vec3, N > pos, ae::Array< float, N >& resultsOut );
	ae::Vec3 pv( GetValue( p ) );
	AE_DEBUG_ASSERT( pv == pv );
	
	ae::Vec3 normal0;
	for( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] += m_params.normalSampleOffset;
		normal0[ i ] = GetValue( nt );
	}
	AE_DEBUG_ASSERT( normal0 != ae::Vec3( 0.0f ) );
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal0 -= pv;
	AE_DEBUG_ASSERT( normal0 != ae::Vec3( 0.0f ) );
	normal0 /= normal0.Length();
	AE_DEBUG_ASSERT( normal0 == normal0 );

	ae::Vec3 normal1;
	for( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] -= m_params.normalSampleOffset;
		normal1[ i ] = GetValue( nt );
	}
	AE_DEBUG_ASSERT( normal1 != ae::Vec3( 0.0f ) );
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal1 = pv - normal1;
	AE_DEBUG_ASSERT( normal1 != ae::Vec3( 0.0f ) );
	normal1 /= normal1.Length();
	AE_DEBUG_ASSERT( normal1 == normal1 );

	return ( normal1 + normal0 ).SafeNormalizeCopy();
}

void IsosurfaceExtractorCache::DrawOctree( ae::DebugLines* debugLines )
{
	const ae::AABB aabb = m_octree.GetAABB();
	debugLines->AddAABB( aabb.GetCenter(), aabb.GetHalfSize(), ae::Color::AetherWhite() );
#if AE_TERRAIN_USE_CACHE
	const uint32_t octantCount = m_octree.GetOctantCount();
	for( uint32_t i = 0; i < octantCount; i++ )
	{
		const SDFOctant* octant = m_octree.GetOctant( i );
		const ae::AABB aabb = octant->GetAABB();
		const ae::Vec3 min = aabb.GetMin();
		const ae::Vec3 max = aabb.GetMax();
		const ae::Vec3 center = aabb.GetCenter();
		if( octant->IntersectsZero() && !octant->GetChildCount() )
		{
			const ae::Color color = ae::Color::AetherRed();
			debugLines->AddLine( ae::Vec3( center.x, center.y, min.z ), ae::Vec3( center.x, center.y, max.z ), color );
			debugLines->AddLine( ae::Vec3( min.x, center.y, center.z ), ae::Vec3( max.x, center.y, center.z ), color );
			debugLines->AddLine( ae::Vec3( center.x, min.y, center.z ), ae::Vec3( center.x, max.y, center.z ), color );
		}
	}
#endif
}

//------------------------------------------------------------------------------
// IsosurfaceExtractor member functions
//------------------------------------------------------------------------------
IsosurfaceExtractor::IsosurfaceExtractor( ae::Tag tag ) :
	vertices( tag ),
	indices( tag )
#if AE_TERRAIN_USE_VERTEX_MAP
	,m_voxelToVertex( tag )
#endif
{}

void IsosurfaceExtractor::Generate( const IsosurfaceExtractorCache* sdf, uint32_t maxVerts, uint32_t maxIndices, ae::Array< ae::Vec3 >* errors )
{
	if( maxVerts == 0 )
	{
		maxVerts = ae::MaxValue< uint32_t >();
	}
	if( maxIndices == 0 )
	{
		maxIndices = ae::MaxValue< uint32_t >();
	}

	vertices.Clear();
	indices.Clear();
#if AE_TERRAIN_USE_VERTEX_MAP
	m_voxelToVertex.Clear();
#else
	memset( m_i, ~(uint8_t)0, sizeof( m_i ) );
	AE_STATIC_ASSERT( std::is_pod_v< decltype(m_i) > );
#endif
	memset( m_tempEdges, 0, kTempChunkSize3 * sizeof( *m_tempEdges ) );
	AE_STATIC_ASSERT( std::is_pod_v< ae::StripType< decltype(*m_tempEdges) > > );

	// @TODO: Description
	const uint16_t EDGE_TOP_FRONT_BIT = ( 1 << 0 );
	const uint16_t EDGE_TOP_RIGHT_BIT = ( 1 << 1 );
	const uint16_t EDGE_SIDE_FRONTRIGHT_BIT = ( 1 << 2 );
	const uint16_t EDGE_VISITED = ( 1 << 3 );
	// For expansion of edge intersections into triangles
	const ae::Int3 offsets_EDGE_TOP_FRONT_BIT[ 4 ] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
	const ae::Int3 offsets_EDGE_TOP_RIGHT_BIT[ 4 ] = { { 0, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 1 } };
	const ae::Int3 offsets_EDGE_SIDE_FRONTRIGHT_BIT[ 4 ] = { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 }, { 1, 1, 0 } };
	// @TODO: Description
	const uint16_t mask[ 3 ] = { EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT };
	// 3 new edges to test
	const ae::Vec3 cornerOffsets[ 3 ] = {
		{ 0, 1, 1 }, // EDGE_TOP_FRONT_BIT
		{ 1, 0, 1 }, // EDGE_TOP_RIGHT_BIT
		{ 1, 1, 0 } // EDGE_SIDE_FRONTRIGHT_BIT
	};

	// @TODO: This is definitely wrong...
	const ae::Vec3 generationOffset = ae::Vec3( kChunkSize / -2.0f );//;-ae::Vec3( sdf->GetHalfSize() ); // -sdf->GetCenter();// 
	const ae::Int3 cornerOffsetInt = ae::Int3( (int32_t)kChunkSize / -2 );
	
	// This phase finds the surface of the SDF and generates the list of
	// vertices along with all of the 'lattice' edge intersections. The vertex
	// positions will be centered within their voxels at the end of this phase,
	// and will be nudged later to the correct position based on the SDF
	// surface.
	auto DoVoxel = [&]( int32_t x, int32_t y, int32_t z )
	{
		const uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		TempEdges* te = &m_tempEdges[ edgeIndex ];
		if( te->b & EDGE_VISITED )
		{
			return true;
		}
		te->b |= EDGE_VISITED;

		const ae::Vec3 voxelPos( x, y, z );
		// This nudge is needed to prevent the SDF from ever being exactly on
		// the voxel grid boundaries (imagine a plane at the origin with a
		// normal facing along a cardinal axis, do the vertices belong to the
		// voxels on the front or back of the plane?). Without this nudge, any
		// vertices exactly on the grid boundary would be skipped resulting in
		// holes in the mesh.
		auto Nudge = []( float v ) { return ( v == 0.0f ) ? 0.0001f : v; };
		const ae::Vec3 sharedCornerOffset( 1.0f );
		const float sharedCornerValue = Nudge( sdf->GetValue( generationOffset + voxelPos + sharedCornerOffset ) );
		if( ae::Abs( sharedCornerValue ) > 2.0f ) // @TODO: This value could be smaller
		{
			// Early out of additional edge intersections if far from the surface
			return true;
		}
		const float cornerValues[ 3 ] =
		{
			Nudge( sdf->GetValue( generationOffset + voxelPos + cornerOffsets[ 0 ] ) ),
			Nudge( sdf->GetValue( generationOffset + voxelPos + cornerOffsets[ 1 ] ) ),
			Nudge( sdf->GetValue( generationOffset + voxelPos + cornerOffsets[ 2 ] ) )
		};
		if( !errors )
		{
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) );
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) );
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) );
		}
		else if( ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) > 1.01f
			|| ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) > 1.01f
			|| ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) > 1.01f )
		{
			errors->Append( generationOffset + voxelPos );
			return true;
		}
		
		// Detect if any of the 3 new edges being tested intersect the implicit surface
		uint16_t edgeBits = 0;
		if( cornerValues[ 0 ] * sharedCornerValue < 0.0f ) { edgeBits |= EDGE_TOP_FRONT_BIT; }
		if( cornerValues[ 1 ] * sharedCornerValue < 0.0f ) { edgeBits |= EDGE_TOP_RIGHT_BIT; }
		if( cornerValues[ 2 ] * sharedCornerValue < 0.0f ) { edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT; }
		te->b = edgeBits;
		
		// Iterate over the 3 edges that this voxel is responsible for. The
		// remaining 9 are handled by adjacent voxels.
		for( int32_t e = 0; e < 3; e++ )
		if( edgeBits & mask[ e ] )
		{
			if( vertices.Length() + 4 > maxVerts || indices.Length() + 6 > maxIndices )
			{
				return false;
			}

			// Sphere trace the voxel edge from the outside corner to the inside
			// corner to find the intersection with the SDF surface
			ae::Vec3 edgeOffset01;
			{
				const bool sharedCornerInside = ( sharedCornerValue < cornerValues[ e ] );
				const ae::Vec3 start = ( sharedCornerInside ? cornerOffsets[ e ] : sharedCornerOffset );
				const ae::Vec3 end = ( sharedCornerInside ? sharedCornerOffset : cornerOffsets[ e ] );
				const ae::Vec3 rayDir = ( end - start ); // No need to normalize since voxel size is 1
				float depth = 0.0f;
				for( int32_t i = 0; i < 8; i++ ) // @TODO: This should probably be adjustable
				{
					edgeOffset01 = start + rayDir * depth;
					const float closestSurfaceDist = sdf->GetValue( generationOffset + voxelPos + edgeOffset01 );
					if( closestSurfaceDist < 0.01f )
					{
						break; // Hit the surface
					}
					depth += closestSurfaceDist;
					if( depth >= 1.0f )
					{
						AE_DEBUG_FAIL_MSG(  "depth >= 1", "depth:#", depth );
						depth = 1.0f;
						break;
					}
				}
			}
			AE_DEBUG_ASSERT( edgeOffset01.x == edgeOffset01.x && edgeOffset01.y == edgeOffset01.y && edgeOffset01.z == edgeOffset01.z );
			AE_DEBUG_ASSERT( edgeOffset01.x >= 0.0f && edgeOffset01.x <= 1.0f );
			AE_DEBUG_ASSERT( edgeOffset01.y >= 0.0f && edgeOffset01.y <= 1.0f );
			AE_DEBUG_ASSERT( edgeOffset01.z >= 0.0f && edgeOffset01.z <= 1.0f );
			
			const ae::Vec3 edgeWorldPos = voxelPos + edgeOffset01;
			te->p[ e ] = edgeOffset01;
			te->n[ e ] = sdf->GetDerivative( generationOffset + edgeWorldPos );
			
			if( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
			{
				continue;
			}
			
			const ae::Int3* offsets; // Array of 4 sampling offsets for this edge
			switch( e )
			{
				case 0: offsets = offsets_EDGE_TOP_FRONT_BIT; break;
				case 1: offsets = offsets_EDGE_TOP_RIGHT_BIT; break;
				case 2: offsets = offsets_EDGE_SIDE_FRONTRIGHT_BIT; break;
				default: AE_FAIL(); offsets = nullptr; break;
			}
			
			// Expand edge intersection into two triangles. New vertices are
			// added as needed for each edge intersection, so this does some of
			// the work for adjacent voxels. Vertices are centered in voxels at
			// this point at this stage.
			IsosurfaceIndex quad[ 4 ];
			for( int32_t j = 0; j < 4; j++ )
			{
				const int32_t ox = x + offsets[ j ][ 0 ];
				const int32_t oy = y + offsets[ j ][ 1 ];
				const int32_t oz = z + offsets[ j ][ 2 ];
				
				// This check allows coordinates to be one out of chunk high end
				// @TODO: It looks like a very similar check is already done above?
				if( ox < 0 || oy < 0 || oz < 0 || ox > kChunkSize || oy > kChunkSize || oz > kChunkSize )
				{
					continue;
				}
				
				// @TODO: Remove this 'in chunk' check, all indices should be recorded
				const bool inCurrentChunk = ox < kChunkSize && oy < kChunkSize && oz < kChunkSize;
				const uint32_t vertexLookupHash = GetInt3Hash( ox, oy, oz );
#if AE_TERRAIN_USE_VERTEX_MAP
				IsosurfaceIndex vertexIndex = m_voxelToVertex.Get( vertexLookupHash, kInvalidIsosurfaceIndex );
#else
				IsosurfaceIndex vertexIndex = m_i[ ox ][ oy ][ oz ];
#endif
				if( !inCurrentChunk || vertexIndex == kInvalidIsosurfaceIndex )
				{
					IsosurfaceVertex vertex;
					vertex.position.x = ox + 0.5f;
					vertex.position.y = oy + 0.5f;
					vertex.position.z = oz + 0.5f;
					vertex.position.w = 1.0f;
					
					AE_DEBUG_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );
					
					vertexIndex = (IsosurfaceIndex)vertices.Length();
					vertices.Append( vertex );
					quad[ j ] = vertexIndex;
					
					// @TODO: Always store indices that are outside of the
					// chunk, because they are they should be included in the
					// final mesh only once.
					if( inCurrentChunk )
					{
						// Record the index of the vertex in the chunk so it can
						// be reused by adjacent quads
#if AE_TERRAIN_USE_VERTEX_MAP
						m_voxelToVertex.Set( vertexLookupHash, vertexIndex );
#else
						m_i[ ox ][ oy ][ oz ] = vertexIndex;
#endif
					}
				}
				else
				{
					AE_DEBUG_ASSERT_MSG( vertexIndex < (IsosurfaceIndex)vertices.Length(), "# < # ox:# oy:# oz:#", index, vertices.Length(), ox, oy, oz );
					AE_DEBUG_ASSERT( ox < kChunkSize );
					AE_DEBUG_ASSERT( oy < kChunkSize );
					AE_DEBUG_ASSERT( oz < kChunkSize );
					quad[ j ] = vertexIndex;
				}
			}
			
			// @TODO: This assumes counter clockwise culling
			bool flip = false;
			if( e == 0 ) { flip = ( sharedCornerValue > 0.0f ); } // EDGE_TOP_FRONT_BIT
			else { flip = ( sharedCornerValue < 0.0f ); } // EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
			if( flip )
			{
				// tri0
				indices.Append( quad[ 0 ] );
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 2 ] );
				// tri1
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 3 ] );
				indices.Append( quad[ 2 ] );
			}
			else
			{
				// tri2
				indices.Append( quad[ 0 ] );
				indices.Append( quad[ 2 ] );
				indices.Append( quad[ 1 ] );
				// tri3
				indices.Append( quad[ 1 ] );
				indices.Append( quad[ 2 ] );
				indices.Append( quad[ 3 ] );
			}
		}
		return true;
	};
	const ae::Int3 generationMin( -1 );
	const ae::Int3 generationMax( kChunkSize + 1 );
	const SDFOctree* octree = sdf->GetOctree();
	const ae::Int3 octreeMin = ae::Max( generationMin, octree->GetAABB().GetMin().FloorCopy() - cornerOffsetInt );
	const ae::Int3 octreeMax = ae::Min( generationMax, octree->GetAABB().GetMax().CeilCopy() - cornerOffsetInt );
#if AE_TERRAIN_ITERATE_OCTREE_LEAVES
	// Iterate over octree nodes instead of voxels, relying on EDGE_VISITED to
	// skip voxels that are already processed. This can be slow since individual
	// octree nodes overlap each other slightly when they are quantized to the
	// voxel grid, the cost of iterating over the octree can be higher than just
	// iterating over all of the voxels exactly once.
	for( uint32_t i = 0; i < octree->GetOctantCount(); i++ )
	{
		const SDFOctant* octant = octree->GetOctant( i );
		if( octant->IntersectsZero() && !octant->GetChildCount() )
		{
			const ae::AABB aabb = octant->GetAABB();
			const ae::Int3 min = ae::Max( octreeMin, aabb.GetMin().FloorCopy() - cornerOffsetInt );
			const ae::Int3 max = ae::Min( octreeMax, aabb.GetMax().CeilCopy() - cornerOffsetInt );
			for( int32_t z = min.z; z < max.z; z++ )
			for( int32_t y = min.y; y < max.y; y++ )
			for( int32_t x = min.x; x < max.x; x++ )
			if( !DoVoxel( x, y, z ) )
			{
				vertices.Clear();
				indices.Clear();
				return;
			}
		}
	}
#else
	for( int32_t z = octreeMin.z; z < octreeMax.z; z++ )
	for( int32_t y = octreeMin.y; y < octreeMax.y; y++ )
	for( int32_t x = octreeMin.x; x < octreeMax.x; x++ )
	{
		if( !DoVoxel( x, y, z ) )
		{
			vertices.Clear();
			indices.Clear();
			return;
		}
	}
#endif
	
	if( indices.Length() == 0 )
	{
		vertices.Clear();
		return;
	}
	
	for( IsosurfaceVertex& vertex : vertices )
	{
		const int32_t x = ae::Floor( vertex.position.x );
		const int32_t y = ae::Floor( vertex.position.y );
		const int32_t z = ae::Floor( vertex.position.z );
		AE_DEBUG_ASSERT( x >= 0 && y >= 0 && z >= 0 );
		AE_DEBUG_ASSERT( x <= kChunkSize && y <= kChunkSize && z <= kChunkSize );
		
		int32_t ec = 0;
		ae::Vec3 p[ 12 ];
		ae::Vec3 n[ 12 ];
		
		uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		TempEdges te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		if( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		if( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + z * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].y -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
		AE_DEBUG_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		
		// Validation
		AE_DEBUG_ASSERT( ec != 0 );
		for( int32_t j = 0; j < ec; j++ )
		{
			AE_DEBUG_ASSERT( p[ j ] == p[ j ] );
			AE_DEBUG_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
			AE_DEBUG_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
			AE_DEBUG_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
			AE_DEBUG_ASSERT( n[ j ] == n[ j ] );
		}

		// Normal
		vertex.normal = ae::Vec3( 0.0f );
		for( int32_t j = 0; j < ec; j++ )
		{
			vertex.normal += n[ j ];
		}
		vertex.normal.SafeNormalize();
		
		// Position
		ae::Vec3 position;
		// Get intersection of edge planes
		{
#if AE_TERRAIN_SIMD
			__m128 c128 = _mm_setzero_ps();
			for( uint32_t i = 0; i < ec; i++ )
			{
				__m128 p128 = _mm_load_ps( (float*)( p + i ) );
				c128 = _mm_add_ps( c128, p128 );
			}
			__m128 div = _mm_set1_ps( 1.0f / ec );
			c128 = _mm_mul_ps( c128, div );
			
			for( uint32_t i = 0; i < 10; i++ )
			for( uint32_t j = 0; j < ec; j++ )
			{
				__m128 p128 = _mm_load_ps( (float*)( p + j ) );
				p128 = _mm_sub_ps( p128, c128 );
				__m128 n128 = _mm_load_ps( (float*)( n + j ) );
				
				__m128 d = _mm_mul_ps( p128, n128 );
				d = _mm_hadd_ps( d, d );
				d = _mm_hadd_ps( d, d );
				
				__m128 s = _mm_set1_ps( 0.5f );
				s = _mm_mul_ps( s, n128 );
				s = _mm_mul_ps( s, d );
				c128 = _mm_add_ps( c128, s );
			}
			_mm_store_ps( (float*)&position, c128 );
#else
			position = ae::Vec3( 0.0f );
			for( uint32_t i = 0; i < ec; i++ )
			{
				position += p[ i ];
			}
			position /= ec;
			for( uint32_t i = 0; i < 10; i++ )
			{
				for( uint32_t j = 0; j < ec; j++ )
				{
					float d = n[ j ].Dot( p[ j ] - position );
					position += n[ j ] * ( d * 0.5f );
				}
			}
#endif
			AE_DEBUG_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
			// @NOTE: Bias towards average of intersection points. This solves some intersecting triangles on sharp edges.
			// Based on notes here: https://www.boristhebrave.com/2018/04/15/dual-contouring-tutorial/
			ae::Vec3 averagePos( 0.0f );
			for( int32_t i = 0; i < ec; i++ )
			{
				averagePos += p[ i ];
			}
			averagePos /= (float)ec;
			position = ae::Lerp( position, averagePos, 0.1f ); // @TODO: This bias should be removed or be adjustable
		}
		// @NOTE: Do not clamp position values to voxel boundary. It's valid for a vertex to be placed
		// outside of the voxel is was generated from. This happens when a voxel has all corners inside
		// or outside of the sdf boundary, while also still having intersections (normally two per edge)
		// on one or more edges of the voxel.
		position.x = x + position.x;
		position.y = y + position.y;
		position.z = z + position.z;
		vertex.position = ae::Vec4( position + generationOffset, 1.0f );
	}

	AE_DEBUG_ASSERT( vertices.Length() <= maxVerts );
	AE_DEBUG_ASSERT( indices.Length() <= maxIndices );
}
