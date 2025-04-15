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
#include "aether.h"

//------------------------------------------------------------------------------
// Types / constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ISOSURFACE = "isosurface";

struct IsosurfaceVertex
{
	ae::Vec4 position;
	ae::Vec3 normal;
};
struct IsosurfaceParams
{
	ae::DebugLines* debug = nullptr;
	float normalSampleOffset = 0.25f;
	float smoothingAmount = 0.05f; // @TODO: This does nothing yet
};
struct TempEdges
{
	int32_t x;
	int32_t y;
	int32_t z;
	uint16_t b;

	// 3 planes whose intersections are used to position vertices within voxel
	// EDGE_TOP_FRONT_BIT, EDGE_TOP_RIGHT_BIT, EDGE_SIDE_FRONTRIGHT_BIT
	ae::Vec3 p[ 3 ];
	ae::Vec3 n[ 3 ];
};
struct Block
{
	enum Type : uint8_t
	{
		Exterior,
		Interior,
		Surface,
		Blocking,
		Unloaded,
		COUNT
	};
};
const uint16_t EDGE_TOP_FRONT_INDEX = 0;
const uint16_t EDGE_TOP_RIGHT_INDEX = 1;
const uint16_t EDGE_TOP_BACK_INDEX = 2;
const uint16_t EDGE_TOP_LEFT_INDEX = 3;
const uint16_t EDGE_SIDE_FRONTLEFT_INDEX = 4;
const uint16_t EDGE_SIDE_FRONTRIGHT_INDEX = 5;
const uint16_t EDGE_SIDE_BACKRIGHT_INDEX = 6;
const uint16_t EDGE_SIDE_BACKLEFT_INDEX = 7;
const uint16_t EDGE_BOTTOM_FRONT_INDEX = 8;
const uint16_t EDGE_BOTTOM_RIGHT_INDEX = 9;
const uint16_t EDGE_BOTTOM_BACK_INDEX = 10;
const uint16_t EDGE_BOTTOM_LEFT_INDEX = 11;
const uint16_t EDGE_TOP_FRONT_BIT = 1 << EDGE_TOP_FRONT_INDEX;
const uint16_t EDGE_TOP_RIGHT_BIT = 1 << EDGE_TOP_RIGHT_INDEX;
const uint16_t EDGE_TOP_BACK_BIT = 1 << EDGE_TOP_BACK_INDEX;
const uint16_t EDGE_TOP_LEFT_BIT = 1 << EDGE_TOP_LEFT_INDEX;
const uint16_t EDGE_SIDE_FRONTLEFT_BIT = 1 << EDGE_SIDE_FRONTLEFT_INDEX;
const uint16_t EDGE_SIDE_FRONTRIGHT_BIT = 1 << EDGE_SIDE_FRONTRIGHT_INDEX;
const uint16_t EDGE_SIDE_BACKRIGHT_BIT = 1 << EDGE_SIDE_BACKRIGHT_INDEX;
const uint16_t EDGE_SIDE_BACKLEFT_BIT = 1 << EDGE_SIDE_BACKLEFT_INDEX;
const uint16_t EDGE_BOTTOM_FRONT_BIT = 1 << EDGE_BOTTOM_FRONT_INDEX;
const uint16_t EDGE_BOTTOM_RIGHT_BIT = 1 << EDGE_BOTTOM_RIGHT_INDEX;
const uint16_t EDGE_BOTTOM_BACK_BIT = 1 << EDGE_BOTTOM_BACK_INDEX;
const uint16_t EDGE_BOTTOM_LEFT_BIT = 1 << EDGE_BOTTOM_LEFT_INDEX;

//------------------------------------------------------------------------------
// aeUnit
//------------------------------------------------------------------------------
template < typename T >
class aeUnit
{
public:
	constexpr aeUnit() : m_v() { AE_STATIC_ASSERT( sizeof(*this) == sizeof(T) ); }
	constexpr aeUnit( const aeUnit& o ) : m_v( o.m_v ) {}
	constexpr explicit aeUnit( const T& vertexCount ) : m_v( vertexCount ) {}

	template < typename U > constexpr explicit operator U () const { return (U)m_v; }

	constexpr bool operator == ( const aeUnit& o ) const { return m_v == o.m_v; }
	constexpr bool operator != ( const aeUnit& o ) const { return m_v != o.m_v; }
	constexpr bool operator < ( const aeUnit& o ) const { return m_v < o.m_v; }
	constexpr bool operator > ( const aeUnit& o ) const { return m_v > o.m_v; }
	constexpr bool operator <= ( const aeUnit& o ) const { return m_v <= o.m_v; }
	constexpr bool operator >= ( const aeUnit& o ) const { return m_v >= o.m_v; }
	
