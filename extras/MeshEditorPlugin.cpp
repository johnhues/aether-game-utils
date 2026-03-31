//------------------------------------------------------------------------------
// MeshEditorPlugin.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
// Created by John Hughes on 10/19/25.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/MeshEditorPlugin.h"
#include "ae/Editor.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_MESH_PLUGIN = "MeshEditorPlugin";

AE_REGISTER_NAMESPACECLASS( (ae, EditorMeshResourceAttribute) );
using MeshAttrib = ae::EditorMeshResourceAttribute;

namespace ae {

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
std::pair< std::string, float > GetMeshResource( const ae::Object* object )
{
	const ae::ClassType* currentType = ae::GetClassTypeFromObject( object );
	while( currentType )
	{
		if( const MeshAttrib* classAttribute = currentType->attributes.TryGet< MeshAttrib >() )
		{
			return { classAttribute->resourceMesh.c_str(), classAttribute->transparent ? 0.7f : 1.0f };
		}
		const uint32_t varCount = currentType->GetVarCount( false );
		for( uint32_t i = 0; i < varCount; i++ )
		{
			const ae::ClassVar* var = currentType->GetVarByIndex( i, false );
			const MeshAttrib* varAttribute = var->attributes.TryGet< MeshAttrib >();
			const ae::BasicType* basicType = var->GetOuterVarType().AsVarType< ae::BasicType >();
			if( varAttribute && basicType )
			{
				if( basicType->GetType() == ae::BasicType::String )
				{
					const ae::ConstDataPointer varPointer( var, object );
					return { basicType->GetVarDataAsString( varPointer ).c_str(), varAttribute->transparent ? 0.7f : 1.0f };
				}
				else
				{
					AE_ERR( "Mesh resource variable '#' must be of type String", var->GetName() );
				}
			}
		}
		currentType = currentType->GetParentType();
	}
	return { "", 0.0f };
}

//------------------------------------------------------------------------------
// MeshEditorPlugin member functions
//------------------------------------------------------------------------------
MeshEditorPlugin::MeshEditorPlugin( const ae::Tag& tag ) :
	EditorPlugin( tag ),
	m_resources( tag ),
	m_components( tag )
{}

MeshEditorPlugin::~MeshEditorPlugin()
{
	AE_DEBUG_ASSERT( !m_components.Length() );
	AE_DEBUG_ASSERT( !m_resources.Length() );
}

ae::EditorPluginConfig MeshEditorPlugin::GetConfig()
{
	// Validate attributes
	const uint32_t typeCount = ae::GetClassTypeCount();
	for( uint32_t i = 0; i < typeCount; i++ )
	{
		const ae::ClassType* type = ae::GetClassTypeByIndex( i );
		if( !type->attributes.Has< ae::EditorTypeAttribute >() )
		{
			continue;
		}
		const uint32_t varCount = type->GetVarCount( true );
		for( uint32_t j = 0; j < varCount; j++ )
		{
			const ae::ClassVar* var = type->GetVarByIndex( j, true );
			const ae::BasicType* varType = var->GetOuterVarType().AsVarType< ae::BasicType >();
			if( var->attributes.Has< MeshAttrib >() &&
				( !varType || varType->GetType() != ae::BasicType::String ))
			{
				AE_ERR( "ae::EditorMeshResourceAttribute variable '#' must be of type String", var->GetName() );
			}
		}
	}

	ae::EditorPluginConfig config( TAG_MESH_PLUGIN );
	config.name = "MeshEditorPlugin";
	return config;
}

void MeshEditorPlugin::OnEvent( const ae::EditorEvent& event )
{
	ae::PushLogTag( "MeshEditorPlugin" );
	ae::RunOnDestroy popLogTag( []() { ae::PopLogTag(); } );

	switch( event.type )
	{
		case ae::EditorEventType::ComponentCreate:
		case ae::EditorEventType::ComponentEdit:
		{
			m_UpdateInstance( event.component, event.transform );
			break;
		}
		case ae::EditorEventType::ComponentDestroy:
		{
			m_DestroyInstance( event.component );
			break;
		}
		case ae::EditorEventType::LevelUnload:
		{
			m_Unload();
			break;
		}
		default:
		{
			break;
		}
	}
}

void MeshEditorPlugin::m_UpdateInstance( const ae::Component* component, const ae::Matrix4& transform )
{
	const auto[ resourceId, opacity ] = GetMeshResource( component );
	ae::EditorMeshInstance* oldInstance = m_components.Get( component, nullptr );
	ae::EditorMeshInstance* newInstance = nullptr;
	if( !resourceId.empty() )
	{
		ae::EditorMeshInstance* resource = m_resources.Get( resourceId, nullptr );
		if( !resource )
		{
			const ae::Optional< ae::EditorMesh > mesh = TryLoad( resourceId.c_str() );
			resource = mesh.TryGet() ? CreateMesh( *mesh.TryGet(), resourceId.c_str(), kNullEntity ) : nullptr;
			if( resource )
			{
				AE_DEBUG( "Create mesh resource-># id:#", resource, resourceId );
				resource->color = ae::Color::Magenta().SetA( 0.0f ); // Disable resource object rendering
				m_resources.Set( resourceId, resource );
			}
		}
		newInstance = CloneMesh( resource, component->GetEntity() );
		AE_ASSERT( !oldInstance || oldInstance != newInstance );
		if( newInstance )
		{
			newInstance->transform = transform;
			newInstance->color = ae::Color::White().SetA( opacity );
		}
	}

	if( newInstance )
	{
		m_components.Set( component, newInstance );
		AE_DEBUG( "Set component reference-># entity:# component:# resource:#", newInstance, component->GetEntity(), component, resourceId );
	}
	else if( oldInstance )
	{
		m_components.Remove( component );
		AE_DEBUG( "Clear component reference-># entity:# component:# resource:#", oldInstance, component->GetEntity(), component, resourceId );
	}

	// Always free the old instance. A new instance is always created if the
	// resource id is valid.
	DestroyMesh( oldInstance );
}

void MeshEditorPlugin::m_DestroyInstance( const ae::Component* component )
{
	if( ae::EditorMeshInstance* instance = m_components.Get( component, nullptr ) )
	{
		AE_DEBUG( "Destroy mesh instance-># entity:# component:#", instance, component->GetEntity(), component );
		DestroyMesh( instance );
		m_components.Remove( component );
	}
}
void MeshEditorPlugin::m_Unload()
{
	AE_DEBUG_ASSERT( !m_components.Length() );
	for( auto& [ _, instance ] : m_resources )
	{
		DestroyMesh( instance );
	}
	m_resources.Clear();
}

} // namespace ae
