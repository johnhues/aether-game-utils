//------------------------------------------------------------------------------
// EditorClient.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
// Created by John Hughes on 11/22/21.
//------------------------------------------------------------------------------
#ifndef EDITORCLIENT_H
#define EDITORCLIENT_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/aeImGui.h"
#include "ImGuizmo.h"
#include "Entity.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EDITOR = ae::Tag( "editor" );

class ResourceManager;
class Level;

namespace ae {

const uint32_t kMaxEditorMessageSize = 1024;

bool EditorMain( int argc, char *argv[] );

//------------------------------------------------------------------------------
// LevelObject
//------------------------------------------------------------------------------
class LevelObject
{
public:
  Entity id = kInvalidEntity;
  ae::Str16 name;
  ae::Matrix4 transform = ae::Matrix4::Identity();
  ae::Map< ae::Str32, ae::Dict > components = TAG_EDITOR;
  
  void Initialize( const void* data, uint32_t length );
  void Sync( const void* data, uint32_t length );
};

//------------------------------------------------------------------------------
// Level functions
//------------------------------------------------------------------------------
typedef std::function< void( const LevelObject& levelObject, Entity entity, class Registry* registry ) > CreateObjectFn;
class Level
{
public:
  void Save( const class Registry* registry );
  bool Load( class Registry* registry, CreateObjectFn fn = nullptr ) const;
  
  bool Write() const;
  bool Read( const char* path );
  
  ae::Str256 filePath;
  ae::Map< Entity, LevelObject > objects = TAG_EDITOR;
};

//------------------------------------------------------------------------------
// Editor class
//------------------------------------------------------------------------------
class EditorClient
{
public:
  EditorClient( const ae::Tag& tag );
  void Initialize( uint16_t port, ae::Axis worldUp );
  void Update();
  void Terminate();
  
  void Launch();
  bool IsConnected() const { return m_sock.IsConnected(); }
  
  bool levelDidChange = false;
  Level level;

private:
  void m_Connect();
  uint16_t m_port = 0;
  ae::Axis m_worldUp = ae::Axis::Z;
  ae::Socket m_sock;
  uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
};

//------------------------------------------------------------------------------
// EditorObject
//------------------------------------------------------------------------------
class EditorObject
{
public:
  void Initialize( Entity entity, ae::Matrix4 transform );
  void SetTransform( const ae::Matrix4& transform, class EditorProgram* program );
  ae::Matrix4 GetTransform( const class EditorProgram* program ) const;
  
  bool IsDirty() const { return m_dirty; }
  void ClearDirty() { m_dirty = false; }
  
  bool hidden = false;
  Entity entity = kInvalidEntity; // @TODO: Should have it's own storage for object properties
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
  void Initialize( class EditorProgram* program );
  void Terminate( class EditorProgram* program );
  void Update( class EditorProgram* program );
  void Render( class EditorProgram* program );
  void ShowUI( class EditorProgram* program );
  
  bool SaveLevel( class EditorProgram* program, bool saveAs );
  bool OpenLevel( class EditorProgram* program );
  
  bool GetActive() const { return m_active; }
  bool GetShowInvisible() const { return m_showInvisible; }
  void SetOpen( bool isOpen );
  
  ae::ListenerSocket sock = TAG_EDITOR;
  
private:
  void m_ShowVar( class EditorProgram* program, Component* component, const ae::Var* var );
  void m_ShowVarValue( class EditorProgram* program, Component* component, const ae::Var* var, int32_t idx = -1 );
  void m_ShowRefVar( class EditorProgram* program, Component* component, const ae::Var* var, int32_t idx = -1 );
  Entity m_PickObject( class EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut );
  void m_ShowEditorObject( EditorProgram* program, Entity entity, ae::Color color );
  bool m_first = true;
  bool m_active = true;
  bool m_showInvisible = false;
  bool m_toHide = false;
  std::function< bool( const ae::Type*, const char*, ae::Object** ) > m_getObjectPointerFromString;

  const ae::Type* m_selectedType = nullptr;
  Entity selected = kInvalidEntity;
  ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
  ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
  class Level* level;

  ae::Array< EditorConnection* > connections = TAG_EDITOR;
  ae::Map< Entity, EditorObject > m_objects = TAG_EDITOR;
  
  uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
  
  struct SelectRef
  {
    bool enabled = false;
    Component* component = nullptr;
    const ae::Var* componentVar = nullptr;
    int32_t varIdx = -1;
    
    Entity pending = kInvalidEntity;
  };
  SelectRef m_selectRef;
};

class EditorProgram
{
public:
  void Initialize( uint32_t port, ae::Axis worldUp );
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
  
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::FileSystem fileSystem;
  ae::TimeStep timeStep;
  aeImGui ui;
  ae::DebugCamera camera;
  ae::DebugLines debugLines;
  EditorServer editor;
  Registry registry;
  class ResourceManager* resourceManager;

  ae::RenderTarget gameTarget;
  ae::Shader shader;
  ae::VertexData vertexData;
  
  uint16_t port = 0;
  ae::Axis worldUp = ae::Axis::Z;
  
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
  float m_dt;
  ae::Matrix4 m_viewToProj = ae::Matrix4::Identity();
  ae::Matrix4 m_worldToView = ae::Matrix4::Identity();
  ae::Matrix4 m_worldToProj = ae::Matrix4::Identity();
  ae::Matrix4 m_projToWorld = ae::Matrix4::Identity();
  float m_fov = 0.46f; // 28mm camera 65.5 degree horizontal fov
  ae::Vec3 m_mouseRay = ae::Vec3( 0.0f );
  float m_barWidth = 0.0f;
};

} // end ae namespace
#endif
