//------------------------------------------------------------------------------
// 15_ObjViewer.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
// Constants
//------------------------------------------------------------------------------
const ae::Tag kObjAllocTag = "obj";
#ifndef DATA_DIR
	#define DATA_DIR "data"
#endif

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_UNIFORM mat4 u_normalToWorld;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_normal;
	AE_IN_HIGHP vec4 a_color;
	AE_OUT_HIGHP vec3 v_normal;
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_color = a_color * u_color;
		v_normal = ( u_normalToWorld * a_normal ).rgb;
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_UNIFORM vec3 u_ambLight;
	AE_UNIFORM vec3 u_lightColor;
	AE_UNIFORM vec3 u_lightDir;
	AE_IN_HIGHP vec3 v_normal;
	AE_IN_HIGHP vec4 v_color;
	void main()
	{
#ifdef FLAT_COLOR
		AE_COLOR = v_color;
#else
		vec3 light = u_lightColor * max( 0.0, dot( -u_lightDir, normalize( v_normal ) ) );
		light += u_ambLight;
		AE_COLOR.rgb = v_color.rgb * light;
		AE_COLOR.a = v_color.a;
#endif
	})";

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
	ae::Shader shader;
	ae::Shader shadowShader;
	ae::FileSystem fs;
	
	ae::VertexBuffer vertexData;
	ae::AABB aabb;
	ae::DebugCamera camera = ae::Axis::Y;
	ae::DebugLines debugLines = kObjAllocTag;
	
	window.Initialize( 800, 600, false, true );
	window.SetTitle( "OBJ Viewer" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fs.Initialize( DATA_DIR, "ae", "obj_viewer" );
	camera.SetDistanceLimits( 0.25f, 10.0f );
	debugLines.Initialize( 1024 );
	
	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetBlending( true );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );
	
	const char* defines[] { "#define FLAT_COLOR" };
	shadowShader.Initialize( kVertShader, kFragShader, defines, countof(defines) );
	shadowShader.SetDepthTest( true );
	shadowShader.SetDepthWrite( true );
	shadowShader.SetBlending( true );
	shadowShader.SetCulling( ae::Culling::CounterclockwiseFront );

	ae::Str256 dataDir;
	fs.GetRootDir( ae::FileSystem::Root::Data, &dataDir );
	AE_INFO( "data dir: '#'", dataDir );
	
	const ae::File* file = fs.Read( ae::FileSystem::Root::Data, "bunny.obj", 2.5f );
	AE_INFO( "Start loading '#'", file->GetUrl() );

	float spin = 0.0f;

	AE_INFO( "Run" );
	auto Update = [&]()
	{
		input.Pump();
		camera.Update( &input, timeStep.GetDt() );
		spin += timeStep.GetDt() * 0.75f;
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		ae::UniformList uniformList;
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.1f, 100.0f );

		if( file && file->GetStatus() != ae::File::Status::Pending )
		{
			AE_INFO( "Load obj" );
			ae::OBJFile objFile = kObjAllocTag;
			if( const uint32_t fileSize = file->GetLength() )
			{
				objFile.Load( file->GetData(), fileSize );
				if( objFile.vertices.Length() )
				{
					aabb = objFile.aabb;
					vertexData.Initialize(
						sizeof(*objFile.vertices.Data()), sizeof(*objFile.indices.Data()),
						objFile.vertices.Length(), objFile.indices.Length(),
						ae::Vertex::Primitive::Triangle,
						ae::Vertex::Usage::Static, ae::Vertex::Usage::Static
					);
					vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( ae::OBJFile::Vertex, position ) );
					vertexData.AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( ae::OBJFile::Vertex, normal ) );
					vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( ae::OBJFile::Vertex, color ) );
					vertexData.UploadVertices( 0, objFile.vertices.Data(), objFile.vertices.Length() );
					vertexData.UploadIndices( 0, objFile.indices.Data(), objFile.indices.Length() );
					
					ae::Vec3 offset = camera.GetPosition() - camera.GetFocus();
					offset = offset.SafeNormalizeCopy() * 3.0f;
					ae::Vec3 focus = aabb.GetCenter() / aabb.GetHalfSize().Length();
					camera.Reset( focus, focus + offset );
				}
				else
				{
					AE_ERR( "Invalid obj file '#'", file->GetUrl() );
				}
			}
			else
			{
				AE_ERR( "Error reading file '#'", file->GetUrl() );
			}

			fs.Destroy( file );
			file = nullptr;
		}
		
		// Draw object
		if( vertexData.GetVertexSize() )
		{
			ae::Matrix4 modelToWorld = ae::Matrix4::RotationY( spin ) * ae::Matrix4::Scaling( 1.0f / aabb.GetHalfSize().Length() );
			uniformList.Set( "u_worldToProj", viewToProj * worldToView * modelToWorld );
			uniformList.Set( "u_normalToWorld", modelToWorld.GetNormalMatrix() );
			uniformList.Set( "u_lightColor", ae::Color::White().Lerp( ae::Color::PicoPeach(), 0.75f ).GetLinearRGB() );
			uniformList.Set( "u_lightDir", ae::Vec3( 7.0f, -3.0f, -5.0f ).NormalizeCopy() );
			uniformList.Set( "u_ambLight", ae::Color::PicoDarkPurple().ScaleRGB( 0.5f ).GetLinearRGB() );
			uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
			vertexData.Bind( &shader, uniformList );
			vertexData.Draw();
			// Shadow
			ae::Matrix4 flat = ae::Matrix4::Scaling( ae::Vec3( 1.0f, 0.0f, 1.0f ) );
			uniformList.Set( "u_worldToProj", viewToProj * worldToView * flat * modelToWorld );
			uniformList.Set( "u_color", ae::Color::PicoDarkPurple().ScaleRGB( 0.6f ).GetLinearRGBA() );
			vertexData.Bind( &shadowShader, uniformList );
			vertexData.Draw();
			// Debug
			debugLines.AddOBB( modelToWorld * aabb.GetTransform(), ae::Color::PicoPink() );
		}
		debugLines.Render( viewToProj * worldToView );
		
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* fn ) { (*(decltype(Update)*)fn)(); }, &Update, 0, 1 );
#else
	while ( Update() ) {}
#endif

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
