//------------------------------------------------------------------------------
// Editor.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
// Created by John Hughes on 11/22/21.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Editor.h"
#include "ae/Entity.h"
// @TODO: Remove ImGui dependencies
#include "ae/aeImGui.h"
#include "ImGuizmo.h"
// @TODO: Remove rapidjson dependency
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#pragma warning( disable : 4018 ) // signed/unsigned mismatch
#pragma warning( disable : 4244 ) // conversion from 'float' to 'int32_t'
#pragma warning( disable : 4267 ) // conversion from 'size_t' to 'uint32_t'

namespace ae {

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

const float kEditorViewDistance = 25000.0f;

//------------------------------------------------------------------------------
// Serialization helpers
//------------------------------------------------------------------------------
void GetComponentTypePrereqs( const ae::Type* type, ae::Array< const ae::Type* >* prereqs );
void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component );
void JsonToRegistry( const ae::Map< ae::Entity, ae::Entity >& entityMap, const rapidjson::Value& jsonObjects, ae::Registry* registry );
void ComponentToJson( const Component* component, const Component* defaultComponent, rapidjson::Document::AllocatorType& allocator, rapidjson::Value* jsonComponent );
bool ValidateLevel( const rapidjson::Value& jsonLevel );

//------------------------------------------------------------------------------
// EditorMsg
//------------------------------------------------------------------------------
enum class EditorMsg : uint8_t
{
	None,
	Heartbeat,
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
	EditorServerMesh( const ae::Tag& tag ) : tag( tag ), vertices( tag ), collision( tag ) {}
	void Initialize( const ae::EditorMesh* mesh );
	const ae::Tag tag;
	ae::Array< Vertex > vertices;
	ae::VertexBuffer data;
	ae::CollisionMesh<> collision;
};

//------------------------------------------------------------------------------
// EditorServerObject
//------------------------------------------------------------------------------
class EditorServerObject
{
public:
	EditorServerObject( const ae::Tag& tag ) {}
	~EditorServerObject() {}
	void Initialize( ae::Entity entity, ae::Matrix4 transform );
	void Terminate();
	void SetTransform( const ae::Matrix4& transform, class EditorProgram* program );
	const ae::Matrix4& GetTransform() const;
	
	void HandleVarChange( class EditorProgram* program, ae::Component* component, const ae::Type* type, const ae::Var* var );

	ae::AABB GetAABB( class EditorProgram* program ) const;
	
	ae::Entity entity = ae::kInvalidEntity;
	bool hidden = false;
	EditorServerMesh* mesh = nullptr;
	bool opaque = true;
	
private:
	ae::Matrix4 m_transform = ae::Matrix4::Identity();
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
	EditorServer( const ae::Tag& tag ) :
		sock( tag ),
		m_tag( tag ),
		m_selected( tag ),
		m_hoverEntities( tag ),
		m_objects( tag ),
		m_registry( tag ),
		m_meshResourceVars( tag ),
		m_meshVisibleVars( tag ),
		m_typeMesh( tag ),
		m_typeInvisible( tag ),
		m_connections( tag )
	{}
	void Initialize( class EditorProgram* program );
	void Terminate( class EditorProgram* program );
	void Update( class EditorProgram* program );
	void Render( class EditorProgram* program );
	void ShowUI( class EditorProgram* program );
	
	bool SaveLevel( class EditorProgram* program, bool saveAs );
	void OpenLevelDialog( class EditorProgram* program );
	void OpenLevel( class EditorProgram* program, const char* path );
	void Unload( class EditorProgram* program );
	
	bool GetShowInvisible() const { return m_showInvisible; }
	
	EditorServerObject* CreateObject( ae::Entity entity, const ae::Matrix4& transform, const char* name );
	void DestroyObject( ae::Entity entity );
	ae::Component* AddComponent( class EditorProgram* program, EditorServerObject* obj, const ae::Type* type );
	void RemoveComponent( class EditorProgram* program, EditorServerObject* obj, ae::Component* component );
	ae::Component* GetComponent( EditorServerObject* obj, const ae::Type* type );
	const ae::Component* GetComponent( const EditorServerObject* obj, const ae::Type* type );
	uint32_t GetObjectCount() const { return m_objects.Length(); }
	EditorServerObject* GetObject( ae::Entity entity ) { return m_objects.Get( entity, nullptr ); }
	const EditorServerObject* GetObject( ae::Entity entity ) const { return m_objects.Get( entity, nullptr ); }
	const EditorServerObject* GetObjectFromComponent( const ae::Component* component );
	const ae::Var* GetMeshResourceVar( const ae::Type* componentType );
	const ae::Var* GetMeshVisibleVar( const ae::Type* componentType );
	ae::AABB GetSelectedAABB( class EditorProgram* program ) const;

	void HandleTransformChange( class EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform );
	void BroadcastVarChange( const ae::Var* var, const ae::Component* component );
	
	ae::ListenerSocket sock;
	
private:
	// Serialization helpers
	void m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::Type*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const;
	// Tools
	void m_CopySelected() const;
	void m_PasteFromClipboard( class EditorProgram* program );
	// Misc helpers
	void m_SetLevelPath( class EditorProgram* program, const char* path );
	void m_SelectWithModifiers( class EditorProgram* program, ae::Entity entity );
	bool m_ShowVar( class EditorProgram* program, ae::Object* component, const ae::Var* var );
	bool m_ShowVarValue( class EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx = -1 );
	bool m_ShowRefVar( class EditorProgram* program, ae::Object* component, const ae::Var* var, int32_t idx = -1 );
	ae::Entity m_PickObject( class EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut );
	void m_ShowEditorObject( EditorProgram* program, ae::Entity entity, ae::Color color );
	ae::Color m_GetColor( ae::Entity entity, bool lines ) const;
	void m_LoadLevel( class EditorProgram* program );
	
	const ae::Tag m_tag;
	bool m_first = true;

	ae::Str256 m_levelPath;
	const ae::File* m_pendingLevel = nullptr;

	bool m_showInvisible = false;

	// Manipulation
	const ae::Type* m_objectListType = nullptr;
	ae::Array< ae::Entity > m_selected;
	ae::Array< ae::Entity > m_hoverEntities;
	ae::Entity uiHoverEntity = kInvalidEntity;
	ae::Vec3 m_mouseHover = ae::Vec3( 0.0f );
	ae::Vec3 m_mouseHoverNormal = ae::Vec3( 0, 1, 0 );
	std::optional< ae::Vec2 > m_boxSelectStart;
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;

	// Object state
	ae::Map< ae::Entity, EditorServerObject* > m_objects;
	ae::Registry m_registry;

	// Configuration
	ae::Map< const ae::Type*, const ae::Var* > m_meshResourceVars;
	ae::Map< const ae::Type*, const ae::Var* > m_meshVisibleVars;
	ae::Map< const ae::Type*, ae::Str32 > m_typeMesh;
	ae::Map< const ae::Type*, bool > m_typeInvisible;
	
	// Connection to client
	double m_nextHeartbeat = 0.0;
	ae::Array< EditorConnection* > m_connections;
	uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
	
	// Selection
	struct SelectRef
	{
		bool enabled = false;
		ae::Object* component = nullptr;
		const ae::Var* componentVar = nullptr;
		int32_t varIdx = -1;
		
		ae::Entity pending = kInvalidEntity;
	};
	SelectRef m_selectRef;
};

class EditorProgram
{
public:
	EditorProgram( const ae::Tag& tag, const EditorParams& params ) :
		m_tag( tag ),
		camera( params.worldUp ),
		debugLines( tag ),
		editor( tag ),
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
	ae::FileSystem fileSystem;
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
	ae::VertexBuffer m_introMesh;
	ae::VertexBuffer m_quad;
	ae::Shader m_introShader;
	ae::Shader m_meshShader;
	ae::Shader m_iconShader;
	ae::Texture2D m_cogTexture;
};

//------------------------------------------------------------------------------
// EditorServerMesh member functions
//------------------------------------------------------------------------------
void EditorServerMesh::Initialize( const ae::EditorMesh* _mesh )
{
	vertices.Clear();
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
	
	data.Initialize( sizeof(Vertex), 0, vertices.Length(), 0, ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	data.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, position ) );
	data.AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( Vertex, normal ) );
	data.UploadVertices( 0, vertices.Data(), vertices.Length() );
	
	collision.Clear();
	collision.AddIndexed(
		ae::Matrix4::Identity(),
		_mesh->verts[ 0 ].data,
		_mesh->verts.Length(),
		sizeof( _mesh->verts[ 0 ] ),
		_mesh->indices.Data(),
		_mesh->indices.Length(),
		sizeof( *_mesh->indices.Data() )
	);
	collision.BuildBVH();
}

