//------------------------------------------------------------------------------
// Entity.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
// Created by John Hughes on 3/13/21.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Entity.h"
#include "ae/Editor.h"

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
// @TODO: Support registering classes in namespaces
//AE_REGISTER_CLASS( ae::Component );
int force_link_aeComponent = 0;
template <> const char* ae::_TypeName< ae::Component >::Get() { return "ae::Component"; }
template <> void ae::_DefineType< ae::Component >( ae::Type *type, uint32_t index ) { type->Init< ae::Component >( "ae::Component", index ); }
static ae::_TypeCreator< ae::Component > ae_type_creator_aeComponent( "ae::Component" );

namespace ae {

//------------------------------------------------------------------------------
// ae::Component member functions
//------------------------------------------------------------------------------
const char* Component::GetEntityName() const
{
	return m_reg->GetNameByEntity( m_entity );
}

Component& Component::GetComponent( const char* typeName )
{
	AE_ASSERT( m_reg );
	return m_reg->GetComponent( m_entity, typeName );
}

Component* Component::TryGetComponent( const char* typeName )
{
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent( m_entity, typeName );
}

const Component& Component::GetComponent( const char* typeName ) const
{
	AE_ASSERT( m_reg );
	return m_reg->GetComponent( m_entity, typeName );
}

const Component* Component::TryGetComponent( const char* typeName ) const
{
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent( m_entity, typeName );
}

//------------------------------------------------------------------------------
// Registry member functions
//------------------------------------------------------------------------------
Registry::Registry( const ae::Tag& tag ) :
	m_tag( tag ),
	m_entityNames( tag ),
	m_components( tag )
{}

void Registry::SetOnCreateFn( std::function< void(Component*) > fn )
{
	m_onCreate = fn;
}

Entity Registry::CreateEntity( const char* name )
{
	m_lastEntity++;
	Entity entity = m_lastEntity;
	
	if ( name && name[ 0 ] )
	{
		AE_ASSERT_MSG( !m_entityNames.TryGet( name ), "Entity with name '#' already exists", name );
		m_entityNames.Set( name, entity );
	}
	
	return entity;
}

Entity Registry::CreateEntity( Entity entity, const char* name )
{
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		AE_ASSERT( !m_components.GetValue( i ).TryGet( entity ) );
	}

	m_lastEntity = ae::Max( m_lastEntity, entity );
	
	if ( name && name[ 0 ] )
	{
		AE_ASSERT_MSG( !m_entityNames.TryGet( name ), "Entity with name '#' already exists", name );
		m_entityNames.Set( name, entity );
	}
	
	return entity;
}

Component* Registry::AddComponent( Entity entity, const char* typeName )
{
	const ae::Type* type = ae::GetTypeByName( typeName );
	if ( !type )
	{
		return nullptr;
	}
	AE_ASSERT_MSG( type->IsType< ae::Component >(), "Type '#' does not inherit from ae::Component", type->GetName() );
	
	ae::Object* object = (ae::Object*)ae::Allocate( m_tag, type->GetSize(), type->GetAlignment() );
	type->New( object );
	
	Component* component = ae::Cast< Component >( object );
	AE_ASSERT( component );
	component->m_entity = entity;
	component->m_reg = this;
	
	ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() );
	if ( !components )
	{
		components = &m_components.Set( type->GetId(), m_tag );
	}
	components->Set( entity, component );
	
	if ( m_onCreate )
	{
		m_onCreate( component );
	}
	return component;
}

Component* Registry::TryGetComponent( const char* name, const char* typeName )
{
	return TryGetComponent( m_entityNames.Get( name, kInvalidEntity ), typeName );
}

Component* Registry::TryGetComponent( Entity entity, const char* typeName )
{
	if ( entity == kInvalidEntity || !typeName || !typeName[ 0 ] )
	{
		return nullptr;
	}
	
	const ae::Type* type = ae::GetTypeByName( typeName );
	if ( !type )
	{
		return nullptr;
	}
	
	if ( ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() ) )
	{
		return components->Get( entity, nullptr );
	}
	
	return nullptr;
}

Component& Registry::GetComponent( Entity entity, const char* typeName )
{
	Component* component = TryGetComponent( entity, typeName );
	AE_ASSERT_MSG( component, "Entity '#' has no compoent '#'", entity );
	return *component;
}

Component& Registry::GetComponent( const char* name, const char* typeName )
{
	AE_ASSERT_MSG( name && name[ 0 ], "No name specified" );
	Component* component = TryGetComponent( name, typeName );
	AE_ASSERT_MSG( component, "Entity '#' has no compoent '#'", name );
	return *component;
}

