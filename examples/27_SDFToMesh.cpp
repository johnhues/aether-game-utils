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

bool s_debugLines = false;
void ToggleDebugLines()
{
	s_debugLines = !s_debugLines;
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
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 500.0f ) );
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
	debugLines.Initialize( ae::NextPowerOfTwo( 1000000 ) );
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

		ae::IsosurfaceExtractor* extractor = ae::New< ae::IsosurfaceExtractor >( TAG_ISOSURFACE, TAG_ISOSURFACE );
		ae::VertexBuffer sdfVertexBuffer;
		ae::Array< ae::Vec3 > errors = TAG_ISOSURFACE;
		double cacheTime = 0.0;
		double meshTime = 0.0;
		void Run( ae::Matrix4 transform, ae::DebugLines* debugLines )
		{
			const ae::Vec3 scale = transform.GetScale();
			transform = transform.GetScaleRemoved();
			const ae::Matrix4 inverseTransform = transform.GetInverse();
			const auto surfaceFn = [&inverseTransform, &scale]( ae::Vec3 _p )
			{
				const float smooth = 2.5f; // World space because scale is applied separately from transform
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
			extractor->Generate(
				{
					.fn=[]( ae::Vec3 position, const void* userData ) { return (*(decltype(surfaceFn)*)userData)( position ); },
					.userData=&surfaceFn,
					.aabb=region,
					.maxVerts=0,
					.maxIndices=0,
					.debug=( s_debugLines ? debugLines : nullptr )
				},
				&errors
			);
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

		void Draw( ae::Shader* shader, ae::UniformList& uniforms, ae::DebugLines* debugLines )
		{
			debugLines->AddAABB( region.GetCenter(), region.GetHalfSize(), s_debugLines ? ae::Color::AetherRed() : color );
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
	
	//*/
	// Single region
	SDFToMesh* sdfToMesh[] =
	{
		ae::New< SDFToMesh >(
			TAG_ISOSURFACE,
			"Mesh0",
			ae::Color::HSV(0.5f, 0.8f, 0.5f ),
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
		if( input.GetPress( ae::Key::Num1 ) ) { ToggleOpacity(); }
		if( input.GetPress( ae::Key::Num2 ) ) { ToggleWireframe(); }
		if( input.GetPress( ae::Key::Num3 ) ) { ToggleAmbientLight(); }
		if( input.GetPress( ae::Key::Num4 ) ) { ToggleDirectionalLight(); }
		if( input.GetPress( ae::Key::Num5 ) ) { ToggleDebugLines(); }
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
				sdf->Run( transform, &debugLines );
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
	while( Update() ) {}
#endif

	AE_LOG( "Terminate" );
	return 0;
}