	constexpr aeUnit< T > operator + ( const aeUnit& v ) const { return aeUnit( m_v + v.m_v ); }
	constexpr aeUnit< T > operator - ( const aeUnit& v ) const { return aeUnit( m_v - v.m_v ); }
	constexpr aeUnit< T >& operator ++ () { m_v++; return *this; }
	constexpr aeUnit< T >& operator -- () { m_v--; return *this; }
	constexpr aeUnit< T > operator ++ ( int ) { aeUnit< T > temp = *this; ++*this; return temp; }
	constexpr aeUnit< T > operator -- ( int ) { aeUnit< T > temp = *this; --*this; return temp; }
	constexpr aeUnit< T >& operator += ( const T& v ) { m_v += v; return *this; }
	constexpr aeUnit< T >& operator -= ( const T& v ) { m_v -= v; return *this; }
	
	constexpr T& Get() { return m_v; }
	constexpr T Get() const { return m_v; }

private:
	operator bool () const = delete;
	T m_v;
};

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const aeUnit< T >& u )
{
	return os << (T)u;
}
typedef aeUnit< uint32_t > VertexCount;

typedef uint32_t IsosurfaceIndex;
const IsosurfaceIndex kInvalidIsosurfaceIndex = ~0;
const uint32_t kChunkSize = 20; // @NOTE: This can't be too high or kMaxChunkVerts will be hit
const int32_t kTempChunkSize = kChunkSize + 2; // Include a 1 voxel border
const int32_t kTempChunkSize3 = kTempChunkSize * kTempChunkSize * kTempChunkSize; // Temp voxel count
const VertexCount kChunkCountEmpty = VertexCount( 0 );

//------------------------------------------------------------------------------
// IsosurfaceExtractorCache class
//------------------------------------------------------------------------------
class IsosurfaceExtractorCache
{
public:
	IsosurfaceExtractorCache();
	~IsosurfaceExtractorCache();

	template < typename Fn > void Generate( const IsosurfaceParams& params, ae::Int3 offset, Fn fn );
	float GetValue( ae::Vec3 pos ) const;
	float GetValue( ae::Int3 pos ) const;
	ae::Vec3 GetDerivative( ae::Vec3 p ) const;
	ae::Vec3 GetOffset() const { return m_offset; }

private:
	float m_GetValue( ae::Int3 pos ) const;

	ae::Vec3 m_offset;

	const int32_t kDim = kChunkSize + 5; // TODO: What should this value actually be? Corresponds to 'chunkPlus'
	static const int32_t kOffset = 2;
	IsosurfaceParams m_p;

	// @TODO: Replace this with aeStaticImage3D
	float* m_values;
};

//------------------------------------------------------------------------------
// IsosurfaceExtractor class
//------------------------------------------------------------------------------
struct IsosurfaceExtractor
{
	void Generate( const IsosurfaceExtractorCache* sdf, IsosurfaceVertex* verticesOut, IsosurfaceIndex* indexOut, VertexCount* vertexCountOut, uint32_t* indexCountOut, uint32_t maxVerts, uint32_t maxIndices );
	void m_SetVertexData( const IsosurfaceVertex* verts, const IsosurfaceIndex* indices, VertexCount vertexCount, uint32_t indexCount );

	ae::VertexBuffer m_data; // @TODO: Remove
	
	TempEdges m_tempEdges[ kTempChunkSize3 ];
	IsosurfaceIndex m_i[ kChunkSize ][ kChunkSize ][ kChunkSize ];

private:
	static void m_GetQuadVertexOffsetsFromEdge( uint32_t edgeBit, int32_t( &offsets )[ 4 ][ 3 ] );
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
	if ( halfSize.x > halfSize.y )
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
	// window.Initialize( 1280, 720, false, true, true );
	{
		ae::Int2 windowPos( 0, 0 );
		ae::Int2 windowSize( 1280, 720 );
		const auto screens = ae::GetScreens();
		auto GetSmallestScreen = [&]() -> const ae::Screen*
		{
			const ae::Screen* result = nullptr;
			for( const ae::Screen& screen : screens )
			{
				if( !result || screen.size.x * screen.scaleFactor <= result->size.x * result->scaleFactor )
				{
					result = &screen;
				}
			}
			return result;
		};
		if ( screens.Length() )
		{
			const ae::Screen* targetScreen = ae::IsDebuggerAttached() ? GetSmallestScreen() : &screens[ 0 ];
			windowPos = targetScreen->position + targetScreen->size / 2;
			windowPos -= windowSize / 2;
		}
		window.Initialize( windowPos, windowSize.x, windowSize.y, true, false );
		if( ae::IsDebuggerAttached() )
		{
			if( screens.Length() >= 2 )
			{
				window.SetFullScreen( true );
			}
			else if( screens.Length() == 1 )
			{
				window.SetAlwaysOnTop( true );
			}
		}
	}
	window.SetTitle( "SDF to Mesh" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 16384 );
	debugLines.SetXRayEnabled( false );

	ae::Shader shader;
	shader.Initialize( kVertShader, kFragShader );
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
	bool wireframe = true;
	auto ToggleWireframe = [&]()
	{
		wireframe = !wireframe;
		shader.SetWireframe( wireframe );
	};
	ToggleWireframe();

