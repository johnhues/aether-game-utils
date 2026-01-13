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

//------------------------------------------------------------------------------
// Types / constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ISOSURFACE = "isosurface";

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

void LogStatus( const ae::IsosurfaceStatus& status )
{
	float vertCount = status.vertexCount;
	double elapsedTime = status.elapsedTime;
	double voxelTime = status.voxelTime;
	double meshTime = status.meshTime;
	float triCount = status.indexCount / 3.0f;
	float voxelWorkingSize = status.voxelWorkingSize;
	float sampleRawCount = status.sampleRawCount;
	float sampleCacheCount = status.sampleCacheCount;
	float sampleBrickCount = status.sampleBrickCount;
	float sampleBrickMissCount = status.sampleBrickMissCount;
	float voxelMissCount = status.voxelMissCount;
	float voxelCheckCount = status.voxelCheckCount;
	const float samplesPerVert = vertCount ? ( sampleRawCount / vertCount ) : 0.0f;

	// Timing
	const char* timeUnits[] = { "s", "ms", "us", "ns" };
	double maxTime = ae::Max(
		elapsedTime,
		voxelTime,
		meshTime
	);
	uint32_t timeIndex = 0;
	while( maxTime && maxTime < 1.0 )
	{
		elapsedTime *= 1000.0;
		voxelTime *= 1000.0;
		meshTime *= 1000.0;

		maxTime *= 1000.0;
		timeIndex++;
	}
	
	// Counts
	const char* counts[] = { "", "K", "M", "B" };
	float maxCount = ae::Max(
		triCount,
		vertCount,
		voxelWorkingSize,
		sampleRawCount,
		sampleCacheCount,
		sampleBrickCount,
		sampleBrickMissCount,
		voxelMissCount,
		voxelCheckCount
	);
	uint32_t countIndex = 0;
	while( maxCount > 1000.0f )
	{
		triCount /= 1000.0f;
		vertCount /= 1000.0f;
		voxelWorkingSize /= 1000.0f;
		sampleRawCount /= 1000.0f;
		sampleCacheCount /= 1000.0f;
		sampleBrickCount /= 1000.0f;
		sampleBrickMissCount /= 1000.0f;
		voxelMissCount /= 1000.0f;
		voxelCheckCount /= 1000.0f;
		
		maxCount /= 1000.0f;
		countIndex++;
	}
	
	// Log stats
	AE_INFO( "Total:## Voxel:## Mesh:## Verts:## Tris:## Miss:## Checked:##",
		elapsedTime, timeUnits[ timeIndex ],
		voxelTime, timeUnits[ timeIndex ],
		meshTime, timeUnits[ timeIndex ],
		vertCount, counts[ countIndex ],
		triCount, counts[ countIndex ],
		voxelMissCount, counts[ countIndex ],
		voxelCheckCount, counts[ countIndex ]
	);
	AE_INFO( "Samples/Vert:# Voxel:## Raw:## Cached:## BrickMiss:#%",
		samplesPerVert,
		voxelWorkingSize, counts[ countIndex ],
		sampleRawCount, counts[ countIndex ],
		sampleCacheCount, counts[ countIndex ],
		( sampleBrickMissCount / (float)sampleBrickCount ) * 100.0f
		
	);

	// Mesh index ratio warning
	if( vertCount > 0.0f )
	{
		const float indexRatio = ( 3.0f * triCount / vertCount );
		if( indexRatio < 5.98 || 6.02 < indexRatio )
		{
			AE_WARN( "Index ratio is #, should be ~6.0", indexRatio );
		}
	}
}

