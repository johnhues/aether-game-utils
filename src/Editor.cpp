//------------------------------------------------------------------------------
// Editor.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
// Created by John Hughes on 11/22/21.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Editor.h"
#if _AE_APPLE_
#include <unistd.h> // fork
#elif _AE_WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#endif

// @TODO: Remove these dependencies
#include "ae/aeImGui.h"
#include "ImGuizmo.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

namespace ae {

// Helper
template < typename T >
struct RunOnDestroy
{
	RunOnDestroy( T&& func ) : m_func( std::forward< T >( func ) ) {}
	~RunOnDestroy() { m_func(); }
	T m_func;
};

class EditorServer;

const uint32_t kCogTextureDataSize = 32;
const uint8_t kCogTextureData[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,83,184,185,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,255,255,218,7,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,209,255,255,248,41,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,122,146,20,0,0,0,65,248,255,255,255,109,0,0,0,23,154,144,0,0,0,0,0,0,0,
	0,0,0,0,0,4,125,255,255,213,80,69,178,244,255,255,255,255,246,178,69,86,218,255,255,147,8,0,0,0,0,0,
	0,0,0,0,0,16,225,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,232,21,0,0,0,0,0,
	0,0,0,0,0,0,90,255,255,255,255,255,255,230,168,131,131,167,230,255,255,255,255,255,255,96,0,0,0,0,0,0,
	0,0,0,0,0,0,0,167,255,255,255,251,135,26,0,0,0,0,26,135,251,255,255,255,174,2,0,0,0,0,0,0,
	0,0,0,0,0,0,0,87,255,255,249,94,0,0,0,0,0,0,0,0,94,250,255,255,89,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,176,255,255,135,0,0,0,0,0,0,0,0,0,0,135,255,255,175,0,0,0,0,0,0,0,
	0,0,0,0,0,6,65,244,255,229,21,0,0,0,0,0,0,0,0,0,0,21,229,255,243,65,6,0,0,0,0,0,
	0,0,0,80,159,208,249,255,255,167,0,0,0,0,0,0,0,0,0,0,0,0,167,255,255,249,208,160,80,0,0,0,
	0,0,0,192,255,255,255,255,255,133,0,0,0,0,0,0,0,0,0,0,0,0,133,255,255,255,255,255,192,0,0,0,
	0,0,0,192,255,255,255,255,255,133,0,0,0,0,0,0,0,0,0,0,0,0,133,255,255,255,255,255,192,0,0,0,
	0,0,0,124,212,248,255,255,255,167,0,0,0,0,0,0,0,0,0,0,0,0,167,255,255,255,248,212,123,0,0,0,
	0,0,0,0,9,44,108,245,255,229,21,0,0,0,0,0,0,0,0,0,0,21,230,255,244,108,44,9,0,0,0,0,
	0,0,0,0,0,0,0,179,255,255,135,0,0,0,0,0,0,0,0,0,0,135,255,255,179,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,69,251,255,249,94,0,0,0,0,0,0,0,0,94,249,255,251,68,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,80,252,255,255,251,135,25,0,0,0,0,25,135,251,255,255,252,84,0,0,0,0,0,0,0,
	0,0,0,0,0,0,17,215,255,255,255,255,255,230,167,131,131,167,230,255,255,255,255,255,220,20,0,0,0,0,0,0,
	0,0,0,0,0,0,149,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,156,0,0,0,0,0,0,
	0,0,0,0,0,0,121,255,255,255,167,91,176,244,255,255,255,255,246,176,92,172,255,255,255,144,1,0,0,0,0,0,
	0,0,0,0,0,0,0,125,214,89,0,0,0,65,248,255,255,255,109,0,0,0,95,223,147,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,16,0,0,0,0,1,209,255,255,248,41,0,0,0,0,22,3,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,255,255,219,8,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,82,184,185,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const float kEditorViewDistance = 1000.0f;

//------------------------------------------------------------------------------
// EditorMsg
//------------------------------------------------------------------------------
enum class EditorMsg : uint8_t
{
	None,
	Modification,
	Load
};

//------------------------------------------------------------------------------
// EditorServerMesh class
//------------------------------------------------------------------------------
class EditorServerMesh
{
public:
	struct Vertex
	{
		ae::Vec4 position;
		ae::Vec4 normal;
	};
	EditorServerMesh( const ae::Tag& tag ) : collision( tag ) {}
	void Initialize( const ae::Tag& tag, const ae::EditorMesh* mesh );
	ae::VertexData data;
	ae::CollisionMesh collision;
};

//------------------------------------------------------------------------------
// EditorServerObject
//------------------------------------------------------------------------------
class EditorServerObject
{
public:
	EditorServerObject( const ae::Tag& tag ) : components( tag ) {}
	~EditorServerObject() { AE_ASSERT( !components.Length() ); }
	void Initialize( EditorObjectId entity, ae::Matrix4 transform );
	void Terminate();
	void SetTransform( const ae::Matrix4& transform, class EditorProgram* program );
	ae::Matrix4 GetTransform( const class EditorProgram* program ) const;
	
	void HandleVarChange( class EditorProgram* program, ae::Object* component, const ae::Type* type, const ae::Var* var );
	bool IsDirty() const { return m_dirty; } // @TODO: Rename, transform only
	void ClearDirty() { m_dirty = false; } // @TODO: Rename, transform only
	
	EditorObjectId entity = kInvalidEditorObjectId;
	ae::Str16 name;
	ae::Array< ae::Object* > components;
	bool hidden = false;
	
	EditorServerMesh* mesh = nullptr;
	bool opaque = true;
	
private:
	ae::Matrix4 m_transform = ae::Matrix4::Identity();
	bool m_dirty = false;
};

//------------------------------------------------------------------------------
// EditorConnection
//------------------------------------------------------------------------------
class EditorConnection
{
public:
	~EditorConnection();
	void Destroy( class EditorServer* editor );
	ae::Socket* sock = nullptr;
};

//------------------------------------------------------------------------------
// EditorServer class
//------------------------------------------------------------------------------
class EditorServer
{
public:
	EditorServer( const ae::Tag& tag, Editor* client ) :
		sock( tag ),
		client( client ),
		m_tag( tag ),
		m_objects( tag ),
		m_components( tag ),
		m_meshResourceVars( tag ),
		m_meshVisibleVars( tag ),
		m_typeMesh( tag ),
		m_typeInvisible( tag ),
		connections( tag )
	{}
	void Initialize( class EditorProgram* program );
	void Terminate( class EditorProgram* program );
	void Update( class EditorProgram* program );
	void Render( class EditorProgram* program );
	void ShowUI( class EditorProgram* program );
	
	bool SaveLevel( class EditorProgram* program, bool saveAs );
	void OpenLevel( class EditorProgram* program );
	void OpenLevel( class EditorProgram* program, const char* path );
	void Unload();
	
	bool GetShowInvisible() const { return m_showInvisible; }
	
	EditorServerObject* CreateObject( EditorObjectId id, const ae::Matrix4& transform );
	ae::Object* AddComponent( class EditorProgram* program, EditorServerObject* obj, const char* typeName );
	ae::Object* GetComponent( EditorServerObject* obj, const char* typeName );
	const ae::Object* GetComponent( const EditorServerObject* obj, const char* typeName );
	uint32_t GetObjectCount() const { return m_objects.Length(); }
	EditorServerObject* GetObject( EditorObjectId entity ) { return m_objects.Get( entity, nullptr ); }
	const EditorServerObject* GetObjectFromComponent( const ae::Object* component );
	const ae::Var* GetMeshResourceVar( const ae::Type* componentType );
	const ae::Var* GetMeshVisibleVar( const ae::Type* componentType );
	
	ae::ListenerSocket sock;
	
private:
	void m_Save( ae::EditorLevel* levelOut ) const;
	bool m_Load( class EditorProgram* program );
	bool m_ShowVar( class EditorProgram* program, ae::Object* component, const ae::Var* var );
	bool m_ShowVarValue( class EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx = -1 );
	bool m_ShowRefVar( class EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx = -1 );
	EditorObjectId m_PickObject( class EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut );
	void m_ShowEditorObject( EditorProgram* program, EditorObjectId entity, ae::Color color );
	ae::Color m_GetColor( EditorObjectId entity, bool lines ) const;
	
	const ae::Tag m_tag;
	bool m_first = true;
	bool m_showInvisible = false;
	std::function< bool( const ae::Type*, const char*, ae::Object** ) > m_getObjectPointerFromString;

	const ae::Type* m_selectedType = nullptr;
	EditorObjectId selected = kInvalidEditorObjectId;
	EditorObjectId hoverEntity = kInvalidEditorObjectId;
	ImGuizmo::OPERATION gizmoOperation = (ImGuizmo::OPERATION)0;
	ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
	class Editor* client;
	uint32_t m_levelSeq_HACK = 0;

	EditorObjectId m_nextEntityId = 1;
	ae::Map< EditorObjectId, EditorServerObject* > m_objects;
	ae::Map< ae::TypeId, ae::Map< EditorObjectId, ae::Object* > > m_components;
	ae::Map< const ae::Type*, const ae::Var* > m_meshResourceVars;
	ae::Map< const ae::Type*, const ae::Var* > m_meshVisibleVars;
	ae::Map< const ae::Type*, ae::Str32 > m_typeMesh;
	ae::Map< const ae::Type*, bool > m_typeInvisible;
	
	ae::Array< EditorConnection* > connections;
	uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
	
	struct SelectRef
	{
		bool enabled = false;
		ae::Object* component = nullptr;
		const ae::Var* componentVar = nullptr;
		int32_t varIdx = -1;
		
		EditorObjectId pending = kInvalidEditorObjectId;
	};
	SelectRef m_selectRef;
};

class EditorProgram
{
public:
	EditorProgram( const ae::Tag& tag, const EditorParams& params, Editor* client ) :
		m_tag( tag ),
		editor( tag, client ),
		params( params ),
		m_meshes( tag )
	{}
	void Initialize();
	void Terminate();
	void Run();
	float GetDt() const { return m_dt; }
	
	ae::RectInt GetRenderRect() const;
	float GetAspectRatio() const;
	ae::Matrix4 GetViewToProj() const { return m_viewToProj; }
	ae::Matrix4 GetWorldToView() const { return m_worldToView; }
	ae::Matrix4 GetWorldToProj() const { return m_worldToProj; }
	ae::Matrix4 GetProjToWorld() const { return m_projToWorld; }
	void SetFOV( float fov ) { m_fov = fov; }
	float GetFOV() const { return m_fov; }
	ae::Vec3 GetMouseRay() const { return m_mouseRay; } // Is normalized
	EditorServerMesh* GetMesh( const char* resourceId );
	void UnloadMeshes();
	
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	aeImGui ui;
	ae::DebugCamera camera;
	ae::DebugLines debugLines;
	EditorServer editor;

	const EditorParams params;
	
	// Serialization
	class Serializer : public ae::Var::Serializer
	{
	public:
		Serializer( EditorProgram* program ) : program( program ) { ae::Var::SetSerializer( this ); }
		std::string ObjectPointerToString( const ae::Object* obj ) const override;
		bool StringToObjectPointer( const char* pointerVal, ae::Object** objOut ) const override;
		EditorProgram* program = nullptr;
	} serializer = this;
	
private:
	const ae::Tag m_tag;
	float m_dt;
	ae::Matrix4 m_viewToProj = ae::Matrix4::Identity();
	ae::Matrix4 m_worldToView = ae::Matrix4::Identity();
	ae::Matrix4 m_worldToProj = ae::Matrix4::Identity();
	ae::Matrix4 m_projToWorld = ae::Matrix4::Identity();
	float m_fov = 0.46f; // 28mm camera 65.5 degree horizontal fov
	ae::Vec3 m_mouseRay = ae::Vec3( 0.0f );
	float m_barWidth = 0.0f;
	
	ae::RenderTarget m_gameTarget;
	ae::Map< std::string, EditorServerMesh* > m_meshes; // @TODO: Dynamic ae::Str
public:
	ae::VertexData m_introMesh;
	ae::VertexData m_quad;
	ae::Shader m_introShader;
	ae::Shader m_meshShader;
	ae::Shader m_iconShader;
	ae::Texture2D m_cogTexture;
};

//------------------------------------------------------------------------------
// EditorServerMesh member functions
//------------------------------------------------------------------------------
void EditorServerMesh::Initialize( const ae::Tag& tag, const ae::EditorMesh* _mesh )
{
	ae::Array< Vertex > vertices = tag;
	
	if ( _mesh->indices.Length() )
	{
		uint32_t triangleCount = _mesh->indices.Length() / 3;
		vertices.Reserve( triangleCount * 3 );
		for ( uint32_t i = 0; i < triangleCount; i++ )
		{
			ae::Vec3 p0 = _mesh->verts[ _mesh->indices[ i * 3 ] ];
			ae::Vec3 p1 = _mesh->verts[ _mesh->indices[ i * 3 + 1 ] ];
			ae::Vec3 p2 = _mesh->verts[ _mesh->indices[ i * 3 + 2 ] ];
			ae::Vec4 n( ( p2 - p1 ).Cross( p0 - p1 ).SafeNormalizeCopy(), 0.0f );
			vertices.Append( { ae::Vec4( p0, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p1, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p2, 1.0f ), n } );
		}
	}
	else
	{
		uint32_t triangleCount = _mesh->verts.Length() / 3;
		vertices.Reserve( triangleCount * 3 );
		for ( uint32_t i = 0; i < triangleCount; i++ )
		{
			ae::Vec3 p0 = _mesh->verts[ i * 3 ];
			ae::Vec3 p1 = _mesh->verts[ i * 3 + 1 ];
			ae::Vec3 p2 = _mesh->verts[ i * 3 + 2 ];
			ae::Vec4 n( ( p2 - p1 ).Cross( p0 - p1 ).SafeNormalizeCopy(), 0.0f );
			vertices.Append( { ae::Vec4( p0, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p1, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p2, 1.0f ), n } );
		}
	}
	
	data.Initialize( sizeof( Vertex ), 0, vertices.Length(), 0, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	data.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, position ) );
	data.AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( Vertex, normal ) );
	data.SetVertices( vertices.Begin(), vertices.Length() );
	data.Upload();
	