//------------------------------------------------------------------------------
// EditorProgram member functions
//------------------------------------------------------------------------------
void EditorProgram::Initialize()
{
	AE_INFO( "Editor Initialize (port: #)", params.port );

	const ae::Array< ae::Screen, 16 > screens = ae::GetScreens();
	window.Initialize( screens[ 0 ].position, screens[ 0 ].size.x, screens[ 0 ].size.y, true );
	window.SetMaximized( true );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	ui.Initialize();
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 10.0f ) );
	debugLines.Initialize( 20480 );
	debugLines.SetXRayEnabled( false );
	fileSystem.Initialize( params.dataDir.c_str(), "ae", "editor" );
	
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
	m_introShader.SetCulling( ae::Culling::CounterclockwiseFront );

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
	m_introMesh.UploadVertices( 0, kCubeVerts, countof( kCubeVerts ) );
	m_introMesh.UploadIndices( 0, kCubeIndices, countof( kCubeIndices ) );
	
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
	m_meshShader.SetCulling( ae::Culling::CounterclockwiseFront );
	
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
	m_quad.UploadVertices( 0, quadVerts, countof( quadVerts ) );
	
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
	m_iconShader.SetCulling( ae::Culling::CounterclockwiseFront );
	
	AE_STATIC_ASSERT( sizeof(kCogTextureData) == kCogTextureDataSize * kCogTextureDataSize );
	m_cogTexture.Initialize( kCogTextureData, kCogTextureDataSize, kCogTextureDataSize, ae::Texture::Format::R8, ae::Texture::Type::Uint8, ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp, true );

	editor.Initialize( this );
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
		m_gameTarget.Clear( ae::Color::AetherBlack() );

		m_worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		m_viewToProj = ae::Matrix4::ViewToProjection( GetFOV(), GetAspectRatio(), 0.25f, kEditorViewDistance );
		m_worldToProj = m_viewToProj * m_worldToView;
		m_projToWorld = m_worldToProj.GetInverse();
		
		// Update mouse ray
		{
			const float scaleFactor = window.GetScaleFactor();
			const ae::RectInt renderRectInt = GetRenderRect();
			const ae::Rect renderRect = ae::Rect::FromPoints(
				ae::Vec2( renderRectInt.GetPos() ) / scaleFactor,
				ae::Vec2( renderRectInt.GetPos() + renderRectInt.GetSize() ) / scaleFactor
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
			m_introMesh.Bind( &m_introShader, uniformList );
			m_introMesh.Draw();
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

		timeStep.Tick();
	}
}

ae::RectInt EditorProgram::GetRenderRect() const
{
	return ae::RectInt::FromPointAndSize( m_barWidth, 0, render.GetWidth() - m_barWidth, render.GetHeight() );
}

float EditorProgram::GetAspectRatio() const
{
	ae::Int2 size = GetRenderRect().GetSize();
	return size.x / (float)size.y;
}

EditorServerMesh* EditorProgram::GetMesh( const char* resourceId )
{
	EditorServerMesh* mesh = m_meshes.Get( resourceId, nullptr );
	if ( !mesh && params.functionPointers.loadMeshFn )
	{
		ae::EditorMesh temp = params.functionPointers.loadMeshFn( params.functionPointers.userData, resourceId );
		if ( temp.verts.Length() )
		{
			mesh = ae::New< EditorServerMesh >( m_tag, m_tag );
			mesh->Initialize( &temp );
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
	m_sock( tag )
{
	AE_ASSERT( m_tag != ae::Tag() );
}

Editor::~Editor()
{
	Terminate();
}

void Editor::Initialize( const EditorParams& params )
{
	AE_ASSERT( params.argc && params.argv );
	AE_ASSERT( params.registry );
	AE_ASSERT( params.worldUp == ae::Axis::Z || params.worldUp == ae::Axis::Y );
	AE_ASSERT( params.port );
	m_params = ae::New< EditorParams >( m_tag, params );
	m_fileSystem.Initialize( params.dataDir.c_str(), "ae", "editor" );

	bool run = params.run;
	if( !run )
	{
		for( int i = 0; i < params.argc; i++ )
		{
			if( strcmp( params.argv[ i ], "--editor" ) == 0 )
			{
				run = true;
				break;
			}
		}
	}

	if( run )
	{
		EditorProgram program( m_tag, params );
		program.Initialize();

		int32_t levelArg = -1;
		for( int i = 0; i < params.argc; i++ )
		{
			if( strcmp( params.argv[ i ], "--level" ) == 0 )
			{
				levelArg = i + 1;
				break;
			}
		}
		if( levelArg >= 0 )
		{
			program.editor.OpenLevel( &program, params.argv[ levelArg ] );
		}
		else if( !params.levelPath.Empty() )
		{
			program.editor.OpenLevel( &program, params.levelPath.c_str() );
		}

		program.Run();
		program.Terminate();
		exit( 0 );
	}
	m_Connect();
}

void Editor::Terminate()
{
#if !_AE_EMSCRIPTEN_
	m_sock.Disconnect();
#endif

	if( m_pendingFile )
	{
		m_fileSystem.Destroy( m_pendingFile );
		m_pendingFile = nullptr;
	}

	ae::Delete( m_params );
	m_params = nullptr;
}

void Editor::Launch()
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );
	m_Connect();
	if ( !m_sock.IsConnected() )
	{
		m_Fork();
	}
}

void Editor::Update()
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );

	m_Read();

	if ( m_params->port == 0 )
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
			case EditorMsg::Heartbeat:
			{
				// Nothing
				break;
			}
			case EditorMsg::Modification:
			{
				ae::Entity entity;
				ae::TypeId typeId;
				ae::Str32 varName;
				ae::Str256 varValue;
				rStream.SerializeUint32( entity );
				rStream.SerializeUint32( typeId );
				rStream.SerializeString( varName );
				rStream.SerializeString( varValue );
				if( rStream.IsValid() )
				{
					if( const ae::Type* type = ae::GetTypeById( typeId ) )
					{
						if( ae::Component* component = m_params->registry->TryGetComponent( entity, type ) )
						{
							if( const ae::Var* var = type->GetVarByName( varName.c_str(), true ) )
							{
								var->SetObjectValueFromString( component, varValue.c_str() );
							}
						}
					}
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

void Editor::QueueRead( const char* levelPath )
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );
	if ( m_pendingFile )
	{
		AE_WARN( "Cancelling level read '#'", m_pendingFile->GetUrl() );
		m_fileSystem.Destroy( m_pendingFile );
	}
	AE_INFO( "Queuing level load '#'", levelPath );
	m_pendingFile = m_fileSystem.Read( ae::FileSystem::Root::Data, levelPath, 2.0f );
}

void Editor::SetFunctionPointers( const ae::EditorFunctionPointers& functionPointers )
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );
	m_params->functionPointers = functionPointers;
}

void Editor::m_Read()
{
	if ( !m_pendingFile || m_pendingFile->GetStatus() == ae::File::Status::Pending )
	{
		return;
	}
	ae::RunOnDestroy destroyFile{ [this]()
	{
		m_fileSystem.Destroy( m_pendingFile );
		m_pendingFile = nullptr;
	} };
	uint32_t fileSize = m_pendingFile->GetLength();
	if ( !fileSize )
	{
		return;
	}

	const char* jsonBuffer = (const char*)m_pendingFile->GetData();
	AE_ASSERT( jsonBuffer[ m_pendingFile->GetLength() ] == 0 );
	rapidjson::Document document;
	if( document.Parse( jsonBuffer ).HasParseError() || !ValidateLevel( document ) )
	{
		return;
	}

	m_lastLoadedLevel = m_pendingFile->GetUrl();
	if( m_params->functionPointers.onLevelLoadStartFn )
	{
		m_params->functionPointers.onLevelLoadStartFn( m_params->functionPointers.userData, m_pendingFile->GetUrl() );
	}

	// State for loading
	ae::Array< const ae::Type* > prereqs = m_tag;
	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag; 
	const auto& jsonObjects = document[ "objects" ];

	// Create all components
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const char* entityName = jsonObject.HasMember( "name" ) ? jsonObject[ "name" ].GetString() : "";
		const ae::Matrix4 entityTransform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = m_params->registry->CreateEntity( jsonEntity, entityName );
		if( entity != jsonEntity )
		{
			entityMap.Set( jsonEntity, entity );
		}
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			AE_ASSERT( componentIter.value.IsObject() );
			const ae::Type* type = ae::GetTypeByName( componentIter.name.GetString() );
			GetComponentTypePrereqs( type, &prereqs );
			for( const ae::Type* prereq : prereqs )
			{
				m_params->registry->AddComponent( entity, prereq );
			}
			m_params->registry->AddComponent( entity, type );
		}
	}

	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, jsonObjects, m_params->registry );

	AE_INFO( "Loaded level '#'", m_pendingFile->GetUrl() );
}

void Editor::m_Connect()
{
#if !_AE_EMSCRIPTEN_
	if ( m_params->port && !m_sock.IsConnected() )
	{
		m_sock.Connect( ae::Socket::Protocol::TCP, "localhost", m_params->port );
	}
#endif
}

//------------------------------------------------------------------------------
// EditorServerObject member functions
//------------------------------------------------------------------------------
void EditorServerObject::Initialize( ae::Entity entity, ae::Matrix4 transform )
{
	this->entity = entity;
	this->m_transform = transform;
}