int main()
{
	AE_LOG( "Initialize (debug #)", (int)_AE_DEBUG_ );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::DebugLines debugLines = TAG_ISOSURFACE;
	ae::DebugCamera camera = ae::Axis::Z;
	ae::Shader shader;
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
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( ae::NextPowerOfTwo( 4000000 ) );
	debugLines.SetXRayEnabled( false );
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 500.0f ) );
	shader.Initialize( kVertShader, kFragShader );

	ae::VertexBuffer sdfVertexBuffer;
	ae::Array< ae::AABB > octree = TAG_ISOSURFACE;
	ae::Array< ae::AABB > brickMap = TAG_ISOSURFACE;
	ae::Array< ae::Vec3 > errors = TAG_ISOSURFACE;
	ae::IsosurfaceExtractor* extractor = ae::New< ae::IsosurfaceExtractor >( TAG_ISOSURFACE, TAG_ISOSURFACE );
	ae::CollisionMesh<> collisionMesh = TAG_ISOSURFACE;
	bool collisionMeshDirty = true;
	bool opaqueMeshEnabled = true;
	bool wireframeMeshEnabled = false;
	bool ambientLightEnabled = true;
	bool directionalLightEnabled = true;
	bool octreeEnabled = false;
	bool brickMapEnabled = false;
	bool tightBoundsEnabled = true;
	bool dualContouringEnabled = false;
	double executionTime = 0.0;
	uint32_t executionCount = 0;
	const ae::Color color = ae::Color::AetherTeal();
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
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.1f, 10000.0f );
		const ae::Matrix4 worldToProj = viewToProj * worldToView;
		const ae::Vec2 mousePosition = (ae::Vec2)input.mouse.position;
		const ae::Vec4 ndc(
			( (float)mousePosition.x / (float)window.GetWidth() ) * 2.0f - 1.0f,
			( (float)mousePosition.y / (float)window.GetHeight() ) * 2.0f - 1.0f,
			0.0f,
			1.0f
		);
		const ae::Vec4 clipPos = worldToProj.GetInverse() * ndc;
		const ae::Vec3 worldPos = ( clipPos.GetXYZ() / clipPos.w );
		const ae::Vec3 ray = ( worldPos - camera.GetPosition() ).SafeNormalizeCopy() * 10000.0f;
		const ae::RaycastResult result = collisionMesh.Raycast( {
			.source=camera.GetPosition(),
			.ray=ray,
		} );
		input.Pump();
		camera.Update( &input, dt );

		// Camera
		if( result.hits.Length() && ( camera.GetMode() == ae::DebugCamera::Mode::None ) )
		{
			debugLines.AddSphere( result.hits[ 0 ].position, 5.0f, ae::Color::Red(), 12 );
			if( input.GetMouseReleaseLeft() )
			{
				camera.Refocus( result.hits[ 0 ].position );
			}
		}
		// Rendering
		if( input.GetPress( ae::Key::Num1 ) ) { opaqueMeshEnabled = !opaqueMeshEnabled; }
		if( input.GetPress( ae::Key::Num2 ) ) { wireframeMeshEnabled = !wireframeMeshEnabled; }
		if( input.GetPress( ae::Key::Num3 ) ) { ambientLightEnabled = !ambientLightEnabled; }
		if( input.GetPress( ae::Key::Num4 ) ) { directionalLightEnabled = !directionalLightEnabled; }
		if( input.GetPress( ae::Key::Num5 ) ) { octreeEnabled = !octreeEnabled; }
		if( input.GetPress( ae::Key::Num6 ) ) { brickMapEnabled = !brickMapEnabled; }
		if( input.GetPress( ae::Key::Num7 ) ) { tightBoundsEnabled = !tightBoundsEnabled; }
		if( input.GetPress( ae::Key::Num8 ) ) { dualContouringEnabled = !dualContouringEnabled; prevTransform = ae::Matrix4::Scaling( 0.0f ); }
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

		window.SetTitle( ae::Str256::Format(
			"SDF to Mesh [1]opaque:# [2]wireframe:# [3]ambient:# [4]directional:# [5]octree:# [6]brickMap:# [7]bounds:# [8]dual:# Ave:#ms",
			(uint32_t)opaqueMeshEnabled,
			(uint32_t)wireframeMeshEnabled,
			(uint32_t)ambientLightEnabled,
			(uint32_t)directionalLightEnabled,
			(uint32_t)octreeEnabled,
			(uint32_t)brickMapEnabled,
			(uint32_t)tightBoundsEnabled,
			(uint32_t)dualContouringEnabled,
			(uint32_t)( executionCount ? ( executionTime / executionCount ) * 1000.0 : 0.0 )
		).c_str() );

		const ae::Quaternion orientation = // ZYX rotation
			ae::Quaternion( ae::Vec3( 1, 0, 0 ), rotation.x ) *
			ae::Quaternion( ae::Vec3( 0, 1, 0 ), rotation.y ) *
			ae::Quaternion( ae::Vec3( 0, 0, 1 ), rotation.z );
		const ae::Matrix4 transform = ae::Matrix4::LocalToWorld( translation, orientation, scale );
		const ae::AABB region = tightBoundsEnabled ? ae::OBB( transform ).GetAABB() : ae::AABB( ae::Vec3( -300, -300, -1000 ), ae::Vec3( 300, 300, 1000 ) );
		if( prevTransform != transform )
		{
			const ae::Matrix4 transformNoScale = transform.GetScaleRemoved();
			const ae::Matrix4 inverseTransform = transformNoScale.GetInverse();
			octree.Clear();
			brickMap.Clear();
			errors.Clear();
			collisionMesh.Clear();
			collisionMeshDirty = true;
			prevTransform = transform;
			
			// Generate
			const auto surfaceFn = [&inverseTransform, &scale]( ae::Vec3 _p )
			{
				float r;
				// for( uint32_t i = 0; i < 100; i++ )
				{
					const float smooth = 2.5f; // World space because scale is applied separately from transform
					const ae::Vec3 p = inverseTransform.TransformPoint3x4( _p );
					r = SDFBox( p, ae::Vec3( 0.5f, 0.25f, 0.25f ) * scale, smooth );
					r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.5f, 0.25f ) * scale, smooth ), smooth );
					r = SDFSmoothUnion( r, SDFBox( p, ae::Vec3( 0.25f, 0.25f, 0.5f ) * scale, smooth ), smooth );
				}
				return r;
			};
			const bool success = extractor->Generate( {
				.sampleFn=[]( const void* userData, ae::Vec3 position ) -> ae::IsosurfaceValue
				{
					return { (*(decltype(surfaceFn)*)userData)( position ), 0.0f };
				},
				.statusFn=[]( const void*, const ae::IsosurfaceStatus& status )
				{
					AE_INFO( "Voxel: #%(#ms) Mesh: #%(#ms)",
						(int32_t)( status.voxelProgress01 * 100.0f ),
						(int32_t)( status.voxelTime * 1000.0 ),
						(int32_t)( status.meshProgress01 * 100.0f ),
						(int32_t)( status.meshTime * 1000.0 )
					);
					return true;
				},
				.userData=&surfaceFn,
				.aabb=region,
				.maxVerts=0,
				.maxIndices=0,
				.dualContouring=dualContouringEnabled,
				.octree=&octree,
				.brickMap=&brickMap,
				.errors=&errors
			} );
			if( extractor->vertices.Length() )
			{
				AE_ASSERT( success );
				// (Re)Initialize ae::VertexArray here only when needed
				if( !sdfVertexBuffer.GetMaxVertexCount() // Not initialized
					|| sdfVertexBuffer.GetMaxVertexCount() < extractor->vertices.Length() // Too little storage for verts
					|| sdfVertexBuffer.GetMaxIndexCount() < extractor->indices.Length() ) // Too little storage for t_chunkIndices
				{
					sdfVertexBuffer.Initialize(
						sizeof( ae::IsosurfaceVertex ),
						sizeof( ae::IsosurfaceIndex ),
						extractor->vertices.Length(),
						extractor->indices.Length(),
						ae::Vertex::Primitive::Triangle,
						ae::Vertex::Usage::Dynamic,
						ae::Vertex::Usage::Dynamic
					);
					sdfVertexBuffer.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( ae::IsosurfaceVertex, position ) );
					sdfVertexBuffer.AddAttribute( "a_normal", 3, ae::Vertex::Type::Float, offsetof( ae::IsosurfaceVertex, normal ) );
				}
	
				// Set vertices
				sdfVertexBuffer.UploadVertices( 0, extractor->vertices.Data(), extractor->vertices.Length() );
				sdfVertexBuffer.UploadIndices( 0, extractor->indices.Data(), extractor->indices.Length() );
			}

			LogStatus( extractor->GetStatus() );
			executionTime += extractor->GetStatus().elapsedTime;
			executionCount++;
		}
		else if( collisionMeshDirty )
		{
			collisionMesh.AddIndexed( {
				.vertexPositions = extractor->vertices.Data()->position.data,
				.vertexPositionStride = sizeof( ae::IsosurfaceVertex ),
				.vertexCount = extractor->vertices.Length(),
				.indices = extractor->indices.Data(),
				.indexCount = extractor->indices.Length(),
				.indexSize = sizeof( ae::IsosurfaceIndex )
			} );
			collisionMesh.BuildBVH();
			collisionMeshDirty = false;
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		debugLines.AddLine( ae::Vec3( -1, 0, 0 ) * 1000.0f, ae::Vec3( 1, 0, 0 ) * 1000.0f, ae::Color::AetherRed() );
		debugLines.AddLine( ae::Vec3( 0, -1, 0 ) * 1000.0f, ae::Vec3( 0, 1, 0 ) * 1000.0f, ae::Color::AetherGreen() );
		debugLines.AddLine( ae::Vec3( 0, 0, -1 ) * 1000.0f, ae::Vec3( 0, 0, 1 ) * 1000.0f, ae::Color::AetherBlue() );
		debugLines.AddOBB( ae::OBB( transform ), color );
		
		ae::UniformList uniforms;
		uniforms.Set( "u_worldToProj", worldToProj );
		debugLines.AddAABB( region.GetCenter(), region.GetHalfSize(), ae::Color::AetherPurple() );
		if( extractor->indices.Length() )
		{
			uniforms.Set( "u_light", ae::Vec2( (float)directionalLightEnabled, (float)ambientLightEnabled ) );
			uniforms.Set( "u_color", color.GetLinearRGB() );
			shader.SetCulling( opaqueMeshEnabled ? ae::Culling::CounterclockwiseFront : ae::Culling::None );
			shader.SetBlending( !opaqueMeshEnabled );
			shader.SetDepthWrite( opaqueMeshEnabled );
			shader.SetDepthTest( opaqueMeshEnabled );
			shader.SetWireframe( wireframeMeshEnabled );
			sdfVertexBuffer.Bind( &shader, uniforms );
			sdfVertexBuffer.Draw( 0, extractor->indices.Length() / 3 );
		}
		if( octreeEnabled )
		{
			for( ae::AABB octant : octree )
			{
				int exponent;
				frexp( octant.GetHalfSize().Length(), &exponent );
				const float distance01 = ae::Delerp01( 8.0f, 0.0f, exponent );
				debugLines.AddAABB( octant, ae::Color::AetherWhite().Lerp( ae::Color::AetherRed(), distance01 ) );
			}
		}
		if( brickMapEnabled )
		{
			for( ae::AABB brick : brickMap )
			{
				debugLines.AddAABB( brick, ae::Color::AetherOrange() );
			}
		}
		for( ae::Vec3 error : errors )
		{
			debugLines.AddSphere( error, 0.5f, ae::Color::AetherRed(), 4 );
		}
		debugLines.Render( worldToProj );
		
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while( Update() ) {}
#endif

	ae::Delete( extractor );
	AE_LOG( "Terminate" );
	return 0;
}
