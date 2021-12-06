//------------------------------------------------------------------------------
// Editor.h
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
class Level;

namespace ae {

const uint32_t kMaxEditorMessageSize = 1024;

//------------------------------------------------------------------------------
// LevelObject
//------------------------------------------------------------------------------
class LevelObject
{
public:
  LevelObject( const ae::Tag& tag ) : components( tag ) {}
  Entity id = kInvalidEntity;
  ae::Str16 name;
  ae::Matrix4 transform = ae::Matrix4::Identity();
  ae::Map< ae::Str32, ae::Dict > components;
  
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
  Level( const ae::Tag& tag ) : tag( tag ), objects( tag ) {}
  
  void Save( const class Registry* registry, class EditorServer* editor_HACK );
  bool Load( class Registry* registry, CreateObjectFn fn = nullptr ) const;
  
  bool Write() const;
  bool Read( const char* path );
  
  const ae::Tag tag;
  ae::Str256 filePath;
  ae::Map< Entity, LevelObject > objects;
};

//------------------------------------------------------------------------------
// ae::EditorMesh class
//------------------------------------------------------------------------------
class EditorMesh
{
public:
  EditorMesh( const ae::Tag& tag );
  ae::Array< ae::Vec3 > verts;
  ae::Array< uint32_t > indices;
};

//------------------------------------------------------------------------------
// ae::Editor class
//------------------------------------------------------------------------------
class Editor
{
public:
  struct Params
  {
    bool IsEditorMode() const;
    
    int argc = 0;
    char** argv = nullptr;
    uint16_t port = 7200;
    //! Only ae::Axis::Z and ae::Axis::Y are supported
    ae::Axis worldUp = ae::Axis::Z;
    //! Implement this so ae::Editor can display editor object meshes. Register a variable with the following tag
    //! to display a mesh: AE_REGISTER_CLASS_PROPERTY_VALUE( MyClass, ae_editor_mesh, myVar );
    //! The contents of 'myVar' will be converted to a string and passed to loadMeshFn() as the resourceId.
    std::function< ae::EditorMesh( const char* resourceId ) > loadMeshFn;
    bool launchEditor = false;
  };
  // @TODO: Replace Create() with ctor?
  static ae::Editor* Create( const ae::Tag& tag, const ae::Editor::Params& params );
  static void Destroy( ae::Editor* editor );
  
  void Update();
  void Launch();
  bool IsConnected() const { return m_sock.IsConnected(); }
  
  bool levelDidChange = false;
  Level level;

private:
  Editor( const ae::Tag& tag, const Params& params );
  void m_Connect();
  const ae::Tag m_tag;
  const Params m_params;
  ae::Socket m_sock;
  uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
};

} // end ae namespace
#endif