	collision.Clear();
	collision.AddIndexed(
		ae::Matrix4::Identity(),
		_mesh->verts.Begin()[ 0 ].data,
		_mesh->verts.Length(),
		sizeof( *_mesh->verts.Begin() ),
		_mesh->indices.Begin(),
		_mesh->indices.Length(),
		sizeof( *_mesh->indices.Begin() )
	);
	collision.BuildBVH();
}

//------------------------------------------------------------------------------
// EditorProgram member functions
//------------------------------------------------------------------------------
void EditorProgram::Initialize()
{
	AE_INFO( "Editor Initialize (port: #)", params.port );

	window.Initialize( 1600, 1200, false, true );
	window.SetTitle( "ae" );
	window.SetMaximized( true );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	ui.Initialize();
	camera.Initialize( params.worldUp, ae::Vec3( 0.0f ), ae::Vec3( 10.0f ) );
	camera.SetEditorControls( true );
	debugLines.Initialize( 20480 );
	debugLines.SetXRayEnabled( false );
	editor.Initialize( this );
	
	const char* kVertShader = "\
		AE_UNIFORM mat4 u_worldToProj;\
		AE_UNIFORM vec4 u_color;\
		AE_IN_HIGHP vec4 a_position;\
		AE_IN_HIGHP vec4 a_color;\
		AE_OUT_HIGHP vec4 v_color;\
		void main()\
		{\
			v_color = a_color * u_color;\
			gl_Position = u_worldToProj * a_position;\
		}";

	const char* kFragShader = "\
		AE_IN_HIGHP vec4 v_color;\
		void main()\
		{\
			AE_COLOR = v_color;\
		}";
	m_introShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	m_introShader.SetDepthTest( true );
	m_introShader.SetDepthWrite( true );
	m_introShader.SetBlending( true );
	m_introShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

	struct Vertex
	{
		ae::Vec4 pos;
		ae::Vec4 color;
	};
	Vertex kCubeVerts[] =
	{
		{ ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA() },
		{ ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoOrange().GetLinearRGBA() },
		{ ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoYellow().GetLinearRGBA() },
		{ ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
		{ ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
		{ ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
		{ ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoPink().GetLinearRGBA() },
		{ ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
	};
	uint16_t kCubeIndices[] =
	{
		4, 5, 6, 4, 6, 7, // Top
		0, 3, 1, 1, 3, 2, // Bottom
		0, 4, 3, 3, 4, 7, // Left
		1, 2, 6, 6, 5, 1, // Right
		6, 2, 3, 6, 3, 7, // Back
		0, 1, 5, 0, 5, 4 // Front
	};
	m_introMesh.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	m_introMesh.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
	m_introMesh.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
	m_introMesh.SetVertices( kCubeVerts, countof( kCubeVerts ) );
	m_introMesh.SetIndices( kCubeIndices, countof( kCubeIndices ) );
	m_introMesh.Upload();
	
	const char* meshVertShader = R"(
		AE_UNIFORM mat4 u_localToProj;
		AE_UNIFORM mat4 u_normalToWorld;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_normal;
		AE_OUT_HIGHP vec4 v_normal;
		void main()
		{
			v_normal = u_normalToWorld * a_normal;
			gl_Position = u_localToProj * a_position;
		})";
	const char* meshFragShader = R"(
		AE_UNIFORM vec3 u_lightDir;
		AE_UNIFORM vec4 u_color;
		AE_IN_HIGHP vec4 v_normal;
		void main()
		{
			float light = dot( -u_lightDir, normalize( v_normal.xyz ) );
			light = 0.6 + 0.4 * max( 0.0, light );
			AE_COLOR.rgb = u_color.rgb * light;
			AE_COLOR.a = u_color.a;
		})";
	m_meshShader.Initialize( meshVertShader, meshFragShader, nullptr, 0 );
	m_meshShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );
	
	struct QuadVertex
	{
		ae::Vec4 pos;
		ae::Vec2 uv;
	};
	QuadVertex quadVerts[] =
	{
		{ ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 1.0f ) },
		{ ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 0.0f ) },
		{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 0.0f ) },
		{ ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 1.0f ) },
		{ ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 0.0f ) },
		{ ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 1.0f ) }
	};
	m_quad.Initialize( sizeof( *quadVerts ), 0, countof( quadVerts ), 0, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	m_quad.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( QuadVertex, pos ) );
	m_quad.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( QuadVertex, uv ) );
	m_quad.SetVertices( quadVerts, countof( quadVerts ) );
	m_quad.Upload();
	
	const char* iconVertexShader = R"(
		AE_UNIFORM mat4 u_worldToProj;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec2 a_uv;
		AE_OUT_HIGHP vec2 v_uv;
		void main()
		{
			v_uv = a_uv;
			gl_Position = u_worldToProj * a_position;
		})";
	const char* iconFragShader = R"(
		AE_UNIFORM sampler2D u_tex;
		AE_UNIFORM vec4 u_color;
		AE_IN_HIGHP vec2 v_uv;
		void main()
		{
			float a = length( v_uv * 2.0 - vec2( 1.0 ) );
			a -= 0.95;
			a /= 0.05;
			a = 1.0 - a;
			a *= u_color.a;
			a = clamp( a, 0.0, 1.0 );
			float r = 0.4 + 0.6 * AE_TEXTURE2D( u_tex, v_uv ).r;
			AE_COLOR.rgb = u_color.rgb * r * a;
			AE_COLOR.a = a;
		})";
	m_iconShader.Initialize( iconVertexShader, iconFragShader, nullptr, 0 );
	m_iconShader.SetDepthTest( true );
	m_iconShader.SetDepthWrite( false );
	m_iconShader.SetBlending( true );
	m_iconShader.SetBlendingPremul( true );
	m_iconShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );
	
	AE_STATIC_ASSERT( sizeof(kCogTextureData) == kCogTextureDataSize * kCogTextureDataSize );
	m_cogTexture.Initialize( kCogTextureData, kCogTextureDataSize, kCogTextureDataSize, ae::Texture::Format::R8, ae::Texture::Type::Uint8, ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp, true );
}

void EditorProgram::Terminate()
{
	AE_INFO( "Terminate" );
	UnloadMeshes();
	m_gameTarget.Terminate();
	editor.Terminate( this );
	debugLines.Terminate();
	ui.Terminate();
	input.Terminate();
	render.Terminate();
	window.Terminate();
}