void EditorServerObject::Terminate()
{}

void EditorServerObject::SetTransform( const ae::Matrix4& transform, EditorProgram* program )
{
	AE_ASSERT( entity != kInvalidEntity );
	if ( m_transform != transform )
	{
		m_transform = transform;
		program->editor.HandleTransformChange( program, entity, transform );
	}
}

const ae::Matrix4& EditorServerObject::GetTransform() const
{
	return m_transform;
}

void EditorServerObject::HandleVarChange( EditorProgram* program, ae::Component* component, const ae::Type* type, const ae::Var* var )
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
	program->editor.BroadcastVarChange( var, component );
}

ae::AABB EditorServerObject::GetAABB( EditorProgram* program ) const
{
	const ae::Matrix4& transform = GetTransform();
	if( mesh )
	{
		return ae::OBB( transform * mesh->collision.GetAABB().GetTransform() ).GetAABB();
	}
	else
	{
		return ae::OBB( transform ).GetAABB();
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
			// @TODO: This should be a class variable property instead
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
			// @TODO: This should be a class variable property instead
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

	m_SetLevelPath( program, "" );
}

void EditorServer::m_LoadLevel( EditorProgram* program )
{
	if( !m_pendingLevel || m_pendingLevel->GetStatus() == ae::File::Status::Pending )
	{
		return;
	}

	ae::RunOnDestroy destroyFile = [&]()
	{
		program->fileSystem.Destroy( m_pendingLevel );
		m_pendingLevel = nullptr;
	};

	uint32_t fileSize = m_pendingLevel->GetLength();
	if( !fileSize )
	{
		AE_ERR( "Could not read level '#'", m_pendingLevel->GetUrl() );
		return;
	}

	if( program->params.functionPointers.preFileEditFn
		&& !program->params.functionPointers.preFileEditFn( program->params.functionPointers.userData, m_pendingLevel->GetUrl() ) )
	{
		ae::Str512 msg = ae::Str512::Format( "File may not be writable '#'", m_pendingLevel->GetUrl() );
		AE_WARN( msg.c_str() );
		ae::ShowMessage( msg.c_str() );
	}

	AE_INFO( "Loading level... '#'", m_pendingLevel->GetUrl() );
	
	const char* jsonBuffer = (const char*)m_pendingLevel->GetData();
	AE_ASSERT( jsonBuffer[ m_pendingLevel->GetLength() ] == 0 );
	
	rapidjson::Document document;
	if( document.Parse( jsonBuffer ).HasParseError() || !ValidateLevel( document ) )
	{
		AE_ERR( "Could not parse level '#'", m_pendingLevel->GetUrl() );
		return;
	}

	// @TODO: Make sure that the existing level has no modifications before unloading
	Unload( program );

	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag;
	
	const auto& jsonObjects = document[ "objects" ];
	AE_ASSERT( jsonObjects.IsArray() );
	// Create all components
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		EditorServerObject* object = CreateObject(
			jsonEntity,
			ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() ),
			jsonObject.HasMember( "name" ) ? jsonObject[ "name" ].GetString() : ""
		);
		if( object->entity != jsonEntity )
		{
			entityMap.Set( jsonEntity, object->entity );
		}
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				continue;
			}
			AddComponent( program, object, ae::GetTypeByName( componentIter.name.GetString() ) );
		}
	}
	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, jsonObjects, &m_registry );
	// Refresh editor objects
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = entityMap.Get( jsonEntity, jsonEntity );
		const ae::Matrix4 transform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		EditorServerObject* object = m_objects.Get( entity );
		AE_ASSERT( object );
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				continue;
			}
			const ae::Type* type = ae::GetTypeByName( componentIter.name.GetString() );
			if( !type )
			{
				continue;
			}
			ae::Component* component = &m_registry.GetComponent( entity, type );
			const uint32_t varCount = type->GetVarCount( true );
			for ( uint32_t j = 0; j < varCount; j++ )
			{
				const ae::Var* var = type->GetVarByIndex( j, true );
				object->HandleVarChange( program, component, type, var );
			}
		}
		// @TODO: Explicitly handle setting transform vars?
	}

	AE_INFO( "Loaded level '#'", m_pendingLevel->GetUrl() );
	m_SetLevelPath( program, m_pendingLevel->GetUrl() );
}

void EditorServer::Terminate( EditorProgram* program )
{
	for ( EditorConnection* conn : m_connections )
	{
		conn->Destroy( this );
		ae::Delete( conn );
	}
	m_connections.Clear();
}

