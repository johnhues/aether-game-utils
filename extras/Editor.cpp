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
#include "imgui.h"
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
AE_REGISTER_NAMESPACECLASS( (ae, EditorDisplayNameAttribute) );

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

#define JSON_SCENE_OBJECTS_NAME "objects"
#define JSON_ENTITY_ID_NAME "id"
#define JSON_ENTITY_NAME_NAME "name"
#define JSON_PARENT_ID_NAME "parent"
#define JSON_TRANSFORM_NAME "transform"
#define JSON_POSITION_NAME "position"
#define JSON_ROTATION_NAME "rotation"
#define JSON_SCALE_NAME "scale"
#define JSON_ENTITY_COMPONENTS_NAME "components"
#define DOCUMENT_ENTITY_PARENT_MEMBER "parent"
#define DOCUMENT_ENTITY_CHILDREN_MEMBER "children"
#define DOCUMENT_ENTITY_COMPONENTS_MEMBER "components"
#define DOCUMENT_ENTITY_COMPONENT_TYPE_MEMBER "@type"

enum class PickingType
{
	Disabled,
	Mesh,
	Logic
};

enum class SelectionModifier
{
	New,
	Add,
	Toggle,
	Remove
};

struct SpecialMemberVar
{
	ae::BasicType::Type type;
	const char* name;
	bool ( *SetObjectValue )( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var );
};
const SpecialMemberVar kSpecialMemberVars[] = {
	{ ae::BasicType::Matrix4, JSON_TRANSFORM_NAME, []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform ); } },
	{ ae::BasicType::Vec3, JSON_POSITION_NAME, []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetTranslation() ); } },
	{ ae::BasicType::Quaternion, JSON_ROTATION_NAME, []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetRotation() ); } },
	{ ae::BasicType::Vec3, JSON_SCALE_NAME, []( const ae::Matrix4& transform, ae::Object* component, const ae::ClassVar* var ) { return var->SetObjectValue( component, transform.GetScale() ); } },
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
void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component, ae::DocumentValue* compDoc = nullptr );
void JsonToRegistry( const ae::Map< ae::Entity, ae::Entity >& entityMap, const rapidjson::Value& jsonObjects, ae::Registry* registry, ae::DocumentValue* docObjects = nullptr );
void ComponentToJson( const Component* component, const Component* defaultComponent, rapidjson::Document::AllocatorType& allocator, rapidjson::Value* jsonComponent );
bool ValidateLevel( const rapidjson::Value& jsonLevel );
template< typename T > const T* TryGetClassOrVarAttribute( const ae::ClassType* type );
ae::Array< const ae::ClassVar*, 8 > GetTypeVarsByName( const ae::ClassType* type, const char* name );
void SendPluginEvent( EditorPluginArray& plugins, const EditorEvent& event );

//------------------------------------------------------------------------------
// JsonComponent helper
//------------------------------------------------------------------------------
struct JsonComponent
{
	const ae::ClassType* type;
	const rapidjson::Value& json;
};

//------------------------------------------------------------------------------
// JsonEntity helper
//------------------------------------------------------------------------------
struct JsonEntity
{
	ae::Entity id;
	std::string name;
	ae::Matrix4 transform;
	ae::Entity parentId;
	const JsonEntity* parent;
	ae::Array< const JsonEntity* > children;
	ae::Array< const JsonComponent* > components;
	const rapidjson::Value& json;
};

//------------------------------------------------------------------------------
// JsonScene helper
//------------------------------------------------------------------------------
struct JsonScene
{
	JsonScene( const ae::Tag& tag, rapidjson::Value& scene, bool allowMissingParents );
	~JsonScene();
	ae::Map< ae::Entity, JsonEntity* > entityLookup;
	ae::ObjectPool< JsonEntity, 64, true > entities;
	ae::ObjectPool< JsonComponent, 128, true > components;
	bool success;
	const rapidjson::Value& json;
};

//------------------------------------------------------------------------------
// DocumentScene class
//------------------------------------------------------------------------------
class DocumentScene
{
	DocumentScene( const ae::Tag& tag );
	~DocumentScene();
private:
	ae::DocumentValue* m_scene = nullptr;
};

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
	std::string levelPath;
};

struct InvalidSceneDialog : public EditorDialog
{
	bool ShowUIAndWaitForButton() override;
	std::string levelPath;
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
	std::string info;
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
	EditorServerObject( const ae::Tag& tag, ae::DocumentValue* object ) : m_object( object ), m_components( tag ) { AE_ASSERT( m_object ); }

	ae::Entity GetEntity() const;
	const char* GetName() const;
	void SetName( const char* name );
	bool SetTransform( const ae::Matrix4& transform, class EditorProgram* program );
	ae::Matrix4 GetTransform() const;

	ae::Entity GetParentEntity() const { return m_object->ObjectTryGet( DOCUMENT_ENTITY_PARENT_MEMBER )->NumberGet< ae::Entity >(); }
	void SetParent( EditorServer* server, EditorServerObject* parent );

	uint32_t GetChildCount() const;
	ae::Entity GetChildEntity( uint32_t index ) const;
	
	void HandleVarChange( class EditorProgram* program, ae::Component* component, const ae::ClassType* type, const ae::ClassVar* var );

	ae::OBB GetOBB( class EditorProgram* program ) const;
	ae::AABB GetAABB( class EditorProgram* program ) const;

	void AddComponent( const EditorComponent* component );
	void RemoveComponent( const EditorComponent* component );
	const EditorComponent* GetComponentByIndex2( int32_t index ) const { return m_components[ index ]; }
	const EditorComponent* GetComponentByType2( const ae::ClassType* type ) const;

	// @TODO: These should be private, and EditorServerObject function should
	// manage the DocumentValue internally instead of exposing it like this.
	ae::DocumentValue& GetDocumentValue() { return *m_object; }
	const ae::DocumentValue& GetDocumentValue() const { return *m_object; }
	ae::DocumentValue* GetComponentByType( const char* typeName );
	const ae::DocumentValue* GetComponentByType( const char* typeName ) const;
	ae::DocumentValue* GetComponentByType( const ae::ClassType* type );
	const ae::DocumentValue* GetComponentByType( const ae::ClassType* type ) const;
	uint32_t GetComponentCount() const;
	ae::DocumentValue& GetComponentByIndex( uint32_t index );
	const ae::DocumentValue& GetComponentByIndex( uint32_t index ) const;
	const ae::ClassType* GetComponentTypeByIndex( uint32_t index ) const;
	
	// @TODO: Should be part of the document for undo/redo
	bool hidden = false;
	bool renderDisabled = false;
	
private:
	ae::DocumentValue* m_object = nullptr;
	ae::Array< const EditorComponent* > m_components;
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
		m_hoverEntities( tag ),
		m_objects( tag ),
		m_componentPool( tag ),
		m_registry( tag ),
		m_doc( tag ),
		m_connections( tag ),
		m_framePickableEntities( tag ),
		m_dialogs( tag )
	{}
	~EditorServer();
	void Initialize( class EditorProgram* program );
	void Terminate( class EditorProgram* program );
	void Update( class EditorProgram* program );
	void Render( class EditorProgram* program );
	void ShowMenuBar( class EditorProgram* program );
	void ShowSideBar( class EditorProgram* program );
	
	bool SaveLevel( class EditorProgram* program, bool saveAs );
	void OpenLevelDialog( class EditorProgram* program );
	void OpenLevelWithPrompts( class EditorProgram* program, const char* path );
	void OpenLevel( EditorProgram* program, const char* filePath );
	void Unload( class EditorProgram* program );
	
	bool GetShowTransparent() const { return m_showTransparent; }
	ae::Color GetBackgroundColor() const { return m_backgroundColor; }
	
	EditorServerObject* CreateObject( class EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform, const char* name, bool undo = true );
	void DestroyObject( class EditorProgram* program, ae::Entity entity, bool undo = true );
	
	ae::Component* AddComponent( class EditorProgram* program, EditorServerObject* obj, const ae::ClassType* type );
	void RemoveComponent( class EditorProgram* program, EditorServerObject* obj, ae::Component* component );
	ae::Component* GetComponent( EditorServerObject* obj, const ae::ClassType* type ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetComponent( obj, type ) ); }
	const ae::Component* GetComponent( const EditorServerObject* obj, const ae::ClassType* type ) const;
	
	uint32_t GetObjectCount() const { return m_objects.Length(); }
	const EditorServerObject* GetObjectFromComponent( const ae::Component* component ); // @TODO: Name this like the others
	const EditorServerObject* GetObjectAssert( ae::Entity entity ) const; // Returns the component or fatally errors if it doesn't exist
	const EditorServerObject* GetObjectNull( ae::Entity entity ) const; // Returns the component if entity is not kNullEntity, otherwise fatally errors if it doesn't exist
	const EditorServerObject* GetObjectSafe( ae::Entity entity ) const; // Returns the component if it exists, otherwise returns nullptr
	EditorServerObject* GetObjectAssert( ae::Entity entity ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetObjectAssert( entity ) ); }
	EditorServerObject* GetObjectNull( ae::Entity entity ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetObjectNull( entity ) ); }
	EditorServerObject* GetObjectSafe( ae::Entity entity ) { return AE_CALL_CONST_MEMBER_FUNCTION( GetObjectSafe( entity ) ); }
	
	bool GetRenderDisabled( ae::Entity entity ) const;
	ae::AABB GetSelectedAABB( class EditorProgram* program ) const;

	void HandleTransformChange( class EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform );
	void BroadcastVarChange( const ae::ClassVar* var, const ae::Component* component );

	void ActiveRefocus( EditorProgram* program );

	void Undo();
	void Redo();

	ae::DocumentValue* GetDocumentObject( ae::Entity entity );
	
	ae::ListenerSocket sock;
	
private:
	// Serialization helpers
	void m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::ClassType*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const;
	// Tools
	void m_CopySelected() const;
	void m_PasteFromClipboard( class EditorProgram* program );
	void m_DeleteSelected( class EditorProgram* program );
	void m_HideSelected();
	void m_ParentSelected( class EditorProgram* program );
	void m_UnparentSelected( class EditorProgram* program );
	// Misc helpers
	void m_SetLevelPath( class EditorProgram* program, const char* path );
	SelectionModifier m_GetSelectionModifier( class EditorProgram* program ) const;
	ae::Str64 m_GetSelectionModifierFormatString( SelectionModifier modifier ) const;
	void m_SelectWithModifier( SelectionModifier modifier, const ae::Entity* entities, uint32_t count );
	// Document selection helpers
	uint32_t m_GetSelectionLength() const { return m_docSelection->ArrayLength(); }
	ae::Entity m_GetSelectedEntity( uint32_t index ) const { return m_docSelection->ArrayGet( index ).NumberGet< ae::Entity >(); }
	int32_t m_FindInSelection( ae::Entity entity ) const;
	void m_SelectAll();
	void m_ClearSelection();
	void m_RemoveFromSelection( ae::Entity entity );
	void m_AddToSelection( ae::Entity entity );
	ae::Array< ae::Entity > m_GetTreeFromEntities( const ae::Entity* entities, uint32_t count ) const;
	// UI helpers
	bool m_ShowVar( class EditorProgram* program, ae::DocumentValue* docValue, ae::Object* component, const ae::ClassVar* var );
	bool m_ShowVarValue( class EditorProgram* program, ae::DocumentValue* varDocValue, ae::Object* component, const ae::ClassVar* var, int32_t idx = -1 );
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
	bool m_showGrid = false;

	// Manipulation
	const ae::ClassType* m_objectListType = nullptr;
	ae::Array< ae::Entity > m_hoverEntities;
	ae::Entity uiHoverEntity = kNullEntity;
	ae::Vec3 m_mouseHover = ae::Vec3( 0.0f );
	ae::Vec3 m_mouseHoverNormal = ae::Vec3( 0, 1, 0 );
	std::optional< ae::Vec2 > m_boxSelectStart;
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
	int32_t m_lastCommandIdx = -1;
	double m_lastCommandTime = 0.0;

	// Object state
	Entity m_lastEntity = kNullEntity;
	ae::Map< ae::Entity, EditorServerObject* > m_objects;
	ae::ObjectPool< EditorComponent, 32, true > m_componentPool;
	ae::Registry m_registry; // @TODO: Remove and rely on ae::Document data instead
	ae::Document m_doc;
	ae::DocumentValue* m_docSelection = nullptr;
	ae::DocumentValue* m_docObjects = nullptr;

	// UI configuration
	ae::Color m_selectionColor = ae::Color::PicoOrange();
	float m_objectHue = 3.7f;
	float m_objectHueRange = 0.3f;
	float m_objectSaturation = 0.25f;
	float m_objectSaturationRange = 0.0f;
	float m_objectValue = 0.3f;
	float m_objectValueRange = 0.5f;
	ae::Color m_backgroundColor = ae::Color::AetherBlack().SetValue( 0.075f );

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
		
		ae::Entity pending = kNullEntity;
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
	uint32_t m_mode = 2;
	uint32_t m_dialogNextId = 1;
	ae::Array< EditorDialog* > m_dialogs;
	uint32_t m_appBundleWarningDialogId = 0;
	bool m_imGuiDemoOpen = false;
};

class EditorProgram
{
public:
	EditorProgram( const ae::Tag& tag, const EditorParams& params, EditorPluginArray& plugins ) :
		camera( params.worldUp ),
		debugLines( tag ),
		editor( tag ),
		plugins( plugins ),
		params( params ),
		m_tag( tag )
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
	float m_fov = 1.14f; // 28mm camera 65.5 degree horizontal fov
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

