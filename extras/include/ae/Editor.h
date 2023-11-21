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
#include "aether.h"
#include "Entity.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
namespace ae {

const uint32_t kMaxEditorMessageSize = 1024;

//------------------------------------------------------------------------------
// ae::EditorComponent class
//------------------------------------------------------------------------------
class EditorComponent
{
public:
	EditorComponent( const ae::Tag& tag ) : members( tag ) {}
	ae::Str32 type;
	ae::Dict members;
};

//------------------------------------------------------------------------------
// ae::EditorObject class
//------------------------------------------------------------------------------
class EditorObject
{
public:
	EditorObject( const ae::Tag& tag ) : components( tag ) {}
	ae::Entity id = kInvalidEntity;
	ae::Str16 name;
	ae::Matrix4 transform = ae::Matrix4::Identity();
	ae::Array< EditorComponent > components;
};

//------------------------------------------------------------------------------
// ae::EditorLevel class
//------------------------------------------------------------------------------
class EditorLevel
{
public:
	EditorLevel( const ae::Tag& tag ) : objects( tag ) {}
	ae::Str256 filePath;
	ae::Map< ae::Entity, EditorObject > objects;
};

//------------------------------------------------------------------------------
// ae::EditorMesh class
//------------------------------------------------------------------------------
class EditorMesh
{
public:
	EditorMesh( const ae::Tag& tag );
	void Load( const ae::OBJFile& file );
	ae::Array< ae::Vec3 > verts;
	ae::Array< uint32_t > indices;
};

//------------------------------------------------------------------------------
// ae::LoadEditorMeshFn
//------------------------------------------------------------------------------
typedef ae::EditorMesh(*LoadEditorMeshFn)( void* userData, const char* resourceId );

//------------------------------------------------------------------------------
// ae::EditorParams class
//------------------------------------------------------------------------------
struct EditorParams
{
	int argc = 0;
	char** argv = nullptr;
	//! If true the editor will always run on initialization, ignoring the command line arguments.
	bool run = false;
	uint16_t port = 7200;
	//! Only ae::Axis::Z and ae::Axis::Y are supported
	ae::Axis worldUp = ae::Axis::Z;
	//! When ae::Editor is given a relative path it will use this instead of the current working directory
	ae::Str256 dataDir;

	//! Implement this so ae::Editor can display editor object meshes. Register a variable with the following tag
	//! to display a mesh: AE_REGISTER_CLASS_PROPERTY_VALUE( MyClass, ae_mesh_resource, myVar );
	//! The contents of 'myVar' will be converted to a string and passed to loadMeshFn() as the resourceId.
	LoadEditorMeshFn loadMeshFn = nullptr;
	//! Provided to loadMeshFn() as the userData parameter
	void* loadMeshUserData = nullptr;
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
	void QueueRead( const char* levelPath );
	
	// @TODO: Editor message queue for: level loaded success/failure, mesh requested, object changed
	ae::EditorLevel* GetWritableLevel() { return m_file ? nullptr : &m_level; }
	const ae::EditorLevel* GetLevel() const { return &m_level; }
	uint32_t GetLevelChangeSeq() const { return m_levelSeq; }
	void SetFunctionPointers( LoadEditorMeshFn loadMeshFn, void* loadMeshUserData );

private:
	friend class EditorServer;
	void m_Fork();
	void m_Connect();
	void m_Read();
	const ae::Tag m_tag;
	EditorParams m_params;
	ae::FileSystem m_fileSystem;
	const ae::File* m_file = nullptr;
	ae::EditorLevel m_level;
	uint32_t m_levelSeq = 0;
	ae::Socket m_sock;
	uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
};

} // end ae namespace
#endif