void EditorServer::Update( EditorProgram* program )
{
	m_LoadLevel( program ); // Checks if a pending level is ready to load
	
	if ( !sock.IsListening() )
	{
		sock.Listen( ae::Socket::Protocol::TCP, false, program->params.port, 8 );
	}
	while ( ae::Socket* newConn = sock.Accept() )
	{
		AE_INFO( "ae::Editor client connected from #:#", newConn->GetResolvedAddress(), newConn->GetPort() );
		EditorConnection* editorConn = m_connections.Append( ae::New< EditorConnection >( m_tag ) );
		editorConn->sock = newConn;
	}
	AE_ASSERT( m_connections.Length() == sock.GetConnectionCount() );
	
	// Send a heartbeat to keep the connection alive, and to check if the client is still connected
	const double currentTime = ae::GetTime();
	if( currentTime > m_nextHeartbeat )
	{
		for( EditorConnection* (&conn) : m_connections )
		{
			if( conn->sock->IsConnected() )
			{
				ae::BinaryStream wStream = ae::BinaryStream::Writer( m_msgBuffer, sizeof(m_msgBuffer) );
				wStream.SerializeRaw( EditorMsg::Heartbeat );
				AE_ASSERT( wStream.IsValid() );
				conn->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
			}
		}
		m_nextHeartbeat = currentTime + 0.1;
	}
	
	for( EditorConnection* (&conn) : m_connections )
	{
		if( conn->sock->IsConnected() )
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
	m_connections.RemoveAllFn( []( const EditorConnection* c ){ return !c; } );
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
		if( !obj->hidden )
		{
			float distanceSq = ( camPos - obj->GetTransform().GetTranslation() ).LengthSquared();
			if ( obj->mesh && obj->opaque ) { opaqueObjects.Append( { obj, distanceSq } ); }
			else if ( obj->mesh && GetShowInvisible() ) { transparentObjects.Append( { obj, distanceSq } ); }
			else { logicObjects.Append( { obj, distanceSq } ); }
		}
	}
	
	// Opaque and transparent meshes helper
	auto renderMesh = [program, worldToProj, lightDir]( const RenderObj& renderObj, ae::Color color )
	{
		const EditorServerObject& obj = *renderObj.obj;
		ae::Matrix4 transform = obj.GetTransform();
		ae::UniformList uniformList;
		uniformList.Set( "u_localToProj", worldToProj * transform );
		uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
		uniformList.Set( "u_lightDir", lightDir );
		uniformList.Set( "u_color", color.GetLinearRGBA() );
		obj.mesh->data.Bind( &program->m_meshShader, uniformList );
		obj.mesh->data.Draw();
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
		const ae::Matrix4 transform = obj.GetTransform();
		const ae::Color color = m_GetColor( obj.entity, false );
		ae::UniformList uniformList;
		ae::Vec3 objPos = transform.GetTranslation();
		ae::Vec3 toCamera = camPos - objPos;
		ae::Matrix4 modelToWorld = ae::Matrix4::Rotation( ae::Vec3(0,0,1), ae::Vec3(0,1,0), toCamera, camUp );
		modelToWorld.SetTranslation( objPos );
		uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
		uniformList.Set( "u_tex", &program->m_cogTexture );
		uniformList.Set( "u_color", color.GetLinearRGBA() );
		program->m_quad.Bind( &program->m_iconShader, uniformList );
		program->m_quad.Draw();
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
	const float dt = program->GetDt();
	const ae::Color cursorColor = ae::Color::PicoOrange();
	
	m_hoverEntities.Clear();
	if( program->camera.GetMode() == ae::DebugCamera::Mode::None && !ImGui::GetIO().WantCaptureMouse )
	{
		const ae::Vec2 mousePos( ImGui::GetMousePos().x, ImGui::GetMousePos().y );
		if( m_boxSelectStart )
		{
			const ImVec2 selectMin = ImVec2( ae::Min( m_boxSelectStart->x, mousePos.x ), ae::Min( m_boxSelectStart->y, mousePos.y ) );
			const ImVec2 selectMax = ImVec2( ae::Max( m_boxSelectStart->x, mousePos.x ), ae::Max( m_boxSelectStart->y, mousePos.y ) );
			const ae::Vec3 fColor = ae::Color::PicoOrange().GetLinearRGB() * 255.0f;
			ImGui::GetBackgroundDrawList()->AddRect( selectMin, selectMax, IM_COL32( fColor.x, fColor.y, fColor.z, 255 ), 1.0f, ImDrawCornerFlags_All, 1.5f );
			ImGui::GetBackgroundDrawList()->AddRectFilled( selectMin, selectMax, IM_COL32( fColor.x, fColor.y, fColor.z, 100 ), 1.0f, ImDrawCornerFlags_All );
		}

		if( program->input.mouse.leftButton )
		{
			if( !program->input.mousePrev.leftButton ) // Press
			{
				m_boxSelectStart = ae::Vec2( mousePos.x, mousePos.y );
			}
			else
			{
				// Box hover logic
			}
		}
		else
		{
			ae::Entity hoverEntity = m_PickObject( program, cursorColor, &m_mouseHover, &m_mouseHoverNormal );
			if( hoverEntity )
			{
				m_hoverEntities.Append( hoverEntity );
			}
		}

		if( !program->input.mouse.leftButton && program->input.mousePrev.leftButton ) // Release
		{
			m_boxSelectStart = std::nullopt;

			ae::Entity hoverEntity = m_hoverEntities.Length() ? m_hoverEntities[ 0 ] : kInvalidEntity;
			if( m_selectRef.enabled )
			{
				uint32_t matchCount = 0;
				const ae::Object* lastMatch = nullptr;
				
				const ae::Type* refType = m_selectRef.componentVar->GetSubType();
				const uint32_t componentTypeCount = m_registry.GetTypeCount();
				for( uint32_t i = 0; i < componentTypeCount; i++ )
				{
					const ae::Type* hoverType = m_registry.GetTypeByIndex( i );
					if( hoverType->IsType( refType ) )
					{
						if( const ae::Component* hoverComponent = m_registry.TryGetComponent( hoverEntity, hoverType ) )
						{
							matchCount++;
							lastMatch = hoverComponent;
						}
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
			else
			{
				m_SelectWithModifiers( program, hoverEntity );
			}
		}
	}
	else
	{
		// Make sure box select is cleared when interrupted by other UI
		m_boxSelectStart = std::nullopt;
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
		const EditorServerObject* pendingObj = GetObject( m_selectRef.pending );
		AE_ASSERT( pendingObj );
		const uint32_t componentTypeCount = m_registry.GetTypeCount();
		for( uint32_t i = 0; i < componentTypeCount; i++ )
		{
			const ae::Type* pendingType = m_registry.GetTypeByIndex( i );
			if( pendingType->IsType( refType ) && ImGui::Selectable( pendingType->GetName(), false ) )
			{
				const uint32_t componentCount = m_registry.GetComponentCountByIndex( i );
				for( uint32_t j = 0; j < componentCount; j++ )
				{
					ae::Component* pendingComponent = &m_registry.GetComponentByIndex( i, j );
					m_selectRef.componentVar->SetObjectValue( m_selectRef.component, pendingComponent, m_selectRef.varIdx );
					m_selectRef = SelectRef();
				}
			}
		}
		ImGui::EndPopup();
	}
	
	if( m_selected.Length() )
	{
		const ae::AABB selectedAABB = GetSelectedAABB( program );
		program->debugLines.AddAABB( selectedAABB.GetCenter(), selectedAABB.GetHalfSize(), ae::Color::PicoOrange() );
		for ( ae::Entity entity : m_selected )
		{
			m_ShowEditorObject( program, entity, m_GetColor( entity, true ) );
		}
	}
	if( m_hoverEntities.Length() )
	{
		program->debugLines.AddCircle( m_mouseHover + m_mouseHoverNormal * 0.025f, m_mouseHoverNormal, 0.5f, cursorColor, 8 );
		for( Entity hoverEntity : m_hoverEntities )
		{
			if( m_selected.Find( hoverEntity ) < 0 )
			{
				m_ShowEditorObject( program, hoverEntity, ae::Color::PicoOrange() ); // m_GetColor( hoverEntity, true )
			}
		}
	}
	if( uiHoverEntity && m_selected.Find( uiHoverEntity ) < 0 )
	{
		m_ShowEditorObject( program, uiHoverEntity, ae::Color::PicoOrange() ); // m_GetColor( hoverEntity, true )
	}
	
	ae::Vec3 debugRefocus( 0.0f );
	program->debugLines.AddSphere( program->camera.GetFocus(), 0.1f, ae::Color::PicoOrange(), 6 );
//	if ( program->camera->GetDebugRefocusTarget( &debugRefocus ) )
//	{
//		program->debugLines.AddSphere( debugRefocus, 0.1f, ae::Color::PicoOrange(), 6 );
//	}
	
	if ( ImGui::GetIO().WantCaptureKeyboard )
	{
		// Keyboard captured
	}
	else if ( program->input.Get( ae::Key::LeftMeta ) )
	{
		// Meta+shift shortcuts
		if( program->input.Get( ae::Key::Shift ) )
		{
			if( program->input.GetPress( ae::Key::Z ) )
			{
				// m_Redo();
			}
		}
		// Meta shortcuts
		else if ( program->input.Get( ae::Key::S ) && !program->input.GetPrev( ae::Key::S ) )
		{
			SaveLevel( program, program->input.Get( ae::Key::LeftShift ) );
		}
		else if ( program->input.Get( ae::Key::O ) && !program->input.GetPrev( ae::Key::O ) )
		{
			OpenLevelDialog( program );
		}
		else if( program->input.GetPress( ae::Key::D ) )
		{
			// m_DuplicateSelected();
		}
		else if( program->input.GetPress( ae::Key::C ) )
		{
			m_CopySelected();
		}
		else if( program->input.GetPress( ae::Key::V ) )
		{
			m_PasteFromClipboard( program );
		}
		else if( program->input.GetPress( ae::Key::X ) )
		{
			// m_CutSelected();
		}
		else if( program->input.GetPress( ae::Key::Z ) )
		{
			// m_Undo();
		}
		else if( program->input.GetPress( ae::Key::A ) )
		{
			// m_SelectAll();
		}
	}
	// Single key shortcuts
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
	// Action shortcuts
	else if ( ( program->input.Get( ae::Key::Delete ) && !program->input.GetPrev( ae::Key::Delete ) )
		|| ( program->input.Get( ae::Key::Backspace ) && !program->input.GetPrev( ae::Key::Backspace ) ) )
	{
		for( ae::Entity entity : m_selected )
		{
			DestroyObject( entity );
		}
		m_selected.Clear();
	}
	else if ( program->input.Get( ae::Key::F ) && !program->input.GetPrev( ae::Key::F ) )
	{
		if ( m_selected.Length() )
		{
			program->camera.Refocus( GetSelectedAABB( program ).GetCenter() );
		}
		else if( m_hoverEntities.Length() )
		{
			program->camera.Refocus( m_mouseHover );
		}
	}
	else if ( program->input.Get( ae::Key::H ) && !program->input.GetPrev( ae::Key::H ) )
	{
		if ( m_selected.Length() )
		{
			bool anyHidden = false;
			bool anyVisible = false;
			for( ae::Entity entity : m_selected )
			{
				EditorServerObject* editorObject = m_objects.Get( entity );
				if( editorObject->hidden )
				{
					anyHidden = true;
				}
				else
				{
					anyVisible = true;
				}
			}
			const bool setHidden = !anyHidden || anyVisible;
			for( ae::Entity entity : m_selected )
			{
				m_objects.Get( entity )->hidden = setHidden;
			}
		}
		else
		{
			AE_INFO( "No objects selected" );
		}
	}
	else if( program->input.Get( ae::Key::Escape ) && !program->input.GetPrev( ae::Key::Escape ) )
	{
		m_selected.Clear();
	}
	
	if ( m_selected.Length() && gizmoOperation )
	{
		const float scaleFactor = program->window.GetScaleFactor();
		const ae::RectInt renderRectInt = program->GetRenderRect();
		const ae::Rect renderRect = ae::Rect::FromPoints(
				ae::Vec2( renderRectInt.GetPos() ) / scaleFactor,
				ae::Vec2( renderRectInt.GetPos() + renderRectInt.GetSize() ) / scaleFactor
		);
		
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::Enable( program->camera.GetMode() == ae::DebugCamera::Mode::None && !m_boxSelectStart );
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::AllowAxisFlip( false );
		ImGuizmo::BeginFrame();
		ImGuizmo::SetRect( renderRect.GetMin().x, renderRect.GetMin().y, renderRect.GetSize().x, renderRect.GetSize().y );
		
		EditorServerObject* selectedObject = m_objects.Get( m_selected[ 0 ] );
		ae::Matrix4 prevTransform = selectedObject->GetTransform();
		ae::Matrix4 transform = prevTransform;
		ImGuizmo::MODE mode = ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode;
		if ( ImGuizmo::Manipulate(
			program->GetWorldToView().data,
			program->GetViewToProj().data,
			gizmoOperation,
			mode,
			transform.data ) )
		{
			selectedObject->SetTransform( transform, program );

			const ae::Matrix4 change = ( mode == ImGuizmo::LOCAL ) ? prevTransform.GetInverse() * transform : transform * prevTransform.GetInverse();
			for ( ae::Entity entity : m_selected )
			{
				if ( entity == m_selected[ 0 ] )
				{
					continue;
				}
				EditorServerObject* editorObject = m_objects.Get( entity );
				if ( mode == ImGuizmo::LOCAL )
				{
					editorObject->SetTransform( editorObject->GetTransform() * change, program );
				}
				else
				{
					editorObject->SetTransform( change * editorObject->GetTransform(), program );
				}
			}
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
			OpenLevelDialog( program );
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
			Unload( program );
		}
		ImGui::SameLine();
		ImGui::BeginDisabled( m_levelPath.Empty() || !m_connections.Length() );
		if ( ImGui::Button( "Game Load" ) && m_connections.Length() )
		{
			uint8_t buffer[ kMaxEditorMessageSize ];
			ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
			wStream.SerializeRaw( EditorMsg::Load );
			wStream.SerializeString( m_levelPath );
			for ( uint32_t i = 0; i < m_connections.Length(); i++ )
			{
				m_connections[ i ]->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if ( ImGui::Button( "Quit" ) )
		{
			program->input.quit = true;
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
	
	if( ImGui::TreeNode( "Operations" ) )
	{
		if( ImGui::Button( "Create" ) )
		{
			ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetFocus() );
			EditorServerObject* editorObject = CreateObject( kInvalidEntity, transform, "" );
			m_selected.Clear();
			m_selected.Append( editorObject->entity );
		}
		ImGui::BeginDisabled( !m_selected.Length() );
		if( ImGui::Button( "Delete" ) )
		{
			for ( ae::Entity entity : m_selected )
			{
				DestroyObject( entity );
			}
			m_selected.Clear();
		}
		if( ImGui::Button( "Copy" ) )
		{
			m_CopySelected();
		}
		ImGui::EndDisabled();
		if( ImGui::Button( "Paste" ) )
		{
			m_PasteFromClipboard( program );
		}
		
		if( ImGui::Button( "Reload Resources" ) )
		{
			program->UnloadMeshes();
			const uint32_t componentTypeCount = m_registry.GetTypeCount();
			for( uint32_t i = 0; i < componentTypeCount; i++ )
			{
				const ae::Type* type = m_registry.GetTypeByIndex( i );
				const uint32_t componentCount = m_registry.GetComponentCountByIndex( i );
				for( uint32_t j = 0; j < componentCount; j++ )
				{
					if( const ae::Var* var = GetMeshResourceVar( type ) )
					{
						ae::Component* component = &m_registry.GetComponentByIndex( i, j );
						EditorServerObject* object = GetObject( component->GetEntity() );
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
		if ( m_selected.Length() == 1 )
		{
			EditorServerObject* selectedObject = m_objects.Get( m_selected[ 0 ] );
			ImGui::Text( "Object %u", selectedObject->entity );
		
			char name[ ae::Str16::MaxLength() ];
			strcpy( name, m_registry.GetNameByEntity( m_selected[ 0 ] ) );
			if ( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				AE_INFO( "Set object name: #", name );
				m_registry.SetEntityName( m_selected[ 0 ], name );
			}
			{
				bool changed = false;
				ae::Matrix4 temp = selectedObject->GetTransform();
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
				if ( ImGui::Button( "Add Component" ) )
				{
					ImGui::OpenPopup( "add_component_popup" );
				}
			}
			
			ae::Component* deleteComponent = nullptr;
			const uint32_t componentTypesCount = m_registry.GetTypeCount();
			for( uint32_t i = 0; i < componentTypesCount; i++ )
			{
				const ae::Type* componentType = m_registry.GetTypeByIndex( i );
				ae::Component* component = m_registry.TryGetComponent( selectedObject->entity, componentType );
				if( component )
				{
					ImGui::Separator();
					if( ImGui::TreeNode( componentType->GetName() ) )
					{
						auto fn = [&]( auto& fn, const ae::Type* type, ae::Component* component ) -> void
						{
							uint32_t varCount = type->GetVarCount( false );
							if( varCount )
							{
								for( uint32_t i = 0; i < varCount; i++ )
								{
									const ae::Var* var = type->GetVarByIndex( i, false );
									if( m_ShowVar( program, component, var ) )
									{
										selectedObject->HandleVarChange( program, component, type, var );
									}
								}
							}
							if( type->GetParentType() )
							{
								fn( fn, type->GetParentType(), component );
							}
						};
						fn( fn, componentType, component );
						if( ImGui::Button( "Remove Component" ) )
						{
							deleteComponent = component;
						}
						ImGui::TreePop();
					}
				}
			}
			if( deleteComponent )
			{
				RemoveComponent( program, selectedObject, deleteComponent );
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
					
					if ( GetComponent( selectedObject, type ) )
					{
						continue;
					}
					
					if ( ImGui::Selectable( type->GetName() ) )
					{
						AE_INFO( "Create #", type->GetName() );
						AddComponent( program, selectedObject, type );
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
		else if ( m_selected.Length() )
		{
			ImGui::Text( "Multiple Objects Selected" );
		}
		else
		{
			ImGui::Text( "No Selection" );
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	uiHoverEntity = kInvalidEntity;
	if ( ImGui::TreeNode( "Object List" ) )
	{
		const char* selectedTypeName = m_objectListType ? m_objectListType->GetName() : "All";
		if ( ImGui::BeginCombo( "Type", selectedTypeName, 0 ) )
		{
			if ( ImGui::Selectable( "All", !m_objectListType ) )
			{
				m_objectListType = nullptr;
			}
			if ( !m_objectListType )
			{
				ImGui::SetItemDefaultFocus();
			}
			const uint32_t componentTypesCount = m_registry.GetTypeCount();
			for ( uint32_t i = 0; i < componentTypesCount; i++ )
			{
				const ae::Type* type = m_registry.GetTypeByIndex( i );
				const bool isSelected = ( m_objectListType == type );
				if ( ImGui::Selectable( type->GetName(), isSelected ) )
				{
					m_objectListType = type;
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
			auto showObjInList = [&]( int idx, ae::Entity entity, const char* entityName )
			{
				ImGui::PushID( idx );
				const bool isSelected = ( m_selected.Find( entity ) >= 0 );
				ae::Str16 name = entityName;
				if ( !name.Length() )
				{
					name = ae::Str16::Format( "#", entity );
				}
				if ( ImGui::Selectable( name.c_str(), isSelected ) )
				{
					m_SelectWithModifiers( program, entity );
				}
				if( ImGui::IsItemHovered() )
				{
					uiHoverEntity = entity;
				}
				if ( isSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			};
			
			if( m_objectListType )
			{
				const int32_t typeIndex = m_registry.GetTypeIndexByType( m_objectListType );
				AE_ASSERT( typeIndex >= 0 );
				const uint32_t componentCount = m_registry.GetComponentCountByIndex( typeIndex );
				for( uint32_t i = 0; i < componentCount; i++ )
				{
					const ae::Component* component = &m_registry.GetComponentByIndex( typeIndex, i );
					showObjInList( i, component->GetEntity(), component->GetEntityName() );
				}
			}
			else
			{
				uint32_t editorObjectCount = m_objects.Length();
				for( uint32_t i = 0; i < editorObjectCount; i++ )
				{
					const EditorServerObject* editorObj = m_objects.GetValue( i );
					showObjInList( i, editorObj->entity, m_registry.GetNameByEntity( editorObj->entity ) );
				}
			}
			ImGui::EndListBox();
		}

		ImGui::TreePop();
	}
	
	m_first = false;
}

EditorServerObject* EditorServer::CreateObject( Entity entity, const ae::Matrix4& transform, const char* name )
{
	entity = m_registry.CreateEntity( entity, name );
	EditorServerObject* editorObject = ae::New< EditorServerObject >( m_tag, m_tag );
	editorObject->Initialize( entity, transform );
	m_objects.Set( entity, editorObject );
	return editorObject;
}

void EditorServer::DestroyObject( ae::Entity entity )
{
	EditorServerObject* editorObject = nullptr;
	if ( m_objects.Remove( entity, &editorObject ) )
	{
		m_registry.Destroy( entity );
		editorObject->Terminate();
		ae::Delete( editorObject );
	}
}

ae::Component* EditorServer::AddComponent( EditorProgram* program, EditorServerObject* obj, const ae::Type* type )
{
	if ( !type )
	{
		return nullptr;
	}
	ae::Component* component = GetComponent( obj, type );
	if ( component )
	{
		return component;
	}

	ae::Array< const ae::Type* > prereqs = m_tag;
	GetComponentTypePrereqs( type, &prereqs );
	for( const ae::Type* prereq : prereqs )
	{
		if( !GetComponent( obj, prereq ) )
		{
			AddComponent( program, obj, prereq );
		}
	}
	component = m_registry.AddComponent( obj->entity, type );
	if( component )
	{
		const auto& meshName = m_typeMesh.Get( type, "" );
		if ( meshName.Length() )
		{
			obj->mesh = program->GetMesh( meshName.c_str() );
		}
		if ( m_typeInvisible.Get( type, false ) )
		{
			obj->opaque = false;
		}
	}
	return component;
}

void EditorServer::RemoveComponent( EditorProgram* program, EditorServerObject* obj, ae::Component* component )
{
	if ( obj && component )
	{
		m_registry.DestroyComponent( component );
	}
}

ae::Component* EditorServer::GetComponent( EditorServerObject* obj, const ae::Type* type )
{
	return const_cast< ae::Component* >( GetComponent( const_cast< const EditorServerObject* >( obj ), type ) );
}

const ae::Component* EditorServer::GetComponent( const EditorServerObject* obj, const ae::Type* type )
{
	return obj ? m_registry.TryGetComponent( obj->entity, type ) : nullptr;
}

const EditorServerObject* EditorServer::GetObjectFromComponent( const ae::Component* component )
{
	return component ? GetObject( component->GetEntity() ) : nullptr;
}

const ae::Var* EditorServer::GetMeshResourceVar( const ae::Type* componentType )
{
	return m_meshResourceVars.Get( componentType, nullptr );
}

const ae::Var* EditorServer::GetMeshVisibleVar( const ae::Type* componentType )
{
	return m_meshVisibleVars.Get( componentType, nullptr );
}

ae::AABB EditorServer::GetSelectedAABB( EditorProgram* program ) const
{
	ae::AABB aabb;
	for( ae::Entity entity : m_selected )
	{
		aabb.Expand( m_objects.Get( entity )->GetAABB( program ) );
	}
	return aabb;
}

void EditorServer::HandleTransformChange( EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform )
{
	EditorServerObject* editorObject = GetObject( entity );
	AE_ASSERT( editorObject );
	const uint32_t typeCounts = m_registry.GetTypeCount();
	for( uint32_t i = 0; i < typeCounts; i++ )
	{
		const ae::Type* type = m_registry.GetTypeByIndex( i );
		if( ae::Component* component = m_registry.TryGetComponent( entity, type ) )
		{
			// @TODO: Update all transform, position, scale, rotation vars
			if( const ae::Var* var = type->GetVarByName( "transform", true ) )
			{
				var->SetObjectValue( component, transform );
				editorObject->HandleVarChange( program, component, type, var );
			}
		}
	}
}

void EditorServer::BroadcastVarChange( const ae::Var* var, const ae::Component* component )
{
	if( var->IsArray() )
	{
		// @TODO: Broadcast array element changes
		return;
	}
	ae::BinaryStream wStream = ae::BinaryStream::Writer( m_msgBuffer, sizeof(m_msgBuffer) );
	wStream.SerializeRaw( EditorMsg::Modification );
	wStream.SerializeUint32( component->GetEntity() );
	wStream.SerializeUint32( ae::GetObjectTypeId( component ) );
	wStream.SerializeString( var->GetName() );
	wStream.SerializeString( var->GetObjectValueAsString( component ).c_str() );
	if( wStream.IsValid() )
	{
		for ( EditorConnection* conn : m_connections )
		{
			conn->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
		}
	}
	else
	{
		AE_WARN( "Could not serialize modification message" );
	}

}

bool EditorServer::SaveLevel( EditorProgram* program, bool saveAs )
{
	bool fileSelected = !m_levelPath.Empty();
	if ( !fileSelected || saveAs )
	{
		fileSelected = false;
		ae::FileDialogParams params;
		params.window = &program->window;
		params.filters.Append( { "Level File", "level" } );
		params.defaultPath = m_levelPath.c_str();
		auto filePath = ae::FileSystem::SaveDialog( params );
		if ( filePath.c_str()[ 0 ] )
		{
			fileSelected = true;
			m_SetLevelPath( program, filePath.c_str() );
		}
	}
	if ( !fileSelected )
	{
		AE_INFO( "No file selected" );
		return false;
	}

	if( program->params.functionPointers.preFileEditFn
		&& !program->params.functionPointers.preFileEditFn( program->params.functionPointers.userData, m_levelPath.c_str() ) )
	{
		AE_ERR( "File failed per edit check '#'", m_levelPath );
		return false;
	}

	AE_INFO( "Saving... '#'", m_levelPath );

	ae::Map< const ae::Type*, ae::Component* > defaults = m_tag;
	ae::RunOnDestroy destroyDefaults = [&]()
	{
		for ( auto& _component : defaults )
		{
			ae::Component* component = _component.value;
			std::destroy_at( component );
			ae::Free( component );
		}
	};

	rapidjson::Document document( rapidjson::kObjectType );
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	{
		rapidjson::Value jsonObjects( rapidjson::kArrayType );
		jsonObjects.Reserve( m_objects.Length(), allocator );

		for( const auto& _obj : m_objects )
		{
			rapidjson::Value jsonObject( rapidjson::kObjectType );
			m_EntityToJson( _obj.value, allocator, &defaults, &jsonObject );
			jsonObjects.PushBack( jsonObject, allocator );
		}

		document.AddMember( "objects", jsonObjects, allocator );
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter< rapidjson::StringBuffer > writer( buffer );
	document.Accept( writer );

	const uint32_t writtenBytes = program->fileSystem.Write( ae::FileSystem::Root::Data, m_levelPath.c_str(), buffer.GetString(), buffer.GetSize(), false );
	AE_ASSERT( writtenBytes == 0 || writtenBytes == buffer.GetSize() );
	if( writtenBytes == 0 )
	{
		ae::Str256 msg = ae::Str256::Format( "Failed to write level '#'", m_levelPath );
		AE_WARN( msg.c_str() );
		ae::ShowMessage( msg.c_str() );
		return false;
	}
	return true;
}

void EditorServer::OpenLevelDialog( EditorProgram* program )
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
	if( m_pendingLevel )
	{
		AE_WARN( "Cancelling level read '#'", m_pendingLevel->GetUrl() );
		program->fileSystem.Destroy( m_pendingLevel );
	}
	AE_INFO( "Reading... '#'", filePath );
	m_pendingLevel = program->fileSystem.Read( ae::FileSystem::Root::Data, filePath, 2.0f );
}

void EditorServer::Unload( EditorProgram* program )
{
	m_objectListType = nullptr;
	m_selected.Clear();
	m_hoverEntities.Clear();
	uiHoverEntity = kInvalidEntity;
	m_selectRef = SelectRef();

	m_SetLevelPath( program, "" );

	while ( m_objects.Length() )
	{
		DestroyObject( m_objects.GetKey( m_objects.Length() - 1 ) );
	}
	AE_ASSERT( m_objects.Length() == 0 );

	m_registry.Clear();
}

void EditorServer::m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::Type*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const
{
	AE_ASSERT( levelObject );
	AE_ASSERT( jsonEntity->IsObject() );

	// Id
	const ae::Entity entity = levelObject->entity;
	jsonEntity->AddMember( "id", entity, allocator );

	// Name
	const char* objectName = m_registry.GetNameByEntity( entity );
	if ( objectName[ 0 ] )
	{
		jsonEntity->AddMember( "name", rapidjson::StringRef( objectName ), allocator );
	}

	// Transform
	rapidjson::Value transformJson;
	const ae::Matrix4 transform = levelObject->GetTransform();
	const auto transformStr = ae::ToString( transform );
	transformJson.SetString( transformStr.c_str(), allocator );
	jsonEntity->AddMember( "transform", transformJson, allocator );

	// Components
	rapidjson::Value jsonComponents( rapidjson::kObjectType );
	const uint32_t componentTypeCount = m_registry.GetTypeCount();
	for( uint32_t i = 0; i < componentTypeCount; i++ )
	{
		const ae::Type* type = m_registry.GetTypeByIndex( i );
		if( const ae::Component* component = m_registry.TryGetComponent( entity, type ) )
		{
			const ae::Component* defaultComponent = defaults ? defaults->Get( type, nullptr ) : nullptr;
			if ( defaults && !defaultComponent )
			{
				defaultComponent = type->New< ae::Component >( ae::Allocate( m_tag, type->GetSize(), type->GetAlignment() ) );
				defaults->Set( type, const_cast< ae::Component* >( defaultComponent ) );
			}

			rapidjson::Value jsonComponent( rapidjson::kObjectType );
			ae::ComponentToJson( component, defaultComponent, allocator, &jsonComponent );
			jsonComponents.AddMember( rapidjson::StringRef( type->GetName() ), jsonComponent, allocator );
		}
	}
	jsonEntity->AddMember( "components", jsonComponents, allocator );
}

void EditorServer::m_CopySelected() const
{
	if( !m_selected.Length() )
	{
		ae::SetClipboardText( "" );
		return;
	}

	// Sort entities so they are pasted in the order they were created, not the
	// order they were selected. ValidateLevel() expects this order.
	ae::Array< ae::Entity > selectedSorted = m_selected;
	std::sort( std::begin( selectedSorted ), std::end( selectedSorted ) );

	rapidjson::Document document( rapidjson::kObjectType );
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	{
		rapidjson::Value jsonObjects( rapidjson::kArrayType );
		jsonObjects.Reserve( selectedSorted.Length(), allocator );

		for( ae::Entity entity : selectedSorted )
		{
			rapidjson::Value jsonObject( rapidjson::kObjectType );
			m_EntityToJson( GetObject( entity ), allocator, nullptr, &jsonObject );
			jsonObjects.PushBack( jsonObject, allocator );
		}

		document.AddMember( "objects", jsonObjects, allocator );
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter< rapidjson::StringBuffer > writer( buffer );
	document.Accept( writer );
	ae::SetClipboardText( buffer.GetString() );
}

void EditorServer::m_PasteFromClipboard( EditorProgram* program )
{
	const auto clipboardText = ae::GetClipboardText();
	if( clipboardText.empty() )
	{
		return;
	}

	// Load / validate
	rapidjson::Document document;
	if( document.Parse( clipboardText.c_str() ).HasParseError() )
	{
		AE_WARN( "Failed to parse clipboard text" );
		return;
	}
	if( !ValidateLevel( document ) )
	{
		AE_WARN( "Unexpected clipboard data format" );
		return;
	}

	// State for loading
	m_selected.Clear();
	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag;
	const auto& jsonObjects = document[ "objects" ];

	// Create all components
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const char* entityName = jsonObject.HasMember( "name" ) ? jsonObject[ "name" ].GetString() : "";
		const ae::Matrix4 entityTransform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		EditorServerObject* editorObject = CreateObject( jsonEntity, entityTransform, entityName );
		if( editorObject->entity != jsonEntity )
		{
			entityMap.Set( jsonEntity, editorObject->entity );
		}
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			AE_ASSERT( componentIter.value.IsObject() );
			AddComponent( program, editorObject, ae::GetTypeByName( componentIter.name.GetString() ) );
		}

		// Select entity
		m_selected.Append( editorObject->entity );
	}

	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, jsonObjects, &m_registry );

	// Refresh editor objects
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = entityMap.Get( jsonEntity, jsonEntity );
		const ae::Matrix4 transform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		EditorServerObject* object = m_objects.Get( entity );
		AE_ASSERT( object );
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				continue;
			}
			const ae::Type* type = ae::GetTypeByName( componentIter.name.GetString() );
			if( !type )
			{
				continue;
			}
			ae::Component* component = &m_registry.GetComponent( entity, type );
			const uint32_t varCount = type->GetVarCount( true );
			for ( uint32_t j = 0; j < varCount; j++ )
			{
				const ae::Var* var = type->GetVarByIndex( j, true );
				object->HandleVarChange( program, component, type, var );
			}
		}
		// @TODO: Explicitly handle setting transform vars?
	}
}

void EditorServer::m_SetLevelPath( EditorProgram* program, const char* filePath )
{
	if( filePath[ 0 ] )
	{
		m_levelPath = filePath;
		program->window.SetTitle( filePath );
	}
	else
	{
		m_levelPath = "";
		program->window.SetTitle( "Untitled" );
	}
}

void EditorServer::m_SelectWithModifiers( EditorProgram* program, ae::Entity entity )
{
	bool shift = program->input.Get( ae::Key::LeftShift );
	bool ctrl = program->input.Get( ae::Key::LeftControl );
	if ( shift && ctrl )
	{
		// Add
		if ( entity != kInvalidEntity && m_selected.Find( entity ) < 0 )
		{
			m_selected.Append( entity );
		}
	}
	else if ( shift )
	{
		// Toggle
		if ( entity != kInvalidEntity )
		{
			int32_t idx = m_selected.Find( entity );
			if ( idx < 0 )
			{
				m_selected.Append( entity );
			}
			else
			{
				m_selected.Remove( idx );
			}
		}
	}
	else if ( ctrl )
	{
		// Remove
		if ( entity != kInvalidEntity )
		{
			int32_t idx = m_selected.Find( entity );
			if ( idx >= 0 )
			{
				m_selected.Remove( idx );
			}
		}
	}
	else
	{
		// New selection
		m_selected.Clear();
		if ( entity != kInvalidEntity )
		{
			m_selected.Append( entity );
		}
	}
}

bool EditorServer::m_ShowVar( EditorProgram* program, ae::Object* component, const ae::Var* var )
{
	if ( var->GetType() == ae::BasicType::Matrix4 && strcmp( var->GetName(), "transform" ) == 0 )
	{
		return false; // Handled by entity transform
	}
	else if ( var->GetType() == ae::BasicType::Vec3 && strcmp( var->GetName(), "position" ) == 0 )
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
			ImGui::BeginDisabled( arrayMaxLength );
			if ( ImGui::Button( "Add" ) )
			{
				var->SetArrayLength( component, arrayLength + 1 );
				changed = true;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled( !arrayLength );
			if ( ImGui::Button( "Remove" ) )
			{
				var->SetArrayLength( component, arrayLength - 1 );
				changed = true;
			}
			ImGui::EndDisabled();
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
	ae::Str64 varName = ( idx < 0 ) ? var->GetName() : ae::Str64::Format( "#", idx );
	switch ( var->GetType() )
	{
		case ae::BasicType::Enum:
		{
			auto currentStr = var->GetObjectValueAsString( component, idx );
			auto valueStr = aeImGui_Enum( var->GetEnum(), varName.c_str(), currentStr.c_str() );
			if ( var->SetObjectValueFromString( component, valueStr.c_str(), idx ) )
			{
				return ( currentStr != valueStr.c_str() );
			}
		}
		case ae::BasicType::Bool:
		{
			bool b = false;
			var->GetObjectValue( component, &b, idx );
			if ( ImGui::Checkbox( varName.c_str(), &b ) )
			{
				return var->SetObjectValue( component, b, idx );
			}
			return false;
		}
		case ae::BasicType::Float:
		{
			float f = 0.0f;
			var->GetObjectValue( component, &f, idx );
			if ( ImGui::InputFloat( varName.c_str(), &f ) )
			{
				return var->SetObjectValue( component, f, idx );
			}
			return false;
		}
		case ae::BasicType::String:
		{
			char buf[ 256 ];
			auto val = var->GetObjectValueAsString( component, idx );
			strlcpy( buf, val.c_str(), sizeof(buf) );
			ImGui::Text( "%s", varName.c_str() );
			if ( ImGui::InputTextMultiline( varName.c_str(), buf, sizeof(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4 ), 0 ) )
			{
				return var->SetObjectValueFromString( component, buf, idx );
			}
			return false;
		}
		case ae::BasicType::Pointer:
		{
			return m_ShowRefVar( program, component, var, idx );
		}
		// @TODO: case ae::BasicType::CustomRef
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
				ae::Object* _selectComp = nullptr;
				if ( program->serializer.StringToObjectPointer( val.c_str(), &_selectComp ) )
				{
					ae::Component* selectComp = ae::Cast< ae::Component >( _selectComp );
					AE_ASSERT( selectComp );
					const EditorServerObject* selectObj = GetObjectFromComponent( selectComp );
					AE_ASSERT( selectObj );
					m_SelectWithModifiers( program, selectObj->entity );
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
	const ae::Component* component = ae::Cast< ae::Component >( obj );
	AE_ASSERT( component );
	const EditorServerObject* editorObj = program->editor.GetObjectFromComponent( component );
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
	ae::Entity entity = 0;
	char typeName[ 16 ];
	typeName[ 0 ] = 0;
	if ( sscanf( pointerVal, "%u %15s", &entity, typeName ) == 2 )
	{
		if ( EditorServerObject* editorObj = program->editor.GetObject( entity ) )
		{
			*objOut = program->editor.GetComponent( editorObj, ae::GetTypeByName( typeName ) );
		}
		return true;
	}
	return false;
};

//------------------------------------------------------------------------------
// EditorPicking functions
//------------------------------------------------------------------------------
ae::Entity EditorServer::m_PickObject( EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut )
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
		if( !editorObj->hidden )
		{
			if ( editorObj->mesh && ( editorObj->opaque || GetShowInvisible() ) )
			{
				raycastParams.userData = editorObj;
				raycastParams.transform = editorObj->GetTransform();
				result = editorObj->mesh->collision.Raycast( raycastParams, result );
			}
			else
			{
				float hitT = INFINITY;
				ae::Vec3 hitPos( 0.0f );
				ae::Sphere sphere( editorObj->GetTransform().GetTranslation(), 0.5f );
				if ( sphere.IntersectRay( mouseRaySrc, mouseRay, &hitPos, &hitT ) )
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
	}
	if ( result.hits.Length() )
	{
		*hitOut = result.hits[ 0 ].position;
		*normalOut = result.hits[ 0 ].normal;
		const EditorServerObject* editorObj = (const EditorServerObject*)result.hits[ 0 ].userData;
		AE_ASSERT( editorObj );
		return editorObj->entity;
	}
	
	return kInvalidEntity;
}

void EditorServer::m_ShowEditorObject( EditorProgram* program, ae::Entity entity, ae::Color color )
{
	if ( entity )
	{
		const EditorServerObject* editorObj = m_objects.Get( entity );
		// if( editorObj->hidden )
		// {
		// 	return;
		// }
		// else
		if ( editorObj->mesh )
		{
			const ae::VertexBuffer* meshData = &editorObj->mesh->data;
			ae::Matrix4 transform = editorObj->GetTransform();
			uint32_t vertexCount = editorObj->mesh->vertices.Length();
			ae::EditorServerMesh::Vertex* verts = editorObj->mesh->vertices.Data();
			program->debugLines.AddMesh( (const ae::Vec3*)&verts->position, sizeof(*verts), vertexCount, transform, color );
		}
		else
		{
			ae::Vec3 pos = editorObj->GetTransform().GetTranslation();
			ae::Vec3 normal = program->camera.GetPosition() - pos;
			program->debugLines.AddCircle( pos, normal, 0.475f, color, 16 );
		}
	}
}

ae::Color EditorServer::m_GetColor( ae::Entity entity, bool lines ) const
{
	const bool isSelected = ( m_selected.Find( entity ) >= 0 );
	uint64_t seed = entity * 43313;
	ae::Color color = ae::Color::HSV( ae::Random( 0.0f, 1.0f, seed ), 0.5, 0.75 );
	if ( isSelected && lines )
	{
		color = ae::Color::PicoOrange();
	}
	else if ( isSelected )
	{
		color = color.Lerp( ae::Color::PicoOrange(), 0.75f );
	}
	return color;
}

void GetComponentTypePrereqs( const ae::Type* type, ae::Array< const ae::Type* >* prereqs )
{
	prereqs->Clear();
	auto fn = [&]( auto& fn, const ae::Type* t ) -> void
	{
		if( t->GetParentType() )
		{
			fn( fn, t->GetParentType() );
		}
		const int32_t propIdx = t->GetPropertyIndex( "ae_editor_dep" );
		const uint32_t propCount = ( propIdx >= 0 ) ? t->GetPropertyValueCount( propIdx ) : 0;
		for( uint32_t i = 0; i < propCount; i++ )
		{
			const ae::Type* prereq = ae::GetTypeByName( t->GetPropertyValue( propIdx, i ) );
			if( type != prereq && prereqs->Find( prereq ) < 0 )
			{
				// @TODO: Handle missing types
				if( prereq )
				{
					prereqs->Append( prereq );
				}
			}
		}
	};
	fn( fn, type );
}

void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component )
{
	const ae::Type* type = ae::GetTypeFromObject( component );
	const uint32_t varCount = type->GetVarCount( true );
	for( uint32_t i = 0; i < varCount; i++ )
	{
		const ae::Var* var = type->GetVarByIndex( i, true );
		if( !jsonComponent.HasMember( var->GetName() ) )
		{
			continue;
		}
		const auto& jsonVar = jsonComponent[ var->GetName() ];
		if( var->IsArray() && jsonVar.IsArray() )
		{
			const auto& jsonVarArray = jsonVar.GetArray();
			const uint32_t arrayLen = var->SetArrayLength( component, jsonVarArray.Size() );
			AE_ASSERT( arrayLen <= jsonVarArray.Size() );
			for( uint32_t i = 0; i < arrayLen; i++ )
			{
				const auto& jsonVarArrayValue = jsonVarArray[ i ];
				var->SetObjectValueFromString( component, jsonVarArrayValue.GetString(), i );
			}
		}
		else if( var->GetType() == ae::BasicType::Matrix4 && strcmp( var->GetName(), "transform" ) == 0 )
		{
			var->SetObjectValue( component, transform );
		}
		else if( var->GetType() == ae::BasicType::Vec3 && strcmp( var->GetName(), "position" ) == 0 )
		{
			var->SetObjectValue( component, transform.GetTranslation() );
		}
		else if( var->GetType() == ae::BasicType::Vec3 && strcmp( var->GetName(), "scale" ) == 0 )
		{
			var->SetObjectValue( component, transform.GetScale() );
		}
		// @TODO: Add 'rotation'
		// @TODO: Handle patching references
		else if( !jsonVar.IsObject() && !jsonVar.IsArray() )
		{
			var->SetObjectValueFromString( component, jsonVar.GetString() );
		}
	}
}

void JsonToRegistry( const ae::Map< ae::Entity, ae::Entity >& entityMap, const rapidjson::Value& jsonObjects, ae::Registry* registry )
{
	// Serialize all components (second phase to handle references)
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = entityMap.Get( jsonEntity, jsonEntity );
		const ae::Matrix4 transform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				continue;
			}
			const ae::Type* type = ae::GetTypeByName( componentIter.name.GetString() );
			if( !type )
			{
				continue;
			}
			ae::Component* component = &registry->GetComponent( entity, type );
			ae::JsonToComponent( transform, componentIter.value, component );
		}
	}
}

void ComponentToJson( const Component* component, const Component* defaultComponent, rapidjson::Document::AllocatorType& allocator, rapidjson::Value* jsonComponent )
{
	AE_ASSERT( jsonComponent->IsObject() );
	const ae::Type* type = ae::GetTypeFromObject( component );
	const uint32_t varCount = type->GetVarCount( true );
	for( uint32_t i = 0; i < varCount; i++ )
	{
		const ae::Var* var = type->GetVarByIndex( i, true );
		const auto varName = rapidjson::StringRef( var->GetName() );
		if( var->IsArray() )
		{
			if( const int32_t arrayLen = var->GetArrayLength( component ) )
			{
				rapidjson::Value jsonArray( rapidjson::kArrayType );
				jsonArray.Reserve( arrayLen, allocator );
				for( uint32_t j = 0; j < arrayLen; j++ )
				{
					rapidjson::Value jsonValue( rapidjson::kStringType );
					jsonValue.SetString( var->GetObjectValueAsString( component, j ).c_str(), allocator );
					jsonArray.PushBack( jsonValue, allocator );
				}
				jsonComponent->AddMember( varName, jsonArray, allocator );
			}
		}
		else
		{
			const auto value = var->GetObjectValueAsString( component );
			rapidjson::Value jsonValue( rapidjson::kStringType );
			if( defaultComponent )
			{
				const auto defaultValue = var->GetObjectValueAsString( defaultComponent );
				if( value != defaultValue )
				{
					jsonValue.SetString( value.c_str(), allocator );
					jsonComponent->AddMember( varName, jsonValue, allocator );
				}
			}
			else
			{
				jsonValue.SetString( value.c_str(), allocator );
				jsonComponent->AddMember( varName, jsonValue, allocator );
			}
		}
	}
}

bool ValidateLevel( const rapidjson::Value& jsonLevel )
{
	if( !jsonLevel.IsObject() || !jsonLevel.HasMember( "objects" ) )
	{
		return false;
	}
	
	const auto& jsonObjects = jsonLevel[ "objects" ];
	if( !jsonObjects.IsArray() )
	{
		return false;
	}

	uint32_t prevId = 0;
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		if( !jsonObject.HasMember( "id" ) ) { return false; }
		if( !jsonObject.HasMember( "transform" ) ) { return false; }
		if( !jsonObject.HasMember( "components" ) ) { return false; }

		const uint32_t id = jsonObject[ "id" ].GetUint();
		if( id <= prevId )
		{
			return false;
		}
		prevId = id;

		const auto& jsonComponents = jsonObject[ "components" ];
		if( !jsonComponents.IsObject() )
		{
			return false;
		}
		for( const auto& componentIter : jsonComponents.GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				return false;
			}
		}
	}
	return true;
}

} // End ae namespace

#if _AE_APPLE_
#include <unistd.h> // fork
#elif _AE_WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#endif

namespace ae {

void Editor::m_Fork()
{
	const char* levelPath = "";
	if( !m_params->levelPath.Empty() )
	{
		levelPath = m_params->levelPath.c_str();
	}
	else if( !m_lastLoadedLevel.Empty() )
	{
		levelPath = m_lastLoadedLevel.c_str();
	}

#if _AE_APPLE_
	if ( !fork() )
	{
		ae::Array< char*, 8 > args;
		args.Append( m_params->argv[ 0 ] );
		args.Append( (char*)"--editor" );
		if( levelPath[ 0 ] )
		{
			args.Append( (char*)"--level" );
			args.Append( (char*)levelPath );
		}
		args.Append( nullptr );
		execv( args[ 0 ], args.Data() );
	}
#elif _AE_WINDOWS_
	STARTUPINFOA startupInfo;
	memset( &startupInfo, 0, sizeof( startupInfo ) );
	startupInfo.cb = sizeof( startupInfo );
	PROCESS_INFORMATION procInfo;
	char args[ 256 ];
	args[ 0 ] = 0;
	strlcat( args, m_params->argv[ 0 ], sizeof(args) );
	strlcat( args, " --editor", sizeof(args) );
	if( levelPath[ 0 ] )
	{
		strlcat( args, " --level ", sizeof(args) );
		strlcat( args, levelPath, sizeof(args) );
	}
	CreateProcessA(
		m_params->argv[ 0 ],
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

} // End ae namespace