			program->editor.OpenLevel( program, levelPath.c_str() );
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
// InvalidSceneDialog
//------------------------------------------------------------------------------
bool InvalidSceneDialog::ShowUIAndWaitForButton()
{
	const char* title = "Error";
	const char* message = "The level data format is invalid. '%s' was not loaded.";
	const char* buttonConfirm = "Confirm";

	bool press = false;
	ImGui::OpenPopup( title );
	if( ImGui::BeginPopupModal( title, nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::TextWrapped( message, ae::FileSystem::GetFileNameFromPath( levelPath.c_str() ) );
		ImGui::Separator();
		if( ImGui::Button( buttonConfirm ) )
		{
			press = true;
			ImGui::CloseCurrentPopup();
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
	
	const char* kVertShader = R"glsl(
		AE_UNIFORM mat4 u_worldToProj;
		AE_UNIFORM vec4 u_color;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_color;
		AE_OUT_HIGHP vec4 v_color;
		void main()
		{
			v_color = a_color * u_color;
			gl_Position = u_worldToProj * a_position;
		}
	)glsl";

	const char* kFragShader = R"glsl(
		AE_IN_HIGHP vec4 v_color;
		void main()
		{
			AE_COLOR = v_color;
		}
	)glsl";
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
		{ ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
		{ ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::AetherDarkGray().ScaleRGB( 0.9f + ae::Random01() * 0.1f ).GetLinearRGBA() },
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
	
	const char* meshVertShader = R"glsl(
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
		}
	)glsl";
	const char* meshFragShader = R"glsl(
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
			float fade = 1.0 - clamp( ( dist - 50.0 ) / 50.0, 0.0, 1.0 );
			checker = 1.0 - 0.1 * checker * fade;
			light *= checker;

			AE_COLOR.rgb = u_color.rgb * light;
			AE_COLOR.a = u_color.a;
		}
	)glsl";
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
	
	const char* iconVertexShader = R"glsl(
		AE_UNIFORM mat4 u_worldToProj;
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec2 a_uv;
		AE_OUT_HIGHP vec2 v_uv;
		void main()
		{
			v_uv = a_uv;
			gl_Position = u_worldToProj * a_position;
		}
	)glsl";
	const char* iconFragShader = R"glsl(
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
		}
	)glsl";
	m_iconShader.Initialize( iconVertexShader, iconFragShader, nullptr, 0 );
	m_iconShader.SetDepthTest( true );
	m_iconShader.SetDepthWrite( false );
	m_iconShader.SetBlending( true );
	m_iconShader.SetBlendingPremul( true );
	m_iconShader.SetCulling( ae::Culling::CounterclockwiseFront );
	
	AE_STATIC_ASSERT( sizeof(kCogTextureData) == kCogTextureDataSize * kCogTextureDataSize );
	m_cogTexture.Initialize( kCogTextureData, kCogTextureDataSize, kCogTextureDataSize, ae::Texture::Format::R8, ae::Texture::Type::UInt8, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Clamp, true );

	editor.Initialize( this );
}

void EditorProgram::Terminate()
{
	// Note that Unload() sends an EditorEventType::LevelUnload here too
	// alongside the following Terminate event.
	editor.Unload( this );
	
	AE_INFO( "Terminate" );
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
			const ImGuiID dockLeftId = ImGui::DockBuilderSplitNode( mainDockSpace, ImGuiDir_Left, 0.2f, nullptr, nullptr );
			ImGui::DockBuilderDockWindow( "Dev", dockLeftId );
			ImGui::DockBuilderGetNode( dockLeftId )->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
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

		editor.ShowMenuBar( this );
		editor.ShowSideBar( this );
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

EditorMeshInstance* EditorPlugin::CreateMesh( const EditorMesh& _mesh, const char* info, ae::Entity selectEntity )
{
	if( !_mesh.verts.Length() )
	{
		return nullptr;
	}
	EditorServerMesh* mesh = ae::New< EditorServerMesh >( m_tag, m_tag );
	mesh->info = info ? info : "";
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
	AE_DEBUG( "Create mesh-># info:# entity:# refs:1", instance, mesh->info, selectEntity );
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
	AE_DEBUG( "Clone mesh-># source:# info:# entity:# refs:#", instance, _instance, mesh->info, selectEntity, refCount );
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
	AE_DEBUG_ASSERT_MSG( refCount >= 1, "Mesh has invalid reference count: # info:# entity:#", refCount, mesh->info, entity );
	refCount--;
	AE_DEBUG( "Destroy mesh instance:# info:# entity:# refs:#", instance, mesh->info, entity, refCount );
	ae::Delete( instance );
	if( refCount == 0 )
	{
		AE_DEBUG( "Destroy mesh entity:# info:# refs:#", entity, mesh->info, refCount );
		m_meshRefs.Remove( mesh );
		ae::Delete( mesh );
	}
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
				rStream.SerializeObject( typeId );
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
		AE_WARN( "Cancelling level read '#'", m_pendingLevel->GetURL() );
		m_fileSystem.Destroy( m_pendingLevel );
	}
	m_pendingLevel = m_fileSystem.Read( ae::FileSystem::Root::Data, levelPath, 2.0f );
	AE_INFO( "Queuing level load '#'", m_pendingLevel->GetURL() );
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
			m_pendingLevel->GetURL(),
			rapidjson::GetParseError_En( parseResult.Code() ),
			parseResult.Offset()
		);
		return;
	}
	const JsonScene scene( m_tag, document, false );
	if( !scene.success )
	{
		AE_ERR( "Level '#' is not a valid scene", m_pendingLevel->GetURL() );
		return;
	}

	m_lastLoadedLevel = m_pendingLevel->GetURL();

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
	
	for( const JsonEntity& sceneEntity : scene.entities )
	{
		const ae::Entity newId = m_params->registry->CreateEntity( sceneEntity.id, sceneEntity.name.c_str() );
		m_editorEntities.Set( newId, true ); // Record which entities have been created by the editor
		if( sceneEntity.id != newId )
		{
			entityMap.Set( sceneEntity.id, newId ); // Record which ids have been remapped
		}
		for( const JsonComponent* sceneComponent : sceneEntity.components )
		{
			GetComponentTypeRequirements( sceneComponent->type, &requirements );
			for( const ae::ClassType* requirement : requirements )
			{
				m_params->registry->AddComponent( newId, requirement );
			}
			m_params->registry->AddComponent( newId, sceneComponent->type );
		}
	}

	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, document[ JSON_SCENE_OBJECTS_NAME ], m_params->registry );

	AE_INFO( "Loaded level '#'", m_pendingLevel->GetURL() );

	{
		EditorEvent event;
		event.type = EditorEventType::LevelLoad;
		event.path = m_pendingLevel->GetURL();
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
ae::Entity EditorServerObject::GetEntity() const
{
	return m_object->ObjectTryGet( "id" )->NumberGet< ae::Entity >();
}

const char* EditorServerObject::GetName() const
{
	return m_object->ObjectTryGet( "name" )->StringGet();
}

void EditorServerObject::SetName( const char* name )
{
	m_object->ObjectTryGet( "name" )->StringSet( name );
}

bool EditorServerObject::SetTransform( const ae::Matrix4& _transform, EditorProgram* program )
{
	const ae::Matrix4 oldTransform = m_object->ObjectTryGet( "transform" )->OpaqueGet( ae::Matrix4::Identity() );
	if( oldTransform != _transform )
	{
		// @TODO: This uses way too much memory when dragging objects around,
		// but otherwise works perfectly. Only consecutive operations on the
		// same object are merged, but that is never the case when dragging
		// more than one object. Maybe its fine if memory usage per action
		// is minimized?
		m_object->ObjectTryGet( "transform" )->OpaqueSet( _transform );

		const DocumentCallback fn = [ program, e = GetEntity(), _transform ]() { program->editor.HandleTransformChange( program, e, _transform ); };
		m_object->GetDocument().AddUndoGroupAction( "Transform", fn, fn );
		fn();

		const ae::Matrix4 relative = _transform * oldTransform.GetInverse();
		const uint32_t childCount = GetChildCount();
		for( uint32_t i = 0; i < childCount; i++ )
		{
			const ae::Entity childEntity = GetChildEntity( i );
			EditorServerObject* child = program->editor.GetObjectSafe( childEntity );
			if( child )
			{
				child->SetTransform( relative * child->GetTransform(), program );
			}
		}
		return true;
	}
	return false;
}

ae::Matrix4 EditorServerObject::GetTransform() const
{
	return m_object->ObjectTryGet( "transform" )->OpaqueGet( ae::Matrix4::Identity() );
}

void EditorServerObject::SetParent( EditorServer* server, EditorServerObject* parent )
{
	const ae::Entity oldParent = GetParentEntity();
	if( oldParent && ( !parent || oldParent != parent->GetEntity() ) )
	{
		// Clear reference to parent
		GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_PARENT_MEMBER )->NumberSet( ae::kNullEntity );
		// Remove this as child
		ae::DocumentValue* parentValue = &server->GetObjectAssert( oldParent )->GetDocumentValue();
		ae::DocumentValue* parentChildren = parentValue->ObjectTryGet( DOCUMENT_ENTITY_CHILDREN_MEMBER );
		for( uint32_t i = 0; i < parentChildren->ArrayLength(); i++ )
		{
			if( parentChildren->ArrayGet( i ).NumberGet< ae::Entity >() == GetEntity() )
			{
				parentChildren->ArrayRemove( i );
				break;
			}
		}
	}

	if( parent )
	{
		GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_PARENT_MEMBER )->NumberSet( parent->GetEntity() );
		parent->GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_CHILDREN_MEMBER )->ArrayAppend().NumberSet( GetEntity() );
	}
}

uint32_t EditorServerObject::GetChildCount() const
{
	const ae::DocumentValue* childrenValue = m_object->ObjectTryGet( DOCUMENT_ENTITY_CHILDREN_MEMBER );
	return childrenValue ? childrenValue->ArrayLength() : 0;
}

ae::Entity EditorServerObject::GetChildEntity( uint32_t index ) const
{
	return m_object->ObjectTryGet( DOCUMENT_ENTITY_CHILDREN_MEMBER )->ArrayGet( index ).NumberGet< ae::Entity >();
}

void EditorServerObject::HandleVarChange( EditorProgram* program, ae::Component* component, const ae::ClassType* type, const ae::ClassVar* var )
{
	renderDisabled = program->editor.GetRenderDisabled( component->GetEntity() );
	
	program->editor.BroadcastVarChange( var, component );

	const EditorComponent* comp = GetComponentByType2( type );
	AE_ASSERT( comp );
	EditorEvent event;
	event.type = EditorEventType::ComponentEdit;
	event.entity = component->GetEntity();
	event.transform = GetTransform();
	event.component = comp;
	event.componentDoc = GetComponentByType( type );
	event.var = var;
	SendPluginEvent( program->plugins, event );
}

ae::OBB EditorServerObject::GetOBB( EditorProgram* program ) const
{
	ae::AABB result;
	for( auto& [ _, plugin ] : program->plugins )
	{
		ae::List< EditorMeshInstance >* entityInstances = plugin->m_entityInstances.Get( GetEntity(), nullptr );
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

void EditorServerObject::AddComponent( const EditorComponent* component )
{
	AE_DEBUG_ASSERT( component );
	AE_DEBUG_ASSERT_MSG( ( m_components.Find( component ) < 0 ), "Attempted adding duplicate component '#'", component->typeName );
	m_components.Append( component );
}

void EditorServerObject::RemoveComponent( const EditorComponent* component )
{
	AE_DEBUG_ASSERT( component );
	const int32_t index = m_components.Find( component );
	AE_DEBUG_ASSERT( index >= 0 );
	m_components.Remove( index );
}

const EditorComponent* EditorServerObject::GetComponentByType2( const ae::ClassType* type ) const
{
	for( const EditorComponent* component : m_components )
	{
		if( strcmp( component->typeName, type->GetName() ) == 0 )
		{
			return component;
		}
	}
	return nullptr;
}

ae::DocumentValue* EditorServerObject::GetComponentByType( const char* typeName )
{
	return AE_CALL_CONST_MEMBER_FUNCTION( GetComponentByType( typeName ) );
}

const ae::DocumentValue* EditorServerObject::GetComponentByType( const char* typeName ) const
{
	return GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER )->ObjectTryGet( typeName );
}

ae::DocumentValue* EditorServerObject::GetComponentByType( const ae::ClassType* type )
{
	return AE_CALL_CONST_MEMBER_FUNCTION( GetComponentByType( type ) );
}

const ae::DocumentValue* EditorServerObject::GetComponentByType( const ae::ClassType* type ) const
{
	return type ? GetComponentByType( type->GetName() ) : nullptr;
}

uint32_t EditorServerObject::GetComponentCount() const
{
	return GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER )->ObjectLength();
}

ae::DocumentValue& EditorServerObject::GetComponentByIndex( uint32_t index )
{
	return GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER )->ObjectGetValue( index );
}

const ae::DocumentValue& EditorServerObject::GetComponentByIndex( uint32_t index ) const
{
	return GetDocumentValue().ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER )->ObjectGetValue( index );
}

