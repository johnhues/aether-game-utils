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

void EditorMain( int argc, char *argv[] );

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

} // end ae namespace
#endif
