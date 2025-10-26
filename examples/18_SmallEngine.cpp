//------------------------------------------------------------------------------
//! 18_SmallEngine.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and /or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "18_SmallEngine.h"
#include "ae/MeshEditorPlugin.h"

//------------------------------------------------------------------------------
// Component
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( Component );

//------------------------------------------------------------------------------
// Resources
//------------------------------------------------------------------------------
extern const char* kVertShader;
extern const char* kFragShader;
void LoadObj( const char* fileName, const ae::FileSystem* fs, ae::VertexBuffer* vertexDataOut, ae::CollisionMesh<>* collisionOut, ae::EditorMesh* editorMeshOut );
void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex );

//------------------------------------------------------------------------------
// SmallEngineEditorPlugin
//------------------------------------------------------------------------------
class SmallEngineEditorPlugin : public ae::MeshEditorPlugin
{
public:
	SmallEngineEditorPlugin( SmallEngine* engine ) : ae::MeshEditorPlugin( TAG_SMALL_ENGINE ), m_engine( engine ) {}
	void OnEvent( const ae::EditorEvent& event ) override;
	ae::Optional< ae::EditorMesh > TryLoad( const char* resourceStr ) override;
private:
	SmallEngine* m_engine = nullptr;
};

//------------------------------------------------------------------------------
// Engine member functions
//------------------------------------------------------------------------------
bool SmallEngine::Initialize( int argc, char* argv[] )
{
	fs.Initialize( "data", "ae", "editor" );
	
	ae::EditorParams editorParams( argc, argv, &registry );
	// editorParams.run = true;
	editor.AddPlugin< SmallEngineEditorPlugin >( this );
	if( editor.Initialize( editorParams ) )
	{
		return false; // Exit, the editor has forked, ran, closed, and returned gracefully
	}
	window.Initialize( 1280, 720, false, true, true );
	window.SetTitle( "Press '~' to Open the Editor" );
	input.Initialize( &window );
	gfx.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 10 * 1024 );

	LoadObj( "bunny.obj", &fs, &bunnyVertexData, &bunnyCollision, nullptr );
	meshShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	meshShader.SetDepthTest( true );
	meshShader.SetDepthWrite( true );
	meshShader.SetCulling( ae::Culling::CounterclockwiseFront );

	ae::Str256 levelPath;
	if( fs.GetRootDir( ae::FileSystem::Root::Data, &levelPath ) )
	{
		ae::FileSystem::AppendToPath( &levelPath, "example.level" );
		editor.QueueRead( levelPath.c_str() );
	}
	return true;
}

