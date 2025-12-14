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
#ifndef AE_EDITOR_CLIENT_H
#define AE_EDITOR_CLIENT_H

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
class Editor;

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
// ae::EditorMeshInstance class
//------------------------------------------------------------------------------
class EditorMeshInstance
{
public:
	ae::Matrix4 transform = ae::Matrix4::Identity();
	ae::Color color = ae::Color::AetherGray();
private:
	friend class EditorPlugin;
	friend class EditorServer;
	friend class EditorServerObject;
	class EditorServerMesh* m_mesh = nullptr;
	ae::Entity m_selectEntity = kNullEntity;
	ae::ListNode< EditorMeshInstance > m_entityInstance = this;
};

//------------------------------------------------------------------------------
// ae::EditorTypeAttribute class
//------------------------------------------------------------------------------
//! This attribute is used to mark an ae::Component type as available for
//! authoring in the editor.
//------------------------------------------------------------------------------
class EditorTypeAttribute final : public ae::Inheritor< ae::Attribute, EditorTypeAttribute >
{
public:
};

//------------------------------------------------------------------------------
// ae::EditorRequiredAttribute class
//------------------------------------------------------------------------------
//! This attribute should be added to an ae::Component to mark other components
//! as required. Components in this list will be added automatically when the
//! component is added to an entity.
//------------------------------------------------------------------------------
class EditorRequiredAttribute final : public ae::Inheritor< ae::Attribute, EditorRequiredAttribute >
{
public:
	EditorRequiredAttribute( const char* className ) : className( className ) {}
	ae::Str64 className;
};

//------------------------------------------------------------------------------
// ae::EditorVisibilityAttribute class
//------------------------------------------------------------------------------
//! This attribute should be added to a component variable (a meta registered
//! bool) to control rendering in the editor.
//------------------------------------------------------------------------------
class EditorVisibilityAttribute final : public ae::Inheritor< ae::Attribute, EditorVisibilityAttribute >
{
public:
};

//------------------------------------------------------------------------------
// ae::EditorDisplayNameAttribute class
//------------------------------------------------------------------------------
//! This attribute is used to flag a variable for use as the display name for
//! a component in the editor.
//------------------------------------------------------------------------------
class EditorDisplayNameAttribute final : public ae::Inheritor< ae::Attribute, EditorDisplayNameAttribute >
{
public:
};

//------------------------------------------------------------------------------
// ae::EditorEventType
//------------------------------------------------------------------------------
enum class EditorEventType
{
	None,

	FileEdit, //!< Received before a file is opened for editing
	
	Update, //!< Received once each editor frame
	Terminate, //!< Received when the editor is shutting down
	LevelLoad, //!< Received right after a level has been loaded
	LevelUnload, //!< Received right before a level will be unloaded
	ReloadResources, //!< Received when the user has requested a resource reload
	
	ComponentCreate, //!< Received when a component has been created
	ComponentDestroy, //!< Received when a component has been destroyed
	ComponentEdit, //!< Received when a component variable has been modified
	ComponentSelect, //!< Received when a component has been selected in the editor
	ComponentDeselect, //!< Received when a component has been deselected in the editor
};

//------------------------------------------------------------------------------
// ae::EditorEvent
//------------------------------------------------------------------------------
struct EditorEvent
{
	ae::EditorEventType type = ae::EditorEventType::None;
	const char* path = "";
	ae::Entity entity = kNullEntity;
	ae::Matrix4 transform = ae::Matrix4::Identity();
	const ae::Component* component = nullptr;
	const ae::ClassVar* var = nullptr;
};

//------------------------------------------------------------------------------
// ae::EditorPlugin class
//------------------------------------------------------------------------------
struct EditorPluginConfig
{
	EditorPluginConfig( ae::Tag tag ) {}
	ae::Str64 name;
};

//------------------------------------------------------------------------------
// ae::EditorPlugin class
//------------------------------------------------------------------------------
class EditorPlugin
{
public:
	EditorPlugin( const ae::Tag& tag );
	virtual ~EditorPlugin() {}
	virtual EditorPluginConfig GetConfig() = 0;
	virtual void OnEvent( const ae::EditorEvent& event ) = 0; // @TODO: Should this return an enum code?

	EditorMeshInstance* CreateMesh( const EditorMesh& mesh, ae::Entity selectEntity = kNullEntity );
	EditorMeshInstance* CloneMesh( const EditorMeshInstance* mesh, ae::Entity selectEntity = kNullEntity );
	void DestroyMesh( EditorMeshInstance* mesh );

private:
	friend class EditorServer;
	friend class EditorProgram;
	friend class EditorServerObject;
	const ae::Tag m_tag;
	ae::Map< class EditorServerMesh*, int32_t > m_meshRefs;
	ae::Map< EditorMeshInstance*, bool > m_instances;
	ae::Map< ae::Entity, ae::List< EditorMeshInstance >* > m_entityInstances;
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
	// @TODO: AddAsyncPlugin()? Multi-threaded plugins should be encouraged
	template< typename T, typename ... Args > T* AddPlugin( Args&& ... args );
	//! If this returns true the editor will have successfully forked, run, and
	//! been closed gracefully. The caller should exit the program in this case.
	bool Initialize( const ae::EditorParams& params );
	void Terminate();
	void Update();
	void Launch();
	bool IsConnected() const { return m_sock.IsConnected(); }
	void QueueRead( const char* levelPath );

private:
	friend class EditorServer;
	friend class EditorPlugin;
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
	ae::Array< std::pair< EditorPluginConfig, EditorPlugin* > > m_plugins;
	ae::Map< ae::Entity, bool > m_editorEntities;
};

//------------------------------------------------------------------------------
// ae::Editor class
//------------------------------------------------------------------------------
template< typename T, typename ... Args >
T* Editor::AddPlugin( Args&& ... args )
{
	AE_STATIC_ASSERT_MSG( (std::is_base_of< ae::EditorPlugin, T >::value), "T must be derived from ae::EditorPlugin" );
	auto& plugin = m_plugins.Append( { m_tag, {} } );
	plugin.second = ae::New< T >( m_tag, std::forward< Args >( args )... );
	plugin.first = plugin.second->GetConfig();
	if( plugin.first.name.Empty() )
	{
		AE_ERROR( "EditorPluginConfig must provide a name" );
		return nullptr;
	}
	AE_INFO( "Registered plugin '#'", plugin.first.name );
	return static_cast< T* >( plugin.second );
}

} // end ae namespace
#endif
