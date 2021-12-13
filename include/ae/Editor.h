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
#include "ae/aether.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
namespace ae {

const uint32_t kMaxEditorMessageSize = 1024;

typedef uint32_t EditorObjectId;
const EditorObjectId kInvalidEditorObjectId = 0;

//------------------------------------------------------------------------------
// ae::EditorObject class
//------------------------------------------------------------------------------
class EditorObject
{
public:
  EditorObject( const ae::Tag& tag ) : components( tag ) {}
  EditorObjectId id = kInvalidEditorObjectId;
  ae::Str16 name;
  ae::Matrix4 transform = ae::Matrix4::Identity();
  ae::Map< ae::Str32, ae::Dict > components;
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
// ae::EditorParams class
//------------------------------------------------------------------------------
struct EditorParams
{
  int argc = 0;
  char** argv = nullptr;
  uint16_t port = 7200;
  //! Only ae::Axis::Z and ae::Axis::Y are supported
  ae::Axis worldUp = ae::Axis::Z;
  //! Implement this so ae::Editor can display editor object meshes. Register a variable with the following tag
  //! to display a mesh: AE_REGISTER_CLASS_PROPERTY_VALUE( MyClass, ae_mesh_resource, myVar );
  //! The contents of 'myVar' will be converted to a string and passed to loadMeshFn() as the resourceId.
  std::function< ae::EditorMesh( const char* resourceId ) > loadMeshFn;
  bool launchEditor = false;
};

//------------------------------------------------------------------------------
// ae::Editor class
//------------------------------------------------------------------------------
class Editor
{
public:
  Editor( const ae::Tag& tag );
  ~Editor();
  void Initialize( const EditorParams& params );
  void Update();
  void Launch();
  bool IsConnected() const { return m_sock.IsConnected(); }
  bool Write() const;
  bool Read( const char* path );
  
  ae::Str256 filePath;
  bool levelDidChange = false;
  ae::Map< EditorObjectId, EditorObject > objects;

private:
  void m_Connect();
  const ae::Tag m_tag;
  EditorParams m_params;
  ae::Socket m_sock;
  uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
};

} // end ae namespace
#endif