void EditorProgram::Run()
{
	float r0 = 0.0f;
	float r1 = 0.0f;
	
	AE_INFO( "Run" );
	while ( !input.quit )
	{
		m_dt = timeStep.GetDt();
		if ( m_dt > 0.0f && m_dt < timeStep.GetTimeStep() * 4.0f )
		{
			m_dt = timeStep.GetDt();
		}
		else
		{
			m_dt = 0.00001f;
		}

		input.Pump();
		ui.NewFrame( &render, &input, GetDt() );
		ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );
		static bool s_once = true;
		static ImGuiID dock_left_id;
		if ( s_once )
		{
			dock_left_id = ImGui::DockBuilderSplitNode( mainDockSpace, ImGuiDir_Left, 0.2f, nullptr, &mainDockSpace );
			ImGui::DockBuilderDockWindow( "Dev", dock_left_id );
			
			ImGuiDockNode* node = ImGui::DockBuilderGetNode( dock_left_id );
			node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			
			s_once = false;
		}

		editor.Update( this );

		r0 += timeStep.GetDt() * 0.6f;
		r1 += timeStep.GetDt() * 0.75f;
		
		camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse );
		camera.Update( &input, GetDt() );
		
		if ( ImGui::Begin( "Dev" ) )
		{
			editor.ShowUI( this );
			ImGui::End();
		}
		if ( const ImGuiWindow* imWin = ImGui::FindWindowByName( "Dev" ) )
		{
			m_barWidth = imWin->Size.x * ImGui::GetIO().DisplayFramebufferScale.x;
		}

		ae::Int2 oldSize( m_gameTarget.GetWidth(), m_gameTarget.GetHeight() );
		ae::Int2 targetSize = GetRenderRect().GetSize();
		if ( oldSize != targetSize )
		{
			m_gameTarget.Initialize( targetSize.x, targetSize.y );
			m_gameTarget.AddTexture( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
			m_gameTarget.AddDepth( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
		}
		m_gameTarget.Activate();
		m_gameTarget.Clear( ae::Color::SRGB8( 85, 66, 82 ) );

		m_worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		m_viewToProj = ae::Matrix4::ViewToProjection( GetFOV(), GetAspectRatio(), 0.25f, kEditorViewDistance );
		m_worldToProj = m_viewToProj * m_worldToView;
		m_projToWorld = m_worldToProj.GetInverse();
		
		// Update mouse ray
		{
			const float scaleFactor = window.GetScaleFactor();
			const ae::RectInt renderRectInt = GetRenderRect();
			const ae::Rect renderRect = ae::Rect::FromPoints(
				ae::Vec2( renderRectInt.x / scaleFactor, renderRectInt.y / scaleFactor ),
				ae::Vec2( ( renderRectInt.x + renderRectInt.w ) / scaleFactor, ( renderRectInt.y + renderRectInt.h ) / scaleFactor )
			);
			
			ae::Vec2 mouseGameWindowPixelPos( input.mouse.position );
			mouseGameWindowPixelPos -= ae::Vec2( renderRect.GetMin() );
			
			ae::Vec2 mouseGameWindowNdcPos = mouseGameWindowPixelPos / ae::Vec2( renderRect.GetSize() );
			mouseGameWindowNdcPos *= 2.0f;
			mouseGameWindowNdcPos -= ae::Vec2( 1.0f );
			
			ae::Vec4 worldPos( mouseGameWindowNdcPos, 1.0f, 1.0f );
			worldPos = m_projToWorld * worldPos;
			worldPos /= worldPos.w;
			m_mouseRay = ( worldPos.GetXYZ() - camera.GetPosition() ).SafeNormalizeCopy();
		}
		
		if ( !editor.GetObjectCount() )
		{
			ae::UniformList uniformList;
			ae::Matrix4 modelToWorld = ae::Matrix4::RotationX( r0 ) * ae::Matrix4::RotationZ( r1 );
			uniformList.Set( "u_worldToProj", m_viewToProj * m_worldToView * modelToWorld );
			uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
			m_introMesh.Render( &m_introShader, uniformList );
		}

		editor.Render( this );
		
		debugLines.Render( m_viewToProj * m_worldToView );

		render.Activate();
		render.Clear( ae::Color::Black() );
		
		float devNdc = ( m_barWidth / render.GetWidth() ) * 2.0f;
		ae::Rect ndcRect = ae::Rect::FromCenterAndSize( ae::Vec2( devNdc - 1.0f + ( 2.0f - devNdc ) * 0.5f, 0.0f ), ae::Vec2( 2.0f - devNdc, 2.0f ) );
		m_gameTarget.Render2D( 0, ndcRect, 0.0f );
		ui.Render();
		render.Present();

		timeStep.Wait();
	}
}

ae::RectInt EditorProgram::GetRenderRect() const
{
	return ae::RectInt( m_barWidth, 0, render.GetWidth() - m_barWidth, render.GetHeight() );
}

float EditorProgram::GetAspectRatio() const
{
	ae::RectInt rect = GetRenderRect();
	return rect.w / (float)rect.h;
}

EditorServerMesh* EditorProgram::GetMesh( const char* resourceId )
{
	EditorServerMesh* mesh = m_meshes.Get( resourceId, nullptr );
	if ( !mesh && params.loadMeshFn )
	{
		ae::EditorMesh temp = params.loadMeshFn( resourceId );
		if ( temp.verts.Length() )
		{
			mesh = ae::New< EditorServerMesh >( m_tag, m_tag );
			mesh->Initialize( m_tag, &temp );
			m_meshes.Set( resourceId, mesh );
		}
	}
	return mesh;
}

void EditorProgram::UnloadMeshes()
{
	for ( uint32_t i = 0; i < m_meshes.Length(); i++ )
	{
		ae::Delete( m_meshes.GetValue( i ) );
	}
	m_meshes.Clear();
}

//------------------------------------------------------------------------------
// ae::EditorMesh member functions
//------------------------------------------------------------------------------
ae::EditorMesh::EditorMesh( const ae::Tag& tag ) :
	verts( tag ),
	indices( tag )
{}

void ae::EditorMesh::Load( const ae::OBJFile& file )
{
	verts.Clear();
	indices.Clear();
	verts.Reserve( file.vertices.Length() );
	indices.Reserve( file.indices.Length() );
	for ( const auto& vert : file.vertices )
	{
		verts.Append( vert.position.GetXYZ() );
	}
	for ( auto index : file.indices )
	{
		indices.Append( index );
	}
}

//------------------------------------------------------------------------------
// Editor member functions
//------------------------------------------------------------------------------
Editor::Editor( const ae::Tag& tag ) :
	m_tag( tag ),
	m_level( tag ),
	m_sock( tag )
{}

Editor::~Editor()
{
#if !_AE_EMSCRIPTEN_
	m_sock.Disconnect();
#endif
}

void Editor::Initialize( const EditorParams& params )
{
	AE_ASSERT( params.argc && params.argv );
	AE_ASSERT( params.worldUp == ae::Axis::Z || params.worldUp == ae::Axis::Y );
	AE_ASSERT( params.port );
	m_params = params;
	m_fileSystem.Initialize( params.dataDir.c_str(), "ae", "editor" );	
	if ( params.argc >= 2 && strcmp( params.argv[ 1 ], "ae_editor" ) == 0 )
	{
		EditorProgram program( m_tag, params, this );
		program.Initialize();
		if ( params.argc >= 3 )
		{
			program.editor.OpenLevel( &program, params.argv[ 2 ] );
		}
		program.Run();
		program.Terminate();
		exit( 0 );
	}
	m_Connect();
}

void Editor::Launch()
{
	m_Connect();
	if ( !m_sock.IsConnected() )
	{
#if _AE_APPLE_
		if ( !fork() )
		{
			char* execArgs[] = { m_params.argv[ 0 ], (char*)"ae_editor", (char*)m_level.filePath.c_str(), nullptr };
			execv( m_params.argv[ 0 ], execArgs );
		}
#elif _AE_WINDOWS_
		STARTUPINFOA startupInfo;
		memset( &startupInfo, 0, sizeof( startupInfo ) );
		startupInfo.cb = sizeof( startupInfo );
		PROCESS_INFORMATION procInfo;
		char args[ 256 ];
		args[ 0 ] = 0;
		strlcat( args, m_params.argv[ 0 ], sizeof(args) );
		strlcat( args, " ae_editor ", sizeof(args) );
		strlcat( args, m_level.filePath.c_str(), sizeof(args) );
		CreateProcessA(
			m_params.argv[ 0 ],
			args,
			nullptr,
			nullptr,
			false,
			NORMAL_PRIORITY_CLASS,
			nullptr,
			nullptr,
			&startupInfo,
			&procInfo );
		// Don't need these so clean them up right away
		CloseHandle( procInfo.hProcess );
		CloseHandle( procInfo.hThread );
#endif
	}
}

void Editor::Update()
{
	m_Read();

	if ( m_params.port == 0 )
	{
		return;
	}
	m_Connect();
	
	uint32_t msgLength = 0;
	while ( ( msgLength = m_sock.ReceiveMsg( m_msgBuffer, sizeof(m_msgBuffer) ) ) )
	{
		EditorMsg msgType = EditorMsg::None;
		ae::BinaryStream rStream = ae::BinaryStream::Reader( m_msgBuffer, sizeof(m_msgBuffer) );
		rStream.SerializeRaw( msgType );
		switch ( msgType )
		{
			case EditorMsg::Modification:
			{
				EditorObjectId entity;
				ae::Matrix4 transform;
				rStream.SerializeUint32( entity );
				rStream.SerializeRaw( transform );
				AE_ASSERT( rStream.IsValid() );
				EditorObject* levelObj = m_level.objects.TryGet( entity );
				if ( levelObj )
				{
					levelObj->transform = transform;
				}
				break;
			}
			case EditorMsg::Load:
			{
				ae::Str256 levelPath;
				rStream.SerializeString( levelPath );
				QueueRead( levelPath.c_str() );
				break;
			}
			default:
				break;
		}
	}
}

bool Editor::Write() const
{
	if ( !m_level.filePath.Length() )
	{
		return false;
	}

	rapidjson::Document document;
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	document.SetObject();
	{
		uint32_t objectCount = m_level.objects.Length();
		rapidjson::Value jsonObjects( rapidjson::kArrayType );
		jsonObjects.Reserve( objectCount, allocator );

		for ( uint32_t objIdx = 0; objIdx < objectCount; objIdx++ )
		{
			const EditorObject& levelObject = m_level.objects.GetValue( objIdx );
			rapidjson::Value jsonObject( rapidjson::kObjectType );
			jsonObject.AddMember( "id", levelObject.id, allocator );
			if ( levelObject.name.Length() )
			{
				jsonObject.AddMember( "name", rapidjson::StringRef( levelObject.name.c_str() ), allocator );
			}
			rapidjson::Value transformJson;
			auto transformStr = ae::ToString( levelObject.transform );
			transformJson.SetString( transformStr.c_str(), allocator );
			jsonObject.AddMember( "transform", transformJson, allocator );
			
			rapidjson::Value jsonComponents( rapidjson::kObjectType );
			for ( const EditorComponent& levelComponent : levelObject.components )
			{
				const ae::Type* type = ae::GetTypeByName( levelComponent.type.c_str() );
				rapidjson::Value jsonComponent( rapidjson::kObjectType );
				uint32_t varCount = type->GetVarCount( true );
				for ( uint32_t i = 0; i < varCount; i++ )
				{
					const ae::Var* var = type->GetVarByIndex( i, true );
					auto k = rapidjson::StringRef( var->GetName() );
					if ( var->IsArray() )
					{
						int32_t arrayLen = levelComponent.members.GetInt( var->GetName(), 0 );
						if ( arrayLen )
						{
							rapidjson::Value jsonArray( rapidjson::kArrayType );
							jsonArray.Reserve( arrayLen, allocator );
							for ( uint32_t arrIdx = 0; arrIdx < arrayLen; arrIdx++ )
							{
								ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
								const char* value = levelComponent.members.GetString( key.c_str(), nullptr );
								AE_ASSERT( value );
								jsonArray.PushBack( rapidjson::StringRef( value ), allocator );
							}
							jsonComponent.AddMember( k, jsonArray, allocator );
						}
					}
					else if ( const char* value = levelComponent.members.GetString( var->GetName(), nullptr ) )
					{
						jsonComponent.AddMember( k, rapidjson::StringRef( value ), allocator );
					}
				}
				jsonComponents.AddMember( rapidjson::StringRef( levelComponent.type.c_str() ), jsonComponent, allocator );
			}
			jsonObject.AddMember( "components", jsonComponents, allocator );
			jsonObjects.PushBack( jsonObject, allocator );
		}

		document.AddMember( "objects", jsonObjects, allocator );
	}
	
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter< rapidjson::StringBuffer > writer( buffer );
	document.Accept( writer );

	uint32_t writtenBytes = m_fileSystem.Write( ae::FileSystem::Root::Data, m_level.filePath.c_str(), buffer.GetString(), buffer.GetSize(), false );
	AE_ASSERT( writtenBytes == 0 || writtenBytes == buffer.GetSize() );
	return writtenBytes != 0;
}

