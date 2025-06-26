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
// #define _AE_DEBUG_ 1
#include "aether.h"
#include <cstdint>
#if _AE_DEBUG_
#define AE_DEBUG_IF( _expr ) if( _expr )
#else
#define AE_DEBUG_IF( _expr ) if constexpr( false )
#endif

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

#define AE_TERRAIN_USE_CACHE 0

//------------------------------------------------------------------------------
// Types / constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ISOSURFACE = "isosurface";

struct VoxelIndex
{
	VoxelIndex() {}
	VoxelIndex( int32_t x, int32_t y, int32_t z ) : x( x ), y( y ), z( z ) {}
	bool operator==( const VoxelIndex& other ) const { return x == other.x && y == other.y && z == other.z; }
	int32_t x;
	int32_t y;
	int32_t z;
};
namespace ae { template <> uint32_t GetHash( const VoxelIndex& index )
{
	// Create a hash using the index as a seed to prevent large consecutive map
	// entries, where collisions become very expensive to handle.
	constexpr uint32_t uint32MaxGridSize = 1625; // UINT32_MAX ^ (1/3)
	return ae::Hash().HashBasicType( index.x + uint32MaxGridSize * ( index.y + index.z * uint32MaxGridSize ) ).Get();
} }
struct IsosurfaceVertex
{
	ae::Vec4 position;
	ae::Vec3 normal;
};
typedef uint32_t IsosurfaceIndex;
using IsosurfaceFn = float( * )( ae::Vec3, const void* );
struct IsosurfaceParams
{
	IsosurfaceFn fn = nullptr;
	const void* userData = nullptr;
	ae::AABB aabb = ae::AABB();
	float normalSampleOffset = 0.1f;
	ae::DebugLines* debug = nullptr;
};

//------------------------------------------------------------------------------
// IsosurfaceExtractorCache class
//------------------------------------------------------------------------------
class IsosurfaceExtractorCache
{
public:
	class Zone
	{
	public:
		static constexpr int32_t Dim = 256;
		static ae::Int3 GetSize() { return ae::Int3( Dim, Dim, Dim ); }
		void SetOffset( ae::Int3 offset ) { m_offset = offset; }
		ae::Int3 GetOffset() const { return m_offset; }
		void Set( ae::Int3 pos, uint8_t value ) { m_values[ pos.z ][ pos.y ][ pos.x ] = value; }
		uint8_t Get( ae::Int3 pos ) const { return m_values[ pos.z ][ pos.y ][ pos.x ]; }
	private:
		ae::Int3 m_offset = ae::Int3( 0 );
		uint8_t m_values[ Dim ][ Dim ][ Dim ] = {}; // Default initialization
	};
	
	IsosurfaceExtractorCache();
	void Generate( const IsosurfaceParams& params );
	void Reset();
	
	float GetValue( ae::Vec3 position ) const;
	ae::Vec3 GetDerivative( ae::Vec3 p ) const;

	const Zone* GetZone( uint32_t i ) const { return m_zones.GetValue( i ); }
	uint32_t GetZoneCount() const { return m_zones.Length(); }
	
	inline const IsosurfaceParams& GetParams() const { return m_params; }
	inline const ae::AABB& GetOctreeAABB() const { return m_surfaceAABB; }
	inline uint32_t GetEstimatedVertexCount() const { return m_estimatedVertexCount; }

private:
	void m_Generate( ae::Vec3 center, ae::Vec3 halfSize );
	IsosurfaceParams m_params;
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
	ae::AABB m_surfaceAABB;
	float m_estimatedVertexCount = 0.0f;
	ae::Map< ae::Int3, Zone* > m_zones = AE_ALLOC_TAG_FIXME;
};

//------------------------------------------------------------------------------
// IsosurfaceExtractor class
//------------------------------------------------------------------------------
struct IsosurfaceExtractor
{
	IsosurfaceExtractor( ae::Tag tag );
	void Generate( const IsosurfaceParams& params, uint32_t maxVerts, uint32_t maxIndices, ae::Array< ae::Vec3 >* errors );
	