	const uint32_t kMaxVerts = 5000; // @TODO: Dynamic array
	const uint32_t kMaxIndices = 10000;
	class SDFToMesh
	{
	public:
		ae::Str32 name;
		ae::Color color;
		SDFToMesh( const char* name, ae::Color color ) :
			name( name ),
			color( color )
		{}

		ae::Int3 offset = ae::Int3( 0 );
		VertexCount vertexCount = VertexCount( 0 );
		uint32_t indexCount = 0;
		IsosurfaceVertex* vertices = ae::NewArray< IsosurfaceVertex >( TAG_ISOSURFACE, kMaxVerts );
		IsosurfaceIndex* indices = ae::NewArray< IsosurfaceIndex >( TAG_ISOSURFACE, kMaxIndices );
		IsosurfaceExtractor* extractor = ae::New< IsosurfaceExtractor >( TAG_ISOSURFACE );
		IsosurfaceExtractorCache* cache = ae::New< IsosurfaceExtractorCache >( TAG_ISOSURFACE );
		void Run( ae::Matrix4 transform )
		{
			// Cache
			AE_INFO( "[#] Caching SDF...", name );
			const double cacheStart = ae::GetTime();
			const ae::Vec3 scale = transform.GetScale();
			transform = transform.GetScaleRemoved();
			const ae::Matrix4 inverseTransform = transform.GetInverse();
			cache->Generate( {}, offset, [&inverseTransform, &scale]( ae::Vec3 _p )
			{
				const float smooth = 2.0f; // World space because scale is applied separately from transform
				const ae::Vec3 p = inverseTransform.TransformPoint3x4( _p );
				float r = SDFBox( p, ae::Vec3( 0.5f, 0.25f, 0.25f ) * scale, smooth );
				r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.5f, 0.25f ) * scale, smooth ), smooth );
				r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.25f, 0.5f ) * scale, smooth ), smooth );
				return r;
				// return ( p - ae::Vec3( 0.7f ) ).Length() - 4.0f;
			} );
			const double cacheEnd = ae::GetTime();
			AE_INFO( "[#] SDF cache complete. sec:#", name, cacheEnd - cacheStart );
			
			// Mesh
			AE_INFO( "[#] Start mesh generation...", name );
			const double isosurfaceStart = ae::GetTime();
			extractor->Generate(
				cache,
				vertices,
				indices,
				&vertexCount,
				&indexCount,
				kMaxVerts,
				kMaxIndices
			);
			const double isosurfaceEnd = ae::GetTime();
			AE_INFO( "[#] Mesh generation complete. sec:# verts:# indices:# [#]",
				name,
				isosurfaceEnd - isosurfaceStart,
				vertexCount.Get(),
				indexCount,
				"@TODO:aabb" );
			if ( vertexCount == kChunkCountEmpty )
			{
				AE_INFO( "[#] No mesh generated", name );
				return;
			}
			extractor->m_SetVertexData( vertices, indices, vertexCount, indexCount );
		}

		bool show = true;
		void Draw( ae::Shader* shader, ae::UniformList& uniforms )
		{
			if ( !show || vertexCount == kChunkCountEmpty )
			{
				return;
			}
			uniforms.Set( "u_color", color.GetLinearRGB() );
			extractor->m_data.Bind( shader, uniforms );
			extractor->m_data.Draw( 0, indexCount / 3 );
		}

		~SDFToMesh()
		{
			ae::Delete( indices );
			ae::Delete( vertices );
			ae::Delete( cache );
			ae::Delete( extractor );
		}
	};
	SDFToMesh sdfToMesh[] =
	{
		{ "Mesh0", ae::Color::HSV(0.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh1", ae::Color::HSV(1.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh2", ae::Color::HSV(2.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh3", ae::Color::HSV(3.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh4", ae::Color::HSV(4.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh5", ae::Color::HSV(5.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh6", ae::Color::HSV(6.0f / 8.0f, 0.7f, 0.5f ) },
		{ "Mesh7", ae::Color::HSV(7.0f / 8.0f, 0.7f, 0.5f ) },
	};
	sdfToMesh[ 0 ].offset = ae::Int3( -1, -1, -1 ) * kChunkSize;
	sdfToMesh[ 1 ].offset = ae::Int3( -1, -1, 0 ) * kChunkSize;
	sdfToMesh[ 2 ].offset = ae::Int3( -1,0, -1 ) * kChunkSize;
	sdfToMesh[ 3 ].offset = ae::Int3( -1,0,0 ) * kChunkSize;
	sdfToMesh[ 4 ].offset = ae::Int3(0, -1, -1 ) * kChunkSize;
	sdfToMesh[ 5 ].offset = ae::Int3(0, -1,0 ) * kChunkSize;
	sdfToMesh[ 6 ].offset = ae::Int3(0,0, -1 ) * kChunkSize;
	sdfToMesh[ 7 ].offset = ae::Int3(0,0,0 ) * kChunkSize;

	ae::Vec3 translation;
	ae::Vec3 rotation;
	ae::Vec3 scale;
	ae::Matrix4 prevTransform = ae::Matrix4::Scaling( 0.0f ); // Different than transform on frame 1
	auto ResetTransform = [&]()
	{
		translation = ae::Vec3( 0.0f );
		rotation = ae::Vec3( 0.0f );
		scale = ae::Vec3( 20.0f );
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
		// Reset
		if( input.GetPress( ae::Key::R ) ) { ResetTransform(); }
		// Translation
		const float transformSpeed = 2.0f * ( scale.x + scale.y + scale.z ) / 3.0f;
		if ( input.Get( ae::Key::D ) ) { translation.x += transformSpeed * dt; }
		if ( input.Get( ae::Key::A ) ) { translation.x -= transformSpeed * dt; }
		if ( input.Get( ae::Key::W ) ) { translation.y += transformSpeed * dt; }
		if ( input.Get( ae::Key::S ) ) { translation.y -= transformSpeed * dt; }
		if ( input.Get( ae::Key::E ) ) { translation.z += transformSpeed * dt; }
		if ( input.Get( ae::Key::Q ) ) { translation.z -= transformSpeed * dt; }
		// Rotation
		if ( input.Get( ae::Key::Z ) ) { rotation.x += 1.0f * dt; }
		if ( input.Get( ae::Key::X ) ) { rotation.y += 1.0f * dt; }
		if ( input.Get( ae::Key::C ) ) { rotation.z += 1.0f * dt; }
		// Scale
		if ( input.Get( ae::Key::L ) ) { scale.x += transformSpeed * dt; }
		if ( input.Get( ae::Key::J ) ) { scale.x -= transformSpeed * dt; }
		if ( input.Get( ae::Key::I ) ) { scale.y += transformSpeed * dt; }
		if ( input.Get( ae::Key::K ) ) { scale.y -= transformSpeed * dt; }
		if ( input.Get( ae::Key::O ) ) { scale.z += transformSpeed * dt; }
		if ( input.Get( ae::Key::U ) ) { scale.z -= transformSpeed * dt; }
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
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		debugLines.AddAABB( ae::Vec3( 0.0f ), ae::Vec3( kChunkSize ), ae::Color::AetherWhite() );
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
			sdf.Draw( &shader, uniforms );
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
template < typename Fn >
void IsosurfaceExtractorCache::Generate( const IsosurfaceParams& params, ae::Int3 offset, Fn fn )
{
	m_offset = ae::Vec3( offset );
	m_p = params;

	ae::Int3 offset2 = offset - ae::Int3( kOffset );
	for ( int32_t z = 0; z < kDim; z++ )
	for ( int32_t y = 0; y < kDim; y++ )
	for ( int32_t x = 0; x < kDim; x++ )
	{
		uint32_t index = x + kDim * ( y + kDim * z );
		ae::Vec3 pos( offset2.x + x, offset2.y + y, offset2.z + z );
		m_values[ index ] = (float)fn( pos );
	}
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
ae::Vec3 GetIntersection( const ae::Vec3* p, const ae::Vec3* n, uint32_t ic )
{
#if AE_TERRAIN_SIMD
	__m128 c128 = _mm_setzero_ps();
	for ( uint32_t i = 0; i < ic; i++ )
	{
		__m128 p128 = _mm_load_ps( (float*)( p + i ) );
		c128 = _mm_add_ps( c128, p128 );
	}
	__m128 div = _mm_set1_ps( 1.0f / ic );
	c128 = _mm_mul_ps( c128, div );
	
	for ( uint32_t i = 0; i < 10; i++ )
	for ( uint32_t j = 0; j < ic; j++ )
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
	ae::Vec3 v;
	_mm_store_ps( (float*)&v, c128 );
	return v;
#else
	ae::Vec3 c( 0.0f );
	for ( uint32_t i = 0; i < ic; i++ )
	{
		c += p[ i ];
	}
	c /= ic;

	for ( uint32_t i = 0; i < 10; i++ )
	{
		for ( uint32_t j = 0; j < ic; j++ )
		{
			float d = n[ j ].Dot( p[ j ] - c );
			c += n[ j ] * ( d * 0.5f );
		}
	}

	return c;
#endif
}

//------------------------------------------------------------------------------
// IsosurfaceExtractorCache member functions
//------------------------------------------------------------------------------
IsosurfaceExtractorCache::IsosurfaceExtractorCache()
{
	m_values = ae::NewArray< float >( AE_ALLOC_TAG_TERRAIN, kDim * kDim * kDim );
}

IsosurfaceExtractorCache::~IsosurfaceExtractorCache()
{
	ae::Delete( m_values );
	m_values = nullptr;
}

float IsosurfaceExtractorCache::GetValue( ae::Vec3 pos ) const
{
	pos += ae::Vec3( kOffset );

	const ae::Int3 cornerPos = pos.FloorCopy();
	pos.x -= cornerPos.x;
	pos.y -= cornerPos.y;
	pos.z -= cornerPos.z;

	float values[ 8 ] =
	{
		m_GetValue( cornerPos ),
		m_GetValue( cornerPos + ae::Int3( 1, 0, 0 ) ),
		m_GetValue( cornerPos + ae::Int3( 0, 1, 0 ) ),
		m_GetValue( cornerPos + ae::Int3( 1, 1, 0 ) ),
		m_GetValue( cornerPos + ae::Int3( 0, 0, 1 ) ),
		m_GetValue( cornerPos + ae::Int3( 1, 0, 1 ) ),
		m_GetValue( cornerPos + ae::Int3( 0, 1, 1 ) ),
		m_GetValue( cornerPos + ae::Int3( 1, 1, 1 ) ),
	};

	float x0 = ae::Lerp( values[ 0 ], values[ 1 ], pos.x );
	float x1 = ae::Lerp( values[ 2 ], values[ 3 ], pos.x );
	float x2 = ae::Lerp( values[ 4 ], values[ 5 ], pos.x );
	float x3 = ae::Lerp( values[ 6 ], values[ 7 ], pos.x );
	float y0 = ae::Lerp( x0, x1, pos.y );
	float y1 = ae::Lerp( x2, x3, pos.y );
	return ae::Lerp( y0, y1, pos.z );
}

float IsosurfaceExtractorCache::GetValue( ae::Int3 pos ) const
{
	return m_GetValue( pos + ae::Int3( kOffset ) );
}

float IsosurfaceExtractorCache::m_GetValue( ae::Int3 pos ) const
{
#if _AE_DEBUG_
	AE_ASSERT( pos.x >= 0 && pos.y >= 0 && pos.z >= 0 );
	AE_ASSERT( pos.x < kDim && pos.y < kDim && pos.z < kDim );
#endif
	return m_values[ pos.x + kDim * ( pos.y + kDim * pos.z ) ];
}

ae::Vec3 IsosurfaceExtractorCache::GetDerivative( ae::Vec3 p ) const
{
	ae::Vec3 pv( GetValue( p ) );
	
	ae::Vec3 normal0;
	for ( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] += m_p.normalSampleOffset;
		normal0[ i ] = GetValue( nt );
	}
	AE_ASSERT( normal0 != ae::Vec3( 0.0f ) );
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal0 -= pv;
	AE_ASSERT( normal0 != ae::Vec3( 0.0f ) );
	normal0 /= normal0.Length();
	AE_ASSERT( normal0 == normal0 );

	ae::Vec3 normal1;
	for ( int32_t i = 0; i < 3; i++ )
	{
		ae::Vec3 nt = p;
		nt[ i ] -= m_p.normalSampleOffset;
		normal1[ i ] = GetValue( nt );
	}
	AE_ASSERT( normal1 != ae::Vec3( 0.0f ) );
	// This should be really close to 0 because it's really
	// close to the surface but not close enough to ignore.
	normal1 = pv - normal1;
	AE_ASSERT( normal1 != ae::Vec3( 0.0f ) );
	normal1 /= normal1.Length();
	AE_ASSERT( normal1 == normal1 );

	return ( normal1 + normal0 ).SafeNormalizeCopy();
}

//------------------------------------------------------------------------------
// IsosurfaceExtractor member functions
//------------------------------------------------------------------------------
void IsosurfaceExtractor::Generate( const IsosurfaceExtractorCache* sdf, IsosurfaceVertex* verticesOut, IsosurfaceIndex* indexOut, VertexCount* vertexCountOut, uint32_t* indexCountOut, uint32_t _maxVerts, uint32_t maxIndices )
{
	const VertexCount maxVerts( _maxVerts );
	VertexCount vertexCount = VertexCount( 0 );
	uint32_t indexCount = 0;

	memset( m_i, ~(uint8_t)0, sizeof( m_i ) );
	memset( m_tempEdges, 0, kTempChunkSize3 * sizeof( *m_tempEdges ) );
	
	uint16_t mask[ 3 ];
	mask[ 0 ] = EDGE_TOP_FRONT_BIT;
	mask[ 1 ] = EDGE_TOP_RIGHT_BIT;
	mask[ 2 ] = EDGE_SIDE_FRONTRIGHT_BIT;
	
	// 3 new edges to test
	ae::Vec3 cornerOffsets[ 3 ][ 2 ];
	// EDGE_TOP_FRONT_BIT
	cornerOffsets[ 0 ][ 0 ] = ae::Vec3( 0, 1, 1 );
	cornerOffsets[ 0 ][ 1 ] = ae::Vec3( 1, 1, 1 );
	// EDGE_TOP_RIGHT_BIT
	cornerOffsets[ 1 ][ 0 ] = ae::Vec3( 1, 0, 1 );
	cornerOffsets[ 1 ][ 1 ] = ae::Vec3( 1, 1, 1 );
	// EDGE_SIDE_FRONTRIGHT_BIT
	cornerOffsets[ 2 ][ 0 ] = ae::Vec3( 1, 1, 0 );
	cornerOffsets[ 2 ][ 1 ] = ae::Vec3( 1, 1, 1 );
	
	// @NOTE: This phase generates the surface mesh for the current chunk. The vertex
	// positions will be centered at the end of this phase, and will be nudged later
	// to the correct position within the voxel.
	const int32_t chunkPlus = kChunkSize + 1;
	for( int32_t z = -1; z < chunkPlus; z++ )
	for( int32_t y = -1; y < chunkPlus; y++ )
	for( int32_t x = -1; x < chunkPlus; x++ )
	{
		float cornerValues[ 3 ][ 2 ];
		for ( int32_t i = 0; i < 3; i++ )
		{
			for ( int32_t j = 0; j < 2; j++ )
			{
				int32_t gx = x + cornerOffsets[ i ][ j ].x;
				int32_t gy = y + cornerOffsets[ i ][ j ].y;
				int32_t gz = z + cornerOffsets[ i ][ j ].z;
				// @TODO: Should pre-calculate, or at least only look up corner (1,1,1) once
				cornerValues[ i ][ j ] = sdf->GetValue( ae::Int3( gx, gy, gz ) );
				if ( cornerValues[ i ][ j ] == 0.0f )
				{
					// @NOTE: Never let a terrain value be exactly 0, or else surface will end up with multiple vertices for the same point in the sdf
					cornerValues[ i ][ j ] = 0.0001f;
				}
			}
		}
		
		// Detect if any of the 3 new edges being tested intersect the implicit surface
		uint16_t edgeBits = 0;
		if ( cornerValues[ 0 ][ 0 ] * cornerValues[ 0 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_FRONT_BIT; }
		if ( cornerValues[ 1 ][ 0 ] * cornerValues[ 1 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_RIGHT_BIT; }
		if ( cornerValues[ 2 ][ 0 ] * cornerValues[ 2 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT; }
		
		uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		TempEdges* te = &m_tempEdges[ edgeIndex ];
		te->b = edgeBits;
		te->x = x;
		te->y = y;
		te->z = z;
		
		// Iterate over voxel edges (only 3 for TempEdges)
		for ( int32_t e = 0; e < 3; e++ )
		if ( edgeBits & mask[ e ] )
		{
			if ( vertexCount + VertexCount( 4 ) > maxVerts || indexCount + 6 > maxIndices )
			{
				*vertexCountOut = VertexCount( 0 );
				*indexCountOut = 0;
				return;
			}

			// Get intersection of edge and implicit surface
			ae::Vec3 edgeVoxelPos;
			// Start edgeVoxelPos calculation
			{
				// Determine which end of edge is inside/outside
				ae::Vec3 c0, c1;
				if ( cornerValues[ e ][ 0 ] < cornerValues[ e ][ 1 ] )
				{
					c0 = cornerOffsets[ e ][ 0 ]; // Inside surface
					c1 = cornerOffsets[ e ][ 1 ]; // Outside surface
				}
				else
				{
					c0 = cornerOffsets[ e ][ 1 ]; // Inside surface
					c1 = cornerOffsets[ e ][ 0 ]; // Outside surface
				}

				// Find actual surface intersection point
				ae::Vec3 ch( x, y, z );
				// @TODO: This should probably be adjustable
				for ( int32_t i = 0; i < 16; i++ )
				{
					// @TODO: This can be simplified by lerping and using the t value to do a binary search
					edgeVoxelPos = ( c0 + c1 ) * 0.5f;
					ae::Vec3 cw = ch + edgeVoxelPos;
					
					float v = sdf->GetValue( cw );
					if ( ae::Abs( v ) < 0.001f )
					{
						break;
					}
					else if ( v < 0.0f )
					{
						c0 = edgeVoxelPos;
					}
					else
					{
						c1 = edgeVoxelPos;
					}
				}
			}
			AE_ASSERT( edgeVoxelPos.x == edgeVoxelPos.x && edgeVoxelPos.y == edgeVoxelPos.y && edgeVoxelPos.z == edgeVoxelPos.z );
			AE_ASSERT( edgeVoxelPos.x >= 0.0f && edgeVoxelPos.x <= 1.0f );
			AE_ASSERT( edgeVoxelPos.y >= 0.0f && edgeVoxelPos.y <= 1.0f );
			AE_ASSERT( edgeVoxelPos.z >= 0.0f && edgeVoxelPos.z <= 1.0f );
			// End edgeVoxelPos calculation
			
			ae::Vec3 edgeWorldPos( x, y, z );
			edgeWorldPos += edgeVoxelPos;

			te->p[ e ] = edgeVoxelPos;
			te->n[ e ] = sdf->GetDerivative( edgeWorldPos );
			
			if ( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
			{
				continue;
			}
			
			IsosurfaceIndex ind[ 4 ];
			int32_t offsets[ 4 ][ 3 ];
			m_GetQuadVertexOffsetsFromEdge( mask[ e ], offsets );
			
			// @NOTE: Expand edge into two triangles. Add new vertices for each edge
			// intersection (centered in voxels at this point). Edges are eventually expanded
			// into quads, so each edge needs 4 vertices. This does some of the work
			// for adjacent voxels.
			for ( int32_t j = 0; j < 4; j++ )
			{
				int32_t ox = x + offsets[ j ][ 0 ];
				int32_t oy = y + offsets[ j ][ 1 ];
				int32_t oz = z + offsets[ j ][ 2 ];
				
				// This check allows coordinates to be one out of chunk high end
				if ( ox < 0 || oy < 0 || oz < 0 || ox > kChunkSize || oy > kChunkSize || oz > kChunkSize )
				{
					continue;
				}
				
				bool inCurrentChunk = ox < kChunkSize && oy < kChunkSize && oz < kChunkSize;
				if ( !inCurrentChunk || m_i[ ox ][ oy ][ oz ] == kInvalidIsosurfaceIndex )
				{
					IsosurfaceVertex vertex;
					vertex.position.x = ox + 0.5f;
					vertex.position.y = oy + 0.5f;
					vertex.position.z = oz + 0.5f;
					vertex.position.w = 1.0f;
					
					AE_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );

					IsosurfaceIndex index = (IsosurfaceIndex)vertexCount;
					verticesOut[ (uint32_t)vertexCount ] = vertex;
					vertexCount++;
					ind[ j ] = index;
					
					if ( inCurrentChunk )
					{
						m_i[ ox ][ oy ][ oz ] = index;
					}
				}
				else
				{
					IsosurfaceIndex index = m_i[ ox ][ oy ][ oz ];
					AE_ASSERT_MSG( index < (IsosurfaceIndex)vertexCount, "# < # ox:# oy:# oz:#", index, vertexCount, ox, oy, oz );
					AE_ASSERT( ox < kChunkSize );
					AE_ASSERT( oy < kChunkSize );
					AE_ASSERT( oz < kChunkSize );
					ind[ j ] = index;
				}
			}
			
			bool flip = false;
			// 0 - EDGE_TOP_FRONT_BIT
			// 1 - EDGE_TOP_RIGHT_BIT
			// 2 - EDGE_SIDE_FRONTRIGHT_BIT
			if ( e == 0 ) { flip = ( cornerValues[ 2 ][ 1 ] > 0.0f ); }
			else if ( e == 1 ) { flip = ( cornerValues[ 2 ][ 1 ] < 0.0f ); }
			else { flip = ( cornerValues[ 2 ][ 1 ] < 0.0f ); }

			// @TODO: This assumes counter clockwise culling
			if ( flip )
			{
				// tri0
				indexOut[ indexCount++ ] = ind[ 0 ];
				indexOut[ indexCount++ ] = ind[ 1 ];
				indexOut[ indexCount++ ] = ind[ 2 ];
				// tri1
				indexOut[ indexCount++ ] = ind[ 1 ];
				indexOut[ indexCount++ ] = ind[ 3 ];
				indexOut[ indexCount++ ] = ind[ 2 ];
			}
			else
			{
				// tri2
				indexOut[ indexCount++ ] = ind[ 0 ];
				indexOut[ indexCount++ ] = ind[ 2 ];
				indexOut[ indexCount++ ] = ind[ 1 ];
				//tri3
				indexOut[ indexCount++ ] = ind[ 1 ];
				indexOut[ indexCount++ ] = ind[ 2 ];
				indexOut[ indexCount++ ] = ind[ 3 ];
			}
		}
	}
	
	if ( indexCount == 0 )
	{
		// @TODO: Should differentiate between empty chunk and full chunk. It's possible though that
		// Chunk::t's are good enough for this though.
		*vertexCountOut = kChunkCountEmpty;
		*indexCountOut = 0;
		return;
	}
	
	const uint32_t vc = (int32_t)vertexCount;
	for ( uint32_t i = 0; i < vc; i++ )
	{
		IsosurfaceVertex* vertex = &verticesOut[ i ];
		int32_t x = ae::Floor( vertex->position.x );
		int32_t y = ae::Floor( vertex->position.y );
		int32_t z = ae::Floor( vertex->position.z );
		AE_ASSERT( x >= 0 && y >= 0 && z >= 0 );
		AE_ASSERT( x <= kChunkSize && y <= kChunkSize && z <= kChunkSize );
		
		int32_t ec = 0;
		ae::Vec3 p[ 12 ];
		ae::Vec3 n[ 12 ];
		
		uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		TempEdges te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if ( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].x -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
		{
			p[ ec ] = te.p[ 2 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].y -= 1.0f;
			n[ ec ] = te.n[ 2 ];
			ec++;
		}
		edgeIndex = x + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].x -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + z * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].y -= 1.0f;
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
		AE_ASSERT( edgeIndex < kTempChunkSize3 );
		te = m_tempEdges[ edgeIndex ];
		if ( te.b & EDGE_TOP_FRONT_BIT )
		{
			p[ ec ] = te.p[ 0 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 0 ];
			ec++;
		}
		if ( te.b & EDGE_TOP_RIGHT_BIT )
		{
			p[ ec ] = te.p[ 1 ];
			p[ ec ].z -= 1.0f;
			n[ ec ] = te.n[ 1 ];
			ec++;
		}
		
		// Validation
		AE_ASSERT( ec != 0 );
		for ( int32_t j = 0; j < ec; j++ )
		{
			AE_ASSERT( p[ j ] == p[ j ] );
			AE_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
			AE_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
			AE_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
			AE_ASSERT( n[ j ] == n[ j ] );
		}

		// Normal
		vertex->normal = ae::Vec3( 0.0f );
		for ( int32_t j = 0; j < ec; j++ )
		{
			vertex->normal += n[ j ];
		}
		vertex->normal.SafeNormalize();
		
		// Position
		ae::Vec3 position = GetIntersection( p, n, ec );
		{
			AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
			// @NOTE: Bias towards average of intersection points. This solves some intersecting triangles on sharp edges.
			// Based on notes here: https://www.boristhebrave.com/2018/04/15/dual-contouring-tutorial/
			ae::Vec3 averagePos( 0.0f );
			for ( int32_t i = 0; i < ec; i++ )
			{
				averagePos += p[ i ];
			}
			averagePos /= (float)ec;
			position = ae::Lerp( position, averagePos, 0.75f );
		}
		// @NOTE: Do not clamp position values to voxel boundary. It's valid for a vertex to be placed
		// outside of the voxel is was generated from. This happens when a voxel has all corners inside
		// or outside of the sdf boundary, while also still having intersections (normally two per edge)
		// on one or more edges of the voxel.
		position.x = x + position.x;
		position.y = y + position.y;
		position.z = z + position.z;
		vertex->position = ae::Vec4( position + sdf->GetOffset(), 1.0f );
	}

	// @TODO: Support kChunkCountInterior for raycasting
	AE_ASSERT( vertexCount <= maxVerts );
	AE_ASSERT( indexCount <= maxIndices );
	*vertexCountOut = vertexCount;
	*indexCountOut = indexCount;
}

void IsosurfaceExtractor::m_SetVertexData( const IsosurfaceVertex* verts, const IsosurfaceIndex* indices, VertexCount vertexCount, uint32_t indexCount )
{
	// (Re)Initialize ae::VertexArray here only when needed
	if ( !m_data.GetMaxVertexCount() // Not initialized
		|| VertexCount( m_data.GetMaxVertexCount() ) < vertexCount // Too little storage for verts
		|| m_data.GetMaxIndexCount() < indexCount ) // Too little storage for t_chunkIndices
	{
		m_data.Initialize( sizeof( IsosurfaceVertex ), sizeof( IsosurfaceIndex ), (uint32_t)vertexCount, indexCount, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Dynamic );
		m_data.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( IsosurfaceVertex, position ) );
		m_data.AddAttribute( "a_normal", 3, ae::Vertex::Type::Float, offsetof( IsosurfaceVertex, normal ) );
	}

	// Set vertices
	m_data.UploadVertices( 0, verts, (uint32_t)vertexCount );
	m_data.UploadIndices( 0, indices, indexCount );
}

void IsosurfaceExtractor::m_GetQuadVertexOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] )
{
	if ( edgeBit == EDGE_TOP_FRONT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
		offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 1;
	}
	else if ( edgeBit == EDGE_TOP_RIGHT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
		offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = 1;
	}
	else if ( edgeBit == EDGE_TOP_BACK_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
		offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 1;
	}
	else if ( edgeBit == EDGE_TOP_LEFT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = -1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
		offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = 1;
	}
	else if ( edgeBit == EDGE_SIDE_FRONTLEFT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = -1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
		offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 0;
	}
	else if ( edgeBit == EDGE_SIDE_FRONTRIGHT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
		offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 0;
	}
	else if ( edgeBit == EDGE_SIDE_BACKRIGHT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
		offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 0;
	}
	else if ( edgeBit == EDGE_SIDE_BACKLEFT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = -1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
		offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 0;
	}
	else if ( edgeBit == EDGE_BOTTOM_FRONT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
		offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = -1;
	}
	else if ( edgeBit == EDGE_BOTTOM_RIGHT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
		offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = -1;
	}
	else if ( edgeBit == EDGE_BOTTOM_BACK_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
		offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = -1;
	}
	else if ( edgeBit == EDGE_BOTTOM_LEFT_BIT )
	{
		offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
		offsets[ 1 ][ 0 ] = -1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
		offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
		offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = -1;
	}
	else
	{
		AE_FAIL();
	}
}
