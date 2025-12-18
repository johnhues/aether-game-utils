//------------------------------------------------------------------------------
// MeshEditorPlugin.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
// Created by John Hughes on 10/19/25.
//------------------------------------------------------------------------------
#ifndef MESH_EDITOR_PLUGIN_H
#define MESH_EDITOR_PLUGIN_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Editor.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::EditorMeshResourceAttribute class
//------------------------------------------------------------------------------
//! This attribute should be added to an ae::Component or to a registered
//! variable when a component needs to display a custom mesh in the editor.
//------------------------------------------------------------------------------
class EditorMeshResourceAttribute final : public ae::Inheritor< ae::Attribute, EditorMeshResourceAttribute >
{
public:
	//! When this attribute is added to a component, this string will be used to
	//! look up a mesh resource with your custom implementation of
	//! MeshEditorPlugin::TryLoad(). This value will be ignored if the attribute
	//! is attached to a class variable, and the contents of that variable will
	//! be used instead.
	ae::Str64 resourceMesh;

	//! If true, the mesh will be displayed differently in the editor. This
	//! is useful for triggers, bounding volumes, or water volumes, etc.
	bool transparent = false;
};

//------------------------------------------------------------------------------
// MeshEditorPlugin class
//------------------------------------------------------------------------------
class MeshEditorPlugin : public ae::EditorPlugin
{
public:
	MeshEditorPlugin( const ae::Tag& tag );
	~MeshEditorPlugin() override;
	ae::EditorPluginConfig GetConfig() override;
	void OnEvent( const ae::EditorEvent& event ) override;

	//! Pure virtual function to load a mesh resource. This should be
	//! implemented by the application using the plugin.
	//! \param resourceStr The resource identifier string.
	//! \return An EditorMesh. If the resource could not be loaded, return an
	//! empty optional.
	virtual ae::Optional< ae::EditorMesh > TryLoad( const char* resourceStr ) = 0;

private:
	ae::Map< std::string, ae::EditorMeshInstance* > m_resources;
	ae::Map< const ae::Component*, ae::EditorMeshInstance* > m_components;
};

} // namespace ae

#endif