	ae::Array< IsosurfaceVertex > vertices;
	ae::Array< IsosurfaceIndex > indices;

	struct Stats
	{
		uint32_t estimatedVertexCount = 0;
		uint32_t iterationCount = 0;
		uint32_t workingCount = 0;
		uint32_t sampleCount = 0;
	};
	const Stats& GetStats() const { return m_stats; }

private:
	static constexpr IsosurfaceIndex kInvalidIsosurfaceIndex = ~0;
	struct Voxel
	{
		// 3 planes whose intersections are used to position vertices within voxel
		// EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
		ae::Vec3 edgeNormal[ 3 ];
		ae::Vec3 edgePos[ 3 ];
		IsosurfaceIndex index = kInvalidIsosurfaceIndex;
		uint16_t edgeBits = 0;
	};
	Stats m_stats;
	ae::Map< VoxelIndex, Voxel > m_voxels;
	IsosurfaceExtractorCache m_sdf;
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
AE_UNIFORM vec2 u_light;
AE_IN_HIGHP vec3 v_normal;
void main()
{
	vec3 light = normalize( vec3( -1.0, -1.0, -1.0 ) );
	vec3 ambient = vec3( u_light.y );
	float diffuse = max( 0.0, -dot( v_normal, light ) ) * u_light.x;
	vec3 color = u_color * ( ambient + vec3( 1.0, 1.0, 1.0 ) * diffuse );
	AE_COLOR = vec4( color, 0.5 );
}
)";

bool s_ambientLight = true;
void ToggleAmbientLight()
{
	s_ambientLight = !s_ambientLight;
};

