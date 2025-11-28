//------------------------------------------------------------------------------
// Editor.cpp
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
#include "ae/Editor.h"
#include "ae/Entity.h"
// @TODO: Remove ImGui dependencies
#include "ae/aeImGui.h"
#include "ImGuizmo.h"
// @TODO: Remove rapidjson dependency
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/error/error.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#if _AE_WINDOWS_
	#pragma warning( disable : 4018 ) // signed/unsigned mismatch
	#pragma warning( disable : 4244 ) // conversion from 'float' to 'int32_t'
	#pragma warning( disable : 4267 ) // conversion from 'size_t' to 'uint32_t'
#endif

AE_REGISTER_NAMESPACECLASS( (ae, EditorTypeAttribute) );
AE_REGISTER_NAMESPACECLASS( (ae, EditorRequiredAttribute) );
AE_REGISTER_NAMESPACECLASS( (ae, EditorVisibilityAttribute) );

namespace ae {

class EditorServer;
using EditorPluginArray = ae::Array< std::pair< EditorPluginConfig, EditorPlugin* > >;

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

enum class PickingType
{
	Disabled,
	Mesh,
	Logic
};

struct SpecialMemberVar
{
	ae::BasicType::Type type;
	const char* name;
	bool ( *SetObjectValue )( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var );
};
const SpecialMemberVar kSpecialMemberVars[] = {
	{ ae::BasicType::Matrix4, "transform", []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform ); } },
	{ ae::BasicType::Vec3, "position", []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetTranslation() ); } },
	{ ae::BasicType::Quaternion, "rotation", []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetRotation() ); } },
	{ ae::BasicType::Vec3, "scale", []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetScale() ); } },
};
const SpecialMemberVar* GetSpecialMemberVar( const ae::ClassVar* var )
{
	for( const auto& specialVar : kSpecialMemberVars )
	{
		if( var->GetType() == specialVar.type && strcmp( var->GetName(), specialVar.name ) == 0 )
		{
			return &specialVar;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
void GetComponentTypeRequirements( const ae::ClassType* type, ae::Array< const ae::ClassType* >* prereqs );
void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component );
void JsonToRegistry( const ae::Map< ae::Entity, ae::Entity >& entityMap, const rapidjson::Value& jsonObjects, ae::Registry* registry );
void ComponentToJson( const Component* component, const Component* defaultComponent, rapidjson::Document::AllocatorType& allocator, rapidjson::Value* jsonComponent );
bool ValidateLevel( const rapidjson::Value& jsonLevel );
template< typename T > const T* TryGetClassOrVarAttribute( const ae::ClassType* type );
ae::Array< const ae::ClassVar*, 8 > GetTypeVarsByName( const ae::ClassType* type, const char* name );
void SendPluginEvent( EditorPluginArray& plugins, const EditorEvent& event );

//------------------------------------------------------------------------------
// EditorNetMsg
//------------------------------------------------------------------------------
enum class EditorNetMsg : uint8_t
{
	None,
	Heartbeat,
	Modification,
	Load
};

//------------------------------------------------------------------------------
// EditorDialog
//------------------------------------------------------------------------------
struct EditorDialog
{
	virtual ~EditorDialog() = default;
	virtual bool ShowUIAndWaitForButton() = 0;
	uint32_t id = 0;
};

//------------------------------------------------------------------------------
// AppBundleWarningDialog
//------------------------------------------------------------------------------
struct AppBundleWarningDialog : public EditorDialog
{
	bool ShowUIAndWaitForButton() override;
	class EditorProgram* program = nullptr;
	std::string path;
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
	
	void HandleVarChange( class EditorProgram* program, ae::Component* component, const ae::ClassType* type, const ae::ClassVar* var );

	ae::OBB GetOBB( class EditorProgram* program ) const;
	ae::AABB GetAABB( class EditorProgram* program ) const;
	
	ae::Entity entity = ae::kInvalidEntity;
	bool hidden = false;
	bool renderDisabled = false;
	
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
		m_connections( tag ),
		m_framePickableEntities( tag ),
		m_dialogs( tag )
	{}
	~EditorServer();
	void Initialize( class EditorProgram* program );
	void Terminate( class EditorProgram* program );
	void Update( class EditorProgram* program );
	void Render( class EditorProgram* program );
	void ShowUI( class EditorProgram* program );
	
	bool SaveLevel( class EditorProgram* program, bool saveAs );
	void OpenLevelDialog( class EditorProgram* program );
	void OpenLevelWithPrompts( class EditorProgram* program, const char* path );
	void OpenLevel( EditorProgram* program, const char* filePath );
	void Unload( class EditorProgram* program );
	
	bool GetShowTransparent() const { return m_showTransparent; }
	ae::Color GetBackgroundColor() const { return m_backgroundColor; }
	
	EditorServerObject* CreateObject( ae::Entity entity, const ae::Matrix4& transform, const char* name );
	void DestroyObject( class EditorProgram* program, ae::Entity entity );
	ae::Component* AddComponent( class EditorProgram* program, EditorServerObject* obj, const ae::ClassType* type );
	void RemoveComponent( class EditorProgram* program, EditorServerObject* obj, ae::Component* component );
	ae::Component* GetComponent( EditorServerObject* obj, const ae::ClassType* type );
	const ae::Component* GetComponent( const EditorServerObject* obj, const ae::ClassType* type );
	uint32_t GetObjectCount() const { return m_objects.Length(); }
	EditorServerObject* GetObject( ae::Entity entity ) { return m_objects.Get( entity, nullptr ); }
	const EditorServerObject* GetObject( ae::Entity entity ) const { return m_objects.Get( entity, nullptr ); }
	const EditorServerObject* GetObjectFromComponent( const ae::Component* component );
	bool GetRenderDisabled( ae::Entity entity ) const;
	ae::AABB GetSelectedAABB( class EditorProgram* program ) const;

	void HandleTransformChange( class EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform );
	void BroadcastVarChange( const ae::ClassVar* var, const ae::Component* component );

	void ActiveRefocus( EditorProgram* program );
	
	ae::ListenerSocket sock;
	
private:
	// Serialization helpers
	void m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::ClassType*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const;
	// Tools
	void m_CopySelected() const;
	void m_PasteFromClipboard( class EditorProgram* program );
	void m_DeleteSelected( class EditorProgram* program );
	void m_HideSelected();
	// Misc helpers
	void m_SetLevelPath( class EditorProgram* program, const char* path );
	void m_SelectWithModifiers( class EditorProgram* program, const ae::Entity* entities, uint32_t count );
	bool m_ShowVar( class EditorProgram* program, ae::Object* component, const ae::ClassVar* var );
	bool m_ShowVarValue( class EditorProgram* program, ae::Object* component, const ae::ClassVar* var, int32_t idx = -1 );
	bool m_ShowRefVar( class EditorProgram* program, ae::Object* component, const ae::ClassVar* var, int32_t idx = -1 );
	ae::Entity m_PickObject( class EditorProgram* program, ae::Vec3* hitOut, ae::Vec3* normalOut );
	ae::Color m_GetColor( ae::Entity entity, bool objectLineColor ) const;
	void m_LoadLevel( class EditorProgram* program );
	
	template< typename T > uint32_t m_PushDialog( const T& dialog );
	template< typename T > T* m_GetDialog( uint32_t id );
	void m_RemoveDialog( uint32_t id );
	
	const ae::Tag m_tag;
	bool m_first = true;

	ae::Str256 m_levelPath;
	const ae::File* m_pendingLevel = nullptr;

	bool m_showTransparent = true;

	// Manipulation
	const ae::ClassType* m_objectListType = nullptr;
	ae::Array< ae::Entity > m_selected;
	ae::Array< ae::Entity > m_hoverEntities;
	ae::Entity uiHoverEntity = kInvalidEntity;
	ae::Vec3 m_mouseHover = ae::Vec3( 0.0f );
	ae::Vec3 m_mouseHoverNormal = ae::Vec3( 0, 1, 0 );
	std::optional< ae::Vec2 > m_boxSelectStart;
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;

	// Object state
	ae::Map< ae::Entity, EditorServerObject*, 0, ae::Hash32, ae::MapMode::Stable > m_objects;
	ae::Registry m_registry;

	// UI configuration
	ae::Color m_selectionColor = ae::Color::PicoOrange();
	float m_objectHue = 3.7f;
	float m_objectHueRange = 0.3f;
	float m_objectSaturation = 1.0f;
	float m_objectSaturationRange = 0.0f;
	float m_objectValue = 0.6f;
	float m_objectValueRange = 0.3f;
	ae::Color m_backgroundColor = ae::Color::AetherBlack();

	// Connection to client
	double m_nextHeartbeat = 0.0;
	ae::Array< EditorConnection* > m_connections;
	uint8_t m_msgBuffer[ kMaxEditorMessageSize ];

	// Selection
	struct SelectRef
	{
		bool enabled = false;
		ae::Object* component = nullptr;
		const ae::ClassVar* componentVar = nullptr;
		int32_t varIdx = -1;
		
		ae::Entity pending = kInvalidEntity;
	};
	SelectRef m_selectRef;

	// Interpolation
	int32_t m_doRefocusImm = 0;
	bool m_doRefocusPrev = false;
	float m_refocusSize = 1.0f;
	float m_refocusHeldTime = 0.0f;

	// Misc
	ae::Map< ae::Entity, PickingType > m_framePickableEntities;

	// UI
	uint32_t m_dialogNextId = 1;
	ae::Array< EditorDialog* > m_dialogs;
	uint32_t m_appBundleWarningDialogId = 0;
};

class EditorProgram
{
public:
	EditorProgram( const ae::Tag& tag, const EditorParams& params, EditorPluginArray& plugins ) :
		m_tag( tag ),
		camera( params.worldUp ),
		debugLines( tag ),
		editor( tag ),
		params( params ),
		plugins( plugins )
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

	bool MakeWritable( const char* path ) const;
	
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	aeImGui ui;
	ae::DebugCamera camera;
	ae::DebugLines debugLines;
	ae::FileSystem fileSystem;
	EditorServer editor;
	EditorPluginArray& plugins;

	const EditorParams params;
	
	// Serialization
	class Serializer : public ae::ClassVar::Serializer
	{
	public:
		Serializer( EditorProgram* program ) : program( program ) { ae::ClassVar::SetSerializer( this ); }
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
public:
	ae::VertexBuffer m_introMesh;
	ae::VertexBuffer m_quad;
	ae::Shader m_introShader;
	ae::Shader m_meshShader;
	ae::Shader m_iconShader;
	ae::Texture2D m_cogTexture;
};

//------------------------------------------------------------------------------
// AppBundleWarningDialog
//------------------------------------------------------------------------------
bool AppBundleWarningDialog::ShowUIAndWaitForButton()
{
	const char* title = "Warning";
	const char* message = "Loading a level from within an app bundle is dangerous and may cause data loss. Are you sure you want to continue?";
	const char* buttonConfirm = "Yes";
	const char* buttonCancel = "No";

	bool press = false;
	ImGui::OpenPopup( title );
	if( ImGui::BeginPopupModal( title, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::TextWrapped( "%s", message );
		ImGui::Separator();
		if( ImGui::Button( buttonConfirm ) )
		{
			press = true;
			ImGui::CloseCurrentPopup();

			program->editor.OpenLevel( program, path.c_str() );
		}
		if( buttonCancel[ 0 ] )
		{
			ImGui::SameLine();
			if( ImGui::Button( buttonCancel ) )
			{
				press = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
	return press;
}

//------------------------------------------------------------------------------
// EditorServerMesh member functions
//------------------------------------------------------------------------------
void EditorServerMesh::Initialize( const ae::EditorMesh* _mesh )
{
	vertices.Clear();
	const uint32_t vertexCount = _mesh->verts.Length();
	const uint32_t indexCount = _mesh->indices.Length();
	if( indexCount )
	{
		vertices.Reserve( indexCount );
		for( uint32_t i = 0; i < indexCount; i += 3 )
		{
			ae::OBJLoader::Index index0 = _mesh->indices[ i ];
			ae::OBJLoader::Index index1 = _mesh->indices[ i + 1 ];
			ae::OBJLoader::Index index2 = _mesh->indices[ i + 2 ];
			if( index0 >= vertexCount || index1 >= vertexCount || index2 >= vertexCount )
			{
				AE_WARN( "Invalid index in editor mesh" );
				continue;
			}
			ae::Vec3 p0 = _mesh->verts[ index0 ];
			ae::Vec3 p1 = _mesh->verts[ index1 ];
			ae::Vec3 p2 = _mesh->verts[ index2 ];
			ae::Vec4 n( ( p2 - p1 ).Cross( p0 - p1 ).SafeNormalizeCopy(), 0.0f );
			vertices.Append( { ae::Vec4( p0, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p1, 1.0f ), n } );
			vertices.Append( { ae::Vec4( p2, 1.0f ), n } );
		}
	}
	else
	{
		vertices.Reserve( vertexCount );
		for( uint32_t i = 0; i < vertexCount; i += 3 )
		{
			ae::Vec3 p0 = _mesh->verts[ i ];
			ae::Vec3 p1 = _mesh->verts[ i + 1 ];
			ae::Vec3 p2 = _mesh->verts[ i + 2 ];
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
	window.Initialize( screens[ 0 ].position, screens[ 0 ].size.x, screens[ 0 ].size.y, true, true );
	window.SetMaximized( true );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	ui.Initialize();
	camera.Reset( ae::Vec3( 0.0f ), ae::Vec3( 10.0f ) );
	debugLines.Initialize( 2048 * 100 );
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
		AE_UNIFORM mat4 u_localToWorld;
		AE_UNIFORM mat4 u_normalToWorld;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_normal;
		AE_OUT_HIGHP vec3 v_position;
		AE_OUT_HIGHP vec3 v_normal;
		void main()
		{
			v_position = ( u_localToWorld * a_position ).xyz;
			v_normal = ( u_normalToWorld * a_normal ).xyz;
			gl_Position = u_localToProj * a_position;
		})";
	const char* meshFragShader = R"(
		AE_UNIFORM vec3 u_cameraPos;
		AE_UNIFORM vec3 u_lightDir;
		AE_UNIFORM vec4 u_color;
		AE_IN_HIGHP vec3 v_position;
		AE_IN_HIGHP vec3 v_normal;
		void main()
		{
			float light = dot( -u_lightDir, normalize( v_normal ) );
			light = 0.3 + 0.7 * max( 0.0, light );
			
			// Blinn-Phong specular
			vec3 viewDir = normalize( u_cameraPos - v_position );
			vec3 halfDir = normalize( -u_lightDir + viewDir );
			float spec = pow( max( dot( halfDir, normalize( v_normal ) ), 0.0 ), 32.0 );
			light += spec;

			// Unit scale 3D checker grid
			vec3 p = v_position;
			vec3 ip = floor( p );
			vec3 fp = p - ip;
			float checker = mod( ip.x + ip.y + ip.z, 2.0 );
			float dist = length( u_cameraPos - v_position );
			float fade = 1.0 - clamp( ( dist - 100.0 ) / 100.0, 0.0, 1.0 );
			checker = 1.0 - 0.2 * checker * fade;
			light *= checker;

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
	m_cogTexture.Initialize( kCogTextureData, kCogTextureDataSize, kCogTextureDataSize, ae::Texture::Format::R8, ae::Texture::Type::UInt8, ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp, true );

	editor.Initialize( this );
}

void EditorProgram::Terminate()
{
	AE_INFO( "Terminate" );
	editor.Unload( this );
	
	{
		EditorEvent event;
		event.type = EditorEventType::Terminate;
		SendPluginEvent( plugins, event );
	}
	for( auto& [ config, plugin ] : plugins )
	{
		AE_ASSERT_MSG( !plugin->m_meshRefs.Length(), "Plugin '#' still has mesh refs on level unload", config.name );
		AE_ASSERT_MSG( !plugin->m_instances.Length(), "Plugin '#' still has instances on level unload", config.name );
		AE_ASSERT_MSG( !plugin->m_entityInstances.Length(), "Plugin '#' still has entity instances on level unload", config.name );
	}
	
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
	while( !input.quit )
	{
		m_dt = timeStep.GetDt();
		if( m_dt > 0.0f && m_dt < timeStep.GetTimeStep() * 4.0f )
		{
			m_dt = timeStep.GetDt();
		}
		else
		{
			m_dt = 0.00001f;
		}

		input.Pump();
		ui.NewFrame( &render, &input, GetDt() );
		const ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );
		static bool s_once = true;
		if( s_once )
		{
			const ImGuiID dockTopId = ImGui::DockBuilderSplitNode( mainDockSpace, ImGuiDir_Left, 0.2f, nullptr, nullptr );
			ImGui::DockBuilderDockWindow( "Dev", dockTopId );
			ImGui::DockBuilderGetNode( dockTopId )->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			const ImGuiID dockBottomId = ImGui::DockBuilderSplitNode( dockTopId, ImGuiDir_Down, 0.2f, nullptr, nullptr );
			ImGui::DockBuilderDockWindow( "Settings", dockBottomId );
			ImGui::DockBuilderGetNode( dockBottomId )->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
			
			s_once = false;
		}

		editor.Update( this );

		r0 += timeStep.GetDt() * 0.6f;
		r1 += timeStep.GetDt() * 0.75f;

		camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse || ImGuizmo::IsOver() );
		camera.Update( &input, GetDt() );

		const ae::Int2 oldSize( m_gameTarget.GetWidth(), m_gameTarget.GetHeight() );
		const ae::Int2 targetSize = GetRenderRect().GetSize();
		if( oldSize != targetSize )
		{
			m_gameTarget.Initialize( targetSize.x, targetSize.y );
			m_gameTarget.AddTexture( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
			m_gameTarget.AddDepth( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
		}
		m_gameTarget.Activate();
		m_gameTarget.Clear( editor.GetBackgroundColor() );

		m_worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
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

		if( ImGui::Begin( "Dev" ) )
		{
			editor.ShowUI( this );
			ImGui::End();
		}
		if( const ImGuiWindow* imWin = ImGui::FindWindowByName( "Dev" ) )
		{
			m_barWidth = imWin->Size.x * ImGui::GetIO().DisplayFramebufferScale.x;
		}
		
		if( !editor.GetObjectCount() )
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

bool EditorProgram::MakeWritable( const char* path ) const
{
	ae::Str256 absolutePath;
	if( !path ||
		!path[ 0 ] ||
		!fileSystem.GetAbsolutePath( ae::FileSystem::Root::Data, path, &absolutePath ) )
	{
		return false;
	}
	else if( fileSystem.IsWritable( absolutePath.c_str() ) )
	{
		return true;
	}

	ae::EditorEvent event;
	event.type = ae::EditorEventType::FileEdit;
	event.path = path;
	SendPluginEvent( plugins, event );
	
	return fileSystem.IsWritable( absolutePath.c_str() );
}

//------------------------------------------------------------------------------
// ae::EditorMesh member functions
//------------------------------------------------------------------------------
ae::EditorMesh::EditorMesh( const ae::Tag& tag ) :
	verts( tag ),
	indices( tag )
{}

void ae::EditorMesh::Load( const ae::OBJLoader& file )
{
	verts.Clear();
	indices.Clear();
	verts.Reserve( file.vertices.Length() );
	indices.Reserve( file.indices.Length() );
	for( const auto& vert : file.vertices )
	{
		verts.Append( vert.position.GetXYZ() );
	}
	for( auto index : file.indices )
	{
		indices.Append( index );
	}
}

//------------------------------------------------------------------------------
// ae::EditorPlugin member functions
//------------------------------------------------------------------------------
EditorPlugin::EditorPlugin( const ae::Tag& tag ) :
	m_tag( tag ),
	m_meshRefs( tag ),
	m_instances( tag ),
	m_entityInstances( tag )
{}

EditorMeshInstance* EditorPlugin::CreateMesh( const EditorMesh& _mesh, ae::Entity selectEntity )
{
	if( !_mesh.verts.Length() )
	{
		return nullptr;
	}
	EditorServerMesh* mesh = ae::New< EditorServerMesh >( m_tag, m_tag );
	mesh->Initialize( &_mesh );
	AE_DEBUG_ASSERT( !m_meshRefs.TryGet( mesh ) );
	m_meshRefs.Set( mesh, 1 );
	
	EditorMeshInstance* instance = ae::New< EditorMeshInstance >( m_tag );
	instance->m_mesh = mesh;
	instance->m_selectEntity = selectEntity;
	if( selectEntity )
	{
		ae::List< EditorMeshInstance >* entityInstances = m_entityInstances.Get( selectEntity, nullptr );
		if( !entityInstances )
		{
			entityInstances = ae::New< ae::List< EditorMeshInstance > >( m_tag );
			m_entityInstances.Set( selectEntity, entityInstances );
		}
		entityInstances->Append( instance->m_entityInstance );
	}
	m_instances.Set( instance, true );
	//AE_DEBUG( "Create mesh-># entity:# refs:1", instance, selectEntity );
	return instance;
}

EditorMeshInstance* EditorPlugin::CloneMesh( const EditorMeshInstance* _instance, ae::Entity selectEntity )
{
	if( !_instance )
	{
		return nullptr;
	}

	EditorServerMesh* mesh = _instance->m_mesh;
	int32_t& refCount = m_meshRefs.Get( mesh );
	AE_ASSERT( refCount >= 1 );
	refCount++;

	EditorMeshInstance* instance = ae::New< EditorMeshInstance >( m_tag );
	instance->m_mesh = mesh;
	instance->m_selectEntity = selectEntity;
	if( selectEntity )
	{
		ae::List< EditorMeshInstance >* entityInstances = m_entityInstances.Get( selectEntity, nullptr );
		if( !entityInstances )
		{
			entityInstances = ae::New< ae::List< EditorMeshInstance > >( m_tag );
			m_entityInstances.Set( selectEntity, entityInstances );
		}
		entityInstances->Append( instance->m_entityInstance );
	}
	m_instances.Set( instance, true );
	//AE_DEBUG( "Clone mesh-># source:# entity:# refs:#", instance, _instance, selectEntity, refCount );
	return instance;
}

void EditorPlugin::DestroyMesh( EditorMeshInstance* instance )
{
	if( !instance )
	{
		return;
	}

	const ae::Entity entity = instance->m_selectEntity;
	const bool removed = m_instances.Remove( instance );
	AE_ASSERT( removed );
	if( entity )
	{
		ae::List< EditorMeshInstance >* entityInstances = m_entityInstances.Get( entity, nullptr );
		AE_ASSERT( entityInstances );
		entityInstances->Remove( instance->m_entityInstance );
		if( !entityInstances->Length() )
		{
			m_entityInstances.Remove( entity );
			ae::Delete( entityInstances );
		}
	}

	EditorServerMesh* mesh = instance->m_mesh;
	int32_t& refCount = m_meshRefs.Get( mesh );
	AE_ASSERT( refCount >= 1 );
	refCount--;
	if( refCount == 0 )
	{
		m_meshRefs.Remove( mesh );
		ae::Delete( mesh );
	}
	ae::Delete( instance );
	//AE_DEBUG( "Destroy mesh entity:# refs:#", entity, refCount );
}

//------------------------------------------------------------------------------
// Editor member functions
//------------------------------------------------------------------------------
Editor::Editor( const ae::Tag& tag ) :
	m_tag( tag ),
	m_sock( tag ),
	m_plugins( tag ),
	m_editorEntities( tag )
{
	AE_ASSERT( m_tag != ae::Tag() );
}

Editor::~Editor()
{
	Terminate();
}

bool Editor::Initialize( const EditorParams& params )
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
		EditorProgram program( m_tag, params, m_plugins );
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
			program.editor.OpenLevelWithPrompts( &program, params.argv[ levelArg ] );
		}
		else if( !params.levelPath.Empty() )
		{
			program.editor.OpenLevelWithPrompts( &program, params.levelPath.c_str() );
		}

		program.Run();
		program.Terminate();
		return true;
	}
	m_Connect();
	return false;
}

void Editor::Terminate()
{
#if !_AE_EMSCRIPTEN_
	m_sock.Disconnect();
#endif

	if( m_pendingLevel )
	{
		m_fileSystem.Destroy( m_pendingLevel );
		m_pendingLevel = nullptr;
	}

	for( auto& [ _, plugin ] : m_plugins )
	{
		ae::Delete( plugin );
	}
	m_plugins.Clear();

	ae::Delete( m_params );
	m_params = nullptr;
}

void Editor::Launch()
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );
	m_Connect();
	if( !m_sock.IsConnected() )
	{
		m_Fork();
	}
}

void Editor::Update()
{
	AE_ASSERT_MSG( m_params, "Must call Editor::Initialize()" );

	m_Read();

	if( m_params->port == 0 )
	{
		return;
	}
	m_Connect();
	
	uint32_t msgLength = 0;
	while( ( msgLength = m_sock.ReceiveMsg( m_msgBuffer, sizeof(m_msgBuffer) ) ) )
	{
		EditorNetMsg msgType = EditorNetMsg::None;
		ae::BinaryReader rStream( m_msgBuffer, sizeof(m_msgBuffer) );
		rStream.SerializeEnum( msgType );
		switch( msgType )
		{
			case EditorNetMsg::Heartbeat:
			{
				// Nothing
				break;
			}
			case EditorNetMsg::Modification:
			{
				ae::Entity entity;
				ae::TypeId typeId;
				ae::Str32 varName;
				ae::Str256 varValue;
				rStream.SerializeUInt32( entity );
				rStream.SerializeUInt32( typeId );
				rStream.SerializeString( varName );
				rStream.SerializeString( varValue );
				if( rStream.IsValid() )
				{
					// Make sure the entity being modified was created by the
					// editor, and is not just another game entity with the same id
					if( m_editorEntities.TryGet( entity ) )
					{
						if( const ae::ClassType* type = ae::GetClassTypeById( typeId ) )
						{
							if( ae::Component* component = m_params->registry->TryGetComponent( entity, type ) )
							{
								if( const ae::ClassVar* var = type->GetVarByName( varName.c_str(), true ) )
								{
									var->SetObjectValueFromString( component, varValue.c_str() );
								}
							}
						}
					}
				}
				break;
			}
			case EditorNetMsg::Load:
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
	if( m_pendingLevel )
	{
		AE_WARN( "Cancelling level read '#'", m_pendingLevel->GetUrl() );
		m_fileSystem.Destroy( m_pendingLevel );
	}
	m_pendingLevel = m_fileSystem.Read( ae::FileSystem::Root::Data, levelPath, 2.0f );
	AE_INFO( "Queuing level load '#'", m_pendingLevel->GetUrl() );
}

// @TODO: Combine. EditorServer::m_LoadLevel(), Editor::m_Read(), and EditorServer::m_PasteFromClipboard() are very similar
void Editor::m_Read()
{
	if( !m_pendingLevel || m_pendingLevel->GetStatus() == ae::File::Status::Pending )
	{
		return;
	}
	ae::RunOnDestroy destroyFile{ [this]()
	{
		m_fileSystem.Destroy( m_pendingLevel );
		m_pendingLevel = nullptr;
	} };
	uint32_t fileSize = m_pendingLevel->GetLength();
	if( !fileSize )
	{
		return;
	}

	const char* jsonBuffer = (const char*)m_pendingLevel->GetData();
	AE_ASSERT( jsonBuffer[ m_pendingLevel->GetLength() ] == 0 );
	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse( jsonBuffer );
	if( parseResult.IsError() )
	{
		AE_ERR( "Could not parse json '#' Error:# (#)",
			m_pendingLevel->GetUrl(),
			rapidjson::GetParseError_En( parseResult.Code() ),
			parseResult.Offset()
		);
		return;
	}

	if( !ValidateLevel( document ) )
	{
		AE_ERR( "Invalid level data format '#'", m_pendingLevel->GetUrl() );
		return;
	}

	m_lastLoadedLevel = m_pendingLevel->GetUrl();

	// @HACK: Currently two LevelUnload are sent
	{
		EditorEvent event;
		event.type = EditorEventType::LevelUnload;
		event.path = m_lastLoadedLevel.c_str();
		SendPluginEvent( m_plugins, event );
	}

	// Clear previous level
	m_editorEntities.Clear();

	// State for loading
	ae::Array< const ae::ClassType* > requirements = m_tag;
	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag;
	const auto& jsonObjects = document[ "objects" ];

	// Create all components
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const char* entityName = jsonObject.HasMember( "name" ) ? jsonObject[ "name" ].GetString() : "";
		const ae::Matrix4 entityTransform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString(), ae::Matrix4::Identity() );
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = m_params->registry->CreateEntity( jsonEntity, entityName );
		
		// Record which entities have been created by the editor
		m_editorEntities.Set( entity, true );
		
		if( entity != jsonEntity )
		{
			entityMap.Set( jsonEntity, entity );
		}
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			AE_ASSERT( componentIter.value.IsObject() );
			const ae::ClassType* type = ae::GetClassTypeByName( componentIter.name.GetString() );
			AE_ASSERT_MSG( type, "Type '#' not found. Register with AE_REGISTER_CLASS(), or if the class isn't directly referenced you may need to use AE_FORCE_LINK().", componentIter.name.GetString() );
			GetComponentTypeRequirements( type, &requirements );
			for( const ae::ClassType* requirement : requirements )
			{
				m_params->registry->AddComponent( entity, requirement );
			}
			m_params->registry->AddComponent( entity, type );
		}
	}

	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, jsonObjects, m_params->registry );

	AE_INFO( "Loaded level '#'", m_pendingLevel->GetUrl() );

	{
		EditorEvent event;
		event.type = EditorEventType::LevelLoad;
		event.path = m_pendingLevel->GetUrl();
		SendPluginEvent( m_plugins, event );
	}
}

void Editor::m_Connect()
{
#if !_AE_EMSCRIPTEN_
	if( m_params->port && !m_sock.IsConnected() )
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
	if( m_transform != transform )
	{
		m_transform = transform;
		program->editor.HandleTransformChange( program, entity, transform );
	}
}

const ae::Matrix4& EditorServerObject::GetTransform() const
{
	return m_transform;
}

void EditorServerObject::HandleVarChange( EditorProgram* program, ae::Component* component, const ae::ClassType* type, const ae::ClassVar* var )
{
	renderDisabled = program->editor.GetRenderDisabled( component->GetEntity() );
	program->editor.BroadcastVarChange( var, component );

	EditorEvent event;
	event.type = EditorEventType::ComponentEdit;
	event.entity = component->GetEntity();
	event.transform = GetTransform();
	event.component = component;
	event.var = var;
	SendPluginEvent( program->plugins, event );
}

ae::OBB EditorServerObject::GetOBB( EditorProgram* program ) const
{
	ae::AABB result;
	for( auto& [ _, plugin ] : program->plugins )
	{
		ae::List< EditorMeshInstance >* entityInstances = plugin->m_entityInstances.Get( entity, nullptr );
		if( entityInstances )
		{
			for( const EditorMeshInstance* instance = entityInstances->GetFirst(); instance; instance = instance->m_entityInstance.GetNext() )
			{
				AE_ASSERT( instance->m_mesh );
				const ae::OBB obb = instance->transform * instance->m_mesh->collision.GetAABB().GetTransform();
				result.Expand( obb.GetAABB() );
			}
		}
	}
	if( result != ae::AABB() )
	{
		return ae::OBB( result.GetTransform() );
	}
	else
	{
		return ae::OBB( GetTransform() ); // Don't include the pivot
	}
}

ae::AABB EditorServerObject::GetAABB( EditorProgram* program ) const
{
	return GetOBB( program ).GetAABB();
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
EditorServer::~EditorServer()
{
	AE_DEBUG_ASSERT( !m_objects.Length() );
	// AE_DEBUG_ASSERT( !m_registry.GetEntityCount() ); // @TODO
	AE_DEBUG_ASSERT( !m_connections.Length() );
}

void EditorServer::Initialize( EditorProgram* program )
{
	uint32_t typeCount = ae::GetClassTypeCount();
	for( uint32_t i = 0; i < typeCount; i++ )
	{
		const ae::ClassType* type = ae::GetClassTypeByIndex( i );
		const bool isEditorType = type->attributes.Has< ae::EditorTypeAttribute >();
		const ae::Str128 typeMessage = ae::Str128::Format( isEditorType ? "Type '#'" : "Non-editor type '#'", type->GetName() );

		const uint32_t requiredCount = type->attributes.GetCount< ae::EditorRequiredAttribute >();
		for( uint32_t i = 0; i < requiredCount; i++ )
		{
			const ae::EditorRequiredAttribute* required = type->attributes.TryGet< ae::EditorRequiredAttribute >( i );
			const char* requiredName = required->className.c_str();
			if( !ae::GetClassTypeByName( requiredName ) )
			{
				AE_ERR( "# has unknown dependency '#'", typeMessage, type->GetName(), requiredName );
			}
		}

		const uint32_t varCount = type->GetVarCount( true );
		for( uint32_t j = 0; j < varCount; j++ )
		{
			const ae::ClassVar* var = type->GetVarByIndex( j, true );
			const ae::BasicType* varType = var->GetOuterVarType().AsVarType< ae::BasicType >();
			if( var->attributes.Has< ae::EditorVisibilityAttribute >() )
			{
				if( !varType || varType->GetType() != ae::BasicType::Bool )
				{
					AE_ERR( "ae::EditorVisibilityAttribute variable '#' must be of type Bool", var->GetName() );
				}
			}
		}
	}

	m_SetLevelPath( program, "" );
}

// @TODO: Combine. EditorServer::m_LoadLevel(), Editor::m_Read(), and EditorServer::m_PasteFromClipboard() are very similar
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

	if( !program->MakeWritable( m_pendingLevel->GetUrl() ) )
	{
		const ae::Str512 msg = ae::Str512::Format( "File may not be writable '#'", m_pendingLevel->GetUrl() );
		AE_WARN( msg.c_str() );
		ae::ShowMessage( msg.c_str() );
	}

	AE_INFO( "Loading level... '#'", m_pendingLevel->GetUrl() );
	
	const char* jsonBuffer = (const char*)m_pendingLevel->GetData();
	AE_ASSERT( jsonBuffer[ m_pendingLevel->GetLength() ] == 0 ); // ae::File::Read() should always add a null terminator
	
	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse( jsonBuffer );
	if( parseResult.IsError() )
	{
		AE_ERR( "Could not parse json '#' Error:# (#)",
			m_pendingLevel->GetUrl(),
			rapidjson::GetParseError_En( parseResult.Code() ),
			parseResult.Offset()
		);
		return;
	}

	if( !ValidateLevel( document ) )
	{
		AE_ERR( "Invalid level data format '#'", m_pendingLevel->GetUrl() );
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
			AddComponent( program, object, ae::GetClassTypeByName( componentIter.name.GetString() ) );
		}
	}
	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, jsonObjects, &m_registry );
	// Refresh editor objects
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ "id" ].GetUint();
		const ae::Entity entity = entityMap.Get( jsonEntity, jsonEntity );
		EditorServerObject* object = m_objects.Get( entity );
		AE_ASSERT( object );
		for( const auto& componentIter : jsonObject[ "components" ].GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				continue;
			}
			const ae::ClassType* type = ae::GetClassTypeByName( componentIter.name.GetString() );
			if( !type )
			{
				continue;
			}
			ae::Component* component = &m_registry.GetComponent( entity, type );
			const uint32_t varCount = type->GetVarCount( true );
			for( uint32_t j = 0; j < varCount; j++ )
			{
				const ae::ClassVar* var = type->GetVarByIndex( j, true );
				object->HandleVarChange( program, component, type, var );
			}
		}
		// @TODO: Explicitly handle setting transform vars?
	}

	AE_INFO( "Loaded level '#'", m_pendingLevel->GetUrl() );
	m_SetLevelPath( program, m_pendingLevel->GetUrl() );
}