void Editor::QueueRead( const char* levelPath )
{
	if ( m_file )
	{
		AE_WARN( "Cancelling level read '#'", m_file->GetUrl() );
		m_fileSystem.Destroy( m_file );
	}
	AE_INFO( "Queuing level load '#'", levelPath );
	m_file = m_fileSystem.Read( ae::FileSystem::Root::Data, levelPath, 2.0f );
}

void Editor::m_Read()
{
	if ( !m_file || m_file->GetStatus() == ae::File::Status::Pending )
	{
		return;
	}

	RunOnDestroy destroyFile{ [this]()
	{
		m_fileSystem.Destroy( m_file );
		m_file = nullptr;
	} };

	uint32_t fileSize = m_file->GetLength();
	if ( !fileSize )
	{
		return;
	}
	
	m_level.objects.Clear();
	
	const char* jsonBuffer = (const char*)m_file->GetData();
	AE_ASSERT( jsonBuffer[ m_file->GetLength() ] == 0 );
	
	rapidjson::Document document;
	AE_ASSERT( !document.Parse( jsonBuffer ).HasParseError() );
	AE_ASSERT( document.IsObject() );
	
	const auto& jsonObjects = document[ "objects" ];
	AE_ASSERT( jsonObjects.IsArray() );
	
	for ( const auto& jsonObject : jsonObjects.GetArray() )
	{
		EditorObjectId entity = jsonObject[ "id" ].GetUint();
		EditorObject& levelObject = m_level.objects.Set( entity, { m_tag } );
		levelObject.id = entity;
		if ( jsonObject.HasMember( "name" ) )
		{
			levelObject.name = jsonObject[ "name" ].GetString();
		}
		levelObject.transform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		for ( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if ( !componentIter.value.IsObject() )
			{
				continue;
			}
			const auto jsonComponent = componentIter.value.GetObject();
			EditorComponent& levelComponent = levelObject.components.Append( m_tag );
			levelComponent.type = componentIter.name.GetString();
			const ae::Type* type = ae::GetTypeByName( levelComponent.type.c_str() );
			if ( !type )
			{
				continue;
			}
			
			uint32_t varCount = type->GetVarCount( true );
			for ( uint32_t varIdx = 0; varIdx < varCount; varIdx++ )
			{
				const ae::Var* var = type->GetVarByIndex( varIdx, true );
				if ( !jsonComponent.HasMember( var->GetName() ) )
				{
					continue;
				}
				const auto& jsonVar = jsonComponent[ var->GetName() ];
				if ( var->IsArray() && jsonVar.IsArray() )
				{
					uint32_t arrIdx = 0;
					const auto& jsonVarArray = jsonVar.GetArray();
					levelComponent.members.SetUint( var->GetName(), jsonVarArray.Size() );
					for ( const auto& jsonVarArrayValue : jsonVarArray )
					{
						ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
						levelComponent.members.SetString( key.c_str(), jsonVarArrayValue.GetString() );
						arrIdx++;
					}
				}
				else if ( !jsonVar.IsObject() && !jsonVar.IsArray() )
				{
					levelComponent.members.SetString( var->GetName(), jsonVar.GetString() );
				}
			}
		}
	}
	
	AE_INFO( "Read level '#'", m_file->GetUrl() );
	m_level.filePath = m_file->GetUrl();
	m_levelSeq++;
}

void Editor::m_Connect()
{
#if !_AE_EMSCRIPTEN_
	if ( m_params.port && !m_sock.IsConnected() )
	{
		m_sock.Connect( ae::Socket::Protocol::TCP, "localhost", m_params.port );
	}
#endif
}

//------------------------------------------------------------------------------
// EditorServerObject member functions
//------------------------------------------------------------------------------
void EditorServerObject::Initialize( EditorObjectId entity, ae::Matrix4 transform )
{
	this->entity = entity;
	this->m_transform = transform;
}

void EditorServerObject::Terminate()
{
	for ( ae::Object* component : components )
	{
		component->~Object();
		ae::Free( component );
	}
	components.Clear();
}

void EditorServerObject::SetTransform( const ae::Matrix4& transform, EditorProgram* program )
{
//	AE_ASSERT( entity != kInvalidEditorObjectId );
//	program->registry.GetComponent< Transform >( entity );
	if ( m_transform != transform )
	{
		m_transform = transform;
		m_dirty = true;
	}
}

ae::Matrix4 EditorServerObject::GetTransform( const EditorProgram* program ) const
{
//	AE_ASSERT( entity != kInvalidEditorObjectId );
//	Registry* registry = const_cast< Registry* >( &program->registry );
//	return registry->GetComponent< Transform >( entity ).transform;
	return m_transform;
}

void EditorServerObject::HandleVarChange( EditorProgram* program, ae::Object* component, const ae::Type* type, const ae::Var* var )
{
	if ( var == program->editor.GetMeshResourceVar( type ) )
	{
		auto varStr = var->GetObjectValueAsString( component );
		if ( varStr != "" )
		{
			mesh = program->GetMesh( varStr.c_str() );
		}
		else
		{
			mesh = nullptr;
		}
	}
	else if ( var == program->editor.GetMeshVisibleVar( type ) )
	{
		var->GetObjectValue( component, &opaque );
	}
}

//------------------------------------------------------------------------------
// EditorConnection member functions
//------------------------------------------------------------------------------
EditorConnection::~EditorConnection()
{
	AE_ASSERT( !sock );
}

void EditorConnection::Destroy( EditorServer* editor )
{
	editor->sock.Destroy( sock );
	sock = nullptr;
}

//------------------------------------------------------------------------------
// EditorServer member functions
//------------------------------------------------------------------------------
void EditorServer::Initialize( EditorProgram* program )
{
	uint32_t typeCount = ae::GetTypeCount();
	for ( uint32_t i = 0; i < typeCount; i++ )
	{
		const ae::Type* type = ae::GetTypeByIndex( i );
		int32_t resourcePropIdx = type->GetPropertyIndex( "ae_mesh_resource" );
		if ( resourcePropIdx >= 0 )
		{
			const char* mustRegisterErr = "Must register a mesh resource member variable with AE_REGISTER_CLASS_PROPERTY_VALUE( #, ae_mesh_resource, memberVar );";
			AE_ASSERT_MSG( type->GetPropertyValueCount( resourcePropIdx ), mustRegisterErr, type->GetName() );
			const char* varName = type->GetPropertyValue( resourcePropIdx, 0 );
			AE_ASSERT_MSG( varName[ 0 ], mustRegisterErr, type->GetName() );
			const ae::Var* var = type->GetVarByName( varName, false );
			AE_ASSERT_MSG( var, "Type '#' does not have a member variable named '#'", type->GetName(), varName );
			m_meshResourceVars.Set( type, var );
		}
		
		int32_t visiblePropIdx = type->GetPropertyIndex( "ae_mesh_visible" );
		if ( visiblePropIdx >= 0 )
		{
			const char* mustRegisterErr = "Must register a mesh resource member variable with AE_REGISTER_CLASS_PROPERTY_VALUE( #, ae_mesh_visible, memberVar );";
			AE_ASSERT_MSG( type->GetPropertyValueCount( visiblePropIdx ), mustRegisterErr, type->GetName() );
			const char* varName = type->GetPropertyValue( visiblePropIdx, 0 );
			AE_ASSERT_MSG( varName[ 0 ], mustRegisterErr, type->GetName() );
			const ae::Var* var = type->GetVarByName( varName, false );
			AE_ASSERT_MSG( var, "Type '#' does not have a member variable named '#'", type->GetName(), varName );
			m_meshVisibleVars.Set( type, var );
		}
		
		int32_t depPropIdx = type->GetPropertyIndex( "ae_editor_dep" );
		if ( depPropIdx >= 0 )
		{
			uint32_t depCount = type->GetPropertyValueCount( depPropIdx );
			for ( uint32_t i = 0; i < depCount; i++ )
			{
				const char* depName = type->GetPropertyValue( depPropIdx, i );
				const ae::Type* depType = ae::GetTypeByName( depName );
				AE_ASSERT_MSG( depType, "Type '#' has invalid dependency '#'", type->GetName(), depName );
				AE_ASSERT_MSG( depType->HasProperty( "ae_editor_type" ), "Type '#' has dependency '#' which is not an editor type. It must have property 'ae_editor_type'.", type->GetName(), depName );
			}
		}
		
		int32_t typeMeshIdx = type->GetPropertyIndex( "ae_type_mesh" );
		if ( typeMeshIdx >= 0 && type->GetPropertyValueCount( typeMeshIdx ) )
		{
			m_typeMesh.Set( type, type->GetPropertyValue( typeMeshIdx, 0 ) );
		}
		
		int32_t typeInvisibleIdx = type->GetPropertyIndex( "ae_type_invisible" );
		if ( typeInvisibleIdx >= 0 )
		{
			m_typeInvisible.Set( type, true );
		}
	}
}

void EditorServer::Terminate( EditorProgram* program )
{
	for ( EditorConnection* conn : connections )
	{
		conn->Destroy( this );
		ae::Delete( conn );
	}
	connections.Clear();
}

void EditorServer::Update( EditorProgram* program )
{
	client->m_Read(); // @TODO: This only partially updates the client
	
	if ( !sock.IsListening() )
	{
		sock.Listen( ae::Socket::Protocol::TCP, false, program->params.port, 8 );
	}
	while ( ae::Socket* newConn = sock.Accept() )
	{
		AE_INFO( "ae::Editor client connected from #:#", newConn->GetResolvedAddress(), newConn->GetPort() );
		EditorConnection* editorConn = connections.Append( ae::New< EditorConnection >( m_tag ) );
		editorConn->sock = newConn;
	}
	AE_ASSERT( connections.Length() == sock.GetConnectionCount() );
	
	for ( uint32_t i = 0; i < m_objects.Length(); i++ )
	{
		EditorServerObject* editorObj = m_objects.GetValue( i );
		if ( editorObj->IsDirty() )
		{
			ae::Matrix4 transform = editorObj->GetTransform( program );
			ae::BinaryStream wStream = ae::BinaryStream::Writer( m_msgBuffer, sizeof(m_msgBuffer) );
			wStream.SerializeRaw( EditorMsg::Modification );
			wStream.SerializeUint32( editorObj->entity );
			wStream.SerializeRaw( transform );
			AE_ASSERT( wStream.IsValid() );
			for ( EditorConnection* conn : connections )
			{
				conn->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
			}
			editorObj->ClearDirty();
		}
	}
	
	for ( EditorConnection* (&conn) : connections )
	{
		if ( conn->sock->IsConnected() )
		{
			conn->sock->SendAll();
		}
		else
		{
			AE_INFO( "ae::Editor client #:# disconnected", conn->sock->GetResolvedAddress(), conn->sock->GetPort() );
			conn->Destroy( this );
			ae::Delete( conn );
			conn = nullptr;
		}
	}
	connections.RemoveAllFn( []( const EditorConnection* c ){ return !c; } );

	if ( m_levelSeq_HACK < client->GetLevelChangeSeq() )
	{
		AE_INFO( "Loading '#'", client->GetLevel()->filePath );
		if ( m_Load( program ) )
		{
			AE_INFO( "Loaded '#'", client->GetLevel()->filePath );
			program->window.SetTitle( client->GetLevel()->filePath.c_str() );
			m_levelSeq_HACK = client->GetLevelChangeSeq();
			return;
		}
		else
		{
			AE_INFO( "Failed to load level '#'", client->GetLevel()->filePath );
		}
	}
}