bool s_directionalLight = true;
void ToggleDirectionalLight()
{
	s_directionalLight = !s_directionalLight;
};

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

	class SDFToMesh
	{
	public:
		ae::Str32 name;
		ae::Color color;
		ae::AABB region;
		SDFToMesh( const char* name, ae::Color color, ae::AABB region ) :
			name( name ),
			color( color ),
			region( region )
		{}

		IsosurfaceExtractor* extractor = ae::New< IsosurfaceExtractor >( TAG_ISOSURFACE, TAG_ISOSURFACE );
		ae::VertexBuffer sdfVertexBuffer;
		ae::Array< ae::Vec3 > errors = TAG_ISOSURFACE;
		double cacheTime = 0.0;
		double meshTime = 0.0;
		void Run( ae::Matrix4 transform )
		{
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
			
			// Mesh
			errors.Clear();
			const double meshStart = ae::GetTime();
			extractor->Generate( {
				.fn=[]( ae::Vec3 position, const void* userData ) { return (*(decltype(surfaceFn)*)userData)( position ); },
				.userData=&surfaceFn,
				.aabb=region
			},
			0, 0, &errors );
			const double meshEnd = ae::GetTime();
			meshTime = ( meshEnd - meshStart );
			if( !extractor->vertices.Length() )
			{
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

		void Draw( ae::Shader* shader, ae::UniformList& uniforms, ae::DebugLines* debugLines )
		{
			debugLines->AddAABB( region.GetCenter(), region.GetHalfSize(), color );
			if( extractor->indices.Length() )
			{
				uniforms.Set( "u_light", ae::Vec2( (float)s_directionalLight, (float)s_ambientLight ) );
				uniforms.Set( "u_color", color.GetLinearRGB() );
				sdfVertexBuffer.Bind( shader, uniforms );
				sdfVertexBuffer.Draw( 0, extractor->indices.Length() / 3 );
			}
			for( ae::Vec3 error : errors )
			{
				debugLines->AddSphere( error, 0.5f, ae::Color::AetherRed(), 4 );
			}
		}

		~SDFToMesh()
		{
			ae::Delete( extractor );
		}
	};
	
	// SDFToMesh sdfToMesh[] =
	// {
	// 	{ "Mesh0", ae::Color::HSV( 0.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh1", ae::Color::HSV( 1.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh2", ae::Color::HSV( 2.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh3", ae::Color::HSV( 3.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh4", ae::Color::HSV( 4.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh5", ae::Color::HSV( 5.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh6", ae::Color::HSV( 6.0f / 8.0f, 0.7f, 0.5f ) },
	// 	{ "Mesh7", ae::Color::HSV( 7.0f / 8.0f, 0.7f, 0.5f ) },
	// };
	// sdfToMesh[ 0 ].center = ae::Vec3( -1, -1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 1 ].center = ae::Vec3( -1, -1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 2 ].center = ae::Vec3( -1, 1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 3 ].center = ae::Vec3( -1, 1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 4 ].center = ae::Vec3( 1, -1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 5 ].center = ae::Vec3( 1, -1, 1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 6 ].center = ae::Vec3( 1, 1, -1 ) * ( kChunkSize * 0.5f );
	// sdfToMesh[ 7 ].center = ae::Vec3( 1, 1, 1 ) * ( kChunkSize * 0.5f );
	
	//*/
	// Single region
	SDFToMesh* sdfToMesh[] =
	{
		ae::New< SDFToMesh >(
			TAG_ISOSURFACE,
			"Mesh0",
			ae::Color::HSV(0.0f, 0.7f, 0.5f ),
			ae::AABB( ae::Vec3( -1000 ), ae::Vec3( 1000 ) )
		)
	};
	/*/
	// Columns
	const uint32_t gridCount = 8;
	const float gridSize = 200.0f;
	const float gridHeight = 1000.0f;
	ae::Array< SDFToMesh* > sdfToMesh = TAG_ISOSURFACE;
	for( uint32_t gridY = 0; gridY < gridCount; gridY++ )
	for( uint32_t gridX = 0; gridX < gridCount; gridX++ )
	{
		const int32_t x = ( gridX - gridCount / 2 );
		const int32_t y = ( gridY - gridCount / 2 );
		const ae::Vec3 min( x * gridSize, y * gridSize, gridHeight * -0.5f );
		const ae::Vec3 max = min + ae::Vec3( gridSize, gridSize, gridHeight ) + ae::Vec3( 1.0f, 1.0f, 0.0f );
		sdfToMesh.Append( ae::New< SDFToMesh >(
			TAG_ISOSURFACE,
			"Mesh",
			ae::Color::HSV( (float)( gridX + gridY ) / (float)( gridCount * 2 ), 0.7f, 0.5f ),
			ae::AABB( min, max )
		) );
	}
	//*/

	ae::Vec3 translation;
	ae::Vec3 rotation;
	ae::Vec3 scale;
	ae::Matrix4 prevTransform = ae::Matrix4::Scaling( 0.0f ); // Different than transform on frame 1
	auto ResetTransform = [&]()
	{
		translation = ae::Vec3( 0.0f );
		rotation = ae::Vec3( 0.0f );
		scale = ae::Vec3( 600.0f, 150.0f, 150.0f );
	};
	ResetTransform();

	auto Update = [&]() -> bool
	{
		const float dt = ae::Min( 0.1f, timeStep.GetDt() );
		input.Pump();
		camera.Update( &input, dt );
		// Rendering
		if( input.GetPress( ae::Key::LeftBracket ) ) { ToggleOpacity(); }
		if( input.GetPress( ae::Key::RightBracket ) ) { ToggleWireframe(); }
		if( input.GetPress( ae::Key::Comma ) ) { ToggleAmbientLight(); }
		if( input.GetPress( ae::Key::Period ) ) { ToggleDirectionalLight(); }
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
			const double startTime = ae::GetTime();
			double cacheTime = 0.0;
			double meshTime = 0.0;
			float vertCount = 0;
			float triCount = 0;
			float estimatedVertexCount = 0;
			float iterationCount = 0.0f;
			float workingCount = 0.0f;
			float sampleCount = 0.0f;
			for( SDFToMesh* sdf : sdfToMesh )
			{
				sdf->Run( transform );
				cacheTime += sdf->cacheTime;
				meshTime += sdf->meshTime;
				vertCount += sdf->extractor->vertices.Length();
				triCount += sdf->extractor->indices.Length() / 3.0f;
				estimatedVertexCount += sdf->extractor->GetStats().estimatedVertexCount;
				iterationCount += sdf->extractor->GetStats().iterationCount;
				workingCount += sdf->extractor->GetStats().workingCount;
				sampleCount += sdf->extractor->GetStats().sampleCount;
			}
			const double endTime = ae::GetTime();
			const float vertSamples = ( sampleCount / vertCount );

			double totalTime = ( endTime - startTime );
			uint32_t timeIndex = 0;
			uint32_t countIndex = 0;
			const char* timeUnits[] = { "s", "ms" };
			const char* counts[] = { "", "K", "M", "B" };
			while( totalTime < 1.0 )
			{
				totalTime *= 1000.0;
				meshTime *= 1000.0;
				cacheTime *= 1000.0;
				timeIndex++;
			}
			while( triCount > 1000.0f )
			{
				triCount /= 1000.0f;
				vertCount /= 1000.0f;
				estimatedVertexCount /= 1000.0f;
				countIndex++;
			}
			AE_INFO( "Total:## Cache:## Mesh:## Verts:## Est.:## Tris:##",
				totalTime, timeUnits[ timeIndex ],
				cacheTime, timeUnits[ timeIndex ],
				meshTime, timeUnits[ timeIndex ],
				vertCount, counts[ countIndex ],
				estimatedVertexCount, counts[ countIndex ],
				triCount, counts[ countIndex ]
			);
			uint32_t countIndex2 = 0;
			while( iterationCount > 1000.0f )
			{
				sampleCount /= 1000.0f;
				workingCount /= 1000.0f;
				iterationCount /= 1000.0f;
				countIndex2++;
			}
			AE_INFO( "VertSamples:# Working:## Samples:## Iters:##",
				vertSamples,
				workingCount, counts[ countIndex2 ],
				sampleCount, counts[ countIndex2 ],
				iterationCount, counts[ countIndex2 ]
			);
			prevTransform = transform;
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		debugLines.AddLine( ae::Vec3( -1, 0, 0 ) * 1000.0f, ae::Vec3( 1, 0, 0 ) * 1000.0f, ae::Color::AetherRed() );
		debugLines.AddLine( ae::Vec3( 0, -1, 0 ) * 1000.0f, ae::Vec3( 0, 1, 0 ) * 1000.0f, ae::Color::AetherGreen() );
		debugLines.AddLine( ae::Vec3( 0, 0, -1 ) * 1000.0f, ae::Vec3( 0, 0, 1 ) * 1000.0f, ae::Color::AetherBlue() );
		debugLines.AddOBB( transform * ae::Matrix4::Scaling( 1.0f ), ae::Color::AetherPurple() );
		
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.1f, 10000.0f );
		const ae::Matrix4 worldToProj = viewToProj * worldToView;
		
		ae::UniformList uniforms;
		uniforms.Set( "u_worldToProj", worldToProj );
		for( SDFToMesh* sdf : sdfToMesh )
		{
			sdf->Draw( &shader, uniforms, &debugLines );
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
// IsosurfaceExtractorCache member functions
//------------------------------------------------------------------------------
IsosurfaceExtractorCache::IsosurfaceExtractorCache()
{
	Reset();
}

void IsosurfaceExtractorCache::Reset()
{
	m_surfaceAABB = ae::AABB( ae::Vec3( 0.0f ), ae::Vec3( 0.0f ) ); // 0 size by default, the position shouldn't matter
	m_estimatedVertexCount = 0.0f;
	
	const uint32_t zoneCount = m_zones.Length();
	for ( uint32_t i = 0; i < zoneCount; i++ )
	{
		ae::Delete( m_zones.GetValue( i ) );
	}
	m_zones.Clear();
}

void IsosurfaceExtractorCache::Generate( const IsosurfaceParams& params )
{
	Reset();
	if( params.aabb.Contains( params.aabb.GetCenter() ) )
	{
		m_params = params;
		m_surfaceAABB = ae::AABB(); // Default (and not 0 size!) so that ae::AABB::Expand() functions work as expected
		m_Generate( m_params.aabb.GetCenter(), params.aabb.GetHalfSize() );
		if( m_surfaceAABB == ae::AABB() )
		{
			Reset(); // Reset octree and aabb so an empty SDF is as simple as possible
		}
	}
}

void IsosurfaceExtractorCache::m_Generate( ae::Vec3 center, ae::Vec3 halfSize )
{
	const float signedSurfaceDistance = m_params.fn( center, m_params.userData );
	const float diagonal = halfSize.Length();
	if( diagonal <= ae::Abs( signedSurfaceDistance ) )
	{
		return; // No intersection, no need to split further
	}
	
	const ae::Vec3 nextHalfSize = halfSize * 0.5f;
	constexpr float minOctantHalfSize = 0.55f;
	if( nextHalfSize.x > minOctantHalfSize &&
		nextHalfSize.y > minOctantHalfSize &&
		nextHalfSize.z > minOctantHalfSize ) // Only split if next octant is large enough
	{
		for( uint32_t i = 0; i < 8; i++ )
		{
			m_Generate(
				center + kChildOffsets[ i ] * nextHalfSize,
				nextHalfSize
			);
		}
	}
	else
	{
		const float leafMult = 1.0f;
		const ae::Vec3 halfSize3( halfSize );
		const ae::AABB leafAABB = ae::AABB( center - halfSize3, center + halfSize3 );
		const ae::Vec3 leafSize = ( leafAABB.GetMax() - leafAABB.GetMin() );
		const ae::Int3 leafGridMin = ( leafAABB.GetMin() / leafMult ).FloorCopy();
		const ae::Int3 leafGridMax = ( leafAABB.GetMax() / leafMult ).CeilCopy();
		
		m_surfaceAABB.Expand( leafAABB );
		
		// @TODO: Bounded region Set() function
		const ae::Int3 zoneSize = Zone::GetSize();
		for( int32_t z = leafGridMin.z; z <= leafGridMax.z; z++ )
		for( int32_t y = leafGridMin.y; y <= leafGridMax.y; y++ )
		for( int32_t x = leafGridMin.x; x <= leafGridMax.x; x++ )
		{
			const ae::Int3 pos( x, y, z );
			const ae::Int3 slot = ae::Int3(
				ae::Floor( (float)pos.x / zoneSize.x ),
				ae::Floor( (float)pos.y / zoneSize.y ),
				ae::Floor( (float)pos.z / zoneSize.z )
			);
			const ae::Int3 localPos = ae::Int3(
				ae::Mod( pos.x, zoneSize.x ),
				ae::Mod( pos.y, zoneSize.y ),
				ae::Mod( pos.z, zoneSize.z )
			);
			Zone* zone = nullptr;
			if ( !m_zones.TryGet( slot, &zone ) )
			{
				zone = ae::New< Zone >( AE_ALLOC_TAG_FIXME );
				zone->SetOffset( slot * zoneSize );
				m_zones.Set( slot, zone );
			}
			zone->Set( localPos, 1 );
		}
		
		m_estimatedVertexCount += ( leafSize.x * leafSize.y * leafSize.z ) * 0.4f; // @HACK: What should this actually be?
	}
}

float IsosurfaceExtractorCache::GetValue( ae::Vec3 position ) const
{
	return m_params.fn( position, m_params.userData );
}

ae::Vec3 IsosurfaceExtractorCache::GetDerivative( ae::Vec3 p ) const
{
	// @TODO: GetValues( ae::Array< ae::Vec3, N > pos, ae::Array< float, N >& resultsOut );
	ae::Vec3 pv( GetValue( p ) );
	AE_DEBUG_IF( pv != pv ) { return ae::Vec3( 0.0f ); }
	
	ae::Vec3 normal0;
	for( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] += m_params.normalSampleOffset;
		normal0[ i ] = GetValue( nt );
	}
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal0 -= pv;
	AE_DEBUG_IF( normal0 == ae::Vec3( 0.0f ) ) { return ae::Vec3( 0.0f ); }
	normal0 /= normal0.Length();
	AE_DEBUG_IF( normal0 != normal0 ) { return ae::Vec3( 0.0f ); }

	ae::Vec3 normal1;
	for( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] -= m_params.normalSampleOffset;
		normal1[ i ] = GetValue( nt );
	}
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal1 = pv - normal1;
	AE_DEBUG_IF( normal1 == ae::Vec3( 0.0f ) ) { return ae::Vec3( 0.0f ); }
	normal1 /= normal1.Length();
	AE_DEBUG_IF( normal1 != normal1 ) { return ae::Vec3( 0.0f ); }

	return ( normal1 + normal0 ).SafeNormalizeCopy();
}

//------------------------------------------------------------------------------
// IsosurfaceExtractor member functions
//------------------------------------------------------------------------------
IsosurfaceExtractor::IsosurfaceExtractor( ae::Tag tag ) :
	vertices( tag ),
	indices( tag ),
	m_voxels( tag )
{}

void IsosurfaceExtractor::Generate( const IsosurfaceParams& params, uint32_t maxVerts, uint32_t maxIndices, ae::Array< ae::Vec3 >* errors )
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
	m_voxels.Clear();
	m_stats = {};
	m_sdf.Generate( params );
	m_stats.estimatedVertexCount = m_sdf.GetEstimatedVertexCount();
	vertices.Reserve( m_sdf.GetEstimatedVertexCount() );
	// This multiplier is very well established, the average indexed mesh
	// has 6 indices per vertex.
	indices.Reserve( m_sdf.GetEstimatedVertexCount() * 6 );
	// This multiplier can make a huge difference, if it's too large then
	// too big of a map will be allocated which will be expensive to access
	// and if it's too small then the map will need to be reallocated during
	// generation.
	m_voxels.Reserve( m_sdf.GetEstimatedVertexCount() * 3.25f );

	// @TODO: Description
	const uint16_t EDGE_TOP_FRONT_BIT = ( 1 << 0 );
	const uint16_t EDGE_TOP_RIGHT_BIT = ( 1 << 1 );
	const uint16_t EDGE_SIDE_FRONTRIGHT_BIT = ( 1 << 2 );
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
	
	const ae::Vec3 generationOffset = m_sdf.GetParams().aabb.GetMin();
	const ae::Int3 cornerOffsetInt = generationOffset.FloorCopy();
	const ae::Int3 generationMin( -1 );
	const ae::Int3 generationMax = m_sdf.GetParams().aabb.GetMax().CeilCopy() + ae::Int3( 1 ) - cornerOffsetInt;
	const ae::Int3 sdfMin = ae::Clip( m_sdf.GetOctreeAABB().GetMin().FloorCopy() - cornerOffsetInt, generationMin, generationMax );
	const ae::Int3 sdfMax = ae::Clip( m_sdf.GetOctreeAABB().GetMax().CeilCopy() - cornerOffsetInt, generationMin, generationMax );
	AE_DEBUG_ASSERT( ( sdfMax.x - sdfMin.x ) >= 0 );
	AE_DEBUG_ASSERT( ( sdfMax.y - sdfMin.y ) >= 0 );
	AE_DEBUG_ASSERT( ( sdfMax.z - sdfMin.z ) >= 0 );
	const uint32_t sdfSize = ( sdfMax.x - sdfMin.x ) * ( sdfMax.y - sdfMin.y ) * ( sdfMax.z - sdfMin.z );
	if( !sdfSize )
	{
		return; // Zero size generation region
	}
	// This phase finds the surface of the SDF and generates the list of
	// vertices along with all of the 'lattice' edge intersections. The vertex
	// positions will be centered within their voxels at the end of this phase,
	// and will be nudged later to the correct position based on the SDF
	// surface.
	auto DoVoxel = [&]( int32_t x, int32_t y, int32_t z )
	{
		const ae::Vec3 voxelPos( x, y, z );
		// This nudge is needed to prevent the SDF from ever being exactly on
		// the voxel grid boundaries (imagine a plane at the origin with a
		// normal facing along a cardinal axis, do the vertices belong to the
		// voxels on the front or back of the plane?). Without this nudge, any
		// vertices exactly on the grid boundary would be skipped resulting in
		// holes in the mesh.
		auto Nudge = []( float v ) { return ( v == 0.0f ) ? 0.0001f : v; };
		const ae::Vec3 sharedCornerOffset( 1.0f );
		const float sharedCornerValue = Nudge( m_sdf.GetValue( generationOffset + voxelPos + sharedCornerOffset ) );
		m_stats.sampleCount++;
		if( ae::Abs( sharedCornerValue ) > 2.0f ) // @TODO: This value could be smaller
		{
			// Early out of additional edge intersections if far from the surface
			return true;
		}
		const float cornerValues[ 3 ] =
		{
			Nudge( m_sdf.GetValue( generationOffset + voxelPos + cornerOffsets[ 0 ] ) ),
			Nudge( m_sdf.GetValue( generationOffset + voxelPos + cornerOffsets[ 1 ] ) ),
			Nudge( m_sdf.GetValue( generationOffset + voxelPos + cornerOffsets[ 2 ] ) )
		};
		m_stats.sampleCount += 3;
		AE_DEBUG_IF( !errors )
		{
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) );
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) );
			AE_DEBUG_ASSERT_MSG( ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) <= 1.01f, "A valid signed distance function is required. The distance detected between two adjacent voxels can't be '#'", ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) );
		}
		else AE_DEBUG_IF( ae::Abs( cornerValues[ 0 ] - sharedCornerValue ) > 1.01f
			|| ae::Abs( cornerValues[ 1 ] - sharedCornerValue ) > 1.01f
			|| ae::Abs( cornerValues[ 2 ] - sharedCornerValue ) > 1.01f )
		{
			errors->Append( generationOffset + voxelPos );
			return true;
		}
		
		Voxel voxel;
		// Detect if any of the 3 new edges being tested intersect the implicit surface
		if( cornerValues[ 0 ] * sharedCornerValue < 0.0f ) { voxel.edgeBits |= EDGE_TOP_FRONT_BIT; }
		if( cornerValues[ 1 ] * sharedCornerValue < 0.0f ) { voxel.edgeBits |= EDGE_TOP_RIGHT_BIT; }
		if( cornerValues[ 2 ] * sharedCornerValue < 0.0f ) { voxel.edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT; }
		
		// Iterate over the 3 edges that this voxel is responsible for. The
		// remaining 9 are handled by adjacent voxels.
		for( int32_t e = 0; e < 3; e++ )
		if( voxel.edgeBits & mask[ e ] )
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
					const float closestSurfaceDist = m_sdf.GetValue( generationOffset + voxelPos + edgeOffset01 );
					m_stats.sampleCount++;
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
			
			voxel.edgePos[ e ] = edgeOffset01;
			voxel.edgeNormal[ e ] = m_sdf.GetDerivative( generationOffset + voxelPos + edgeOffset01 );
			m_stats.sampleCount += 7;
			AE_DEBUG_IF( errors && voxel.edgeNormal[ e ] == ae::Vec3( 0.0f ) ) { errors->Append( generationOffset + voxelPos + edgeOffset01 ); }

			// Don't allow verts to be added on the very edge of the generation
			// area. A border of at least 1 voxel is needed so that vertices can
			// be positioned, since voxel edges are stored in neighbors to
			// avoid duplication.
			if( x >= sdfMax.x || y >= sdfMax.y || z >= sdfMax.z )
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

				Voxel* quadVoxel = m_voxels.TryGet( { ox, oy, oz } );
				quadVoxel = quadVoxel ? quadVoxel : &m_voxels.Set( { ox, oy, oz }, {} );
				IsosurfaceIndex* vertexIndex = &quadVoxel->index;
				if( *vertexIndex == kInvalidIsosurfaceIndex )
				{
					IsosurfaceVertex vertex;
					vertex.position.x = ox + 0.5f;
					vertex.position.y = oy + 0.5f;
					vertex.position.z = oz + 0.5f;
					vertex.position.w = 1.0f;
					AE_DEBUG_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );
					
					// Record the index of the vertex in the chunk so it can
					// be reused by adjacent quads
					*vertexIndex = (IsosurfaceIndex)vertices.Length();
					vertices.Append( vertex );
				}
				AE_DEBUG_ASSERT_MSG( *vertexIndex < (IsosurfaceIndex)vertices.Length(), "# < # ox:# oy:# oz:#", index, vertices.Length(), ox, oy, oz );
				quad[ j ] = *vertexIndex;
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
		m_voxels.Set( { x + 1, y + 1, z + 1 }, voxel );
		return true;
	};
	const uint32_t zoneCount = m_sdf.GetZoneCount();
	for( uint32_t i = 0; i < zoneCount; i++ )
	{
		const IsosurfaceExtractorCache::Zone* zone = m_sdf.GetZone( i );
		const ae::Int3 zoneOffset = zone->GetOffset() - cornerOffsetInt;
		const ae::Int3 zoneMin = ae::Clip( zoneOffset, sdfMin, sdfMax );
		const ae::Int3 zoneMax = ae::Clip( zoneOffset + zone->GetSize(), sdfMin, sdfMax );
		const ae::AABB zoneAABB( (ae::Vec3)zoneMin, (ae::Vec3)zoneMax );
		for( int32_t z = zoneMin.z; z < zoneMax.z; z++ )
		for( int32_t y = zoneMin.y; y < zoneMax.y; y++ )
		for( int32_t x = zoneMin.x; x < zoneMax.x; x++ )
		{
			const ae::Int3 zonePos = ae::Int3( x, y, z ) - zoneOffset;
			if( zone->Get( zonePos ) && !DoVoxel( x, y, z ) )
			{
				vertices.Clear();
				indices.Clear();
				return;
			}
		}
		m_stats.iterationCount += ( zoneMax.x - zoneMin.x ) * ( zoneMax.y - zoneMin.y ) * ( zoneMax.z - zoneMin.z );
	}
	m_stats.workingCount = m_voxels.Length();
	
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
		AE_DEBUG_ASSERT( x >= sdfMin.x && sdfMin.y >= 0 && sdfMin.z >= 0 );
		AE_DEBUG_ASSERT( x <= sdfMax.x && y <= sdfMax.y && z <= sdfMax.z );
		
		int32_t ec = 0;
		ae::Vec3 p[ 12 ];
		ae::Vec3 n[ 12 ];
		Voxel te = m_voxels.Get( { x + 1, y + 1, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y + 1, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y, z + 1 }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y, z + 1 }, {} );
		if( te.edgeBits & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 2 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.edgeNormal[ 2 ];
			ec++;
		}
		te = m_voxels.Get( { x, y + 1, z }, {} );
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y, z }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].y -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		te = m_voxels.Get( { x + 1, y + 1, z }, {} );
		if( te.edgeBits & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.edgePos[ 0 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 0 ];
			ec++;
		}
		if( te.edgeBits & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.edgePos[ 1 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.edgeNormal[ 1 ];
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