template< typename T >
uint32_t EditorServer::m_PushDialog( const T& _dialog )
{
	T* dialog = ae::New< T >( m_tag, _dialog );
	m_dialogs.Append( dialog );
	dialog->id = m_dialogNextId;
	m_dialogNextId++;
	return dialog->id;
}

template< typename T >
T* EditorServer::m_GetDialog( uint32_t id )
{
	for( EditorDialog* dialog : m_dialogs )
	{
		if( dialog->id == id )
		{
			return static_cast< T* >( dialog );
		}
	}
	return nullptr;
}

void EditorServer::m_RemoveDialog( uint32_t id )
{
	if( id )
	{
		const int32_t index = m_dialogs.FindFn( [ id ]( const EditorDialog* d ) { return d->id == id; } );
		if( index >= 0 )
		{
			ae::Delete( m_dialogs[ index ] );
			m_dialogs.Remove( index );
		}
	}
}

void EditorServer::Terminate( EditorProgram* program )
{
	Unload( program );
	for( EditorConnection* conn : m_connections )
	{
		conn->Destroy( this );
		ae::Delete( conn );
	}
	m_connections.Clear();
}

void EditorServer::Update( EditorProgram* program )
{
	const double currentTime = ae::GetTime();
	m_LoadLevel( program ); // Checks if a pending level is ready to load
	
	if( !sock.IsListening() )
	{
		sock.Listen( ae::Socket::Protocol::TCP, false, program->params.port, 8 );
	}
	while( ae::Socket* newConn = sock.Accept() )
	{
		AE_INFO( "ae::Editor client connected from #:#", newConn->GetResolvedAddress(), newConn->GetPort() );
		EditorConnection* editorConn = m_connections.Append( ae::New< EditorConnection >( m_tag ) );
		editorConn->sock = newConn;
	}
	AE_ASSERT( m_connections.Length() == sock.GetConnectionCount() );
	
	// Send a heartbeat to keep the connection alive, and to check if the client is still connected
	if( currentTime > m_nextHeartbeat )
	{
		for( EditorConnection* (&conn) : m_connections )
		{
			if( conn->sock->IsConnected() )
			{
				ae::BinaryWriter wStream( m_msgBuffer, sizeof(m_msgBuffer) );
				wStream.SerializeEnum( EditorNetMsg::Heartbeat );
				AE_ASSERT( wStream.IsValid() );
				conn->sock->QueueMsg( wStream.GetData(), (uint16_t)wStream.GetOffset() );
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

	// Camera update
	const float kTapTime = 0.15f;
	const bool couldSnapRefocus = ( m_refocusHeldTime < kTapTime );
	ae::DebugCamera* camera = &program->camera;
	if( m_doRefocusImm > 0 )
	{
		const float dt = program->timeStep.GetDt();
		const ae::Vec3 cameraPivot = camera->GetPivot();
		const float posDiff = ( cameraPivot - m_mouseHover ).Length();
		const float resize = posDiff - m_refocusSize;
		if( resize > 0.0001f )
		{
			// Linearly grow refocus speed based on zoom level
			m_refocusSize += camera->GetDistanceFromPivot() * 0.1f * dt;
		}
		else
		{
			// Exponentially shrink refocus speed based on pivot distance from the target
			m_refocusSize = ae::DtLerp( m_refocusSize, 2.5f, dt, posDiff );
		}
		m_refocusSize = ae::Max( m_refocusSize, 0.0f );
		const ae::Sphere sphere( cameraPivot, m_refocusSize );
		const ae::Vec3 nearestPoint = sphere.GetNearestPointOnSurface( m_mouseHover );
		if( !couldSnapRefocus )
		{
			camera->Refocus( nearestPoint, 0.5f );
		}

		m_refocusHeldTime += dt;
		m_doRefocusImm--;
		if( m_doRefocusImm == 0 )
		{
			camera->Reset( camera->GetPivot(), camera->GetPosition() );
		}
	}
	
	if( !m_doRefocusImm && m_doRefocusPrev && couldSnapRefocus )
	{
		camera->Refocus( program->editor.m_selected.Length() ? program->editor.GetSelectedAABB( program ).GetCenter() : m_mouseHover );
	}
	m_doRefocusPrev = ( m_doRefocusImm > 0 );
}

void EditorServer::Render( EditorProgram* program )
{
	// Constants
	const ae::Vec3 camPos = program->camera.GetPosition();
	const ae::Vec3 camUp = program->camera.GetUp();
	const ae::Matrix4 worldToProj = program->GetWorldToProj();
	const ae::Vec3 lightDir = ( ( program->params.worldUp == ae::Axis::Z ) ? ae::Vec3( -2.0f, -3.0f, -4.0f ) : ae::Vec3( -2.0f, -4.0f, -3.0f ) ).SafeNormalizeCopy();

	// Reset entities for picking
	m_framePickableEntities.Clear();

	// Categories
	struct RenderObj
	{
		ae::Matrix4 transform;
		ae::Color color;
		const EditorServerMesh* mesh;
		float distanceSq;
	};
	ae::Array< RenderObj > opaqueObjects = m_tag;
	ae::Array< RenderObj > transparentObjects = m_tag;
	ae::Array< RenderObj > logicObjects = m_tag;

	// @TODO: Frustum culling

	// Split up render passes
	for( auto& [ _, plugin ] : program->plugins )
	{
		for( auto& [ instance, _ ] : plugin->m_instances )
		{
			if( instance->color.a > 0.01f && instance->m_mesh )
			{
				const EditorServerObject* obj = m_objects.Get( instance->m_selectEntity, nullptr );
				if( obj && obj->hidden )
				{
					continue;
				}

				ae::Array< RenderObj >* meshes = nullptr;
				if( instance->color.a > 0.99f )
				{
					meshes = &opaqueObjects;
				}
				else if( GetShowTransparent() )
				{
					meshes = &transparentObjects;
				}

				if( meshes )
				{
					const ae::Color editorColor = m_GetColor( obj->entity, false );
					RenderObj& renderObj = meshes->Append( {} );
					renderObj.transform = instance->transform;
					renderObj.color = editorColor;//instance->color.SetA( 1.0f ).Lerp( editorColor, editorColor.a ).SetA( instance->color.a );
					renderObj.mesh = instance->m_mesh;
					renderObj.distanceSq = ( camPos - instance->transform.GetTranslation() ).LengthSquared();
	
					// To enable picking for the next frame
					m_framePickableEntities.Set( instance->m_selectEntity, PickingType::Mesh );
				}
			}
		}
	}

	// @TODO: Add logic object cubes
	const uint32_t editorObjectCount = m_objects.Length();
	for( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* obj = m_objects.GetValue( i );
		if( !obj->renderDisabled && !obj->hidden && !m_framePickableEntities.TryGet( obj->entity ) )
		{
			RenderObj& renderObj = logicObjects.Append( {} );
			renderObj.transform = obj->GetTransform();
			renderObj.color = m_GetColor( obj->entity, false );
			renderObj.mesh = nullptr;
			renderObj.distanceSq = ( camPos - obj->GetTransform().GetTranslation() ).LengthSquared();

			// To enable picking for the next frame
			m_framePickableEntities.Set( obj->entity, PickingType::Logic );
		}
	}

	// Opaque and transparent meshes helper
	auto renderMesh = [ program, worldToProj, lightDir ]( const RenderObj& renderObj )
	{
		const ae::Matrix4& transform = renderObj.transform;
		ae::UniformList uniformList;
		uniformList.Set( "u_localToProj", worldToProj * transform );
		uniformList.Set( "u_localToWorld", transform );
		uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
		uniformList.Set( "u_cameraPos", program->camera.GetPosition() );
		uniformList.Set( "u_lightDir", lightDir );
		uniformList.Set( "u_color", renderObj.color.GetLinearRGBA() );
		renderObj.mesh->data.Bind( &program->m_meshShader, uniformList );
		renderObj.mesh->data.Draw();
	};

	// Opaque objects
	std::sort( opaqueObjects.begin(), opaqueObjects.end(), []( const RenderObj& a, const RenderObj& b ) { return a.distanceSq < b.distanceSq; } );
	program->m_meshShader.SetDepthTest( true );
	program->m_meshShader.SetDepthWrite( true );
	program->m_meshShader.SetBlending( false );
	for( const RenderObj& renderObj : opaqueObjects )
	{
		renderMesh( renderObj );
	}

	// Logic objects
	std::sort( logicObjects.begin(), logicObjects.end(), []( const RenderObj& a, const RenderObj& b ) { return a.distanceSq > b.distanceSq; } );
	for( const RenderObj& renderObj : logicObjects )
	{
		ae::UniformList uniformList;
		ae::Vec3 objPos = renderObj.transform.GetTranslation();
		ae::Vec3 toCamera = camPos - objPos;
		ae::Matrix4 modelToWorld = ae::Matrix4::Rotation( ae::Vec3( 0, 0, 1 ), ae::Vec3( 0, 1, 0 ), toCamera, camUp );
		modelToWorld.SetTranslation( objPos );
		uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
		uniformList.Set( "u_tex", &program->m_cogTexture );
		uniformList.Set( "u_color", renderObj.color.GetLinearRGBA() );
		program->m_quad.Bind( &program->m_iconShader, uniformList );
		program->m_quad.Draw();
		program->debugLines.AddOBB( renderObj.transform, renderObj.color );
	}

	// Transparent objects
	std::sort( transparentObjects.begin(), transparentObjects.end(), []( const RenderObj& a, const RenderObj& b )
	{
		return a.distanceSq > b.distanceSq;
	} );
	program->m_meshShader.SetDepthTest( true );
	program->m_meshShader.SetDepthWrite( false );
	program->m_meshShader.SetBlending( true );
	for( const RenderObj& renderObj : transparentObjects )
	{
		renderMesh( renderObj );
	}
}

void EditorServer::ShowUI( EditorProgram* program )
{
	const float dt = program->GetDt();

	if( ImGui::Begin( "Settings" ) )
	{
		ImGui::ColorEdit3( "Selection", m_selectionColor.data );
		ImGui::SliderFloat( "Hue", &m_objectHue, 0.0f, ae::TwoPi );
		ImGui::SliderFloat( "Hue Range", &m_objectHueRange, 0.0f, ae::TwoPi );
		ImGui::SliderFloat( "Saturation", &m_objectSaturation, 0.0f, 1.0f );
		ImGui::SliderFloat( "Saturation Range", &m_objectSaturationRange, 0.0f, 1.0f );
		ImGui::SliderFloat( "Value", &m_objectValue, 0.0f, 1.0f );
		ImGui::SliderFloat( "Value Range", &m_objectValueRange, 0.0f, 1.0f );
		ImGui::ColorEdit3( "Background", m_backgroundColor.data );
		ImGui::End();
	}

	if( m_dialogs.Length() )
	{
		EditorDialog* dialog = m_dialogs[ 0 ];
		if( dialog->ShowUIAndWaitForButton() )
		{
			m_RemoveDialog( dialog->id );
		}
	}
	
	if( program->camera.GetMode() != ae::DebugCamera::Mode::None || ImGui::GetIO().WantCaptureMouse )
	{
		// Make sure box select is cleared when interrupted by other UI
		m_boxSelectStart = std::nullopt;
		m_hoverEntities.Clear();
	}
	else
	{
		// Handle selection release and box select before updating hovered entities
		if( !program->input.mouse.leftButton && program->input.mousePrev.leftButton ) // Release
		{
			m_boxSelectStart = std::nullopt;

			ae::Entity hoverEntity = m_hoverEntities.Length() ? m_hoverEntities[ 0 ] : kInvalidEntity;
			if( m_selectRef.enabled )
			{
				uint32_t matchCount = 0;
				const ae::Object* lastMatch = nullptr;
				
				const ae::ClassType* refType = m_selectRef.componentVar->GetSubType();
				const uint32_t componentTypeCount = m_registry.GetTypeCount();
				for( uint32_t i = 0; i < componentTypeCount; i++ )
				{
					const ae::ClassType* hoverType = m_registry.GetTypeByIndex( i );
					if( hoverType->IsType( refType ) )
					{
						if( const ae::Component* hoverComponent = m_registry.TryGetComponent( hoverEntity, hoverType ) )
						{
							matchCount++;
							lastMatch = hoverComponent;
						}
					}
				}
				
				if( matchCount == 0 )
				{
					AE_INFO( "Invalid selection. No '#' component.", refType->GetName() );
				}
				else if( matchCount == 1 )
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
				m_SelectWithModifiers( program, m_hoverEntities.Data(), m_hoverEntities.Length() );
			}
		}

		// Update hovered entities and box select
		bool doPickingHover = false;
		m_hoverEntities.Clear();
		const ae::Vec2 mousePos( ImGui::GetMousePos().x, ImGui::GetMousePos().y );
		if( !program->input.mouse.leftButton ) // No press, just cursor hover
		{
			doPickingHover = true;
		}
		else if( !program->input.mousePrev.leftButton ) // Initial press
		{
			m_boxSelectStart = mousePos;
		}
		else if( m_boxSelectStart ) // Drag
		{
			const ae::Rect uiRect = ae::Rect::FromPoints( m_boxSelectStart.value(), mousePos );
			if( uiRect.GetSize().Length() < 4.0f )
			{
				doPickingHover = true;
			}
			else if( uiRect.GetWidth() > 2.0f && uiRect.GetHeight() > 2.0f )
			{
				const ae::Vec2 uiScale( ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplayFramebufferScale.y );
				const ae::RectInt renderRect = program->GetRenderRect();
				const ae::Vec2 selectStart = ( uiRect.GetMin() * uiScale - ae::Vec2( renderRect.GetPos() ) ) / ae::Vec2( renderRect.GetSize() );
				const ae::Vec2 selectEnd = ( uiRect.GetMax() * uiScale - ae::Vec2( renderRect.GetPos() ) ) / ae::Vec2( renderRect.GetSize() );
				const ae::Rect rect = ae::Rect::FromPoints( selectStart, selectEnd );
				const ae::Vec2 rectPos = rect.GetMin();

				// https://topic.alibabacloud.com/a/scissoring-by-projection-matrix_8_8_31885482.html
				ae::Matrix4 projToScissor;
				projToScissor.SetAxis( 0, ae::Vec4( 1.0f / rect.GetWidth(), 0.0f, 0.0f, 0.0f ) );
				projToScissor.SetAxis( 1, ae::Vec4( 0.0f, 1.0f / rect.GetHeight(), 0.0f, 0.0f ) );
				projToScissor.SetAxis( 2, ae::Vec4( 0.0f, 0.0f, 1.0f, 0.0f ) );
				float tx = ( 1.0f - ( 2.0f * rectPos.x + rect.GetWidth() ) ) / rect.GetWidth();
				float ty = ( -1.0f + ( 2.0f * rectPos.y + rect.GetHeight() ) ) / rect.GetHeight();
				projToScissor.SetAxis( 3, ae::Vec4( tx, ty, 0.0f, 1.0f ) );
				const ae::Matrix4 worldToScissor = projToScissor * program->GetWorldToProj();
				const ae::Frustum frustum( worldToScissor );

				// Mesh overlap
				for( auto&[_, plugin] : program->plugins )
				{
					for( auto&[instance, _] : plugin->m_instances )
					{
						AE_DEBUG_ASSERT( instance->m_mesh );
						if( !instance->m_mesh ||
							instance->color.a < 0.01f || // Skip invisible objects
							( instance->color.a < 0.99f && !GetShowTransparent() ) ) // Skip transparent objects
						{
							continue;
						}
						const EditorServerObject* editorObj = GetObject( instance->m_selectEntity ); // May be null
						if( editorObj && editorObj->hidden )
						{
							continue;
						}
						// @TODO: Triangle overlap instead of just vertices
						for( const EditorServerMesh::Vertex& vertex : instance->m_mesh->vertices )
						{
							if( frustum.Intersects( instance->transform.TransformPoint3x4( ae::Vec3( vertex.position ) ) ) )
							{
								m_hoverEntities.Append( instance->m_selectEntity );
								break;
							}
						}
					}
				}

				const uint32_t entityCount = m_objects.Length();
				for( uint32_t i = 0; i < entityCount; i++ )
				{
					const EditorServerObject* obj = m_objects.GetValue( i );
					if( !obj->hidden &&
						( m_framePickableEntities.Get( obj->entity, PickingType::Disabled ) == PickingType::Logic ) &&
						frustum.Intersects( ae::Sphere( obj->GetOBB( program ) ) ) )
					{
						const ae::Matrix4 modelToWorld = obj->GetTransform();
						const ae::Vec3 obbCorners[] =
						{
							ae::Vec3( -0.5f, -0.5f, -0.5f ),
							ae::Vec3(  0.5f, -0.5f, -0.5f ),
							ae::Vec3( -0.5f,  0.5f, -0.5f ),
							ae::Vec3(  0.5f,  0.5f, -0.5f ),
							ae::Vec3( -0.5f, -0.5f,  0.5f ),
							ae::Vec3(  0.5f, -0.5f,  0.5f ),
							ae::Vec3( -0.5f,  0.5f,  0.5f ),
							ae::Vec3(  0.5f,  0.5f,  0.5f )
						};
						// @TODO: Cube face overlap instead of just corners
						for( const ae::Vec3& corner : obbCorners )
						{
							if( frustum.Intersects( modelToWorld.TransformPoint3x4( corner ) ) )
							{
								m_hoverEntities.Append( obj->entity );
								break;
							}
						}
					}
				}
			}
		}
		
		if( doPickingHover )
		{
			const ae::Entity hoverEntity = m_PickObject( program, &m_mouseHover, &m_mouseHoverNormal );
			if( hoverEntity )
			{
				m_hoverEntities.Append( hoverEntity );
			}
		}
		else if( m_boxSelectStart )
		{
			const ImVec2 selectMin = ImVec2( ae::Min( m_boxSelectStart->x, mousePos.x ), ae::Min( m_boxSelectStart->y, mousePos.y ) );
			const ImVec2 selectMax = ImVec2( ae::Max( m_boxSelectStart->x, mousePos.x ), ae::Max( m_boxSelectStart->y, mousePos.y ) );
			const ae::Vec3 fColor = m_selectionColor.GetLinearRGB() * 255.0f;
			ImGui::GetBackgroundDrawList()->AddRect( selectMin, selectMax, IM_COL32( fColor.x, fColor.y, fColor.z, 255 ), 1.0f, ImDrawCornerFlags_All, 1.5f );
			ImGui::GetBackgroundDrawList()->AddRectFilled( selectMin, selectMax, IM_COL32( fColor.x, fColor.y, fColor.z, 100 ), 1.0f, ImDrawCornerFlags_All );
		}
	}
	
	if( m_selectRef.enabled && m_selectRef.pending )
	{
		ImGui::OpenPopup( "ref_select_popup" );
	}
	if( ImGui::BeginPopup( "ref_select_popup" ) )
	{
		const ae::ClassType* refType = m_selectRef.componentVar->GetSubType();
		ImGui::Text( "Select %s", refType->GetName() );
		ImGui::Separator();
		const EditorServerObject* pendingObj = GetObject( m_selectRef.pending );
		AE_ASSERT( pendingObj );
		const uint32_t componentTypeCount = m_registry.GetTypeCount();
		for( uint32_t i = 0; i < componentTypeCount; i++ )
		{
			const ae::ClassType* pendingType = m_registry.GetTypeByIndex( i );
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
	
	// Debug lines
	ae::Optional< ae::Vec3 > gridLineCenter;
	const ae::Vec3 pivot = program->camera.GetPivot();
	const float zoom = program->camera.GetDistanceFromPivot();
	{
		const ae::Vec3 showPivot = pivot - program->camera.GetForward() * ( zoom / 1000.0f );
		ae::Vec3 gridPivot = showPivot;
		gridPivot.x = ae::Floor( gridPivot.x );
		if( program->params.worldUp == ae::Axis::Z )
		{
			gridPivot.y = ae::Floor( gridPivot.y );
		}
		else
		{
			gridPivot.z = ae::Floor( gridPivot.z );
		}
		program->debugLines.AddSphere( showPivot, zoom / 55.0f, m_selectionColor, 4 );
		for( int i = -10; i <= 10; i++ )
		{
			const ae::Color gridColor = m_selectionColor.SetA( ae::Delerp01( 60.0f, 50.0f, zoom ) );
			program->debugLines.AddLine( gridPivot + ae::Vec3( i, -10, 0 ), gridPivot + ae::Vec3( i, 10, 0 ), gridColor );
			program->debugLines.AddLine( gridPivot + ae::Vec3( -10, i, 0 ), gridPivot + ae::Vec3( 10, i, 0 ), gridColor );
		}
	}
	if( m_selected.Length() )
	{
		const ae::Vec3 center = GetSelectedAABB( program ).GetCenter();
		const float distance = ( program->camera.GetPosition() - center ).Length();
		gridLineCenter = center - program->camera.GetForward() * ( distance / 1000.0f);
	}
	auto DrawBoxLocator = []( ae::DebugLines* debugLines, ae::Vec3 from, ae::Vec3 to, ae::Color color )
	{
		debugLines->AddLine( from, to, color );
		const ae::Vec3 cursorClosest0( from.x, from.y, to.z );
		debugLines->AddLine( from, cursorClosest0, color );
		debugLines->AddLine( to, cursorClosest0, color );
		const ae::Vec3 cursorClosest1( to.x, to.y, from.z );
		debugLines->AddLine( from, cursorClosest1, color );
		debugLines->AddLine( to, cursorClosest1, color );
	};
	if( m_hoverEntities.Length() )
	{
		const float distance = ( program->camera.GetPosition() - m_mouseHover ).Length();
		const ae::Vec3 pushOut = ( m_mouseHoverNormal * distance ) / 1000.0f;
		const ae::Vec3 mousePoint = m_mouseHover + pushOut;
		if( m_selected.Length() )
		{
			const ae::Vec3 selected = GetSelectedAABB( program ).GetCenter();
			DrawBoxLocator( &program->debugLines, mousePoint, selected, m_selectionColor );
		}
		program->debugLines.AddCircle( mousePoint, m_mouseHoverNormal, zoom / 55.0f, m_selectionColor, 4 );
		if( !m_selected.Length() )
		{
			gridLineCenter = mousePoint;
		}
	}
	if( ae::Vec3* gridCenter = gridLineCenter.TryGet() )
	{
		const float lineLength = 500.0f;
		program->debugLines.AddLine( *gridCenter + ae::Vec3( lineLength, 0, 0 ), *gridCenter - ae::Vec3( lineLength, 0, 0 ), ae::Color::Red() );
		program->debugLines.AddLine( *gridCenter + ae::Vec3( 0, lineLength, 0 ), *gridCenter - ae::Vec3( 0, lineLength, 0 ), ae::Color::Green() );
		program->debugLines.AddLine( *gridCenter + ae::Vec3( 0, 0, lineLength ), *gridCenter - ae::Vec3( 0, 0, lineLength ), ae::Color::Blue() );

		DrawBoxLocator( &program->debugLines, pivot, *gridCenter, m_selectionColor );
	}
	if( m_selected.Length() )
	{
		const ae::AABB selectedAABB = GetSelectedAABB( program );
		program->debugLines.AddAABB( selectedAABB.GetCenter(), selectedAABB.GetHalfSize(), m_selectionColor );
	}
	for( auto&[ entity, _ ] : m_objects )
	{
		const ae::Color color = m_GetColor( entity, true );
		const EditorServerObject* editorObj = m_objects.Get( entity );
		if( ( color.a >= 0.001f ) && editorObj )
		{
			switch( m_framePickableEntities.Get( entity, PickingType::Disabled ) )
			{
				case PickingType::Mesh:
				{
					for( auto&[_, plugin] : program->plugins )
					{
						for( auto&[instance, _] : plugin->m_instances )
						{
							if( instance->m_selectEntity == entity && instance->m_mesh )
							{
								const ae::Matrix4 transform = instance->transform;
								const uint32_t vertexCount = instance->m_mesh->vertices.Length();
								const ae::EditorServerMesh::Vertex* verts = instance->m_mesh->vertices.Data();
								program->debugLines.AddMesh( (const ae::Vec3*)&verts->position, sizeof(*verts), vertexCount, transform, color );
							}
						}
					}
					break;
				}
				case PickingType::Logic:
				{
					ae::Vec3 pos = editorObj->GetTransform().GetTranslation();
					ae::Vec3 normal = program->camera.GetPosition() - pos;
					program->debugLines.AddCircle( pos, normal, 0.475f, color, 16 );
					break;
				}
				case PickingType::Disabled:
					continue;
			}
		}
	}

	
	if( ImGui::GetIO().WantCaptureKeyboard )
	{
		// Keyboard captured
	}
	else
	{
		struct Command
		{
			ae::Array< ae::Key, 4 > modifiers;
			ae::Key key;
			void (*fn)( EditorProgram* );
			bool continuous = false; // If true, the command will be executed continuously while the keys are pressed
		};
		Command commands[] =
		{
			/* Save */ { { ae::Key::Meta }, ae::Key::S, []( EditorProgram* program ) { program->editor.SaveLevel( program, false ); } },
			/* Save  As*/ { { ae::Key::Meta, ae::Key::Shift }, ae::Key::S, []( EditorProgram* program ) { program->editor.SaveLevel( program, true ); } },
			/* Open */ { { ae::Key::Meta }, ae::Key::O, []( EditorProgram* program ) { program->editor.OpenLevelDialog( program ); } },
			/* Copy */ { { ae::Key::Meta }, ae::Key::C, []( EditorProgram* program ) { program->editor.m_CopySelected(); } },
			/* Paste */ { { ae::Key::Meta }, ae::Key::V, []( EditorProgram* program ) { program->editor.m_PasteFromClipboard( program ); } },
			/* Select */ { {}, ae::Key::Q, []( EditorProgram* program ) { program->editor.gizmoOperation = ImGuizmo::OPERATION( 0 ); } },
			// Translate
			{
				{}, ae::Key::W,
				[]( EditorProgram* program )
				{
					if( program->editor.gizmoOperation == ImGuizmo::TRANSLATE )
					{
						program->editor.gizmoMode = ( program->editor.gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
					}
					else
					{
						program->editor.gizmoOperation = ImGuizmo::TRANSLATE;
					}
				}
			},
			// Rotate
		{
				{}, ae::Key::E,
				[]( EditorProgram* program )
				{
					if( program->editor.gizmoOperation == ImGuizmo::ROTATE )
					{
						program->editor.gizmoMode = ( program->editor.gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
					}
					else
					{
						program->editor.gizmoOperation = ImGuizmo::ROTATE;
					}
				}
			},
			// Scale
		{
				{}, ae::Key::R,
				[]( EditorProgram* program )
				{
					if( program->editor.gizmoOperation == ImGuizmo::SCALE )
					{
						program->editor.gizmoMode = ( program->editor.gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
					}
					else
					{
						program->editor.gizmoOperation = ImGuizmo::SCALE;
					}
				}
			},
			/* Toggle transparent */ { {}, ae::Key::I, []( EditorProgram* program ) { program->editor.m_showTransparent = !program->editor.m_showTransparent; } },
			/* Delete */ { {}, ae::Key::Delete, []( EditorProgram* program ) { program->editor.m_DeleteSelected( program ); } },
			/* Delete */ { {}, ae::Key::Backspace, []( EditorProgram* program ) { program->editor.m_DeleteSelected( program ); } },
			/* Focus */ { {}, ae::Key::F, []( EditorProgram* program ) { program->editor.ActiveRefocus( program ); }, true },
			/* Hide */ { {}, ae::Key::H, []( EditorProgram* program ) { program->editor.m_HideSelected(); } },
			/* Select none */ { {}, ae::Key::Escape, []( EditorProgram* program ) { program->editor.m_selected.Clear(); } },
		};
		// Longer combos first
		std::sort( std::begin( commands ), std::end( commands ),
			[]( const Command& a, const Command& b )
			{
				return a.modifiers.Length() > b.modifiers.Length();
			}
		);
		// Check for commands with more keys first to handle potential conflicts
		const uint32_t modifierCount =
			(uint32_t)program->input.Get( ae::Key::Meta ) +
			(uint32_t)program->input.Get( ae::Key::Shift ) +
			(uint32_t)program->input.Get( ae::Key::Control ) +
			(uint32_t)program->input.Get( ae::Key::Alt );
		for( const Command& command : commands )
		{
			if( modifierCount == command.modifiers.Length() ) // Exact match only
			{
				if( program->input.GetPress( command.key ) ||
					( command.continuous && program->input.Get( command.key ) ) )
				{
					bool allPressed = true;
					for( const ae::Key& modifier : command.modifiers )
					{
						if( !program->input.Get( modifier ) )
						{
							allPressed = false;
							break;
						}
					}
					if( allPressed )
					{
						command.fn( program );
						break; // Only execute the first matching command
					}
				}
			}
		}
	}
	
	if( m_selected.Length() && gizmoOperation )
	{
		const float scaleFactor = program->window.GetScaleFactor();
		const ae::RectInt renderRectInt = program->GetRenderRect();
		const ae::Rect renderRect = ae::Rect::FromPoints(
			ae::Vec2( renderRectInt.GetPos() ) / scaleFactor,
			ae::Vec2( renderRectInt.GetPos() + renderRectInt.GetSize() ) / scaleFactor
		);

		ImGuizmo::Enable( program->camera.GetMode() == ae::DebugCamera::Mode::None && !m_boxSelectStart );
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::AllowAxisFlip( false );
		ImGuizmo::BeginFrame();
		ImGuizmo::SetRect( renderRect.GetMin().x, renderRect.GetMin().y, renderRect.GetSize().x, renderRect.GetSize().y );
		
		EditorServerObject* selectedObject = m_objects.Get( m_selected[ 0 ] );
		ae::Matrix4 prevTransform = selectedObject->GetTransform();
		ae::Matrix4 transform = prevTransform;
		ImGuizmo::MODE mode = ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode;
		if( ImGuizmo::Manipulate(
			program->GetWorldToView().data,
			program->GetViewToProj().data,
			gizmoOperation,
			mode,
			transform.data ) )
		{
			selectedObject->SetTransform( transform, program );

			const ae::Matrix4 change = ( mode == ImGuizmo::LOCAL ) ? prevTransform.GetInverse() * transform : transform * prevTransform.GetInverse();
			for( ae::Entity entity : m_selected )
			{
				if( entity == m_selected[ 0 ] )
				{
					continue;
				}
				EditorServerObject* editorObject = m_objects.Get( entity );
				if( mode == ImGuizmo::LOCAL )
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
	
	if( m_first )
	{
		ImGui::SetNextItemOpen( true );
	}
	if( ImGui::TreeNode( "File" ) )
	{
		if( ImGui::Button( "Open" ) )
		{
			OpenLevelDialog( program );
		}
		ImGui::SameLine();
		if( ImGui::Button( "Save" ) )
		{
			SaveLevel( program, false );
		}
		ImGui::SameLine();
		if( ImGui::Button( "Save As" ) )
		{
			SaveLevel( program, true );
		}
		if( ImGui::Button( "New" ) )
		{
			AE_INFO( "New level" );
			Unload( program );
		}
		ImGui::SameLine();
		ImGui::BeginDisabled( m_levelPath.Empty() || !m_connections.Length() );
		if( ImGui::Button( "Game Load" ) && m_connections.Length() )
		{
			uint8_t buffer[ kMaxEditorMessageSize ];
			ae::BinaryWriter wStream( buffer, sizeof(buffer) );
			wStream.SerializeEnum( EditorNetMsg::Load );
			wStream.SerializeString( m_levelPath );
			for( uint32_t i = 0; i < m_connections.Length(); i++ )
			{
				m_connections[ i ]->sock->QueueMsg( wStream.GetData(), (uint16_t)wStream.GetOffset() );
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if( ImGui::Button( "Quit" ) )
		{
			program->input.quit = true;
		}
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if( m_first )
	{
		ImGui::SetNextItemOpen( true );
	}
	if( ImGui::TreeNode( "Tools" ) )
	{
		if( ImGui::RadioButton( "O", gizmoOperation == (ImGuizmo::OPERATION)0 ) )
		{
			gizmoOperation = (ImGuizmo::OPERATION)0;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "T", gizmoOperation == ImGuizmo::TRANSLATE ) )
		{
			gizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "R", gizmoOperation == ImGuizmo::ROTATE ) )
		{
			gizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "S", gizmoOperation == ImGuizmo::SCALE ) )
		{
			gizmoOperation = ImGuizmo::SCALE;
		}
		if( ImGui::RadioButton( "World", gizmoMode == ImGuizmo::WORLD ) )
		{
			gizmoMode = ImGuizmo::WORLD;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Local", gizmoMode == ImGuizmo::LOCAL ) )
		{
			gizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::Checkbox( "Show Transparent", &m_showTransparent );
		
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if( ImGui::TreeNode( "Operations" ) )
	{
		if( ImGui::Button( "Create" ) )
		{
			ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetPivot() );
			EditorServerObject* editorObject = CreateObject( kInvalidEntity, transform, "" );
			m_selected.Clear();
			m_selected.Append( editorObject->entity );
		}
		ImGui::BeginDisabled( !m_selected.Length() );
		if( ImGui::Button( "Delete" ) )
		{
			for( ae::Entity entity : m_selected )
			{
				DestroyObject( program, entity );
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
			EditorEvent event;
			event.type = EditorEventType::ReloadResources;
			SendPluginEvent( program->plugins, event );
		}
		
		static bool s_imGuiDemo = false;
		if( ImGui::Button( "Show ImGui Demo" ) )
		{
			s_imGuiDemo = !s_imGuiDemo;
		}
		if( s_imGuiDemo )
		{
			ImGui::ShowDemoWindow( &s_imGuiDemo );
		}
		
		ImGui::TreePop();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	if( ImGui::TreeNode( "Object properties" ) )
	{
		if( m_selected.Length() == 1 )
		{
			EditorServerObject* selectedObject = m_objects.Get( m_selected[ 0 ] );
			ImGui::Text( "Object %u", selectedObject->entity );
		
			char name[ ae::Str16::MaxLength() ];
			strcpy( name, m_registry.GetNameByEntity( m_selected[ 0 ] ) );
			if( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
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
				if( changed )
				{
					ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, temp.data );
					selectedObject->SetTransform( temp, program );
				}
				if( ImGui::Button( "Add Component" ) )
				{
					ImGui::OpenPopup( "add_component_popup" );
				}
			}
			
			ae::Component* deleteComponent = nullptr;
			const uint32_t componentTypesCount = m_registry.GetTypeCount();
			for( uint32_t i = 0; i < componentTypesCount; i++ )
			{
				const ae::ClassType* componentType = m_registry.GetTypeByIndex( i );
				ae::Component* component = m_registry.TryGetComponent( selectedObject->entity, componentType );
				if( component )
				{
					ImGui::Separator();
					if( ImGui::TreeNode( componentType->GetName() ) )
					{
						auto fn = [&]( auto& fn, const ae::ClassType* type, ae::Component* component ) -> void
						{
							uint32_t varCount = type->GetVarCount( false );
							if( varCount )
							{
								for( uint32_t i = 0; i < varCount; i++ )
								{
									const ae::ClassVar* var = type->GetVarByIndex( i, false );
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
			
			if( ImGui::BeginPopup( "add_component_popup" ) )
			{
				bool anyValid = false;
				bool foundAny = false;
				uint32_t typeCount = ae::GetClassTypeCount();
				for( uint32_t i = 0; i < typeCount; i++ )
				{
					const ae::ClassType* type = ae::GetClassTypeByIndex( i );
					if( !type->attributes.Has< EditorTypeAttribute >() || !type->IsDefaultConstructible() )
					{
						continue;
					}
					anyValid = true;
					
					if( GetComponent( selectedObject, type ) )
					{
						continue;
					}
					
					if( ImGui::Selectable( type->GetName() ) )
					{
						AE_INFO( "Create #", type->GetName() );
						AddComponent( program, selectedObject, type );
					}
					foundAny = true;
				}
				if( !anyValid )
				{
					ImGui::Text( "Use AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (MyComponent), (ae, EditorTypeAttribute), {} ) to register new types" );
				}
				else if( !foundAny )
				{
					ImGui::Text( "None" );
				}
				ImGui::EndPopup();
			}
		}
		else if( m_selected.Length() )
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
	if( ImGui::TreeNode( "Object List" ) )
	{
		const char* selectedTypeName = m_objectListType ? m_objectListType->GetName() : "All";
		if( ImGui::BeginCombo( "Type", selectedTypeName, 0 ) )
		{
			if( ImGui::Selectable( "All", !m_objectListType ) )
			{
				m_objectListType = nullptr;
			}
			if( !m_objectListType )
			{
				ImGui::SetItemDefaultFocus();
			}
			const uint32_t componentTypesCount = m_registry.GetTypeCount();
			for( uint32_t i = 0; i < componentTypesCount; i++ )
			{
				const ae::ClassType* type = m_registry.GetTypeByIndex( i );
				const bool isSelected = ( m_objectListType == type );
				if( ImGui::Selectable( type->GetName(), isSelected ) )
				{
					m_objectListType = type;
				}
				if( isSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if( ImGui::BeginListBox("##listbox", ImVec2( -FLT_MIN, 16 * ImGui::GetTextLineHeightWithSpacing() ) ) )
		{
			auto showObjInList = [&]( int idx, ae::Entity entity, const char* entityName )
			{
				ImGui::PushID( idx );
				const bool isSelected = ( m_selected.Find( entity ) >= 0 );
				ae::Str16 name = entityName;
				if( !name.Length() )
				{
					name = ae::Str16::Format( "#", entity );
				}
				if( ImGui::Selectable( name.c_str(), isSelected ) )
				{
					m_SelectWithModifiers( program, &entity, 1 );
				}
				if( ImGui::IsItemHovered() )
				{
					uiHoverEntity = entity;
				}
				if( isSelected )
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

void EditorServer::DestroyObject( EditorProgram* program, ae::Entity entity )
{
	const uint32_t idx = m_objects.GetIndex( entity );
	if( idx < 0 )
	{
		return;
	}
	EditorServerObject* editorObject = m_objects.GetValue( idx );
	AE_ASSERT( editorObject );

	const uint32_t componentTypesCount = m_registry.GetTypeCount();
	for( uint32_t i = 0; i < componentTypesCount; i++ )
	{
		const ae::ClassType* componentType = m_registry.GetTypeByIndex( i );
		if( ae::Component* component = m_registry.TryGetComponent( entity, componentType ) )
		{
			RemoveComponent( program, editorObject, component );
		}
	}

	if( m_objects.Remove( entity, &editorObject ) )
	{
		m_registry.Destroy( entity );
		editorObject->Terminate();
		ae::Delete( editorObject );
	}
}

ae::Component* EditorServer::AddComponent( EditorProgram* program, EditorServerObject* obj, const ae::ClassType* type )
{
	if( !type )
	{
		return nullptr;
	}
	ae::Component* component = GetComponent( obj, type );
	if( component )
	{
		return component;
	}

	ae::Array< const ae::ClassType* > prereqs = m_tag;
	GetComponentTypeRequirements( type, &prereqs );
	for( const ae::ClassType* prereq : prereqs )
	{
		if( !GetComponent( obj, prereq ) )
		{
			AddComponent( program, obj, prereq );
		}
	}
	component = m_registry.AddComponent( obj->entity, type );
	if( component )
	{
		EditorEvent event;
		event.type = EditorEventType::ComponentCreate;
		event.entity = obj->entity;
		event.transform = obj->GetTransform();
		event.component = component;
		SendPluginEvent( program->plugins, event );
	}
	return component;
}

void EditorServer::RemoveComponent( EditorProgram* program, EditorServerObject* obj, ae::Component* component )
{
	if( obj && component )
	{
		EditorEvent event;
		event.type = EditorEventType::ComponentDestroy;
		event.entity = obj->entity;
		event.transform = obj->GetTransform();
		event.component = component;
		SendPluginEvent( program->plugins, event );
		
		m_registry.DestroyComponent( component );
	}
}

ae::Component* EditorServer::GetComponent( EditorServerObject* obj, const ae::ClassType* type )
{
	return const_cast< ae::Component* >( GetComponent( const_cast< const EditorServerObject* >( obj ), type ) );
}

const ae::Component* EditorServer::GetComponent( const EditorServerObject* obj, const ae::ClassType* type )
{
	return obj ? m_registry.TryGetComponent( obj->entity, type ) : nullptr;
}

const EditorServerObject* EditorServer::GetObjectFromComponent( const ae::Component* component )
{
	return component ? GetObject( component->GetEntity() ) : nullptr;
}

bool EditorServer::GetRenderDisabled( ae::Entity entity ) const
{
	// @TODO: Gather 'ae::EditorVisibilityAttribute' from all entity components
	// const uint32_t variableCount = meshComponentType->GetVarCount( true );
	// for( uint32_t i = 0; i < variableCount; i++ )
	// {
	// 	const ae::ClassVar* var = meshComponentType->GetVarByIndex( i, true );
	// 	if( var->attributes.TryGet< ae::EditorVisibilityAttribute >() )
	// 	{
	// 		const ae::ConstDataPointer varData( var, meshComponent );
	// 		const ae::BasicType* basicType = varData.GetVarType().AsVarType< ae::BasicType >();
	// 		if( basicType && basicType->GetType() == ae::BasicType::Type::Bool )
	// 		{
	// 			bool visible = true;
	// 			if( basicType->GetVarData( varData, &visible ) && !visible )
	// 			{
	// 				return true;
	// 			}
	// 		}
	// 	}
	// }
	return false;
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

	EditorEvent event;
	event.type = EditorEventType::ComponentEdit;
	event.entity = entity;
	event.transform = editorObject->GetTransform();
	SendPluginEvent( program->plugins, event );

	const uint32_t typeCounts = m_registry.GetTypeCount();
	for( uint32_t i = 0; i < typeCounts; i++ )
	{
		const ae::ClassType* componentType = m_registry.GetTypeByIndex( i );
		if( ae::Component* component = m_registry.TryGetComponent( entity, componentType ) )
		{
			for( const auto& specialVar : kSpecialMemberVars )
			{
				const auto vars = GetTypeVarsByName( componentType, specialVar.name );
				for( const ae::ClassVar* var : vars )
				{
					if( specialVar.SetObjectValue( transform, component, var ) )
					{
						editorObject->HandleVarChange( program, component, componentType, var );
					}
				}
			}
		}
	}
}

void EditorServer::BroadcastVarChange( const ae::ClassVar* var, const ae::Component* component )
{
	if( var->IsArray() )
	{
		// @TODO: Broadcast array element changes
		return;
	}
	ae::BinaryWriter wStream( m_msgBuffer, sizeof(m_msgBuffer) );
	wStream.SerializeEnum( EditorNetMsg::Modification );
	wStream.SerializeUInt32( component->GetEntity() );
	wStream.SerializeUInt32( ae::GetObjectTypeId( component ) );
	wStream.SerializeString( var->GetName() );
	wStream.SerializeString( var->GetObjectValueAsString( component ).c_str() );
	if( wStream.IsValid() )
	{
		for( EditorConnection* conn : m_connections )
		{
			conn->sock->QueueMsg( wStream.GetData(), (uint16_t)wStream.GetOffset() );
		}
	}
	else
	{
		AE_WARN( "Could not serialize modification message" );
	}
}

void EditorServer::ActiveRefocus( EditorProgram* program )
{
	if( m_selected.Length() || m_hoverEntities.Length() )
	{
		if( !m_doRefocusImm )
		{
			m_refocusHeldTime = 0.0f;
		}
		m_doRefocusImm = 2;
	}
}

bool EditorServer::SaveLevel( EditorProgram* program, bool saveAs )
{
	bool fileSelected = !m_levelPath.Empty();
	if( !fileSelected || saveAs )
	{
		fileSelected = false;
		ae::FileDialogParams params;
		params.window = &program->window;
		params.filters.Append( { "Level File", "level" } );
		params.defaultPath = m_levelPath.c_str();
		auto filePath = ae::FileSystem::SaveDialog( params );
		if( filePath.c_str()[ 0 ] )
		{
			fileSelected = true;
			m_SetLevelPath( program, filePath.c_str() );
		}
	}
	if( !fileSelected )
	{
		AE_INFO( "No file selected" );
		return false;
	}

	if( !program->MakeWritable( m_levelPath.c_str() ) )
	{
		ae::Str512 msg = ae::Str512::Format( "Failed to make file writable '#'", m_levelPath );
		AE_ERR( msg.c_str() );
		ae::ShowMessage( msg.c_str() );
		return false;
	}

	AE_INFO( "Saving... '#'", m_levelPath );

	ae::Map< const ae::ClassType*, ae::Component* > defaults = m_tag;
	ae::RunOnDestroy destroyDefaults = [&]()
	{
		for( auto& _component : defaults )
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

	const uint32_t writtenBytes = program->fileSystem.Write( ae::FileSystem::Root::Data, m_levelPath.c_str(), buffer.GetString(), (uint32_t)buffer.GetSize(), false );
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
	if( filePath.Length() )
	{
		return OpenLevelWithPrompts( program, filePath[ 0 ].c_str() );
	}
	else
	{
		AE_INFO( "No file selected" );
	}
}

void EditorServer::OpenLevelWithPrompts( EditorProgram* program, const char* filePath )
{
	if( !filePath || !filePath[ 0 ] )
	{
		return;
	}

	if( m_appBundleWarningDialogId )
	{
		m_RemoveDialog( m_appBundleWarningDialogId );
		m_appBundleWarningDialogId = 0;
	}

	if( strstr( filePath, ".app/Contents/Resources" ) )
	{
		AppBundleWarningDialog dialog;
		dialog.program = program;
		dialog.path = filePath;
		m_appBundleWarningDialogId = m_PushDialog( dialog );
	}
	else
	{
		OpenLevel( program, filePath );
	}
}

void EditorServer::OpenLevel( EditorProgram* program, const char* filePath )
{
	if( m_pendingLevel )
	{
		AE_WARN( "Cancelling level read '#'", m_pendingLevel->GetUrl() );
		program->fileSystem.Destroy( m_pendingLevel );
		m_pendingLevel = nullptr;
	}

	AE_INFO( "Reading... '#'", filePath );
	m_pendingLevel = program->fileSystem.Read( ae::FileSystem::Root::Data, filePath, 2.0f );
}

void EditorServer::Unload( EditorProgram* program )
{
	// @HACK: Currently two LevelUnload's are sent
	EditorEvent event;
	event.type = EditorEventType::LevelUnload;
	event.path = m_levelPath.c_str();
	SendPluginEvent( program->plugins, event );

	m_objectListType = nullptr;
	m_selected.Clear();
	m_hoverEntities.Clear();
	uiHoverEntity = kInvalidEntity;
	m_selectRef = SelectRef();
	m_framePickableEntities.Clear();

	m_SetLevelPath( program, "" );

	while( m_objects.Length() )
	{
		DestroyObject( program, m_objects.GetKey( m_objects.Length() - 1 ) );
	}
	AE_ASSERT( m_objects.Length() == 0 );

	m_registry.Clear();

	// @TODO: Enable cleanup validation
	for( auto& [ config, plugin ] : program->plugins )
	{
		for( auto& [ instance, _ ] : plugin->m_instances )
		{
			// AE_ASSERT_MSG( !instance->m_selectEntity, "Plugin '#' has entity references on level unload", config.name );
		}
		AE_ASSERT_MSG( !plugin->m_entityInstances.Length(), "Plugin '#' still has entity instances on level unload", config.name );
	}
}

void EditorServer::m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::ClassType*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const
{
	AE_ASSERT( levelObject );
	AE_ASSERT( jsonEntity->IsObject() );

	// Id
	const ae::Entity entity = levelObject->entity;
	jsonEntity->AddMember( "id", entity, allocator );

	// Name
	const char* objectName = m_registry.GetNameByEntity( entity );
	if( objectName[ 0 ] )
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
		const ae::ClassType* type = m_registry.GetTypeByIndex( i );
		if( const ae::Component* component = m_registry.TryGetComponent( entity, type ) )
		{
			const ae::Component* defaultComponent = defaults ? defaults->Get( type, nullptr ) : nullptr;
			if( defaults && !defaultComponent )
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

// @TODO: Combine. EditorServer::m_LoadLevel(), Editor::m_Read(), and EditorServer::m_PasteFromClipboard() are very similar
void EditorServer::m_PasteFromClipboard( EditorProgram* program )
{
	const auto clipboardText = ae::GetClipboardText();
	if( clipboardText.empty() )
	{
		return;
	}

	// Load / validate
	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse( clipboardText.c_str() );
	if( parseResult.IsError() )
	{
		AE_ERR( "Could not parse clipboard json. Error:# (#)",
			rapidjson::GetParseError_En( parseResult.Code() ),
			parseResult.Offset()
		);
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
			AddComponent( program, editorObject, ae::GetClassTypeByName( componentIter.name.GetString() ) );
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
			const ae::ClassType* type = ae::GetClassTypeByName( componentIter.name.GetString() );
			if( !type )
			{
				continue;
			}
			ae::Component* component = &m_registry.GetComponent( entity, type );
			const uint32_t varCount = type->GetVarCount( true );
			for( uint32_t j = 0; j < varCount; j++ )
			{
				const ae::ClassVar* var = type->GetVarByIndex( j, true );
				object->HandleVarChange( program, component, type, var );
			}
		}
		// @TODO: Explicitly handle setting transform vars?
	}
}

void EditorServer::m_DeleteSelected( EditorProgram* program )
{
	for( ae::Entity entity : m_selected )
	{
		DestroyObject( program, entity );
	}
	m_selected.Clear();
}

void EditorServer::m_HideSelected()
{
	if( m_selected.Length() )
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

void EditorServer::m_SelectWithModifiers( EditorProgram* program, const ae::Entity* entities, uint32_t count )
{
	bool shift = program->input.Get( ae::Key::LeftShift );
	bool ctrl = program->input.Get( ae::Key::LeftControl );
	if( shift && ctrl )
	{
		// Add
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kInvalidEntity && m_selected.Find( entity ) < 0 )
			{
				m_selected.Append( entity );
			}
		}
	}
	else if( shift )
	{
		// Toggle
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kInvalidEntity )
			{
				int32_t idx = m_selected.Find( entity );
				if( idx < 0 )
				{
					m_selected.Append( entity );
				}
				else
				{
					m_selected.Remove( idx );
				}
			}
		}
	}
	else if( ctrl )
	{
		// Remove
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kInvalidEntity )
			{
				int32_t idx = m_selected.Find( entity );
				if( idx >= 0 )
				{
					m_selected.Remove( idx );
				}
			}
		}
	}
	else
	{
		// New selection
		m_selected.Clear();
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kInvalidEntity )
			{
				m_selected.Append( entity );
			}
		}
	}
}

bool EditorServer::m_ShowVar( EditorProgram* program, ae::Object* component, const ae::ClassVar* var )
{
	if( GetSpecialMemberVar( var ) )
	{
		return false;
	}
	bool changed = false;
	ImGui::PushID( var->GetName() );
	if( var->IsArray() )
	{
		uint32_t arrayLength = var->GetArrayLength( component );
		ImGui::Text( "%s", var->GetName() );
		ImVec2 size( ImGui::GetContentRegionAvail().x, 8 * ImGui::GetTextLineHeightWithSpacing() );
		ImGui::BeginChild( "ChildL", size, true, 0 );
		for( int i = 0; i < arrayLength; i++ )
		{
			ImGui::PushID( i );
			changed |= m_ShowVarValue( program, component, var, i );
			ImGui::PopID();
		}
		ImGui::EndChild();
		if( !var->IsArrayFixedLength() )
		{
			bool arrayMaxLength = ( var->GetArrayLength( component ) >= var->GetArrayMaxLength() );
			ImGui::BeginDisabled( arrayMaxLength );
			if( ImGui::Button( "Add" ) )
			{
				var->SetArrayLength( component, arrayLength + 1 );
				changed = true;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled( !arrayLength );
			if( ImGui::Button( "Remove" ) )
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

bool EditorServer::m_ShowVarValue( EditorProgram* program, ae::Object* component, const ae::ClassVar* var, int32_t idx )
{
	ae::Str64 varName = ( idx < 0 ) ? var->GetName() : ae::Str64::Format( "#", idx );
	switch( var->GetType() )
	{
		case ae::BasicType::Enum:
		{
			auto currentStr = var->GetObjectValueAsString( component, idx );
			auto valueStr = aeImGui_Enum( var->GetEnumType(), varName.c_str(), currentStr.c_str() );
			if( var->SetObjectValueFromString( component, valueStr.c_str(), idx ) )
			{
				return ( currentStr != valueStr.c_str() );
			}
			return false;
		}
		case ae::BasicType::Bool:
		{
			bool b = false;
			var->GetObjectValue( component, &b, idx );
			if( ImGui::Checkbox( varName.c_str(), &b ) )
			{
				return var->SetObjectValue( component, b, idx );
			}
			return false;
		}
		case ae::BasicType::Float:
		{
			float f = 0.0f;
			var->GetObjectValue( component, &f, idx );
			if( ImGui::InputFloat( varName.c_str(), &f ) )
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
			if( ImGui::InputTextMultiline( varName.c_str(), buf, sizeof(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4 ), 0 ) )
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

bool EditorServer::m_ShowRefVar( EditorProgram* program, ae::Object* component, const ae::ClassVar* var, int32_t idx )
{
	const ae::ClassType* componentType = ae::GetClassTypeFromObject( component );
	auto val = var->GetObjectValueAsString( component, idx );
	
	if( idx < 0 )
	{
		ImGui::Text( "%s", var->GetName() );
	}
	const ae::ClassType* refType = var->GetSubType();
	AE_ASSERT( refType );
	if( m_selectRef.enabled
		&& m_selectRef.component == component
		&& m_selectRef.componentVar == var
		&& m_selectRef.varIdx == idx )
	{
		if( ImGui::Button( "Cancel" ) )
		{
			m_selectRef = SelectRef();
		}
	}
	else
	{
		ImGui::Text( "%s", val.c_str() );
		ImGui::SameLine();
		if( val == "NULL" )
		{
			if( ImGui::Button( "Set" ) )
			{
				m_selectRef.enabled = true;
				m_selectRef.component = component;
				m_selectRef.componentVar = var;
				m_selectRef.varIdx = idx;
			}
		}
		else
		{
			if( ImGui::Button( "Select" ) )
			{
				ae::Object* _selectComp = nullptr;
				if( program->serializer.StringToObjectPointer( val.c_str(), &_selectComp ) )
				{
					ae::Component* selectComp = ae::Cast< ae::Component >( _selectComp );
					AE_ASSERT( selectComp );
					const EditorServerObject* selectObj = GetObjectFromComponent( selectComp );
					AE_ASSERT( selectObj );
					m_SelectWithModifiers( program, &selectObj->entity, 1 );
				}
			}
			ImGui::SameLine();
			if( ImGui::Button( "Clear" ) )
			{
				var->SetObjectValueFromString( component, "NULL", idx );
			}
		}
	}
	return false; // @TODO: Handle ref vars changing
}

std::string EditorProgram::Serializer::ObjectPointerToString( const ae::Object* obj ) const
{
	if( !obj )
	{
		return "NULL";
	}
	const ae::ClassType* type = ae::GetClassTypeFromObject( obj );
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
	if( strcmp( pointerVal, "NULL" ) == 0 )
	{
		*objOut = nullptr;
		return true;
	}
	ae::Entity entity = 0;
	char typeName[ 16 ];
	typeName[ 0 ] = 0;
	if( sscanf( pointerVal, "%u %15s", &entity, typeName ) == 2 )
	{
		if( EditorServerObject* editorObj = program->editor.GetObject( entity ) )
		{
			*objOut = program->editor.GetComponent( editorObj, ae::GetClassTypeByName( typeName ) );
		}
		return true;
	}
	return false;
};

//------------------------------------------------------------------------------
// EditorPicking functions
//------------------------------------------------------------------------------
ae::Entity EditorServer::m_PickObject( EditorProgram* program, ae::Vec3* hitOut, ae::Vec3* normalOut )
{
	const ae::Vec3 mouseRay = program->GetMouseRay();
	const ae::Vec3 mouseRaySrc = program->camera.GetPosition();

	// @TODO: Frustum culling

	ae::RaycastParams raycastParams;
	raycastParams.source = mouseRaySrc;
	raycastParams.ray = mouseRay * kEditorViewDistance;
	raycastParams.hitClockwise = false;
	raycastParams.hitCounterclockwise = true;
	// raycastParams.debug = &program->debugLines; // Debug only
	ae::RaycastResult result;
	for( auto& [ _, plugin ] : program->plugins )
	{
		for( auto& [ instance, _ ] : plugin->m_instances )
		{
			AE_DEBUG_ASSERT( instance->m_mesh );
			if( !instance->m_mesh || instance->color.a < 0.01f || // Skip invisible objects
				( instance->color.a < 0.99f && !GetShowTransparent() ) ) // Skip transparent objects
			{
				continue;
			}
			const EditorServerObject* editorObj = GetObject( instance->m_selectEntity ); // May be null
			if( editorObj && editorObj->hidden )
			{
				continue;
			}
			raycastParams.userData = editorObj;
			raycastParams.transform = instance->transform;
			result = instance->m_mesh->collision.Raycast( raycastParams, result );
		}
	}
	const uint32_t editorObjectCount = m_objects.Length();
	for( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* editorObj = m_objects.GetValue( i );
		if( !editorObj->hidden && m_framePickableEntities.Get( editorObj->entity, PickingType::Disabled ) == PickingType::Logic )
		{
			float hitT = INFINITY;
			ae::Vec3 hitPos( 0.0f );
			const ae::Sphere sphere( editorObj->GetTransform().GetTranslation(), 0.5f );
			if( sphere.IntersectRay( mouseRaySrc, mouseRay, &hitPos, &hitT ) )
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
	if( result.hits.Length() )
	{
		*hitOut = result.hits[ 0 ].position;
		*normalOut = result.hits[ 0 ].normal;
		const EditorServerObject* editorObj = (const EditorServerObject*)result.hits[ 0 ].userData;
		return editorObj ? editorObj->entity : kInvalidEntity;
	}

	return kInvalidEntity;
}

ae::Color EditorServer::m_GetColor( ae::Entity entity, bool objectLineColor ) const
{
	const EditorServerObject* editorObj = GetObject( entity );
	AE_ASSERT( editorObj );
	const bool isHovered = ( m_hoverEntities.Find( entity ) >= 0 );
	const bool isSelected = ( m_selected.Find( entity ) >= 0 );
	uint64_t seed = entity * 43313;
	// @TODO: This is isn't working properly when the range is two pi
	const float hueMin = m_objectHue - m_objectHueRange * 0.5f;
	const float hueMax = m_objectHue + m_objectHueRange * 0.5f;
	const float hue = ae::Mod( ae::Random( hueMin, hueMax, &seed ), ae::TwoPi ) / ae::TwoPi;
	const float saturationMin = ae::Clip01( m_objectSaturation - m_objectSaturationRange * 0.5f );
	const float saturationMax = ae::Clip01( m_objectSaturation + m_objectSaturationRange * 0.5f );
	const float saturation = ae::Random( saturationMin, saturationMax, &seed );
	const float valueMin = ae::Clip01( m_objectValue - m_objectValueRange * 0.5f );
	const float valueMax = ae::Clip01( m_objectValue + m_objectValueRange * 0.5f );
	const float value = ae::Random( m_objectValue, m_objectValueRange, &seed );
	if( objectLineColor )
	{
		const ae::Color baseColor = ae::Color::HSV( hue, 1.0f, 1.0f );
		if( isHovered )
		{
			return m_selectionColor;
		}
		else if( editorObj->renderDisabled )
		{
			if( isSelected )
			{
				// Modify line color when selected only when 'renderDisabled',
				// normally selection would be indicated by the mesh color.
				return baseColor.Lerp( m_selectionColor, 0.65f );
			}
			return baseColor;
		}
		return baseColor.SetA( 0.0f );
	}
	else
	{
		const ae::Color baseColor = ae::Color::HSV( hue, saturation, value );
		if( isHovered && isSelected )
		{
			return m_selectionColor;
		}
		else if( isSelected )
		{
			return baseColor.Lerp( m_selectionColor, 0.65f );
		}
		else if( isHovered )
		{
			return baseColor.Lerp( m_selectionColor, 0.35f );
		}
		return baseColor;
	}
}

void GetComponentTypeRequirements( const ae::ClassType* type, ae::Array< const ae::ClassType* >* requirementsOut )
{
	if( !type || !requirementsOut )
	{
		return;
	}
	AE_ASSERT( type );
	requirementsOut->Clear();
	auto fn = [&]( auto& fn, const ae::ClassType* t ) -> void
	{
		if( t->GetParentType() )
		{
			fn( fn, t->GetParentType() );
		}
		const uint32_t requiredCount = type->attributes.GetCount< EditorRequiredAttribute >();
		for( uint32_t i = 0; i < requiredCount; i++ )
		{
			const EditorRequiredAttribute* requiredAttribute = type->attributes.TryGet< EditorRequiredAttribute >( i );
			const ae::ClassType* requiredType = ae::GetClassTypeByName( requiredAttribute->className.c_str() );
			if( requiredType &&
				type != requiredType &&
				requiredType->attributes.Has< ae::EditorTypeAttribute >() &&
				requirementsOut->Find( requiredType ) < 0 )
			{
				requirementsOut->Append( requiredType );
			}
			// @TODO: Log missing types
		}
	};
	fn( fn, type );
}

void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component )
{
	const ae::ClassType* type = ae::GetClassTypeFromObject( component );
	const uint32_t varCount = type->GetVarCount( true );
	for( uint32_t i = 0; i < varCount; i++ )
	{
		const ae::ClassVar* var = type->GetVarByIndex( i, true );
		if( const SpecialMemberVar* specialVar = GetSpecialMemberVar( var ) )
		{
			specialVar->SetObjectValue( transform, component, var );
			continue;
		}
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
			const ae::ClassType* type = ae::GetClassTypeByName( componentIter.name.GetString() );
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
	const ae::ClassType* type = ae::GetClassTypeFromObject( component );
	const uint32_t varCount = type->GetVarCount( true );
	for( uint32_t i = 0; i < varCount; i++ )
	{
		const ae::ClassVar* var = type->GetVarByIndex( i, true );
		if( GetSpecialMemberVar( var ) )
		{
			continue;
		}
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
		AE_ERR( "Invalid 'objects' array" );
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
		if( !jsonObject.HasMember( "id" ) )
		{
			// @TODO
			return false;
		}
		const uint32_t id = jsonObject[ "id" ].GetUint();
		if( id == prevId )
		{
			AE_ERR( "Duplicate entity id '#'", id );
			return false;
		}
		if( id < prevId )
		{
			AE_ERR( "Entity id '#' out of sequence (# > #)", id, prevId, id );
			return false;
		}
		prevId = id;

		if( !jsonObject.HasMember( "transform" ) )
		{
			AE_ERR( "Entity '#' has no transform data", id );
			return false;
		}

		if( !jsonObject.HasMember( "components" ) )
		{
			AE_ERR( "Entity '#' has no components", id );
			return false;
		}
		const auto& jsonComponents = jsonObject[ "components" ];
		if( !jsonComponents.IsObject() )
		{
			AE_ERR( "Unexpected data when reading entity '#' 'components'", id );
			return false;
		}
		for( const auto& componentIter : jsonComponents.GetObject() )
		{
			if( !componentIter.value.IsObject() )
			{
				AE_ERR( "Entity '#' has unexpected component data", id );
				return false;
			}
		}
	}
	return true;
}

template< typename T >
const T* TryGetClassOrVarAttribute( const ae::ClassType* type )
{
	const ae::ClassType* currentType = type;
	while( currentType )
	{
		if( const T* classAttribute = currentType->attributes.TryGet< T >() )
		{
			return classAttribute;
		}
		const uint32_t varCount = currentType->GetVarCount( false );
		for( uint32_t i = 0; i < varCount; i++ )
		{
			const ae::ClassVar* var = currentType->GetVarByIndex( i, false );
			if( const T* varAttribute = var->attributes.TryGet< T >() )
			{
				return varAttribute;
			}
		}
		currentType = currentType->GetParentType();
	}
	return nullptr;
}

ae::Array< const ae::ClassVar*, 8 > GetTypeVarsByName( const ae::ClassType* type, const char* name )
{
	ae::Array< const ae::ClassVar*, 8 > vars;
	const ae::ClassType* currentType = type;
	while( currentType && vars.Length() < vars.Size() )
	{
		if( const ae::ClassVar* var = currentType->GetVarByName( name, false ) )
		{
			vars.Append( var );
		}
		currentType = currentType->GetParentType();
	}
	return vars;
}

void SendPluginEvent( EditorPluginArray& plugins, const EditorEvent& event )
{
	for( auto& [ _, plugin ] : plugins )
	{
		plugin->OnEvent( event );
	}
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
	if( !fork() )
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
