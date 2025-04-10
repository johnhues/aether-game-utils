//------------------------------------------------------------------------------
// Editor.h
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
	ae::Dict<> members;
};

//------------------------------------------------------------------------------
// ae::EditorMesh class
//------------------------------------------------------------------------------
class EditorMesh
{
public:
	EditorMesh( const ae::Tag& tag );
	void Load( const ae::OBJLoader& file );
	ae::Array< ae::Vec3 > verts;
	ae::Array< uint32_t > indices;
};

//------------------------------------------------------------------------------
// Editor callback functions
//------------------------------------------------------------------------------
typedef void(*OnLevelLoadStartFn)( void* userData, const char* levelPath );
typedef ae::Optional< ae::EditorMesh >(*LoadEditorMeshFn)( void* userData, const char* resourceId );
typedef bool(*PreFileEditFn)( void* userData, const char* filePath );

//------------------------------------------------------------------------------
// ae::EditorFunctionPointers
//------------------------------------------------------------------------------
struct EditorFunctionPointers
{
	//! @TODO
	ae::OnLevelLoadStartFn onLevelLoadStartFn = nullptr;
	//! Implement this so ae::Editor can display editor object meshes. Register
	//! a variable with the following tag to display a mesh:
	//! AE_REGISTER_CLASS_PROPERTY_VALUE( MyClass, ae_mesh_resource, myVar );
	//! The contents of 'myVar' will be converted to a string and passed to
	//! loadMeshFn() as the resourceId. The given function should return a
	//! complete ae::Optional< ae::EditorMesh >.
	ae::LoadEditorMeshFn loadMeshFn = nullptr;
	//! Called before a file is opened for editing. Return false to prevent the
	//! file from being opened. Useful for checking out files from source control.
	ae::PreFileEditFn preFileEditFn = nullptr;
	//! Provided to all callback functions as the userData parameter
	void* userData = nullptr;
};

//------------------------------------------------------------------------------
// ae::EditorParams class
//------------------------------------------------------------------------------
struct EditorParams
{
	EditorParams( int argc, char** argv, ae::Registry* registry ) : argc( argc ), argv( argv ), registry( registry ) {}
	EditorParams( const EditorParams& ) = default;
	
	//! Program command line argument count
	int argc = 0;
	//! Program command line arguments
	char** argv = nullptr;
	//! @TODO
	ae::Registry* registry = nullptr;

	//! If true the editor will always run on initialization, ignoring the
	//! command line arguments.
	bool run = false;
	//! The port used for connections between ae::Editor and the launched editor
	//! application
	uint16_t port = 7200;
	//! Only ae::Axis::Z and ae::Axis::Y are supported
	ae::Axis worldUp = ae::Axis::Z;
	//! When ae::Editor is given a relative path it will use this instead of
	//! the current working directory
	ae::Str256 dataDir;
	//! Function pointers for editor callbacks
	ae::EditorFunctionPointers functionPointers;
	//! If a level is specified it will be opened on editor launch. The path can
	//! be absolute or relative. See 'dataDir' for more information on relative
	//! paths. This field will be ignored if a level is specified on the command
	//! line with '--level'.
	ae::Str256 levelPath;
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
	void Terminate();
	void SetFunctionPointers( const ae::EditorFunctionPointers& functionPointers );
	void Update();
	void Launch();
	bool IsConnected() const { return m_sock.IsConnected(); }
	void QueueRead( const char* levelPath );

private:
	friend class EditorServer;
	void m_Fork();
	void m_Connect();
	void m_Read();
	const ae::Tag m_tag;
	EditorParams* m_params = nullptr;
	ae::Str256 m_lastLoadedLevel;
	ae::FileSystem m_fileSystem;
	const ae::File* m_pendingLevel = nullptr;
	ae::Socket m_sock;
	uint8_t m_msgBuffer[ kMaxEditorMessageSize ];
	ae::Map< ae::Entity, bool > m_editorEntities;
};

} // end ae namespace
#endif