const ae::ClassType* EditorServerObject::GetComponentTypeByIndex( uint32_t index ) const
{
	const ae::DocumentValue* typeValue = GetComponentByIndex( index ).ObjectTryGet( DOCUMENT_ENTITY_COMPONENT_TYPE_MEMBER );
	AE_ASSERT( typeValue );
	return ae::GetClassTypeByName( typeValue->StringGet() );
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
	AE_DEBUG_ASSERT( !m_connections.Length() );
}

void EditorServer::Initialize( EditorProgram* program )
{
	ImGuiStyle* style = &ImGui::GetStyle();
	for( ImVec4& imColor : style->Colors )
	{
		const ae::Vec3 hsv = ae::Color::RGB( imColor.x, imColor.y, imColor.z ).GetHSV();
		bool isText = false;
		switch( &imColor - &style->Colors[ 0 ] )
		{
		case ImGuiCol_Text:
		case ImGuiCol_TextDisabled:
		case ImGuiCol_ScrollbarGrab:
		case ImGuiCol_CheckMark:
		case ImGuiCol_SliderGrab:
		case ImGuiCol_SliderGrabActive:
		case ImGuiCol_SeparatorActive:
		case ImGuiCol_ResizeGripActive:
			isText = true;
			break;
		default:
			break;
		}
		const float value = isText ? hsv.z : ae::Min( hsv.z * 0.5f, 0.1f );
		const ae::Vec3 c = ae::Color::HSV( 0.0f, 0.0f, value ).GetLinearRGB();
		imColor.x = c.x;
		imColor.y = c.y;
		imColor.z = c.z;
	}

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

	AE_DEBUG_ASSERT( !m_doc.EndUndoGroup() );
	m_doc.ObjectInitialize();
	m_docSelection = &m_doc.ObjectSet( "selection" ).ArrayInitialize();
	m_docObjects = &m_doc.ObjectSet( "objects" ).ObjectInitialize();
	m_doc.ClearUndo();
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
		AE_ERR( "Could not read level '#'", m_pendingLevel->GetURL() );
		return;
	}

	m_doc.ClearUndo();

	if( !program->MakeWritable( m_pendingLevel->GetURL() ) )
	{
		const ae::Str512 msg = ae::Str512::Format( "File may not be writable '#'", m_pendingLevel->GetURL() );
		AE_WARN( msg.c_str() );
		ae::ShowMessage( msg.c_str() );
	}

	AE_INFO( "Loading level... '#'", m_pendingLevel->GetURL() );
	
	const char* jsonBuffer = (const char*)m_pendingLevel->GetData();
	AE_ASSERT( jsonBuffer[ m_pendingLevel->GetLength() ] == 0 ); // ae::File::Read() should always add a null terminator
	
	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse( jsonBuffer );
	if( parseResult.IsError() )
	{
		AE_ERR( "Could not parse json '#' Error:# (#)",
			m_pendingLevel->GetURL(),
			rapidjson::GetParseError_En( parseResult.Code() ),
			parseResult.Offset()
		);
		return;
	}

	const JsonScene scene( m_tag, document, false );
	if( !scene.success )
	{
		InvalidSceneDialog dialog;
		dialog.levelPath = m_pendingLevel->GetURL();
		m_PushDialog( dialog );
		AE_ERR( "Invalid level data format '#'", m_pendingLevel->GetURL() );
		return;
	}

	// @TODO: Make sure that the existing level has no modifications before unloading
	Unload( program );

	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag;
	
	for( const JsonEntity& sceneEntity : scene.entities )
	{
		EditorServerObject* object = CreateObject( program, sceneEntity.id, sceneEntity.transform, sceneEntity.name.c_str() );
		if( object->GetEntity() != sceneEntity.id )
		{
			entityMap.Set( sceneEntity.id, object->GetEntity() );
		}
		for( const JsonComponent* component : sceneEntity.components )
		{
			AddComponent( program, object, component->type );
		}
	}
	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, document[ JSON_SCENE_OBJECTS_NAME ], &m_registry, m_docObjects );
	// Refresh editor objects
	for( const JsonEntity& sceneEntity : scene.entities )
	{
		const ae::Entity entityId = entityMap.Get( sceneEntity.id, sceneEntity.id );
		EditorServerObject* object = GetObjectAssert( entityId );
		if( sceneEntity.parentId )
		{
			const ae::Entity parent = entityMap.Get( sceneEntity.parentId, sceneEntity.parentId );
			EditorServerObject* parentObject = GetObjectAssert( parent );
			object->SetParent( this, parentObject );
		}
		for( const JsonComponent* sceneComponent : sceneEntity.components )
		{
			const ae::ClassType* type = sceneComponent->type;
			ae::Component* component = &m_registry.GetComponent( entityId, type );
			const uint32_t varCount = type->GetVarCount( true );
			for( uint32_t j = 0; j < varCount; j++ )
			{
				const ae::ClassVar* var = type->GetVarByIndex( j, true );
				object->HandleVarChange( program, component, type, var );
			}
		}
		// @TODO: Explicitly handle setting transform vars?
	}

	AE_INFO( "Loaded level '#'", m_pendingLevel->GetURL() );
	m_SetLevelPath( program, m_pendingLevel->GetURL() );
	m_doc.ClearUndo();
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
	const float kTapTime = 0.2f;
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
		camera->Refocus( program->editor.m_docSelection->ArrayLength() ? program->editor.GetSelectedAABB( program ).GetCenter() : m_mouseHover, 4.0f );
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
				// m_selectEntity is allowed to be invalid if the mesh isn't
				// used for object selection. The lifetime of the mesh instance
				// is controlled by the plugin, so the mesh may outlive the
				// object it represents.
				const EditorServerObject* obj = GetObjectSafe( instance->m_selectEntity );
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
					const ae::Color editorColor = m_GetColor( obj->GetEntity(), false );
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
		if( !obj->renderDisabled && !obj->hidden && !m_framePickableEntities.TryGet( obj->GetEntity() ) )
		{
			RenderObj& renderObj = logicObjects.Append( {} );
			renderObj.transform = obj->GetTransform();
			renderObj.color = m_GetColor( obj->GetEntity(), false );
			renderObj.mesh = nullptr;
			renderObj.distanceSq = ( camPos - obj->GetTransform().GetTranslation() ).LengthSquared();

			// To enable picking for the next frame
			m_framePickableEntities.Set( obj->GetEntity(), PickingType::Logic );
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

void EditorServer::ShowMenuBar( EditorProgram* program )
{
	if( !ImGui::BeginMainMenuBar() )
	{
		return;
	}

	if( ImGui::BeginMenu( "File" ) )
	{
		if( ImGui::MenuItem( "New" ) )
		{
			AE_INFO( "New level" );
			Unload( program );
		}
		if( ImGui::MenuItem( "Open" ) )
		{
			OpenLevelDialog( program );
		}
		if( ImGui::MenuItem( "Save" ) )
		{
			SaveLevel( program, false );
		}
		if( ImGui::MenuItem( "Save As" ) )
		{
			SaveLevel( program, true );
		}
		ImGui::Separator();
		ImGui::BeginDisabled( m_levelPath.Empty() || !m_connections.Length() );
		if( ImGui::MenuItem( "Game Load" ) && m_connections.Length() )
		{
			uint8_t buffer[ kMaxEditorMessageSize ];
			ae::BinaryWriter wStream( buffer, sizeof( buffer ) );
			wStream.SerializeEnum( EditorNetMsg::Load );
			wStream.SerializeString( m_levelPath );
			for( uint32_t i = 0; i < m_connections.Length(); i++ )
			{
				m_connections[ i ]->sock->QueueMsg( wStream.GetData(), (uint16_t)wStream.GetOffset() );
			}
		}
		ImGui::EndDisabled();
		if( ImGui::MenuItem( "Reload Resources" ) )
		{
			EditorEvent event;
			event.type = EditorEventType::ReloadResources;
			SendPluginEvent( program->plugins, event );
		}
		ImGui::Separator();
		if( ImGui::MenuItem( "Quit" ) )
		{
			program->input.quit = true;
		}
		ImGui::EndMenu();
	}

	if( ImGui::BeginMenu( "Edit" ) )
	{
		if( ImGui::MenuItem( "Create" ) )
		{
			ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetPivot() );
			EditorServerObject* editorObject = CreateObject( program, kNullEntity, transform, "" );
			m_docSelection->ArrayClear();
			m_docSelection->ArrayAppend().NumberSet( editorObject->GetEntity() );
			m_doc.EndUndoGroup();
		}
		ImGui::BeginDisabled( !m_docSelection->ArrayLength() );
		if( ImGui::MenuItem( "Delete" ) )
		{
			for( uint32_t i = 0; i < m_docSelection->ArrayLength(); i++ )
			{
				ae::Entity entity = m_docSelection->ArrayGet( i ).NumberGet< ae::Entity >();
				DestroyObject( program, entity );
			}
			m_doc.EndUndoGroup();
		}
		ImGui::EndDisabled();
		ImGui::Separator();

		// Copy and paste
		ImGui::BeginDisabled( !m_docSelection->ArrayLength() );
		if( ImGui::MenuItem( "Copy" ) )
		{
			m_CopySelected();
		}
		ImGui::EndDisabled();
		if( ImGui::MenuItem( "Paste" ) )
		{
			m_PasteFromClipboard( program );
		}
		ImGui::Separator();

		// Undo and redo
		ImGui::BeginDisabled( !m_doc.GetUndoStackSize() );
		if( ImGui::MenuItem( "Undo" ) )
		{
			Undo();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( !m_doc.GetRedoStackSize() );
		if( ImGui::MenuItem( "Redo" ) )
		{
			Redo();
		}
		ImGui::EndDisabled();
		ImGui::Separator();

		// Parenting
		ImGui::BeginDisabled( m_docSelection->ArrayLength() < 2 );
		if( ImGui::MenuItem( "Parent" ) )
		{
			m_ParentSelected( program );
			m_doc.EndUndoGroup();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled( !m_docSelection->ArrayLength() );
		if( ImGui::MenuItem( "Unparent" ) )
		{
			m_UnparentSelected( program );
			m_doc.EndUndoGroup();
		}
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}

	if( ImGui::BeginMenu( "Select" ) )
	{
		ImGui::EndMenu();
	}

	if( ImGui::BeginMenu( "Tools" ) )
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

		ImGui::EndMenu();
	}
	
	if( ImGui::BeginMenu( "View" ) )
	{
		ImGui::Checkbox( "Show Transparent", &m_showTransparent );
		ImGui::Checkbox( "Show Grid", &m_showGrid );
		ImGui::Checkbox( "ImGui Demo", &m_imGuiDemoOpen );
		ImGui::Separator();

		if( ImGui::RadioButton( "Zen", ( m_mode == 0 ) ) ){ m_mode = 0; }
		if( ImGui::RadioButton( "Object Properties", ( m_mode == 1 ) ) ){ m_mode = 1; }
		if( ImGui::RadioButton( "Scene Hierarchy", ( m_mode == 2 ) ) ){ m_mode = 2; }
		ImGui::Separator();

		ImGui::ColorEdit3( "Selection", m_selectionColor.data );
		ImGui::SliderFloat( "Hue", &m_objectHue, 0.0f, ae::TwoPi );
		ImGui::SliderFloat( "Hue Range", &m_objectHueRange, 0.0f, ae::TwoPi );
		ImGui::SliderFloat( "Saturation", &m_objectSaturation, 0.0f, 1.0f );
		ImGui::SliderFloat( "Saturation Range", &m_objectSaturationRange, 0.0f, 1.0f );
		ImGui::SliderFloat( "Value", &m_objectValue, 0.0f, 1.0f );
		ImGui::SliderFloat( "Value Range", &m_objectValueRange, 0.0f, 1.0f );
		ImGui::ColorEdit3( "Background", m_backgroundColor.data );

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void EditorServer::ShowSideBar( EditorProgram* program )
{
	const float dt = program->GetDt();

	// @TODO: This function has a lot of stuff mixed into it, it should only
	// handle what's in the sidebar in case it is hidden or split.

	if( m_dialogs.Length() )
	{
		EditorDialog* dialog = m_dialogs[ 0 ];
		if( dialog->ShowUIAndWaitForButton() )
		{
			m_RemoveDialog( dialog->id );
		}
	}

	if( m_imGuiDemoOpen )
	{
		ImGui::ShowDemoWindow( &m_imGuiDemoOpen );
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

			ae::Entity hoverEntity = m_hoverEntities.Length() ? m_hoverEntities[ 0 ] : kNullEntity;
			if( m_selectRef.enabled )
			{
				uint32_t matchCount = 0;
				const ae::Object* lastMatch = nullptr;
				
				const ae::ClassType* refType = m_selectRef.componentVar->GetSubType();
				const uint32_t componentTypeCount = ae::GetClassTypeCount();
				for( uint32_t i = 0; i < componentTypeCount; i++ )
				{
					const ae::ClassType* hoverType = ae::GetClassTypeByIndex( i );
					if( !hoverType->attributes.Has< ae::EditorTypeAttribute >() ) { continue; }
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
				m_SelectWithModifier( m_GetSelectionModifier( program ), m_hoverEntities.Data(), m_hoverEntities.Length() );
				m_doc.EndUndoGroup();
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
						// m_selectEntity is allowed to be invalid if the mesh isn't
						// used for object selection. The lifetime of the mesh instance
						// is controlled by the plugin, so the mesh may outlive the
						// object it represents.
						const EditorServerObject* editorObj = GetObjectSafe( instance->m_selectEntity );
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
						( m_framePickableEntities.Get( obj->GetEntity(), PickingType::Disabled ) == PickingType::Logic ) &&
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
								m_hoverEntities.Append( obj->GetEntity() );
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
		const EditorServerObject* pendingObj = GetObjectAssert( m_selectRef.pending );
		const uint32_t typeCount = ae::GetClassTypeCount();
		for( uint32_t i = 0; i < typeCount; i++ )
		{
			const ae::ClassType* pendingType = ae::GetClassTypeByIndex( i );
			if( pendingType->attributes.Has< ae::EditorTypeAttribute >() &&
				pendingType->IsType( refType ) &&
				ImGui::Selectable( pendingType->GetName(), false ) )
			{
				const uint32_t objectCount = m_objects.Length();
				for( uint32_t j = 0; j < objectCount; j++ )
				{
					ae::Component* pendingComponent = m_registry.TryGetComponent( m_objects.GetKey( j ), pendingType );
					if( pendingComponent )
					{
						m_selectRef.componentVar->SetObjectValue( m_selectRef.component, pendingComponent, m_selectRef.varIdx );
						m_selectRef = SelectRef();
					}
				}
			}
		}
		ImGui::EndPopup();
	}
	
	// Debug lines
	ae::Optional< ae::Vec3 > gridLineCenter;
	const ae::Vec3 pivot = program->camera.GetPivot();
	const float zoom = program->camera.GetDistanceFromPivot();
	if( m_showGrid )
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
	if( m_docSelection->ArrayLength() )
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
		if( m_docSelection->ArrayLength() )
		{
			const ae::Vec3 selected = GetSelectedAABB( program ).GetCenter();
			DrawBoxLocator( &program->debugLines, mousePoint, selected, m_selectionColor );
		}
		program->debugLines.AddCircle( mousePoint, m_mouseHoverNormal, zoom / 55.0f, m_selectionColor, 4 );
		if( !m_docSelection->ArrayLength() )
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
	if( m_GetSelectionLength() )
	{
		const ae::AABB selectedAABB = GetSelectedAABB( program );
		program->debugLines.AddAABB( selectedAABB.GetCenter(), selectedAABB.GetHalfSize(), m_selectionColor );
	}
	for( auto&[ entity, _ ] : m_objects )
	{
		const ae::Color color = m_GetColor( entity, true );
		const EditorServerObject* editorObj = GetObjectAssert( entity );
		if( ( color.a >= 0.001f ) )
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
			ae::Str32 name;
			ae::Array< ae::Key, 4 > modifiers;
			ae::Key key;
			void ( *fn )( EditorProgram* );
			bool continuous = false; // If true, the command will be executed every frame while the keys are pressed
			bool repeat = false; // If true the command will be executed multiple times at a specific "interactive" rate while held
			bool ignoreModifiers = false; // If true, ignore any additional modifiers being held down
		};
		Command commands[] = {
			{ "Save", { ae::Key::Meta }, ae::Key::S, []( EditorProgram* program ) { program->editor.SaveLevel( program, false ); } },
			{ "Save As", { ae::Key::Meta, ae::Key::Shift }, ae::Key::S, []( EditorProgram* program ) { program->editor.SaveLevel( program, true ); } },
			{ "Open", { ae::Key::Meta }, ae::Key::O, []( EditorProgram* program ) { program->editor.OpenLevelDialog( program ); } },
			{ "Create", {}, ae::Key::C,
				[]( EditorProgram* program )
				{
					ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetPivot() );
					EditorServerObject* editorObject = program->editor.CreateObject( program, kNullEntity, transform, "" );
					program->editor.m_docSelection->ArrayClear();
					program->editor.m_docSelection->ArrayAppend().NumberSet( editorObject->GetEntity() );
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Copy", { ae::Key::Meta }, ae::Key::C, []( EditorProgram* program ) { program->editor.m_CopySelected(); } },
			{ "Paste", { ae::Key::Meta }, ae::Key::V,
				[]( EditorProgram* program )
				{
					program->editor.m_PasteFromClipboard( program );
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Undo", { ae::Key::Meta }, ae::Key::Z, []( EditorProgram* program ) { program->editor.Undo(); }, false, true },
			{ "Redo", { ae::Key::Meta, ae::Key::Shift }, ae::Key::Z, []( EditorProgram* program ) { program->editor.Redo(); }, false, true },
			{ "Select", {}, ae::Key::Q, []( EditorProgram* program ) { program->editor.gizmoOperation = ImGuizmo::OPERATION( 0 ); } },
			{ "Translate", {}, ae::Key::W, []( EditorProgram* program ) { program->editor.gizmoOperation = ImGuizmo::TRANSLATE; } },
			{ "Rotate", {}, ae::Key::E, []( EditorProgram* program ) { program->editor.gizmoOperation = ImGuizmo::ROTATE; } },
			{ "Scale", {}, ae::Key::R, []( EditorProgram* program ) { program->editor.gizmoOperation = ImGuizmo::SCALE; } },
			{ "Toggle transparent", {}, ae::Key::I, []( EditorProgram* program ) { program->editor.m_showTransparent = !program->editor.m_showTransparent; } },
			{ "Delete", {}, ae::Key::X,
				[]( EditorProgram* program )
				{
					program->editor.m_DeleteSelected( program );
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Focus", {}, ae::Key::F, []( EditorProgram* program ) { program->editor.ActiveRefocus( program ); }, true },
			{ "Hide", {}, ae::Key::H, []( EditorProgram* program ) { program->editor.m_HideSelected(); } },
			{ "Select All", {}, ae::Key::A,
				[]( EditorProgram* program )
				{
					program->editor.m_SelectAll();
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Select none", {}, ae::Key::Escape,
				[]( EditorProgram* program )
				{
					program->editor.m_docSelection->ArrayClear();
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Parent", {}, ae::Key::P,
				[]( EditorProgram* program )
				{
					program->editor.m_ParentSelected( program );
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Unparent", { ae::Key::Shift }, ae::Key::P,
				[]( EditorProgram* program )
				{
					program->editor.m_UnparentSelected( program );
					program->editor.m_doc.EndUndoGroup();
				} },
			{ "Zen", {}, ae::Key::Num0, []( EditorProgram* program ) { program->editor.m_mode = 0; } },
			{ "Object properties", {}, ae::Key::Num1, []( EditorProgram* program ) { program->editor.m_mode = 1; } },
			{ "Object list", {}, ae::Key::Num2, []( EditorProgram* program ) { program->editor.m_mode = 2; } },
			{ "Cycle modes", {}, ae::Key::Tab,
				[]( EditorProgram* program )
				{
					program->editor.m_mode++;
					program->editor.m_mode %= 3;
				} },
			{ .name = "Prev Entity",
				.modifiers = {},
				.key = ae::Key::Up,
				.fn =
					[]( EditorProgram* program )
				{
					program->editor.m_mode = 2;
					const uint32_t selectCount = program->editor.m_docSelection->ArrayLength();
					if( selectCount )
					{
						const ae::Entity currentEntity = [ & ]()
						{
							ae::Entity result = ae::MaxValue< ae::Entity >();
							for( uint32_t i = 0; i < selectCount; i++ )
							{
								result = ae::Min( result, program->editor.m_docSelection->ArrayGet( i ).NumberGet< ae::Entity >() );
							}
							return result;
						}();
						const ae::Entity prevEntity = [ program, currentEntity ]()
						{
							ae::Entity result = kNullEntity;
							const uint32_t entityCount = program->editor.m_docObjects->ObjectLength();
							for( uint32_t i = 0; i < entityCount; i++ )
							{
								const ae::Entity entity = program->editor.m_docObjects->ObjectGetValue( i ).ObjectTryGet( "id" )->NumberGet< ae::Entity >();
								if( entity < currentEntity )
								{
									result = ae::Max( result, entity );
								}
							}
							return result;
						}();
						if( prevEntity )
						{
							const SelectionModifier modifier = program->editor.m_GetSelectionModifier( program );
							program->editor.m_SelectWithModifier( modifier, &prevEntity, 1 );
							program->editor.m_doc.EndUndoGroup();
						}
					}
				},
				.repeat = true,
				.ignoreModifiers = true },
			{ .name = "Next Entity",
				.modifiers = {},
				.key = ae::Key::Down,
				.fn =
					[]( EditorProgram* program )
				{
					program->editor.m_mode = 2;
					const uint32_t selectCount = program->editor.m_docSelection->ArrayLength();
					if( selectCount )
					{
						const ae::Entity currentEntity = [ & ]()
						{
							ae::Entity result = kNullEntity;
							for( uint32_t i = 0; i < selectCount; i++ )
							{
								result = ae::Max( result, program->editor.m_docSelection->ArrayGet( i ).NumberGet< ae::Entity >() );
							}
							return result;
						}();
						const ae::Entity prevEntity = [ program, currentEntity ]()
						{
							ae::Entity result = ae::MaxValue< ae::Entity >();
							const uint32_t entityCount = program->editor.m_docObjects->ObjectLength();
							for( uint32_t i = 0; i < entityCount; i++ )
							{
								const ae::Entity entity = program->editor.m_docObjects->ObjectGetValue( i ).ObjectTryGet( "id" )->NumberGet< ae::Entity >();
								if( entity > currentEntity )
								{
									result = ae::Min( result, entity );
								}
							}
							return result;
						}();
						if( prevEntity != ae::MaxValue< ae::Entity >() )
						{
							const SelectionModifier modifier = program->editor.m_GetSelectionModifier( program );
							program->editor.m_SelectWithModifier( modifier, &prevEntity, 1 );
							program->editor.m_doc.EndUndoGroup();
						}
					}
				},
				.repeat = true,
				.ignoreModifiers = true },
		};
		std::sort( std::begin( commands ), std::end( commands ),
			[]( const Command& a, const Command& b )
			{
				constexpr uint32_t maxModifiers = decltype(a.modifiers)::Size();
				const uint32_t aModifierCount = a.ignoreModifiers ? ( maxModifiers + 1 ) : (uint32_t)a.modifiers.Length();
				const uint32_t bModifierCount = b.ignoreModifiers ? ( maxModifiers + 1 ) : (uint32_t)b.modifiers.Length();
				return aModifierCount > bModifierCount; // Check commands with more keys first to handle potential conflicts
			} );
		const uint32_t modifierCount = (uint32_t)program->input.Get( ae::Key::Meta ) + (uint32_t)program->input.Get( ae::Key::Shift ) + (uint32_t)program->input.Get( ae::Key::Control ) + (uint32_t)program->input.Get( ae::Key::Alt );
		for( const Command& command : commands )
		{
			if( command.ignoreModifiers || ( modifierCount == command.modifiers.Length() ) ) // Exact match only
			{
				const int32_t currentIdx = (int32_t)( &command - commands );
				const bool doRepeat = command.continuous || ( command.repeat && ( currentIdx == m_lastCommandIdx ) && ( ae::GetTime() - m_lastCommandTime ) >= 0.175 );
				if( ( program->input.GetPress( command.key ) ) || ( doRepeat && program->input.Get( command.key ) ) )
				{
					const bool allPressed = [ & ]()
					{
						if( !command.ignoreModifiers )
						{
							for( const ae::Key& modifier : command.modifiers )
							{
								if( !program->input.Get( modifier ) )
								{
									return false;
								}
							}
						}
						return true;
					}();
					if( allPressed )
					{
						m_lastCommandTime = ae::GetTime();
						m_lastCommandIdx = currentIdx;
						if( !command.continuous )
						{
							AE_INFO( "Command: #", command.name.c_str() );
						}
						command.fn( program );
						break; // Only execute the first matching command
					}
				}
			}
		}
	}

	if( m_docSelection->ArrayLength() && gizmoOperation )
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
		
		ae::Entity firstEntity = m_docSelection->ArrayGet( 0 ).NumberGet< ae::Entity >();
		EditorServerObject* selectedObject = GetObjectAssert( firstEntity );
		ae::Matrix4 prevTransform = selectedObject->GetTransform();
		ae::Matrix4 transform = prevTransform;
		ImGuizmo::MODE mode = ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode;
		if( ImGuizmo::Manipulate(
			program->GetWorldToView().data,
			program->GetViewToProj().data,
			gizmoOperation,
			mode,
			transform.data ) && !ImGuizmo::IsUsing() )
		{
			// @TODO: Only call this once after 
			m_doc.EndUndoGroup();
		}
		else if( selectedObject->SetTransform( transform, program ) )
		{
			const ae::Matrix4 change = ( mode == ImGuizmo::LOCAL ) ? prevTransform.GetInverse() * transform : transform * prevTransform.GetInverse();
			for( uint32_t i = 0; i < m_docSelection->ArrayLength(); i++ )
			{
				const ae::Entity entity = m_docSelection->ArrayGet( i ).NumberGet< ae::Entity >();
				if( entity == firstEntity )
				{
					continue;
				}
				EditorServerObject* editorObject = GetObjectAssert( entity );
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

	const bool zenMode = ( m_mode == 0 );
	if( !zenMode && ImGui::Begin( "Dev" ) )
	{
		const float closedSectionHeight = 35.0f; // @TODO: Calculate this
		int32_t currentModePanel = -1;
		const ImVec4 textColor = ImGui::GetStyleColorVec4( ImGuiCol_Text );
		const ImVec4 borderColor = ImGui::GetStyleColorVec4( ImGuiCol_Border );
		auto BeginModePanel = [&]( int32_t mode, const char* title )
		{
			AE_ASSERT( currentModePanel < 0 );
			ImVec2 size = ImVec2( 0, 0 );
			if( mode == 1 )
			{
				size = ImVec2( 0, ( m_mode == mode ) ? -closedSectionHeight : closedSectionHeight );
			}
			ImGui::PushStyleColor( ImGuiCol_Border, ( m_mode == mode ) ? textColor : borderColor );
			if( ImGui::BeginChild( title, size, true ) )
			{
				currentModePanel = mode;
				ImGui::PushStyleColor( ImGuiCol_Text, ( m_mode == mode ) ? textColor : borderColor );
				ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.0f, 0.5f ) ); // Left justify
				ImGui::Text( "%s", title );
				ImGui::PopStyleVar();
				ImGui::PopStyleColor( 1 );
				return ( m_mode == mode );
			}
			return false;
		};
		auto EndModePanel = [&]()
		{
			AE_ASSERT( currentModePanel > 0 );
			ImGui::EndChild();
			ImGui::PopStyleColor(); // Border color
			if( ( m_mode != currentModePanel ) && ImGui::IsItemHovered() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
			{
				m_mode = currentModePanel;
			}
			currentModePanel = -1;
		};
	
		const ae::Str64 objectPropertiesTitle = [&]() -> ae::Str64
		{
			if( m_docSelection->ArrayLength() == 1 )
			{
				ae::Entity firstEntity = m_docSelection->ArrayGet( 0 ).NumberGet< ae::Entity >();
				const EditorServerObject* const firstObj = GetObjectSafe( firstEntity );
			const char* name = firstObj ? firstObj->GetName() : "";
				if( name[ 0 ] )
				{
					return ae::Str64::Format( "[1] Entity # '#' Properties", firstEntity, name );
				}
				else
				{
					return ae::Str64::Format( "[1] Entity # Properties", firstEntity );
				}
			}
			else if( m_GetSelectionLength() )
			{
				return "[1] Multiple Objects Selected";
			}
			else
			{
				return "[1] No Selection";
			}
		}();
		if( BeginModePanel( 1, objectPropertiesTitle.c_str() ) && ( m_GetSelectionLength() == 1 ) )
		{
			ae::Entity firstEntity = m_GetSelectedEntity( 0 );
			EditorServerObject* selectedObject = GetObjectAssert( firstEntity );
			ae::DocumentValue* selectedDocObject = GetDocumentObject( firstEntity );
			{
				char name[ 128 ];
				strlcpy( name, selectedObject->GetName(), countof(name) );
				if( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					AE_INFO( "Set object name: #", name );
					selectedObject->SetName( name );
					m_doc.EndUndoGroup();
				}
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
					m_doc.EndUndoGroup();
				}
				if( ImGui::Button( "Add Component" ) )
				{
					ImGui::OpenPopup( "add_component_popup" );
				}
				ImGui::SameLine();
				ImGui::BeginDisabled( !selectedObject->GetParentEntity() );
				if( ImGui::Button( "Select Parent" ) )
				{
					m_ClearSelection();
					m_AddToSelection( selectedObject->GetParentEntity() );
				}
				ImGui::EndDisabled();
			}
			
			ae::Component* deleteComponent = nullptr;
			ae::DocumentValue* componentsDocValue = selectedDocObject->ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER );
			const uint32_t componentCount = componentsDocValue->ObjectLength();
			for( uint32_t i = 0; i < componentCount; i++ )
			{
				ae::DocumentValue* componentDocValue = &componentsDocValue->ObjectGetValue( i );
				const char* typeName = componentDocValue->ObjectTryGet( DOCUMENT_ENTITY_COMPONENT_TYPE_MEMBER )->StringGet();
				const ae::ClassType* componentType = ae::GetClassTypeByName( typeName );
				ae::Component* component = &m_registry.GetComponent( selectedObject->GetEntity(), componentType );
				{
					ImGui::Separator();
					if( ImGui::TreeNodeEx( componentType->GetName(), ImGuiTreeNodeFlags_DefaultOpen ) )
					{
						ImGui::Unindent( ImGui::GetTreeNodeToLabelSpacing() );
						auto fn = [ & ]( auto& fn, const ae::ClassType* type, ae::Component* component ) -> void
						{
							uint32_t varCount = type->GetVarCount( false );
							if( varCount )
							{
								for( uint32_t i = 0; i < varCount; i++ )
								{
									const ae::ClassVar* var = type->GetVarByIndex( i, false );
									if( m_ShowVar( program, componentDocValue, component, var ) )
									{
										// Wrap variable change in undo group for undo/redo support
										selectedObject->HandleVarChange( program, component, type, var );
										m_doc.EndUndoGroup();
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
						ImGui::Indent( ImGui::GetTreeNodeToLabelSpacing() );
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
		EndModePanel();
	
		uiHoverEntity = kNullEntity;
		if( BeginModePanel( 2, "[2] Object List" ) )
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
				const uint32_t typeCount = ae::GetClassTypeCount();
				for( uint32_t i = 0; i < typeCount; i++ )
				{
					const ae::ClassType* type = ae::GetClassTypeByIndex( i );
					if( !type->attributes.Has< EditorTypeAttribute >() || !type->IsType< ae::Component >() )
					{
						continue;
					}
					const bool anyOfTypeExist = [ & ]()
					{
						for( const auto& [ _, obj ] : m_objects )
						{
							if( obj->GetComponentByType( type ) )
							{
								return true;
							}
						}
						return false;
					}();
					if( anyOfTypeExist )
					{
						if( ImGui::Selectable( type->GetName(), m_objectListType == type ) )
						{
							m_objectListType = type;
						}
						if( m_objectListType == type )
						{
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::EndCombo();
			}
			if( ImGui::BeginListBox( "##listbox", ImVec2( -FLT_MIN, -FLT_MIN ) ) )
			{
				auto showObj = [ & ]( auto& showObj, const EditorServerObject* editorObj ) -> void
				{
					auto hasDescendantWithType = [ & ]( auto& hasDescendantWithType, const EditorServerObject* obj ) -> bool
					{
						const uint32_t childCount = obj->GetChildCount();
						for( uint32_t i = 0; i < childCount; i++ )
						{
							const EditorServerObject* childObj = GetObjectAssert( obj->GetChildEntity( i ) );
							if( childObj && ( childObj->GetComponentByType( m_objectListType ) || hasDescendantWithType( hasDescendantWithType, childObj ) ) )
							{
								return true;
							}
						}
						return false;
					};
					auto hasSelectedDescendant = [ & ]( auto& hasSelectedDescendant, const EditorServerObject* obj ) -> bool
					{
						const uint32_t childCount = obj->GetChildCount();
						for( uint32_t i = 0; i < childCount; i++ )
						{
							const EditorServerObject* childObj = GetObjectAssert( obj->GetChildEntity( i ) );
							if( childObj && ( m_FindInSelection( childObj->GetEntity() ) >= 0 || hasSelectedDescendant( hasSelectedDescendant, childObj ) ) )
							{
								return true;
							}
						}
						return false;
					};

					if( m_objectListType && !editorObj->GetComponentByType( m_objectListType ) && !hasDescendantWithType( hasDescendantWithType, editorObj ) )
					{
						return;
					}

					ae::Str64 name = editorObj->GetName();
					if( !name.Length() )
					{
						const uint32_t componentCount = editorObj->GetComponentCount();
						for( uint32_t i = 0; i < componentCount && !name.Length(); i++ )
						{
							const ae::DocumentValue* component = &editorObj->GetComponentByIndex( i );
							const ae::ClassType* componentType = editorObj->GetComponentTypeByIndex( i );
							const uint32_t typeVarCount = componentType->GetVarCount( true );
							for( uint32_t j = 0; j < typeVarCount && !name.Length(); j++ )
							{
								const ae::ClassVar* classVar = componentType->GetVarByIndex( j, true );
								const ae::BasicType* varType = classVar->GetOuterVarType().AsVarType< ae::BasicType >();
								if( varType && classVar->attributes.Has< EditorDisplayNameAttribute >() )
								{
									const ae::DocumentValue* var = component->ObjectTryGet( classVar->GetName() );
									AE_ASSERT( var );
									name = var->StringGet();
									break;
								}
							}
						}
						if( !name.Length() )
						{
							if( editorObj->GetComponentCount() == 1 )
							{
								name = editorObj->GetComponentTypeByIndex( 0 )->GetName();
							}
							else
							{
								name = ae::Str64::Format( "Entity #", editorObj->GetEntity() );
							}
						}
					}
					const bool isSelected = ( m_FindInSelection( editorObj->GetEntity() ) >= 0 );
					if( isSelected )
					{
						name += " +";
					}
					if( hasSelectedDescendant( hasSelectedDescendant, editorObj ) )
					{
						name += "*";
					}
					const bool hasChildren = editorObj->GetChildCount() > 0;
					const ImGuiTreeNodeFlags flags = ( hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf ) | ( isSelected ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
					name += "###";
					name += ae::ToString( editorObj->GetEntity() ).c_str();
					const SelectionModifier selectionModifier = m_GetSelectionModifier( program );
					const bool isExpanded = ImGui::TreeNodeEx( name.c_str(), flags );
					if( ImGui::IsItemHovered() )
					{
						uiHoverEntity = editorObj->GetEntity();
					}
					if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
					{
						ActiveRefocus( program );
					}
					else if( ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() ) // Ignore clicks that open/close the tree node
					{
						const ae::Entity clickedEntity = editorObj->GetEntity();
						m_SelectWithModifier( selectionModifier, &clickedEntity, 1 );
						m_doc.EndUndoGroup();
					}
					if( ( selectionModifier != SelectionModifier::New ) &&  ImGui::IsItemHovered( ImGuiHoveredFlags_DelayShort ) )
					{
						const auto modifierFormat = m_GetSelectionModifierFormatString( selectionModifier );
						const ae::Str128 selectLabel = ae::Str128::Format( modifierFormat.c_str(), "entity" );
						ImGui::SetTooltip( "%s", selectLabel.c_str() );
					}
					if( ImGui::BeginPopupContextItem() )
					{
						const auto modifierFormat = m_GetSelectionModifierFormatString( selectionModifier );
						ImGui::BeginDisabled( !hasChildren );
						const ae::Str128 selectChildrenLabel = ae::Str128::Format( modifierFormat.c_str(), "children" );
						if( ImGui::MenuItem( selectChildrenLabel.c_str() ) )
						{
							ae::Array< ae::Entity > toSelect = m_tag;
							const uint32_t childCount = editorObj->GetChildCount();
							for( uint32_t i = 0; i < childCount; i++ )
							{
								toSelect.Append( editorObj->GetChildEntity( i ) );
							}
							m_SelectWithModifier( selectionModifier, toSelect.Data(), toSelect.Length() );
							m_doc.EndUndoGroup();
						}
						const ae::Str128 selectTreeLabel = ae::Str128::Format( modifierFormat.c_str(), "tree" );
						if( ImGui::MenuItem( selectTreeLabel.c_str() ) )
						{
							const ae::Entity clickedEntity = editorObj->GetEntity();
							const ae::Array< ae::Entity > toSelect = m_GetTreeFromEntities( &clickedEntity, 1 );
							m_SelectWithModifier( selectionModifier, toSelect.Data(), toSelect.Length() );
							m_doc.EndUndoGroup();
						}
						ImGui::EndDisabled();
						ImGui::EndPopup();
					}
					
					if( isExpanded )
					{
						const uint32_t childCount = editorObj->GetChildCount();
						for( uint32_t i = 0; i < childCount; i++ )
						{
							const ae::Entity childEntity = editorObj->GetChildEntity( i );
							const EditorServerObject* childObj = GetObjectAssert( childEntity );
							showObj( showObj, childObj );
						}
						ImGui::TreePop();
					}
				};

				const uint32_t docObjectCount = m_docObjects->ObjectLength();
				for( uint32_t i = 0; i < docObjectCount; i++ )
				{
					const ae::DocumentValue& docObject = m_docObjects->ObjectGetValue( i );
					const ae::Entity entity = docObject.ObjectTryGet( "id" )->NumberGet< ae::Entity >();
					const EditorServerObject* editorObj = GetObjectAssert( entity );
					if( !editorObj->GetParentEntity() )
					{
						showObj( showObj, editorObj );
					}
				}
				ImGui::EndListBox();
			}
		}
		EndModePanel();
		ImGui::End();
	}
	
	m_first = false;
}

EditorServerObject* EditorServer::CreateObject( EditorProgram* program, Entity entity, const ae::Matrix4& transform, const char* name, bool undo )
{
	if( undo && entity <= m_lastEntity )
	{
		m_lastEntity++;
		entity = m_lastEntity;
	}
	m_lastEntity = ae::Max( m_lastEntity, entity );
	ae::DocumentValue* docObject = &m_docObjects->ObjectSet( ae::ToString( entity ).c_str() );
	const ae::DocumentCallback action = [ this, entity, docObject ]()
	{
		EditorServerObject* editorObject = ae::New< EditorServerObject >( m_tag, m_tag, docObject );
		m_objects.Set( entity, editorObject );
	};
	if( undo )
	{
		docObject->ObjectInitialize( 4 );
		docObject->ObjectSet( "id" ).NumberSet( entity );
		docObject->ObjectSet( "name" ).StringSet( name ? name : "" );
		docObject->ObjectSet( DOCUMENT_ENTITY_PARENT_MEMBER ).NumberSet( ae::kNullEntity );
		docObject->ObjectSet( DOCUMENT_ENTITY_CHILDREN_MEMBER ).ArrayInitialize();
		docObject->ObjectSet( "transform" ).OpaqueSet( transform );
		docObject->ObjectSet( DOCUMENT_ENTITY_COMPONENTS_MEMBER ).ObjectInitialize();
		docObject->GetDocument().AddUndoGroupAction( "Create Object", [ this, program, entity ]() { DestroyObject( program, entity, false ); }, action );
	}
	action();
	return GetObjectAssert( entity );
}

void EditorServer::DestroyObject( EditorProgram* program, ae::Entity entity, bool undo )
{
	if( entity && m_objects.TryGet( entity ) )
	{
		// Collect and destroy descendants first (before any document removal)
		// Collect and destroy descendants first (before any document removal)
		EditorServerObject* editorObject = GetObjectSafe( entity );
		if( editorObject )
		{
			ae::Array< ae::Entity > childEntities = m_tag;
			const uint32_t childCount = editorObject->GetChildCount();
			for( uint32_t i = 0; i < childCount; i++ )
			{
				childEntities.Append( editorObject->GetChildEntity( i ) );
			}
			for( const ae::Entity child : childEntities )
			{
				DestroyObject( program, child, undo );
			}
		}
		const ae::DocumentValue* docObject = m_docObjects->ObjectTryGet( ae::ToString( entity ).c_str() );
		// Send ComponentDestroy events while editorObject is still valid
		if( editorObject )
		{
			const uint32_t typeCount = ae::GetClassTypeCount();
			for( uint32_t i = 0; i < typeCount; i++ )
			{
				const ae::ClassType* type = ae::GetClassTypeByIndex( i );
				if( !type->attributes.Has< ae::EditorTypeAttribute >() )
				{
					continue;
				}
				ae::Component* component = m_registry.TryGetComponent( entity, type );
				if( component )
				{
					RemoveComponent( program, editorObject, component );
				}
			}
		}
		const ae::DocumentCallback action = [ this, entity ]()
		{
			m_RemoveFromSelection( entity );
			m_registry.Destroy( entity );
			EditorServerObject* editorObject = nullptr;
			if( m_objects.Remove( entity, &editorObject ) )
			{
				ae::Delete( editorObject );
			}
		};
		if( undo )
		{
			m_RemoveFromSelection( entity );
			AE_ASSERT( docObject );
			m_doc.AddUndoGroupAction(
				"Destroy Object",
				[ this, program, entity, docObject ]()
				{
					AE_DEBUG_ASSERT( entity == docObject->ObjectTryGet( "id" )->NumberGet< ae::Entity >() );
					const ae::Matrix4 transform = docObject->ObjectTryGet( "transform" )->OpaqueGet< ae::Matrix4 >( ae::Matrix4::Identity() );
					const char* name = docObject->ObjectTryGet( "name" )->StringGet();
					CreateObject( program, entity, transform, name, false );
				},
				action );
			m_docObjects->ObjectRemove( ae::ToString( entity ).c_str() );
		}
		action();
	}
}

static void PopulateDocFromVarData( const ae::Type* varType, ae::ConstDataPointer varData, ae::DocumentValue* doc )
{
	if( const ae::ArrayType* arrayType = varType->AsVarType< ae::ArrayType >() )
	{
		doc->ArrayInitialize();
		const ae::Type* innerType = &arrayType->GetInnerVarType();
		const uint32_t len = arrayType->GetLength( varData );
		for( uint32_t j = 0; j < len; j++ )
		{
			PopulateDocFromVarData( innerType, arrayType->GetElement( varData, j ), &doc->ArrayAppend() );
		}
	}
	else if( const ae::BasicType* basicType = varType->AsVarType< ae::BasicType >() )
	{
		doc->StringSet( basicType->GetVarDataAsString( varData ).c_str() );
	}
	else if( const ae::EnumType* enumType = varType->AsVarType< ae::EnumType >() )
	{
		doc->StringSet( enumType->GetVarDataAsString( varData ).c_str() );
	}
	else if( const ae::ClassType* classType = varType->AsVarType< ae::ClassType >() )
	{
		doc->ObjectInitialize();
		doc->ObjectSet( DOCUMENT_ENTITY_COMPONENT_TYPE_MEMBER ).StringSet( classType->GetName() );
		const uint32_t varCount = classType->GetVarCount( true );
		for( uint32_t i = 0; i < varCount; i++ )
		{
			const ae::ClassVar* var = classType->GetVarByIndex( i, true );
			const void* varPtr = static_cast< const uint8_t* >( varData.Get() ) + var->GetOffset();
			ae::ConstDataPointer nestedData( var->GetOuterVarType(), varPtr );
			PopulateDocFromVarData( &var->GetOuterVarType(), nestedData, &doc->ObjectSet( var->GetName() ) );
		}
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
	component = m_registry.AddComponent( obj->GetEntity(), type );

	{
		ae::DocumentValue* objDocValue = m_docObjects->ObjectTryGet( ae::ToString( obj->GetEntity() ).c_str() );
		AE_ASSERT( objDocValue );
		ae::DocumentValue* compDoc = &objDocValue->ObjectSet( DOCUMENT_ENTITY_COMPONENTS_MEMBER ).ObjectSet( type->GetName() );
		PopulateDocFromVarData( type, ae::ConstDataPointer( *type, component ), compDoc );
	}

	if( component )
	{
		const ae::ClassType* type = ae::GetClassTypeFromObject( component );
		EditorComponent* comp = m_componentPool.New( EditorComponent{ obj->GetEntity(), type->GetName() } );
		obj->AddComponent( comp );
		EditorEvent event;
		event.type = EditorEventType::ComponentCreate;
		event.entity = obj->GetEntity();
		event.transform = obj->GetTransform();
		event.component = comp;
		event.componentDoc = obj->GetComponentByType( type );
		SendPluginEvent( program->plugins, event );
	}
	return component;
}

void EditorServer::RemoveComponent( EditorProgram* program, EditorServerObject* obj, ae::Component* component )
{
	if( obj && component )
	{
		const ae::ClassType* type = ae::GetClassTypeFromObject( component );
		const EditorComponent* comp = obj->GetComponentByType2( type );
		AE_ASSERT( comp );
		EditorEvent event;
		event.type = EditorEventType::ComponentDestroy;
		event.entity = obj->GetEntity();
		event.transform = obj->GetTransform();
		event.component = comp;
		event.componentDoc = obj->GetComponentByType( type );
		SendPluginEvent( program->plugins, event );
		obj->RemoveComponent( comp );
		m_componentPool.Delete( const_cast< EditorComponent* >( comp ) );
		m_registry.DestroyComponent( component );
	}
}

const ae::Component* EditorServer::GetComponent( const EditorServerObject* obj, const ae::ClassType* type ) const
{
	return obj ? m_registry.TryGetComponent( obj->GetEntity(), type ) : nullptr;
}

const EditorServerObject* EditorServer::GetObjectFromComponent( const ae::Component* component )
{
	return component ? GetObjectAssert( component->GetEntity() ) : nullptr;
}

const EditorServerObject* EditorServer::GetObjectAssert( ae::Entity entity ) const
{
	AE_ASSERT_MSG( entity != ae::kNullEntity, "Invalid entity" );
	const ae::EditorServerObject* obj = GetObjectSafe( entity );
	AE_ASSERT_MSG( obj, "Could not find object #", entity );
	return obj;
}

const EditorServerObject* EditorServer::GetObjectNull( ae::Entity entity ) const
{
	if( entity == ae::kNullEntity )
	{
		return nullptr;
	}
	return GetObjectAssert( entity );
}

const EditorServerObject* EditorServer::GetObjectSafe( ae::Entity entity ) const
{
	if( entity == ae::kNullEntity )
	{
		return nullptr;
	}
	return m_objects.Get( entity, nullptr );
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
	for( uint32_t i = 0; i < m_GetSelectionLength(); i++ )
	{
		aabb.Expand( GetObjectAssert( m_GetSelectedEntity( i ) )->GetAABB( program ) );
	}
	return aabb;
}

void EditorServer::HandleTransformChange( EditorProgram* program, ae::Entity entity, const ae::Matrix4& transform )
{
	// @TODO: How to make sure this is called when undoing?
	EditorServerObject* editorObject = GetObjectAssert( entity );

	EditorEvent event;
	event.type = EditorEventType::ComponentEdit;
	event.entity = entity;
	event.transform = editorObject->GetTransform();

	const uint32_t typeCounts = ae::GetClassTypeCount();
	for( uint32_t i = 0; i < typeCounts; i++ )
	{
		const ae::ClassType* componentType = ae::GetClassTypeByIndex( i );
		if( !componentType->attributes.Has< ae::EditorTypeAttribute >() ) { continue; }
		if( ae::Component* component = m_registry.TryGetComponent( entity, componentType ) )
		{
			event.component = editorObject->GetComponentByType2( componentType );
			event.componentDoc = editorObject->GetComponentByType( componentType );
			SendPluginEvent( program->plugins, event );

			for( const auto& specialVar : kSpecialMemberVars )
			{
				const auto vars = GetTypeVarsByName( componentType, specialVar.name );
				for( const ae::ClassVar* var : vars )
				{
					if( specialVar.SetObjectValue( transform, component, var ) )
					{
						editorObject->HandleVarChange( program, component, componentType, var );
						// Note: Undo group for transform changes is managed by the caller (e.g., ImGuizmo manipulation)
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
	wStream.SerializeObject( ae::GetObjectTypeId( component ) );
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
	if( m_GetSelectionLength() || m_hoverEntities.Length() )
	{
		if( !m_doRefocusImm )
		{
			m_refocusHeldTime = 0.0f;
		}
		m_doRefocusImm = 2;
	}
}

void EditorServer::Undo()
{
	m_doc.Undo();
}

void EditorServer::Redo()
{
	m_doc.Redo();
}

ae::DocumentValue* EditorServer::GetDocumentObject( ae::Entity entity )
{
	if( entity == ae::kNullEntity )
	{
		return nullptr;
	}
	return m_docObjects->ObjectTryGet( ae::ToString( entity ).c_str() );
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

		document.AddMember( JSON_SCENE_OBJECTS_NAME, jsonObjects, allocator );
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
		dialog.levelPath = filePath;
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
		AE_WARN( "Cancelling level read '#'", m_pendingLevel->GetURL() );
		program->fileSystem.Destroy( m_pendingLevel );
		m_pendingLevel = nullptr;
	}

	AE_INFO( "Reading... '#'", filePath );
	m_pendingLevel = program->fileSystem.Read( ae::FileSystem::Root::Data, filePath, 2.0f );
}

void EditorServer::Unload( EditorProgram* program )
{
	AE_INFO( "Unload" );

	m_lastEntity = kNullEntity;
	m_objectListType = nullptr;
	m_ClearSelection();
	m_hoverEntities.Clear();
	uiHoverEntity = kNullEntity;
	m_selectRef = SelectRef();
	m_framePickableEntities.Clear();

	m_SetLevelPath( program, "" );

	while( m_objects.Length() )
	{
		DestroyObject( program, m_objects.GetKey( m_objects.Length() - 1 ) );
	}
	AE_ASSERT( m_objects.Length() == 0 );

	m_registry.Clear();

	// @HACK: Currently two LevelUnload's are sent
	EditorEvent event;
	event.type = EditorEventType::LevelUnload;
	event.path = m_levelPath.c_str();
	SendPluginEvent( program->plugins, event );
	bool pluginUnloadError = false;
	for( auto& [ config, plugin ] : program->plugins )
	{
		if( plugin->m_instances.Length() )
		{
			AE_ERROR( "Plugin '#' has # unfreed mesh instances on level unload", config.name, plugin->m_instances.Length() );
			pluginUnloadError = true;
		}
		if( plugin->m_entityInstances.Length() )
		{
			AE_ERROR( "Plugin '#' has # entity references remaining on level unload", config.name, plugin->m_entityInstances.Length() );
			pluginUnloadError = true;
		}
	}
	AE_DEBUG_ASSERT_MSG( !pluginUnloadError, "Plugin unload errors detected. See log for details." );
}

void EditorServer::m_EntityToJson( const EditorServerObject* levelObject, rapidjson::Document::AllocatorType& allocator, ae::Map< const ae::ClassType*, ae::Component* >* defaults, rapidjson::Value* jsonEntity ) const
{
	AE_ASSERT( levelObject );
	AE_ASSERT( jsonEntity->IsObject() );

	// Id
	const ae::Entity entity = levelObject->GetEntity();
	jsonEntity->AddMember( JSON_ENTITY_ID_NAME, entity, allocator );

	// Name
	const char* objectName = levelObject->GetName();
	if( objectName[ 0 ] )
	{
		jsonEntity->AddMember( JSON_ENTITY_NAME_NAME, rapidjson::StringRef( objectName ), allocator );
	}

	// Transform
	rapidjson::Value transformJson;
	const ae::Matrix4 transform = levelObject->GetTransform();
	const auto transformStr = ae::ToString( transform );
	transformJson.SetString( transformStr.c_str(), allocator );
	jsonEntity->AddMember( JSON_TRANSFORM_NAME, transformJson, allocator );
	if( levelObject->GetParentEntity() )
	{
		jsonEntity->AddMember( JSON_PARENT_ID_NAME, (uint32_t)levelObject->GetParentEntity(), allocator );
	}

	// Components
	rapidjson::Value jsonComponents( rapidjson::kObjectType );
	const uint32_t componentTypeCount = ae::GetClassTypeCount();
	for( uint32_t i = 0; i < componentTypeCount; i++ )
	{
		const ae::ClassType* type = ae::GetClassTypeByIndex( i );
		if( !type->attributes.Has< ae::EditorTypeAttribute >() ) { continue; }
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
	jsonEntity->AddMember( JSON_ENTITY_COMPONENTS_NAME, jsonComponents, allocator );
}

void EditorServer::m_CopySelected() const
{
	if( !m_GetSelectionLength() )
	{
		ae::SetClipboardText( "" );
		return;
	}
	// Copy all selected entities and their entire tree
	ae::Array< ae::Entity > toCopy = m_tag;
	toCopy.Reserve( m_GetSelectionLength() );
	for( uint32_t i = 0; i < m_GetSelectionLength(); i++ )
	{
		toCopy.Append( m_GetSelectedEntity( i ) );
	}
	toCopy = m_GetTreeFromEntities( toCopy.Data(), toCopy.Length() );
	// Sort entities so they are pasted in the order they were created, not the
	// order they were selected. ValidateLevel() expects this order.
	std::sort( std::begin( toCopy ), std::end( toCopy ) );
	// Create json scene
	rapidjson::Document document( rapidjson::kObjectType );
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	{
		rapidjson::Value jsonObjects( rapidjson::kArrayType );
		jsonObjects.Reserve( toCopy.Length(), allocator );

		for( ae::Entity entity : toCopy )
		{
			rapidjson::Value jsonObject( rapidjson::kObjectType );
			m_EntityToJson( GetObjectAssert( entity ), allocator, nullptr, &jsonObject );
			jsonObjects.PushBack( jsonObject, allocator );
		}

		document.AddMember( JSON_SCENE_OBJECTS_NAME, jsonObjects, allocator );
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

	const JsonScene scene( m_tag, document, true );
	if( !scene.success )
	{
		InvalidSceneDialog dialog;
		m_PushDialog( dialog );
		AE_WARN( "Unexpected clipboard data format" );
		return;
	}

	// State for loading
	m_ClearSelection();
	ae::Map< ae::Entity, ae::Entity > entityMap = m_tag;

	// Create all components
	for( const JsonEntity& sceneEntity : scene.entities )
	{
		const char* name = ""; // @TODO: sceneEntity.name.c_str() handle duplicate names
		EditorServerObject* editorObject = CreateObject( program, sceneEntity.id, sceneEntity.transform, name );
		if( editorObject->GetEntity() != sceneEntity.id )
		{
			entityMap.Set( sceneEntity.id, editorObject->GetEntity() );
		}
		for( const JsonComponent* sceneComponent : sceneEntity.components )
		{
			AddComponent( program, editorObject, sceneComponent->type );
		}

		// Select pasted 'root' entities only, translating them separately from
		// their parents would be unexpected behavior.
		if( !sceneEntity.parent )
		{
			m_AddToSelection( editorObject->GetEntity() );
		}
	}

	// Serialize all components (second phase to handle references)
	JsonToRegistry( entityMap, document[ JSON_SCENE_OBJECTS_NAME ], &m_registry, m_docObjects );
	for( const JsonEntity& sceneEntity : scene.entities )
	{
		const ae::Entity entityId = entityMap.Get( sceneEntity.id, sceneEntity.id );
		EditorServerObject* object = GetObjectAssert( entityId );

		if( sceneEntity.parentId )
		{
			const ae::Entity parent = entityMap.Get( sceneEntity.parentId, sceneEntity.parentId );
			EditorServerObject* parentObject = GetObjectAssert( parent );
			object->SetParent( this, parentObject );
		}

		for( const JsonComponent* sceneComponent : sceneEntity.components )
		{
			const ae::ClassType* type = sceneComponent->type;
			ae::Component* component = &m_registry.GetComponent( entityId, type );
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
	uint32_t length;
	while( ( length = m_GetSelectionLength() ) )
	{
		DestroyObject( program, m_GetSelectedEntity( length - 1 ) );
	}
}

void EditorServer::m_HideSelected()
{
	if( m_GetSelectionLength() )
	{
		bool anyHidden = false;
		bool anyVisible = false;
		for( uint32_t i = 0; i < m_GetSelectionLength(); i++ )
		{
			ae::Entity entity = m_GetSelectedEntity( i );
			EditorServerObject* editorObject = GetObjectAssert( entity );
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
		for( uint32_t i = 0; i < m_GetSelectionLength(); i++ )
		{
			GetObjectAssert( m_GetSelectedEntity( i ) )->hidden = setHidden;
		}
	}
	else
	{
		AE_INFO( "No objects selected" );
	}
}

void EditorServer::m_ParentSelected( EditorProgram* program )
{
	if( m_GetSelectionLength() < 2 )
	{
		return;
	}
	const ae::Entity parentEntity = m_GetSelectedEntity( m_GetSelectionLength() - 1 );
	ae::EditorServerObject* parentObject = GetObjectAssert( parentEntity );
	if( !parentObject )
	{
		AE_FAIL_MSG( "Could not find parent object '#'" , parentEntity );
		return;
	}
	auto isAncestorOf = [&]( const ae::EditorServerObject* obj ) -> bool
	{
		const ae::EditorServerObject* current = parentObject;
		while( current )
		{
			if( obj == current )
			{
				return true;
			}
			current = current->GetParentEntity() ? GetObjectAssert( current->GetParentEntity() ) : nullptr;
		}
		return false;
	};
	for( uint32_t i = 0; i < m_GetSelectionLength() - 1; i++ )
	{
		ae::EditorServerObject* childObject = GetObjectAssert( m_GetSelectedEntity( i ) );
		if( !childObject )
		{
			AE_FAIL_MSG( "Could not find child object '#'", m_GetSelectedEntity( i ) );
			continue;
		}
		if( isAncestorOf( childObject ) )
		{
			AE_WARN( "Cannot set '#' parent, it is a descendant of '#'", parentObject->GetEntity(), childObject->GetEntity() );
			continue;
		}
		childObject->SetParent( this, parentObject );
		AE_DEBUG( "Set '#' as the parent of '#'", parentObject->GetEntity(), childObject->GetEntity() );
	}
}

void EditorServer::m_UnparentSelected( EditorProgram* program )
{
	for( uint32_t i = 0; i < m_GetSelectionLength(); i++ )
	{
		ae::EditorServerObject* childObject = GetObjectAssert( m_GetSelectedEntity( i ) );
		if( !childObject )
		{
			AE_FAIL_MSG( "Could not find child object #", m_GetSelectedEntity( i ) );
			continue;
		}
		if( childObject->GetParentEntity() )
		{
			childObject->SetParent( this, nullptr );
			AE_DEBUG( "Unparented #", childObject->GetEntity() );
		}
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

SelectionModifier EditorServer::m_GetSelectionModifier( class EditorProgram* program ) const
{
	const bool shift = program->input.Get( ae::Key::LeftShift );
	const bool ctrl = program->input.Get( ae::Key::LeftControl );
	if( shift && ctrl )
	{
		return SelectionModifier::Add;
	}
	else if( shift )
	{
		return SelectionModifier::Toggle;
	}
	else if( ctrl )
	{
		return SelectionModifier::Remove;
	}
	else
	{
		return SelectionModifier::New;
	}
}

ae::Str64 EditorServer::m_GetSelectionModifierFormatString( SelectionModifier modifier ) const
{
	switch( modifier )
	{
		case SelectionModifier::Add:
			return "Add # to selection (Shift + Ctrl)";
		case SelectionModifier::Toggle:
			return "Toggle # selection (Shift)";
		case SelectionModifier::Remove:
			return "Remove # from selection (Ctrl)";
		case SelectionModifier::New:
		default:
			return "Select #";
	}
}

void EditorServer::m_SelectWithModifier( SelectionModifier modifier, const ae::Entity* entities, uint32_t count )
{
	if( modifier == SelectionModifier::Add )
	{
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kNullEntity && m_FindInSelection( entity ) < 0 )
			{
				m_AddToSelection( entity );
			}
		}
	}
	else if( modifier == SelectionModifier::Toggle )
	{
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kNullEntity )
			{
				int32_t idx = m_FindInSelection( entity );
				if( idx < 0 )
				{
					m_AddToSelection( entity );
				}
				else
				{
					m_RemoveFromSelection( entity );
				}
			}
		}
	}
	else if( modifier == SelectionModifier::Remove )
	{
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kNullEntity )
			{
				int32_t idx = m_FindInSelection( entity );
				if( idx >= 0 )
				{
					m_RemoveFromSelection( entity );
				}
			}
		}
	}
	else if( modifier == SelectionModifier::New )
	{
		m_ClearSelection();
		for( uint32_t i = 0; i < count; i++ )
		{
			ae::Entity entity = entities[ i ];
			if( entity != kNullEntity )
			{
				m_AddToSelection( entity );
			}
		}
	}
}

// Document selection helper implementations
int32_t EditorServer::m_FindInSelection( ae::Entity entity ) const
{
	for( uint32_t i = 0; i < m_docSelection->ArrayLength(); i++ )
	{
		if( m_docSelection->ArrayGet( i ).NumberGet< ae::Entity >() == entity )
		{
			return i;
		}
	}
	return -1;
}

void EditorServer::m_SelectAll()
{
	if( m_docObjects->ObjectLength() != m_docSelection->ArrayLength() )
	{
		m_ClearSelection();
		for( const auto& _obj : m_objects )
		{
			m_docSelection->ArrayAppend().NumberSet( _obj.key );
		}
	}
}

void EditorServer::m_ClearSelection()
{
	m_docSelection->ArrayClear();
}

void EditorServer::m_RemoveFromSelection( ae::Entity entity )
{
	const int32_t idx = m_FindInSelection( entity );
	if( idx >= 0 )
	{
		m_docSelection->ArrayRemove( idx );
	}
}

void EditorServer::m_AddToSelection( ae::Entity entity )
{
	if( m_FindInSelection( entity ) < 0 )
	{
		m_docSelection->ArrayAppend().NumberSet( entity );
	}
}

ae::Array< ae::Entity > EditorServer::m_GetTreeFromEntities( const ae::Entity* entities, uint32_t count ) const
{
	ae::Map< ae::Entity, bool > visited = m_tag;
	ae::Array< ae::Entity > result = m_tag;
	auto collectChildren = [&]( auto& collectChildren, const EditorServerObject* obj ) -> void
	{
		const ae::Entity entity = obj->GetEntity();
		if( visited.GetIndex( entity ) >= 0 )
		{
			return;
		}
		visited.Set( entity, true );
		result.Append( entity );
		const uint32_t childCount = obj->GetChildCount();
		for( uint32_t i = 0; i < childCount; i++ )
		{
			const EditorServerObject* childObj = GetObjectAssert( obj->GetChildEntity( i ) );
			if( childObj )
			{
				collectChildren( collectChildren, childObj );
			}
		}
	};
	for( uint32_t i = 0; i < count; i++ )
	{
		const EditorServerObject* editorObj = GetObjectAssert( entities[ i ] );
		if( editorObj )
		{
			collectChildren( collectChildren, editorObj );
		}
	}
	return result;
}

bool EditorServer::m_ShowVar( EditorProgram* program, ae::DocumentValue* docValue, ae::Object* component, const ae::ClassVar* var )
{
	if( GetSpecialMemberVar( var ) )
	{
		return false;
	}

	ae::DocumentValue* varDocValue = docValue->ObjectTryGet( var->GetName() );
	AE_ASSERT( varDocValue );

	bool changed = false;
	ImGui::PushID( var->GetName() );
	if( var->IsArray() )
	{
		const uint32_t arrayLength = varDocValue->ArrayLength();
		AE_ASSERT( arrayLength == var->GetArrayLength( component ) ); // @TODO: Remove
		ImGui::Text( "%s", var->GetName() );
		ImVec2 size( ImGui::GetContentRegionAvail().x, 8 * ImGui::GetTextLineHeightWithSpacing() );
		ImGui::BeginChild( "ChildL", size, true, 0 );
		for( int i = 0; i < arrayLength; i++ )
		{
			ImGui::PushID( i );
			changed |= m_ShowVarValue( program, &varDocValue->ArrayGet( i ), component, var, i );
			ImGui::PopID();
		}
		ImGui::EndChild();
		if( !var->IsArrayFixedLength() )
		{
			bool arrayMaxLength = ( var->GetArrayLength( component ) >= var->GetArrayMaxLength() );
			ImGui::BeginDisabled( arrayMaxLength );
			if( ImGui::Button( "Add" ) )
			{
				// @TODO: varDocValue
				var->SetArrayLength( component, arrayLength + 1 );
				changed = true;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled( !arrayLength );
			if( ImGui::Button( "Remove" ) )
			{
				// @TODO: varDocValue
				var->SetArrayLength( component, arrayLength - 1 );
				changed = true;
			}
			ImGui::EndDisabled();
		}
	}
	else
	{
		changed |= m_ShowVarValue( program, varDocValue, component, var );
	}
	ImGui::PopID();
	return changed;
}

bool EditorServer::m_ShowVarValue( EditorProgram* program, ae::DocumentValue* varDocValue, ae::Object* component, const ae::ClassVar* var, int32_t idx )
{
	const ae::Str64 varName = ( idx < 0 ) ? var->GetName() : ae::Str64::Format( "#", idx );
	switch( var->GetType() )
	{
		case ae::BasicType::Enum:
		{
			const char* currentStr = varDocValue->StringGet();
			const auto result = aeImGui_Enum( var->GetEnumType(), varName.c_str(), currentStr );
			if( result != currentStr )
			{
				varDocValue->StringSet( result.c_str() );
				var->SetObjectValueFromString( component, result.c_str(), idx );
				return true;
			}
			return false;
		}
		case ae::BasicType::Bool:
		{
			bool b = ae::FromString( varDocValue->StringGet(), false );
			if( ImGui::Checkbox( varName.c_str(), &b ) )
			{
				varDocValue->StringSet( ae::ToString( b ).c_str() );
				var->SetObjectValue( component, b, idx ); // @TODO: Remove
				return true;
			}
			return false;
		}
		case ae::BasicType::Float:
		{
			float f = ae::FromString( varDocValue->StringGet(), 0.0f );
			if( ImGui::InputFloat( varName.c_str(), &f ) )
			{
				varDocValue->StringSet( ae::ToString( f ).c_str() );
				var->SetObjectValue( component, f, idx );
				return true;
			}
			return false;
		}
		case ae::BasicType::String:
		{
			char buf[ 256 ];
			ae::_strlcpy( buf, varDocValue->StringGet(), sizeof(buf) );
			ImGui::Text( "%s", varName.c_str() );
			if( ImGui::InputTextMultiline( varName.c_str(), buf, sizeof(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4 ), 0 ) )
			{
				varDocValue->StringSet( buf );
				var->SetObjectValueFromString( component, buf, idx );
				return true;
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
					if( selectObj )
					{
						const ae::Entity selectEntity = selectObj->GetEntity();
						m_SelectWithModifier( m_GetSelectionModifier( program ), &selectEntity, 1 );
						m_doc.EndUndoGroup();
					}
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
	std::ostringstream str;
	str << editorObj->GetEntity() << " " << type->GetName();
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
		if( EditorServerObject* editorObj = program->editor.GetObjectAssert( entity ) )
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

	ae::RaycastParams raycastParams;
	raycastParams.source = mouseRaySrc;
	raycastParams.ray = mouseRay * kEditorViewDistance;
	if( program->input.Get( ae::Key::M ) )
	{
		raycastParams.maxHits = -1;
	}
	ae::CollisionMeshRaycastParams meshParams;
	meshParams.hitClockwise = false;
	meshParams.hitCounterclockwise = true;
	meshParams.debug = ( program->input.Get( ae::Key::N ) || program->input.Get( ae::Key::M ) ) ? &program->debugLines : nullptr; // Debug only
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
			// m_selectEntity is allowed to be invalid if the mesh isn't used
			// for object selection. The lifetime of the mesh instance is
			// controlled by the plugin, so the mesh may outlive the object it
			// represents.
			const EditorServerObject* editorObj = GetObjectSafe( instance->m_selectEntity );
			if( editorObj && editorObj->hidden )
			{
				continue;
			}
			raycastParams.userData = editorObj;
			meshParams.transform = instance->transform;
			result = instance->m_mesh->collision.Raycast( raycastParams, meshParams, result );
		}
	}
	const uint32_t editorObjectCount = m_objects.Length();
	for( uint32_t i = 0; i < editorObjectCount; i++ )
	{
		const EditorServerObject* editorObj = m_objects.GetValue( i );
		if( !editorObj->hidden && m_framePickableEntities.Get( editorObj->GetEntity(), PickingType::Disabled ) == PickingType::Logic )
		{
			float hitT = INFINITY;
			ae::Vec3 hitPos( 0.0f );
			ae::Vec3 normal( 0.0f );
			const ae::Sphere sphere( editorObj->GetTransform().GetTranslation(), 0.5f );
			if( sphere.Raycast( raycastParams.source, raycastParams.ray, &hitPos, &normal, &hitT ) )
			{
				raycastParams.userData = editorObj;
				result = ae::Raycast( sphere, raycastParams, result );
			}
		}
	}
	if( result.hits.Length() )
	{
		if( meshParams.debug )
		{
			std::string str;
			for( uint32_t i = 0; i < result.hits.Length(); i++ )
			{
				const EditorServerObject* editorObj = result.hits[ i ].userData.Get< const EditorServerObject* >();
				str += ae::Str256::Format( "Hit # at #, object: #\n", i, result.hits[ i ].distance, editorObj ? editorObj->GetEntity() : 0 ).c_str();
			}
			ImGui::SetTooltip( "%s", str.c_str() );
		}

		*hitOut = result.hits[ 0 ].position;
		*normalOut = result.hits[ 0 ].normal;
		const EditorServerObject* editorObj = result.hits[ 0 ].userData.Get< const EditorServerObject* >();
		return editorObj ? editorObj->GetEntity() : kNullEntity;
	}

	return kNullEntity;
}

ae::Color EditorServer::m_GetColor( ae::Entity entity, bool objectLineColor ) const
{
	// @TODO: There's a bug here somewhere causing entity colors to change when
	// an entity is deleted.
	const EditorServerObject* editorObj = GetObjectAssert( entity );
	const bool isHovered = ( uiHoverEntity == entity || m_hoverEntities.Find( entity ) >= 0 );
	const bool isSelected = ( m_FindInSelection( entity ) >= 0 );
	const bool isChildOfSelected = [&]() -> bool
	{
		ae::Entity current = editorObj->GetParentEntity();
		while( current )
		{
			if( m_FindInSelection( current ) >= 0 )
			{
				return true;
			}
			const EditorServerObject* currentObj = GetObjectAssert( current );
			current = currentObj ? currentObj->GetParentEntity() : ae::kNullEntity;
		}
		return false;
	}();
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
		else if( isHovered || isChildOfSelected ) // Okay that these are the same color, since hover also adds lines
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
				// @TODO: GetComponentTypeRequirements() to handle transitive requirements
				requirementsOut->Append( requiredType );
			}
			// @TODO: Log missing types
		}
	};
	fn( fn, type );
}

void JsonToComponent( const ae::Matrix4& transform, const rapidjson::Value& jsonComponent, Component* component, ae::DocumentValue* compDoc )
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
		ae::DocumentValue* varDoc = compDoc ? compDoc->ObjectTryGet( var->GetName() ) : nullptr;
		if( var->IsArray() && jsonVar.IsArray() )
		{
			const auto& jsonVarArray = jsonVar.GetArray();
			const uint32_t arrayLen = var->SetArrayLength( component, jsonVarArray.Size() );
			AE_ASSERT( arrayLen <= jsonVarArray.Size() );
			if( varDoc )
			{
				varDoc->ArrayClear();
			}
			for( uint32_t j = 0; j < arrayLen; j++ )
			{
				const auto& jsonElement = jsonVarArray[ j ];
				var->SetObjectValueFromString( component, jsonElement.GetString(), j );
				if( varDoc )
				{
					varDoc->ArrayAppend().StringSet( jsonElement.GetString() );
				}
			}
		}
		// @TODO: Handle patching references
		else if( !jsonVar.IsObject() && !jsonVar.IsArray() )
		{
			var->SetObjectValueFromString( component, jsonVar.GetString() );
			if( varDoc )
			{
				varDoc->StringSet( jsonVar.GetString() );
			}
		}
	}
}

void JsonToRegistry( const ae::Map< ae::Entity, ae::Entity >& entityMap, const rapidjson::Value& jsonObjects, ae::Registry* registry, ae::DocumentValue* docObjects )
{
	// Serialize all components (second phase to handle references)
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		const ae::Entity jsonEntity = jsonObject[ JSON_ENTITY_ID_NAME ].GetUint();
		const ae::Entity entity = entityMap.Get( jsonEntity, jsonEntity );
		const ae::Matrix4 transform = ae::FromString< ae::Matrix4 >( jsonObject[ JSON_TRANSFORM_NAME ].GetString(), ae::Matrix4::Identity() );
		for( const auto& componentIter : jsonObject[ JSON_ENTITY_COMPONENTS_NAME ].GetObject() )
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
			ae::DocumentValue* compDoc = nullptr;
			if( docObjects )
			{
				const ae::Str32 entityKey = ae::ToString( entity ).c_str();
				ae::DocumentValue* entityDoc = docObjects->ObjectTryGet( entityKey.c_str() );
				ae::DocumentValue* components = entityDoc
					? entityDoc->ObjectTryGet( DOCUMENT_ENTITY_COMPONENTS_MEMBER )
					: nullptr;
				compDoc = components ? components->ObjectTryGet( type->GetName() ) : nullptr;
			}
			ae::JsonToComponent( transform, componentIter.value, component, compDoc );
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

// @TODO: Fold this into JsonScene
bool ValidateLevel( const rapidjson::Value& jsonLevel )
{
	if( !jsonLevel.IsObject() || !jsonLevel.HasMember( JSON_SCENE_OBJECTS_NAME ) )
	{
		AE_ERR( "Invalid 'objects' array" );
		return false;
	}
	
	const auto& jsonObjects = jsonLevel[ JSON_SCENE_OBJECTS_NAME ];
	if( !jsonObjects.IsArray() )
	{
		return false;
	}

	uint32_t prevId = 0;
	for( const auto& jsonObject : jsonObjects.GetArray() )
	{
		if( !jsonObject.HasMember( JSON_ENTITY_ID_NAME ) )
		{
			// @TODO
			return false;
		}
		const uint32_t id = jsonObject[ JSON_ENTITY_ID_NAME ].GetUint();
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

		if( !jsonObject.HasMember( JSON_TRANSFORM_NAME ) )
		{
			AE_ERR( "Entity '#' has no transform data", id );
			return false;
		}

		if( !jsonObject.HasMember( JSON_ENTITY_COMPONENTS_NAME ) )
		{
			AE_ERR( "Entity '#' has no components", id );
			return false;
		}
		const auto& jsonComponents = jsonObject[ JSON_ENTITY_COMPONENTS_NAME ];
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

//------------------------------------------------------------------------------
// JsonScene helper
//------------------------------------------------------------------------------
JsonScene::JsonScene( const ae::Tag& tag, rapidjson::Value& scene, bool allowMissingParents ) : entityLookup( tag ), entities( tag ), components( tag ), success( false ), json( scene )
{
	if( !ValidateLevel( scene ) )
	{
		return;
	}
	auto Clear = [ & ]()
	{
		entityLookup.Clear();
		entities.DeleteAll();
		components.DeleteAll();
	};
	const rapidjson::Value& jsonEntities = scene[ JSON_SCENE_OBJECTS_NAME ];
	// Create all entities first
	for( rapidjson::SizeType i = 0; i < jsonEntities.Size(); i++ )
	{
		const rapidjson::Value& jsonEntity = jsonEntities[ i ];
		const char* transformString = ( jsonEntity.HasMember( JSON_TRANSFORM_NAME ) ? jsonEntity[ JSON_TRANSFORM_NAME ].GetString() : "" );
		JsonEntity* entity = entities.New( JsonEntity{ .id = static_cast< ae::Entity >( jsonEntity[ JSON_ENTITY_ID_NAME ].GetUint() ),
			.name = ( jsonEntity.HasMember( JSON_ENTITY_NAME_NAME ) ? jsonEntity[ JSON_ENTITY_NAME_NAME ].GetString() : "" ),
			.transform = ae::FromString( transformString, ae::Matrix4::Identity() ),
			.parentId = ( jsonEntity.HasMember( JSON_PARENT_ID_NAME ) ? jsonEntity[ JSON_PARENT_ID_NAME ].GetUint() : ae::kNullEntity ),
			.parent = nullptr,
			.children = tag,
			.components = tag,
			.json = jsonEntity } );
		if( !entity->id )
		{
			AE_ERROR( "Entity ID cannot be zero" );
			Clear();
			return;
		}
		entityLookup.Set( entity->id, entity );

		// Allocate entity components
		for( const auto& [ jsonTypeStr, jsonComponent ] : jsonEntity[ JSON_ENTITY_COMPONENTS_NAME ].GetObject() )
		{
			const ae::ClassType* componentType = ae::GetClassTypeByName( jsonTypeStr.GetString() );
			if( !componentType )
			{
				AE_ERROR( "Unknown component type '#'", jsonTypeStr.GetString() );
				Clear();
				return;
			}
			entity->components.Append( components.New( JsonComponent{ .type = componentType, .json = jsonComponent } ) );
		}
	}
	// All entities created, link parents and children
	for( const auto& [ _, entity ] : entityLookup )
	{
		if( entity->parentId )
		{
			entity->parent = entityLookup.Get( entity->parentId, nullptr );
			if( entity->parent )
			{
				const_cast< JsonEntity* >( entity->parent )->children.Append( entity );
			}
			else if( !allowMissingParents )
			{
				Clear();
				return;
			}
		}
	}
	// @TODO: Validate that there are no cycles in parent/child relationships
	success = true;
}

JsonScene::~JsonScene()
{
	components.DeleteAll();
	entities.DeleteAll();
	entityLookup.Clear();
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
	ae::_strlcat( args, m_params->argv[ 0 ], sizeof(args) );
	ae::_strlcat( args, " --editor", sizeof(args) );
	if( levelPath[ 0 ] )
	{
		ae::_strlcat( args, " --level ", sizeof(args) );
		ae::_strlcat( args, levelPath, sizeof(args) );
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