void EditorServer::Render( EditorProgram* program )
{
	// Constants
	const ae::Vec3 camPos = program->camera.GetPosition();
	const ae::Vec3 camUp = program->camera.GetLocalUp();
	const ae::Matrix4 worldToProj = program->GetWorldToProj();
	const ae::Vec3 lightDir = ( ( program->params.worldUp == ae::Axis::Z ) ? ae::Vec3( -2.0f, -3.0f, -4.0f ) : ae::Vec3( -2.0f, -4.0f, -3.0f ) ).SafeNormalizeCopy();
	
	// Categories
	struct RenderObj
	{
		const EditorServerObject* obj = nullptr;
		float distanceSq = INFINITY;
	};
	ae::Array< RenderObj > opaqueObjects = m_tag;
	ae::Array< RenderObj > transparentObjects = m_tag;
	ae::Array< RenderObj > logicObjects = m_tag;
	
	// Split up render passes
	uint32_t editorObjectCount = m_objects.Length();
	for ( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* obj = m_objects.GetValue( i );
		float distanceSq = ( camPos - obj->GetTransform( program ).GetTranslation() ).LengthSquared();
		if ( obj->mesh && obj->opaque ) { opaqueObjects.Append( { obj, distanceSq } ); }
		else if ( obj->mesh && GetShowInvisible() ) { transparentObjects.Append( { obj, distanceSq } ); }
		else { logicObjects.Append( { obj, distanceSq } ); }
	}
	
	// Opaque and transparent meshes helper
	auto renderMesh = [program, worldToProj, lightDir]( const RenderObj& renderObj, ae::Color color )
	{
		const EditorServerObject& obj = *renderObj.obj;
		ae::Matrix4 transform = obj.GetTransform( program );
		ae::UniformList uniformList;
		uniformList.Set( "u_localToProj", worldToProj * transform );
		uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
		uniformList.Set( "u_lightDir", lightDir );
		uniformList.Set( "u_color", color.GetLinearRGBA() );
		obj.mesh->data.Render( &program->m_meshShader, uniformList );
	};
	
	// Opaque objects
	std::sort( opaqueObjects.begin(), opaqueObjects.end(), []( const RenderObj& a, const RenderObj& b )
	{
		return a.distanceSq < b.distanceSq;
	} );
	program->m_meshShader.SetDepthTest( true );
	program->m_meshShader.SetDepthWrite( true );
	program->m_meshShader.SetBlending( false );
	for ( const RenderObj& renderObj : opaqueObjects )
	{
		const EditorServerObject& obj = *renderObj.obj;
		ae::Color color = m_GetColor( obj.entity, false );
		renderMesh( renderObj, color );
	}
	
	// Logic objects
	std::sort( logicObjects.begin(), logicObjects.end(), []( const RenderObj& a, const RenderObj& b )
	{
		return a.distanceSq > b.distanceSq;
	} );
	for ( const RenderObj& renderObj : logicObjects )
	{
		const EditorServerObject& obj = *renderObj.obj;
		const ae::Matrix4 transform = obj.GetTransform( program );
		const ae::Color color = m_GetColor( obj.entity, false );
		ae::UniformList uniformList;
		ae::Vec3 objPos = transform.GetTranslation();
		ae::Vec3 toCamera = camPos - objPos;
		ae::Matrix4 modelToWorld = ae::Matrix4::Rotation( ae::Vec3(0,0,1), ae::Vec3(0,1,0), toCamera, camUp );
		modelToWorld.SetTranslation( objPos );
		uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
		uniformList.Set( "u_tex", &program->m_cogTexture );
		uniformList.Set( "u_color", color.GetLinearRGBA() );
		program->m_quad.Render( &program->m_iconShader, uniformList );
		program->debugLines.AddOBB( transform, color );
	}
	
	// Transparent objects
	std::sort( transparentObjects.begin(), transparentObjects.end(), []( const RenderObj& a, const RenderObj& b )
	{
		return a.distanceSq > b.distanceSq;
	} );
	program->m_meshShader.SetDepthTest( true );
	program->m_meshShader.SetDepthWrite( false );
	program->m_meshShader.SetBlending( true );
	for ( const RenderObj& renderObj : transparentObjects )
	{
		const EditorServerObject& obj = *renderObj.obj;
		ae::Color color = m_GetColor( obj.entity, false ).ScaleA( 0.5f );
		renderMesh( renderObj, color );
	}
}