void SmallEngine::Run()
{
	while( !input.quit )
	{
		// Update
		input.Pump();
		editor.Update();
		if( input.GetMousePressLeft() ) { input.SetMouseCaptured( true ); }
		if( input.GetPress( ae::Key::F ) ) { window.SetFullScreen( !window.GetFullScreen() ); input.SetMouseCaptured( window.GetFullScreen() ); }
		if( input.GetPress( ae::Key::Escape ) ) { input.SetMouseCaptured( false ); window.SetFullScreen( false ); }
		if( input.Get( ae::Key::Tilde ) && !input.GetPrev( ae::Key::Tilde ) ) { editor.Launch(); }
		registry.CallFn< Component >( [&]( Component* component )
		{
			if( !component->initialized )
			{
				component->Initialize( this );
				component->initialized = true;
			}
		} );
		registry.CallFn< Component >( [&]( Component* c ){ c->Update( this ); } );
		
		// Render
		gfx.Activate();
		gfx.Clear( skyColor );
		worldToView = ae::Matrix4::WorldToView( cameraPos, cameraDir, ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		viewToProj = ae::Matrix4::ViewToProjection( 1.1f, gfx.GetAspectRatio(), 0.1f, 500.0f );
		worldToProj = viewToProj * worldToView;
		registry.CallFn< Component >( [&]( Component* c ){ c->Render( this ); } );
		debugLines.Render( worldToProj );
		gfx.Present();
		timeStep.Tick();
	}
}

void SmallEngine::GetUniforms( ae::UniformList* uniformList )
{
	uniformList->Set( "u_lightColor", ae::Color::PicoPeach().ScaleRGB( 1.0f ).GetLinearRGB() );
	uniformList->Set( "u_lightDir", ae::Vec3( -7.0f, 5.0f, -3.0f ).NormalizeCopy() );
	uniformList->Set( "u_ambLight", skyColor.GetLinearRGB() );
}

//------------------------------------------------------------------------------
// SmallEngineEditorPlugin member functions
//------------------------------------------------------------------------------
void SmallEngineEditorPlugin::OnEvent( const ae::EditorEvent& event )
{
	ae::MeshEditorPlugin::OnEvent( event );
	if( event.type == ae::EditorEventType::LevelUnload )
	{
		m_engine->registry.CallFn< Component >( [&]( Component* c ){ c->Terminate( m_engine ); } );
		m_engine->registry.Clear();
	}
}

ae::Optional< ae::EditorMesh > SmallEngineEditorPlugin::TryLoad( const char* resourceStr )
{
	ae::EditorMesh result = TAG_SMALL_ENGINE;
	LoadObj( resourceStr, &m_engine->fs, nullptr, nullptr, &result );
	return result;
}

//------------------------------------------------------------------------------
// Resources
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_UNIFORM mat4 u_normalToWorld;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_normal;
	AE_IN_HIGHP vec2 a_uv;
	AE_IN_HIGHP vec4 a_color;
	AE_OUT_HIGHP vec3 v_normal;
	AE_OUT_HIGHP vec2 v_uv;
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_color = a_color * u_color;
		v_normal = ( u_normalToWorld * a_normal ).xyz;
		v_uv = a_uv;
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_UNIFORM vec3 u_ambLight;
	AE_UNIFORM vec3 u_lightColor;
	AE_UNIFORM vec3 u_lightDir;
#ifdef DIFFUSE
	uniform sampler2D u_tex;
#endif
	AE_IN_HIGHP vec3 v_normal;
	AE_IN_HIGHP vec2 v_uv;
	AE_IN_HIGHP vec4 v_color;
	void main()
	{
		vec3 light = u_lightColor * max( 0.0, dot( -u_lightDir, normalize( v_normal ) ) );
		light += u_ambLight + u_lightColor * 0.2;
		AE_COLOR.rgb = v_color.rgb * light;
#ifdef DIFFUSE
		AE_COLOR.rgb *= AE_TEXTURE2D( u_tex, v_uv ).rgb;
#endif
		AE_COLOR.a = v_color.a;
	})";

void LoadObj( const char* fileName, const ae::FileSystem* fs, ae::VertexBuffer* vertexDataOut, ae::CollisionMesh<>* collisionOut, ae::EditorMesh* editorMeshOut )
{
	ae::OBJLoader objFile = TAG_SMALL_ENGINE;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	objFile.Load( { fileBuffer.Data(), fileBuffer.Length() } );
	if( objFile.vertices.Length() )
	{
		if( vertexDataOut )
		{
			vertexDataOut->Initialize(
				sizeof(*objFile.vertices.Data()), sizeof(*objFile.indices.Data()),
				objFile.vertices.Length(), objFile.indices.Length(),
				ae::Vertex::Primitive::Triangle,
				ae::Vertex::Usage::Static, ae::Vertex::Usage::Static
			);
			vertexDataOut->AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, position ) );
			vertexDataOut->AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, normal ) );
			vertexDataOut->AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, texture ) );
			vertexDataOut->AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, color ) );
			vertexDataOut->UploadVertices( 0, objFile.vertices.Data(), objFile.vertices.Length() );
			vertexDataOut->UploadIndices( 0, objFile.indices.Data(), objFile.indices.Length() );
		}

		objFile.InitializeCollisionMesh( collisionOut );
		
		if( editorMeshOut )
		{
			editorMeshOut->verts.Reserve( objFile.vertices.Length() );
			for( uint32_t i = 0; i < objFile.vertices.Length(); i++ )
			{
				editorMeshOut->verts.Append( objFile.vertices[ i ].position.GetXYZ() );
			}
			editorMeshOut->indices.AppendArray( objFile.indices.Data(), objFile.indices.Length() );
		}
	}
}

void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex )
{
	ae::TargaFile tgaFile = TAG_SMALL_ENGINE;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	tgaFile.Load( fileBuffer.Data(), fileBuffer.Length() );
	tex->Initialize( tgaFile.textureParams );
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	SmallEngine engine;
	if( engine.Initialize( argc, argv ) )
	{
		engine.Run();
	}
	return 0;
}