Entity Registry::GetEntityByName( const char* name ) const
{
	return m_entityNames.Get( name, kInvalidEntity );
}

const char* Registry::GetNameByEntity( Entity entity ) const
{
	for ( uint32_t i = 0; i < m_entityNames.Length(); i++ )
	{
		if ( m_entityNames.GetValue( i ) == entity )
		{
			return m_entityNames.GetKey( i ).c_str();
		}
	}
	return "";
}

void Registry::SetEntityName( Entity entity, const char* name )
{
	if ( !entity )
	{
		return;
	}
	
	for ( uint32_t i = 0; i < m_entityNames.Length(); i++ )
	{
		if ( m_entityNames.GetValue( i ) == entity )
		{
			m_entityNames.Remove( m_entityNames.GetKey( i ) );
		}
	}
	
	if ( name[ 0 ] )
	{
		m_entityNames.Set( name, entity );
	}
}

uint32_t Registry::GetTypeCount() const
{
	return m_components.Length();
}

const ae::Type* Registry::GetTypeByIndex( uint32_t index ) const
{
	return ae::GetTypeById( m_components.GetKey( index ) );
}

int32_t Registry::GetTypeIndexByType( const ae::Type* type ) const
{
	if ( !type )
	{
		return -1;
	}
	return m_components.GetIndex( type->GetId() );
}

uint32_t Registry::GetComponentCountByIndex( uint32_t typeIndex ) const
{
	return m_components.GetValue( typeIndex ).Length();
}

const Component& Registry::GetComponentByIndex( uint32_t typeIndex, uint32_t componentIndex ) const
{
	return *m_components.GetValue( typeIndex ).GetValue( componentIndex );
}

Component& Registry::GetComponentByIndex( uint32_t typeIndex, uint32_t componentIndex )
{
	return *m_components.GetValue( typeIndex ).GetValue( componentIndex );
}

void Registry::Clear()
{
	m_lastEntity = kInvalidEntity;
	m_entityNames.Clear();
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		const ae::Map< Entity, Component* >& components = m_components.GetValue( i );
		for ( uint32_t j = 0; j < components.Length(); j++ )
		{
			Component* c = components.GetValue( j );
			c->~Component();
			ae::Free( c );
		}
	}
	m_components.Clear();
}

bool Registry::Load( const ae::Editor* editor, CreateObjectFn fn )
{
	Clear();

	uint32_t objectCount = editor->objects.Length();
	// Create all components
	for ( uint32_t i = 0; i < objectCount; i++ )
	{
		const ae::EditorObject& levelObject = editor->objects.GetValue( i );
		Entity entity = CreateEntity( levelObject.id, levelObject.name.c_str() );
		if ( fn )
		{
			fn( levelObject, entity, this );
		}
		for ( uint32_t j = 0; j < levelObject.components.Length(); j++ )
		{
			const char* typeName = levelObject.components.GetKey( j ).c_str();
			AddComponent( entity, typeName );
		}
	}
	// Serialize all components (second phase to handle references)
	for ( uint32_t i = 0; i < objectCount; i++ )
	{
		const ae::EditorObject& levelObject = editor->objects.GetValue( i );
		Entity entity = levelObject.id;
		for ( uint32_t j = 0; j < levelObject.components.Length(); j++ )
		{
			const char* typeName = levelObject.components.GetKey( j ).c_str();
			const ae::Type* type = ae::GetTypeByName( typeName );
			const ae::Dict& props = levelObject.components.GetValue( j );

			Component* component = TryGetComponent( entity, typeName );
			if ( !component )
			{
				continue;
			}
			uint32_t varCount = type->GetVarCount();
			for ( uint32_t k = 0; k < varCount; k++ )
			{
				const ae::Var* var = type->GetVarByIndex( k );
				if ( var->IsArray() )
				{
					ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), "COUNT" );
					uint32_t length = props.GetInt( key.c_str(), 0 );
					length = var->SetArrayLength( component, length );

					for ( uint32_t arrIdx = 0; arrIdx < length; arrIdx++ )
					{
						key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
						if ( const char* value = props.GetString( key.c_str(), nullptr ) )
						{
							var->SetObjectValueFromString( component, value, arrIdx );
						}
					}
				}
				else if ( const char* value = props.GetString( var->GetName(), nullptr ) )
				{
					var->SetObjectValueFromString( component, value );
				}
			}
		}
	}
	return true;
}

} // End ae namespace