void EditorServer::ShowUI( EditorProgram* program )
{
	float dt = program->GetDt();
	
	ae::Color cursorColor = ae::Color::PicoGreen();
	ae::Vec3 mouseHover( 0.0f );
	ae::Vec3 mouseHoverNormal( 0, 1, 0 );
	if ( program->camera.GetMode() == ae::DebugCamera::Mode::None
		&& !ImGuizmo::IsUsing() && ( !gizmoOperation || !ImGuizmo::IsOver() ) )
	{
		hoverEntity = m_PickObject( program, cursorColor, &mouseHover, &mouseHoverNormal );
	}
	else
	{
		hoverEntity = kInvalidEditorObjectId;
	}
	
	static float s_hold = 0.0f;
	static ae::Vec2 s_mouseMove( 0.0f );
	if ( !ImGui::GetIO().WantCaptureMouse && program->input.mouse.leftButton && s_hold >= 0.0f )
	{
		s_hold += dt;
		s_mouseMove += ae::Vec2( program->input.mouse.movement );
		
		if ( s_mouseMove.Length() > 4.0f )
		{
			s_hold = -1.0f;
		}
		else if ( hoverEntity )
		{
			cursorColor = ae::Color::PicoOrange();
			if ( s_hold > 0.35f )
			{
				program->camera.Refocus( mouseHover );
				s_hold = -1.0f;
			}
		}
	}
	else if ( !program->input.mouse.leftButton )
	{
		s_hold = 0.0f;
		s_mouseMove = ae::Vec2( 0.0f );
	}

	if ( !program->input.mouse.leftButton && program->input.mousePrev.leftButton // Release
		&& !ImGui::GetIO().WantCaptureMouse && program->camera.GetMode() == ae::DebugCamera::Mode::None )
	{
		if ( m_selectRef.enabled )
		{
			uint32_t matchCount = 0;
			const ae::Object* lastMatch = nullptr;
			
			const ae::Type* refType = m_selectRef.componentVar->GetSubType();
			const EditorServerObject* hoverObj = GetObject( hoverEntity );
			AE_ASSERT( hoverObj );
			for ( const ae::Object* otherComp : hoverObj->components )
			{
				const ae::Type* otherType = ae::GetTypeFromObject( otherComp );
				if ( otherType->IsType( refType ) )
				{
					matchCount++;
					lastMatch = otherComp;
				}
			}
			
			if ( matchCount == 0 )
			{
				AE_INFO( "Invalid selection. No '#' component.", refType->GetName() );
			}
			else if ( matchCount == 1 )
			{
				m_selectRef.componentVar->SetObjectValue( m_selectRef.component, lastMatch, m_selectRef.varIdx );
				m_selectRef = SelectRef();
			}
			else
			{
				m_selectRef.pending = hoverEntity;
			}
		}
		else if ( !hoverEntity && selected )
		{
			AE_INFO( "Deselect Object" );
			selected = kInvalidEditorObjectId;
		}
		else if ( hoverEntity != selected )
		{
			AE_INFO( "Select Object" );
			selected = hoverEntity;
		}
	}
	
	if ( m_selectRef.enabled && m_selectRef.pending )
	{
		ImGui::OpenPopup( "ref_select_popup" );
	}
	if ( ImGui::BeginPopup( "ref_select_popup" ) )
	{
		const ae::Type* refType = m_selectRef.componentVar->GetSubType();
		ImGui::Text( "Select %s", refType->GetName() );
		ImGui::Separator();
		const EditorServerObject* otherObj = GetObject( m_selectRef.pending );
		AE_ASSERT( otherObj );
		for ( const ae::Object* otherComp : otherObj->components )
		{
			const ae::Type* otherType = ae::GetTypeFromObject( otherComp );
			AE_ASSERT( otherType );
			if ( otherType->IsType( refType ) && ImGui::Selectable( otherType->GetName(), false ) )
			{
				m_selectRef.componentVar->SetObjectValue( m_selectRef.component, otherComp, m_selectRef.varIdx );
				m_selectRef = SelectRef();
			}
		}
		ImGui::EndPopup();
	}
	
	m_ShowEditorObject( program, selected, m_GetColor( selected, true ) );
	if ( hoverEntity )
	{
		program->debugLines.AddCircle( mouseHover + mouseHoverNormal * 0.025f, mouseHoverNormal, 0.5f, cursorColor, 8 );
		if ( selected != hoverEntity )
		{
			m_ShowEditorObject( program, hoverEntity, m_GetColor( hoverEntity, true ) );
		}
	}
	
	ae::Vec3 debugRefocus( 0.0f );
	program->debugLines.AddSphere( program->camera.GetFocus(), 0.1f, ae::Color::Green(), 6 );
//	if ( program->camera->GetDebugRefocusTarget( &debugRefocus ) )
//	{
//		program->debugLines.AddSphere( debugRefocus, 0.1f, ae::Color::PicoOrange(), 6 );
//	}
	
	if ( ImGui::GetIO().WantCaptureKeyboard )
	{
		// keyboard captured
	}
	else if ( selected && program->input.Get( ae::Key::F ) && !program->input.GetPrev( ae::Key::F ) )
	{
		program->camera.Refocus( m_objects.Get( selected )->GetTransform( program ).GetTranslation() );
	}
	else if ( program->input.Get( ae::Key::Q ) && !program->input.GetPrev( ae::Key::Q ) )
	{
		gizmoOperation = (ImGuizmo::OPERATION)0;
	}
	else if ( program->input.Get( ae::Key::W ) && !program->input.GetPrev( ae::Key::W ) )
	{
		if ( gizmoOperation == ImGuizmo::TRANSLATE )
		{
			gizmoMode = ( gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
		}
		else
		{
			gizmoOperation = ImGuizmo::TRANSLATE;
		}
	}
	else if ( program->input.Get( ae::Key::E ) && !program->input.GetPrev( ae::Key::E ) )
	{
		if ( gizmoOperation == ImGuizmo::ROTATE )
		{
			gizmoMode = ( gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
		}
		else
		{
			gizmoOperation = ImGuizmo::ROTATE;
		}
	}
	else if ( program->input.Get( ae::Key::R ) && !program->input.GetPrev( ae::Key::R ) )
	{
		gizmoOperation = ImGuizmo::SCALE;
	}
	else if ( program->input.Get( ae::Key::I ) && !program->input.GetPrev( ae::Key::I ) )
	{
		m_showInvisible = !m_showInvisible;
	}
	else if ( program->input.Get( ae::Key::LeftMeta ) )
	{
		if ( program->input.Get( ae::Key::S ) && !program->input.GetPrev( ae::Key::S ) )
		{
			SaveLevel( program, program->input.Get( ae::Key::LeftShift ) );
		}
		else if ( program->input.Get( ae::Key::O ) && !program->input.GetPrev( ae::Key::O ) )
		{
			OpenLevel( program );
		}
		else if ( program->input.Get( ae::Key::H ) && !program->input.GetPrev( ae::Key::H ) )
		{
			if ( selected )
			{
				EditorServerObject* editorObject = m_objects.Get( selected );
				editorObject->hidden = !editorObject->hidden;
			}
			else
			{
				AE_INFO( "No objects selected" );
			}
		}
	}
	
	if ( selected && gizmoOperation )
	{
		const float scaleFactor = program->window.GetScaleFactor();
		const ae::RectInt renderRectInt = program->GetRenderRect();
		const ae::Rect renderRect = ae::Rect::FromPoints(
				ae::Vec2( renderRectInt.x / scaleFactor, renderRectInt.y / scaleFactor ),
				ae::Vec2( ( renderRectInt.x + renderRectInt.w ) / scaleFactor, ( renderRectInt.y + renderRectInt.h ) / scaleFactor )
		);
		
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::Enable( program->camera.GetMode() == ae::DebugCamera::Mode::None );
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::AllowAxisFlip( false );
		ImGuizmo::BeginFrame();
		ImGuizmo::SetRect( renderRect.GetMin().x, renderRect.GetMin().y, renderRect.GetSize().x, renderRect.GetSize().y );
		
		EditorServerObject* selectedObject = m_objects.Get( selected );
		ae::Matrix4 transform = selectedObject->GetTransform( program );
		if ( ImGuizmo::Manipulate(
			program->GetWorldToView().data,
			program->GetViewToProj().data,
			gizmoOperation,
			( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode,
			transform.data
		) )
		{
			selectedObject->SetTransform( transform, program );
		}
	}
	
	if ( m_first )
	{
		ImGui::SetNextItemOpen( true );
	}
	if ( ImGui::TreeNode( "File" ) )
	{
		if ( ImGui::Button( "Open" ) )
		{
			OpenLevel( program );
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Save" ) )
		{
			SaveLevel( program, false );
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Save As" ) )
		{
			SaveLevel( program, true );
		}
		if ( ImGui::Button( "New" ) )
		{
			AE_INFO( "New level" );
			EditorLevel* level = client->GetWritableLevel();
			if ( level )
			{
				level->objects.Clear();
				level->filePath = "";
				client->m_levelSeq++;
			}
			
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Game Load" ) && connections.Length() )
		{
			uint8_t buffer[ kMaxEditorMessageSize ];
			ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
			wStream.SerializeRaw( EditorMsg::Load );
			wStream.SerializeString( client->GetLevel()->filePath );
			for ( uint32_t i = 0; i < connections.Length(); i++ )
			{
				connections[ i ]->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
			}
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if ( m_first )
	{
		ImGui::SetNextItemOpen( true );
	}
	if ( ImGui::TreeNode( "Tools" ) )
	{
		if ( ImGui::RadioButton( "O", gizmoOperation == (ImGuizmo::OPERATION)0 ) )
		{
			gizmoOperation = (ImGuizmo::OPERATION)0;
		}
		ImGui::SameLine();
		if ( ImGui::RadioButton( "T", gizmoOperation == ImGuizmo::TRANSLATE ) )
		{
			gizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if ( ImGui::RadioButton( "R", gizmoOperation == ImGuizmo::ROTATE ) )
		{
			gizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if ( ImGui::RadioButton( "S", gizmoOperation == ImGuizmo::SCALE ) )
		{
			gizmoOperation = ImGuizmo::SCALE;
		}
		if ( ImGui::RadioButton( "World", gizmoMode == ImGuizmo::WORLD ) )
		{
			gizmoMode = ImGuizmo::WORLD;
		}
		ImGui::SameLine();
		if ( ImGui::RadioButton( "Local", gizmoMode == ImGuizmo::LOCAL ) )
		{
			gizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::Checkbox( "Show Invisible", &m_showInvisible );
		
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if ( ImGui::TreeNode( "Operations" ) )
	{
		if ( ImGui::Button( "Create" ) )
		{
			ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetFocus() );
			EditorServerObject* editorObject = CreateObject( m_nextEntityId, transform );
			selected = editorObject->entity;
		}
		
		if ( ImGui::Button( "Reload Resources" ) )
		{
			program->UnloadMeshes();
			for ( auto [ _, object ] : m_objects )
			{
				for ( ae::Object* component : object->components )
				{
					const ae::Type* type = ae::GetTypeFromObject( component );
					if ( const ae::Var* var = GetMeshResourceVar( type ) )
					{
						object->HandleVarChange( program, component, type, var );
					}
				}
			}
		}
		
		static bool s_imGuiDemo = false;
		if ( ImGui::Button( "Show ImGui Demo" ) )
		{
			s_imGuiDemo = !s_imGuiDemo;
		}
		if ( s_imGuiDemo )
		{
			ImGui::ShowDemoWindow( &s_imGuiDemo );
		}
		
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if ( ImGui::TreeNode( "Object properties" ) )
	{
		if ( selected )
		{
			EditorServerObject* selectedObject = m_objects.Get( selected );
			ImGui::Text( "Object %u", selected );
		
			char name[ ae::Str16::MaxLength() ];
			strcpy( name, selectedObject->name.c_str() );
			if ( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				AE_INFO( "Set object name: #", name );
				selectedObject->name = name;
			}
			{
				bool changed = false;
				ae::Matrix4 temp = selectedObject->GetTransform( program );
				float matrixTranslation[ 3 ], matrixRotation[ 3 ], matrixScale[ 3 ];
				ImGuizmo::DecomposeMatrixToComponents( temp.data, matrixTranslation, matrixRotation, matrixScale );
				changed |= ImGui::InputFloat3( "translation", matrixTranslation );
				changed |= ImGui::InputFloat3( "rotation", matrixRotation );
				changed |= ImGui::InputFloat3( "scale", matrixScale );
				if ( changed )
				{
					ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, temp.data );
					selectedObject->SetTransform( temp, program );
				}
			}
			
			for ( ae::Object* component : selectedObject->components )
			{
				ImGui::Separator();
				const ae::Type* componentType = ae::GetTypeFromObject( component );
				if ( ImGui::TreeNode( componentType->GetName() ) )
				{
					std::function< void(const ae::Type*, ae::Object*) > fn = [&]( const ae::Type* type, ae::Object* component )
					{
						uint32_t varCount = type->GetVarCount( false );
						if ( varCount )
						{
							ImGui::Separator();
							if ( type != componentType )
							{
								ImGui::Text( "%s", type->GetName() );
								ImGui::Separator();
							}
							for ( uint32_t i = 0; i < varCount; i++ )
							{
								const ae::Var* var = type->GetVarByIndex( i, false );
								if ( m_ShowVar( program, component, var ) )
								{
									selectedObject->HandleVarChange( program, component, type, var );
								}
							}
						}
						if ( type->GetParentType() )
						{
							fn( type->GetParentType(), component );
						}
					};
					fn( componentType, component );
					ImGui::TreePop();
				}
			}
			
			ImGui::Separator();
			if ( ImGui::Button( "Add Component" ) )
			{
				ImGui::OpenPopup( "add_component_popup" );
			}
			if ( ImGui::BeginPopup( "add_component_popup" ) )
			{
				bool anyValid = false;
				bool foundAny = false;
				const char* editorTypeProperty = "ae_editor_type";
				uint32_t typeCount = ae::GetTypeCount();
				for ( uint32_t i = 0; i < typeCount; i++ )
				{
					const ae::Type* type = ae::GetTypeByIndex( i );
					if ( !type->HasProperty( editorTypeProperty ) || !type->IsDefaultConstructible() )
					{
						continue;
					}
					anyValid = true;
					
					if ( GetComponent( selectedObject, type->GetName() ) )
					{
						continue;
					}
					
					if ( ImGui::Selectable( type->GetName() ) )
					{
						AE_INFO( "Create #", type->GetName() );
						AddComponent( program, selectedObject, type->GetName() );
					}
					foundAny = true;
				}
				if ( !anyValid )
				{
					ImGui::Text( "Use AE_REGISTER_CLASS_PROPERTY( MyComponent, %s ) to register new types", editorTypeProperty );
				}
				else if ( !foundAny )
				{
					ImGui::Text( "None" );
				}
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::Text( "No Selection" );
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if ( ImGui::TreeNode( "Object List" ) )
	{
		const char* selectedTypeName = m_selectedType ? m_selectedType->GetName() : "All";
		if ( ImGui::BeginCombo( "Type", selectedTypeName, 0 ) )
		{
			if ( ImGui::Selectable( "All", !m_selectedType ) )
			{
				m_selectedType = nullptr;
			}
			if ( !m_selectedType )
			{
				ImGui::SetItemDefaultFocus();
			}
			uint32_t componentTypesCount = m_components.Length();
			for ( uint32_t i = 0; i < componentTypesCount; i++ )
			{
				const ae::Type* type = ae::GetTypeById( m_components.GetKey( i ) );
				const bool isSelected = ( m_selectedType == type );
				if ( ImGui::Selectable( type->GetName(), isSelected ) )
				{
					m_selectedType = type;
				}
				if ( isSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if ( ImGui::BeginListBox("##listbox", ImVec2( -FLT_MIN, 16 * ImGui::GetTextLineHeightWithSpacing() ) ) )
		{
			auto& selected = this->selected;
			auto showObjInList = [&selected]( int idx, EditorObjectId entity, const char* entityName )
			{
				ImGui::PushID( idx );
				const bool isSelected = ( entity == selected );
				ae::Str16 name = entityName;
				if ( !name.Length() )
				{
					name = ae::Str16::Format( "#", entity );
				}
				if ( ImGui::Selectable( name.c_str(), isSelected ) )
				{
					selected = entity;
				}
				if ( isSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			};
			
			if ( m_selectedType )
			{
				auto& components = m_components.Get( m_selectedType->GetId() );
				uint32_t componentCount = components.Length();
				for ( uint32_t i = 0; i < componentCount; i++ )
				{
					const EditorServerObject* obj = m_objects.Get( components.GetKey( i ) );
					const ae::Object* c = components.GetValue( i );
					showObjInList( i, obj->entity, obj->name.c_str() );
				}
			}
			else
			{
				uint32_t editorObjectCount = m_objects.Length();
				for ( uint32_t i = 0; i < editorObjectCount; i++ )
				{
					const EditorServerObject* editorObj = m_objects.GetValue( i );
					showObjInList( i, editorObj->entity, editorObj->name.c_str() );
				}
			}
			ImGui::EndListBox();
		}

		ImGui::TreePop();
	}
	
	m_first = false;
}

EditorServerObject* EditorServer::CreateObject( EditorObjectId id, const ae::Matrix4& transform )
{
	AE_ASSERT( !GetObject( id ) );
	EditorServerObject* editorObject = ae::New< EditorServerObject >( m_tag, m_tag );
	editorObject->Initialize( id, transform );
	m_objects.Set( id, editorObject );
	m_nextEntityId = ae::Max( m_nextEntityId, id + 1 );
	return editorObject;
}

ae::Object* EditorServer::AddComponent( EditorProgram* program, EditorServerObject* obj, const char* typeName )
{
	const ae::Type* type = ae::GetTypeByName( typeName );
	if ( !type )
	{
		return nullptr;
	}
	ae::Object* component = GetComponent( obj, typeName );
	if ( component )
	{
		return component;
	}
	
	std::function< void(const ae::Type*, ae::Object*) > fn = [&]( const ae::Type* t, ae::Object* component )
	{
		if ( t->GetParentType() )
		{
			fn( t->GetParentType(), component );
		}
		int32_t propIdx = t->GetPropertyIndex( "ae_editor_dep" );
		if ( propIdx >= 0 )
		{
			uint32_t propCount = t->GetPropertyValueCount( propIdx );
			for ( uint32_t i = 0; i < propCount; i++ )
			{
				const char* prop = t->GetPropertyValue( propIdx, i );
				if ( strcmp( type->GetName(), prop ) != 0 && !GetComponent( obj, typeName ) )
				{
					AddComponent( program, obj, prop );
				}
			}
		}
	};
	fn( type, component );
	
	component = (ae::Object*)ae::Allocate( m_tag, type->GetSize(), type->GetAlignment() );
	type->New( component );
	
	obj->components.Append( component );
	ae::Map< EditorObjectId, ae::Object* >* typeComponents = m_components.TryGet( type->GetId() );
	if ( !typeComponents )
	{
		typeComponents = &m_components.Set( type->GetId(), m_tag );
	}
	typeComponents->Set( obj->entity, component );
	
	const auto& meshName = m_typeMesh.Get( type, "" );
	if ( meshName.Length() )
	{
		obj->mesh = program->GetMesh( meshName.c_str() );
	}
	if ( m_typeInvisible.Get( type, false ) )
	{
		obj->opaque = false;
	}
	
	return component;
}

ae::Object* EditorServer::GetComponent( EditorServerObject* obj, const char* typeName )
{
	return const_cast< ae::Object* >( GetComponent( const_cast< const EditorServerObject* >( obj ), typeName ) );
}

const ae::Object* EditorServer::GetComponent( const EditorServerObject* obj, const char* typeName )
{
	const ae::Type* type = ae::GetTypeByName( typeName );
	AE_ASSERT( type );
	for ( const ae::Object* component : obj->components )
	{
		if ( type == ae::GetTypeFromObject( component ) )
		{
			return component;
		}
	}
	return nullptr;
}

const EditorServerObject* EditorServer::GetObjectFromComponent( const ae::Object* component )
{
	const ae::Type* type = ae::GetTypeFromObject( component );
	const ae::Map< EditorObjectId, ae::Object* >* components = m_components.TryGet( type->GetId() );
	if ( components )
	{
		for ( uint32_t i = 0; i < components->Length(); i++ )
		{
			if ( components->GetValue( i ) == component )
			{
				return GetObject( components->GetKey( i ) );
			}
		}
	}
	return nullptr;
}

const ae::Var* EditorServer::GetMeshResourceVar( const ae::Type* componentType )
{
	return m_meshResourceVars.Get( componentType, nullptr );
}

const ae::Var* EditorServer::GetMeshVisibleVar( const ae::Type* componentType )
{
	return m_meshVisibleVars.Get( componentType, nullptr );
}

bool EditorServer::SaveLevel( EditorProgram* program, bool saveAs )
{
	if ( !client->GetWritableLevel() )
	{
		return false;
	}

	// @TODO: Techincally ae::Editor::Level is already an intermidiate representation of the level, so this should be cleaned up.
	ae::EditorLevel tempLevel = m_tag;
	tempLevel.filePath = client->GetLevel()->filePath;
	
	bool fileSelected = tempLevel.filePath.Length();
	if ( !fileSelected || saveAs )
	{
		fileSelected = false;
		ae::FileDialogParams params;
		params.window = &program->window;
		params.filters.Append( { "Level File", "level" } );
		params.defaultPath = tempLevel.filePath.c_str();
		auto filePath = ae::FileSystem::SaveDialog( params );
		if ( filePath.c_str()[ 0 ] )
		{
			fileSelected = true;
			tempLevel.filePath = filePath.c_str();
		}
	}
	
	if ( fileSelected )
	{
		m_Save( &tempLevel );
		ae::EditorLevel* level = client->GetWritableLevel();
		AE_ASSERT( level );
		*level = std::move( tempLevel );
		if ( client->Write() )
		{
			AE_INFO( "Saved '#'", level->filePath );
			program->window.SetTitle( level->filePath.c_str() );
			return true;
		}
		else
		{
			AE_INFO( "Failed to save '#'", level->filePath );
		}
	}
	else
	{
		AE_INFO( "No file selected" );
	}
	return false;
}

void EditorServer::OpenLevel( EditorProgram* program )
{
	ae::FileDialogParams params;
	params.window = &program->window;
	params.filters.Append( { "Level File", "level" } );
	auto filePath = ae::FileSystem::OpenDialog( params );
	if ( filePath.Length() )
	{
		return OpenLevel( program, filePath[ 0 ].c_str() );
	}
	else
	{
		AE_INFO( "No file selected" );
	}
}

void EditorServer::OpenLevel( EditorProgram* program, const char* filePath )
{
	if ( !filePath || !filePath[ 0 ] )
	{
		return;
	}
	AE_INFO( "Level '#'", filePath );
	AE_INFO( "Reading..." );
	client->QueueRead( filePath );
}

void EditorServer::Unload()
{
	m_selectedType = nullptr;
	selected = kInvalidEditorObjectId;
	hoverEntity = kInvalidEditorObjectId;
	m_nextEntityId = 1;

	for ( uint32_t i = 0; i < m_objects.Length(); i++ )
	{
		EditorServerObject* editorObj = m_objects.GetValue( i );
		editorObj->Terminate();
		ae::Delete( editorObj );
	}
	m_objects.Clear();
	m_components.Clear();
}

void EditorServer::m_Save( ae::EditorLevel* levelOut ) const
{
	ae::Map< ae::Str32, ae::Object* > defaults = m_tag;
	
	auto readMembers = []( const ae::Type* type, const EditorObject* levelObj, const ae::Object* component, const ae::Object* defaultComponent, ae::Dict* propsOut ) -> void
	{
		uint32_t varCount = type->GetVarCount( true );
		for ( uint32_t varIdx = 0; varIdx < varCount; varIdx++ )
		{
			const ae::Var* var = type->GetVarByIndex( varIdx, true );
			if ( var->IsArray() )
			{
				uint32_t length = var->GetArrayLength( component );
				propsOut->SetUint( var->GetName(), length );
				for ( uint32_t arrIdx = 0; arrIdx < length; arrIdx++ )
				{
					ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
					auto value = var->GetObjectValueAsString( component, arrIdx );
					propsOut->SetString( key.c_str(), value.c_str() );
				}
			}
			else if ( var->GetType() == ae::Var::Matrix4 && strcmp( var->GetName(), "transform" ) == 0 )
			{
				propsOut->SetMatrix4( var->GetName(), levelObj->transform );
			}
			else if ( var->GetType() == ae::Var::Vec3 && strcmp( var->GetName(), "position" ) == 0 )
			{
				propsOut->SetVec3( var->GetName(), levelObj->transform.GetTranslation() );
			}
			// @TODO: 'scale' and 'rotation'
			else
			{
				auto value = var->GetObjectValueAsString( component );
				auto defaultValue = var->GetObjectValueAsString( defaultComponent );
				if ( value != defaultValue )
				{
					propsOut->SetString( var->GetName(), value.c_str() );
				}
			}
		}
	};

	levelOut->objects.Clear();
	uint32_t editorObjectCount = m_objects.Length();
	for ( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* editorObj = m_objects.GetValue( i );
		EditorObject* levelObj = &levelOut->objects.Set( editorObj->entity, { m_tag } );
		levelObj->id = editorObj->entity;
		levelObj->name = editorObj->name;
		levelObj->transform = editorObj->GetTransform( nullptr );
		
		for ( const ae::Object* component : editorObj->components )
		{
			const ae::Type* type = ae::GetTypeFromObject( component );
			const ae::Object* defaultComponent = defaults.Get( type->GetName(), nullptr );
			if ( !defaultComponent )
			{
				auto c = (ae::Object*)ae::Allocate( m_tag, type->GetSize(), type->GetAlignment() );
				type->New( c );
				defaultComponent = c;
			}
			
			EditorComponent& levelComponent = levelObj->components.Append( m_tag );
			levelComponent.type = type->GetName();
			readMembers( type, levelObj, component, defaultComponent, &levelComponent.members );
		}
	}
	
	for ( auto& _obj : defaults )
	{
		ae::Object* obj = _obj.value;
		obj->~Object();
		ae::Free( obj );
	}
}

bool EditorServer::m_Load( EditorProgram* program )
{
	Unload();
	const ae::EditorLevel* level = client->GetLevel();
	
	uint32_t objectCount = level->objects.Length();
	// Create all components
	for ( uint32_t i = 0; i < objectCount; i++ )
	{
		const EditorObject& levelObject = level->objects.GetValue( i );
		EditorServerObject* editorObj = CreateObject( levelObject.id, levelObject.transform );
		editorObj->name = levelObject.name;
		for ( const EditorComponent& levelComponent : levelObject.components )
		{
			AddComponent( program, editorObj, levelComponent.type.c_str() );
		}
	}
	// Serialize all components (second phase to handle references)
	for ( uint32_t i = 0; i < objectCount; i++ )
	{
		const EditorObject& levelObject = level->objects.GetValue( i );
		EditorServerObject* editorObj = GetObject( levelObject.id );
		AE_ASSERT( editorObj );
		for ( const EditorComponent& levelComponent : levelObject.components )
		{
			const char* typeName = levelComponent.type.c_str();
			const ae::Type* type = ae::GetTypeByName( typeName );
			if ( !type )
			{
				continue;
			}
			ae::Object* component = GetComponent( editorObj, typeName );
			AE_ASSERT( component );
			AE_ASSERT( ae::GetTypeFromObject( component ) == type );
			uint32_t varCount = type->GetVarCount( true );
			for ( uint32_t varIdx = 0; varIdx < varCount; varIdx++ )
			{
				const ae::Var* var = type->GetVarByIndex( varIdx, true );
				if ( var->IsArray() )
				{
					uint32_t length = levelComponent.members.GetInt( var->GetName(), 0 );
					if ( length )
					{
						length = var->SetArrayLength( component, length );
						for ( uint32_t arrIdx = 0; arrIdx < length; arrIdx++ )
						{
							ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
							if ( const char* value = levelComponent.members.GetString( key.c_str(), nullptr ) )
							{
								var->SetObjectValueFromString( component, value, arrIdx );
							}
						}
					}
				}
				else
				{
					const char* value = levelComponent.members.GetString( var->GetName(), nullptr );
					if ( value )
					{
						var->SetObjectValueFromString( component, value );
					}
				}
				editorObj->HandleVarChange( program, component, type, var );
			}
		}
	}
	return true;
}

bool EditorServer::m_ShowVar( EditorProgram* program, ae::Object* component, const ae::Var* var )
{
	if ( var->GetType() == ae::Var::Matrix4 && strcmp( var->GetName(), "transform" ) == 0 )
	{
		return false; // Handled by entity transform
	}
	else if ( var->GetType() == ae::Var::Vec3 && strcmp( var->GetName(), "position" ) == 0 )
	{
		return false; // Handled by entity transform
	}
	bool changed = false;
	ImGui::PushID( var->GetName() );
	if ( var->IsArray() )
	{
		uint32_t arrayLength = var->GetArrayLength( component );
		ImGui::Text( "%s", var->GetName() );
		ImVec2 size( ImGui::GetContentRegionAvail().x, 8 * ImGui::GetTextLineHeightWithSpacing() );
		ImGui::BeginChild( "ChildL", size, true, 0 );
		for ( int i = 0; i < arrayLength; i++ )
		{
			ImGui::PushID( i );
			changed |= m_ShowVarValue( program, component, var, i );
			ImGui::PopID();
		}
		ImGui::EndChild();
		if ( !var->IsArrayFixedLength() )
		{
			bool arrayMaxLength = ( var->GetArrayLength( component ) >= var->GetArrayMaxLength() );
			if ( arrayMaxLength )
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
			}
			if ( ImGui::Button( "Add" ) )
			{
				var->SetArrayLength( component, arrayLength + 1 );
				changed = true;
			}
			if ( arrayMaxLength )
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Remove" ) && arrayLength )
			{
				var->SetArrayLength( component, arrayLength - 1 );
				changed = true;
			}
		}
	}
	else
	{
		changed |= m_ShowVarValue( program, component, var );
	}
	ImGui::PopID();
	return changed;
}

bool EditorServer::m_ShowVarValue( EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx )
{
	switch ( var->GetType() )
	{
		case ae::Var::Type::Enum:
		{
			auto currentStr = var->GetObjectValueAsString( component, idx );
			auto valueStr = aeImGui_Enum( var->GetEnum(), var->GetName(), currentStr.c_str() );
			if ( var->SetObjectValueFromString( component, valueStr.c_str(), idx ) )
			{
				return ( currentStr != valueStr.c_str() );
			}
		}
		case ae::Var::Type::Bool:
		{
			bool b = false;
			var->GetObjectValue( component, &b, idx );
			if ( ImGui::Checkbox( var->GetName(), &b ) )
			{
				return var->SetObjectValue( component, b, idx );
			}
			return false;
		}
		case ae::Var::Type::Float:
		{
			float f = 0.0f;
			var->GetObjectValue( component, &f, idx );
			if ( ImGui::InputFloat( var->GetName(), &f ) )
			{
				return var->SetObjectValue( component, f, idx );
			}
			return false;
		}
		case ae::Var::Type::String:
		{
			char buf[ 256 ];
			auto val = var->GetObjectValueAsString( component, idx );
			strlcpy( buf, val.c_str(), sizeof(buf) );
			ImGui::Text( "%s", var->GetName() );
			if ( ImGui::InputTextMultiline( var->GetName(), buf, sizeof(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4 ), 0 ) )
			{
				return var->SetObjectValueFromString( component, buf, idx );
			}
			return false;
		}
		case ae::Var::Type::Ref:
		{
			return m_ShowRefVar( program, component, var, idx );
		}
		default:
			ImGui::Text( "%s (Unsupported type)", var->GetName() );
			break;
	}
	return false;
}

bool EditorServer::m_ShowRefVar( EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx )
{
	const ae::Type* componentType = ae::GetTypeFromObject( component );
	auto val = var->GetObjectValueAsString( component, idx );
	
	if ( idx < 0 )
	{
		ImGui::Text( "%s", var->GetName() );
	}
	const ae::Type* refType = var->GetSubType();
	AE_ASSERT( refType );
	if ( m_selectRef.enabled
		&& m_selectRef.component == component
		&& m_selectRef.componentVar == var
		&& m_selectRef.varIdx == idx )
	{
		if ( ImGui::Button( "Cancel" ) )
		{
			m_selectRef = SelectRef();
		}
	}
	else
	{
		ImGui::Text( "%s", val.c_str() );
		ImGui::SameLine();
		if ( val == "NULL" )
		{
			if ( ImGui::Button( "Set" ) )
			{
				m_selectRef.enabled = true;
				m_selectRef.component = component;
				m_selectRef.componentVar = var;
				m_selectRef.varIdx = idx;
			}
		}
		else
		{
			if ( ImGui::Button( "Select" ) )
			{
				ae::Object* selectComp = nullptr;
				if ( program->serializer.StringToObjectPointer( val.c_str(), &selectComp ) )
				{
					AE_ASSERT( selectComp );
					const EditorServerObject* selectObj = GetObjectFromComponent( selectComp );
					AE_ASSERT( selectObj );
					selected = selectObj->entity;
				}
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Clear" ) )
			{
				var->SetObjectValueFromString( component, "NULL", idx );
			}
		}
	}
	return false; // @TODO: Handle ref vars changing
}

std::string EditorProgram::Serializer::ObjectPointerToString( const ae::Object* obj ) const
{
	if ( !obj )
	{
		return "NULL";
	}
	const ae::Type* type = ae::GetTypeFromObject( obj );
	AE_ASSERT( type );
	const EditorServerObject* editorObj = program->editor.GetObjectFromComponent( obj );
	AE_ASSERT( editorObj );
	std::ostringstream str;
	str << editorObj->entity << " " << type->GetName();
	return str.str();
};

bool EditorProgram::Serializer::StringToObjectPointer( const char* pointerVal, ae::Object** objOut ) const
{
	if ( strcmp( pointerVal, "NULL" ) == 0 )
	{
		*objOut = nullptr;
		return true;
	}
	EditorObjectId entity = 0;
	char typeName[ 16 ];
	typeName[ 0 ] = 0;
	if ( sscanf( pointerVal, "%u %15s", &entity, typeName ) == 2 )
	{
		if ( EditorServerObject* editorObj = program->editor.GetObject( entity ) )
		{
			*objOut = program->editor.GetComponent( editorObj, typeName );
		}
		return true;
	}
	return false;
};

//------------------------------------------------------------------------------
// EditorPicking functions
//------------------------------------------------------------------------------
EditorObjectId EditorServer::m_PickObject( EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut )
{
	ae::Vec3 mouseRay = program->GetMouseRay();
	ae::Vec3 mouseRaySrc = program->camera.GetPosition();
	
	ae::RaycastParams raycastParams;
	raycastParams.source = mouseRaySrc;
	raycastParams.ray = mouseRay * kEditorViewDistance;
	raycastParams.hitClockwise = false;
	raycastParams.hitCounterclockwise = true;
	//raycastParams.debug = &program->debugLines;
	ae::RaycastResult result;
	uint32_t editorObjectCount = m_objects.Length();
	for ( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* editorObj = m_objects.GetValue( i );
		if ( editorObj->mesh && ( editorObj->opaque || GetShowInvisible() ) )
		{
			raycastParams.userData = editorObj;
			raycastParams.transform = editorObj->GetTransform( program );
			result = editorObj->mesh->collision.Raycast( raycastParams, result );
		}
		else
		{
			float hitT = INFINITY;
			ae::Vec3 hitPos( 0.0f );
			ae::Sphere sphere( editorObj->GetTransform( program ).GetTranslation(), 0.5f );
			if ( sphere.Raycast( mouseRaySrc, mouseRay, &hitT, &hitPos ) )
			{
				raycastParams.userData = nullptr;
				raycastParams.transform = ae::Matrix4::Identity();
				ae::RaycastResult sphereResult;
				auto* hit = &sphereResult.hits.Append( {} );
				hit->position = hitPos;
				hit->normal = ( mouseRaySrc - hitPos ).SafeNormalizeCopy();
				hit->distance = hitT;
				hit->userData = editorObj;
				ae::RaycastResult::Accumulate( raycastParams, sphereResult, &result );
			}
		}
	}
	if ( result.hits.Length() )
	{
		*hitOut = result.hits[ 0 ].position;
		*normalOut = result.hits[ 0 ].normal;
		const EditorServerObject* editorObj = (const EditorServerObject*)result.hits[ 0 ].userData;
		AE_ASSERT( editorObj );
		return editorObj->entity;
	}
	
	return kInvalidEditorObjectId;
}

void EditorServer::m_ShowEditorObject( EditorProgram* program, EditorObjectId entity, ae::Color color )
{
	if ( entity )
	{
		const EditorServerObject* editorObj = m_objects.Get( entity );
		if ( editorObj->mesh )
		{
			const ae::VertexData* meshData = &editorObj->mesh->data;
			ae::Matrix4 transform = editorObj->GetTransform( program );
			auto verts = meshData->GetVertices< ae::EditorServerMesh::Vertex >();
			uint32_t vertexCount = meshData->GetVertexCount();
			const void* indices = meshData->GetIndices();
			uint32_t indexCount = meshData->GetIndexCount();
			program->debugLines.AddMesh( (const ae::Vec3*)&verts->position, sizeof(*verts), vertexCount, indices, meshData->GetIndexSize(), indexCount, transform, color );
		}
		else
		{
			ae::Vec3 pos = editorObj->GetTransform( program ).GetTranslation();
			ae::Vec3 normal = program->camera.GetPosition() - pos;
			program->debugLines.AddCircle( pos, normal, 0.475f, color, 16 );
		}
	}
}

ae::Color EditorServer::m_GetColor( EditorObjectId entity, bool lines ) const
{
	uint64_t seed = entity * 43313;
	ae::Color color = ae::Color::HSV( ae::Random( 0.0f, 1.0f, seed ), 0.5, 0.75 );
	if ( entity == selected && lines )
	{
		color = ae::Color::PicoOrange();
	}
	else if ( entity == selected )
	{
		color = color.Lerp( ae::Color::PicoOrange(), 0.75f );
	}
	return color;
};

}
