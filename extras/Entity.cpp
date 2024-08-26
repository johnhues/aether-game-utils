//------------------------------------------------------------------------------
// Entity.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
// Created by John Hughes on 3/13/21.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Entity.h"
#include "ae/Editor.h"

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( ae, Component );

namespace ae {

//------------------------------------------------------------------------------
// ae::Component member functions
//------------------------------------------------------------------------------
const char* Component::GetEntityName() const
{
	return m_reg->GetNameByEntity( m_entity );
}

Component& Component::GetComponent( const ae::Type* type )
{
	AE_ASSERT( m_reg );
	return m_reg->GetComponent( m_entity, type );
}

Component* Component::TryGetComponent( const ae::Type* type )
{
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent( m_entity, type );
}

const Component& Component::GetComponent( const ae::Type* type ) const
{
	AE_ASSERT( m_reg );
	return m_reg->GetComponent( m_entity, type );
}

const Component* Component::TryGetComponent( const ae::Type* type ) const
{
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent( m_entity, type );
}

//------------------------------------------------------------------------------
// Registry member functions
//------------------------------------------------------------------------------
Registry::Registry( const ae::Tag& tag ) :
	m_tag( tag ),
	m_entityNames( tag ),
	m_components( tag )
{}

void Registry::SetOnCreateFn( void* userData, void(*fn)(void*, Component*) )
{
	m_onCreateFn = fn;
	m_onCreateUserData = userData;
}

void Registry::SetOnDestroyFn( void* userData, void(*fn)(void*, Component*) )
{
	m_onDestroyFn = fn;
	m_onDestroyUserData = userData;
}

Entity Registry::CreateEntity( const char* name )
{
	AE_ASSERT_MSG( !m_destroying, "Cannot create an entity while destroying" );
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
	AE_ASSERT_MSG( !m_destroying, "Cannot create an entity while destroying" );

	if( entity > m_lastEntity )
	{
		m_lastEntity = entity;
	}
	else
	{
		m_lastEntity++;
		entity = m_lastEntity;
	}

	if ( name && name[ 0 ] )
	{
		// @TODO: Allow multiple entities to have the same name
		AE_ASSERT_MSG( !m_entityNames.TryGet( name ), "Entity with name '#' already exists", name );
		m_entityNames.Set( name, entity );
	}
	
	return entity;
}

Component* Registry::AddComponent( Entity entity, const char* typeName )
{
	return AddComponent( entity, ae::GetTypeByName( typeName ) );
}

Component* Registry::AddComponent( Entity entity, const ae::Type* type )
{
	AE_ASSERT_MSG( !m_destroying, "Cannot add a component while destroying" );
	if( !type )
	{
		return nullptr;
	}
	AE_ASSERT_MSG( type->IsType< ae::Component >(), "Type '#' does not inherit from ae::Component", type->GetName() );
	AE_ASSERT_MSG( !type->IsAbstract(), "Type '#' is abstract", type->GetName() );
	AE_ASSERT_MSG( type->IsDefaultConstructible(), "Type '#' is not default constructible", type->GetName() );
	if( TryGetComponent( entity, type ) )
	{
		return nullptr;
	}

	ae::Component* component = (ae::Component*)ae::Allocate( m_tag, type->GetSize(), type->GetAlignment() );
	AE_ASSERT( component );
	type->New( component );
	component->m_entity = entity;
	component->m_reg = this;
	
	ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() );
	if( !components )
	{
		components = &m_components.Set( type->GetId(), m_tag );
	}
	components->Set( entity, component );
	
	if( m_onCreateFn )
	{
		m_onCreateFn( m_onCreateUserData, component );
	}
	return component;
}

Component& Registry::GetComponent( Entity entity, const ae::Type* type )
{
	AE_ASSERT_MSG( type, "No type specified" );
	Component* component = TryGetComponent( entity, type );
	AE_ASSERT_MSG( component, "Entity '#' has no component '#'", entity, type->GetName() );
	return *component;
}

Component* Registry::TryGetComponent( Entity entity, const ae::Type* type )
{
	return const_cast< Component* >( const_cast< const Registry* >( this )->TryGetComponent( entity, type ) );
}

Component& Registry::GetComponent( const char* name, const ae::Type* type )
{
	AE_ASSERT_MSG( name && name[ 0 ], "No name specified" );
	AE_ASSERT_MSG( type, "No type specified" );
	Component* component = TryGetComponent( name, type );
	AE_ASSERT_MSG( component, "Entity '#' has no component '#'", name, type->GetName() );
	return *component;
}

Component* Registry::TryGetComponent( const char* name, const ae::Type* type )
{
	AE_ASSERT( name );
	if( !name[ 0 ] )
	{
		return nullptr;
	}
	return TryGetComponent( m_entityNames.Get( name, kInvalidEntity ), type );
}

const Component& Registry::GetComponent( Entity entity, const ae::Type* type ) const
{
	AE_ASSERT_MSG( entity != kInvalidEntity, "Invalid entity" );
	AE_ASSERT_MSG( type, "No type specified" );
	const Component* component = TryGetComponent( entity, type );
	AE_ASSERT_MSG( component, "Entity '#' has no component '#'", entity, type->GetName() );
	return *component;
}

const Component* Registry::TryGetComponent( Entity entity, const ae::Type* type ) const
{
	if ( entity == kInvalidEntity || !type )
	{
		return nullptr;
	}
	
	if ( const ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() ) )
	{
		return components->Get( entity, nullptr );
	}
	
	return nullptr;
}

const Component& Registry::GetComponent( const char* name, const ae::Type* type ) const
{
	AE_ASSERT_MSG( name && name[ 0 ], "No name specified" );
	const Component* component = TryGetComponent( name, type );
	AE_ASSERT_MSG( component, "Entity '#' has no component '#'", name );
	return *component;
}

const Component* Registry::TryGetComponent( const char* name, const ae::Type* type ) const
{
	AE_ASSERT( name );
	if( !name[ 0 ] )
	{
		return nullptr;
	}
	return TryGetComponent( m_entityNames.Get( name, kInvalidEntity ), type );
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

	AE_ASSERT_MSG( !m_destroying, "Cannot set an entities name while destroying" );
	
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

uint32_t Registry::GetComponentCountByIndex( int32_t typeIndex ) const
{
	if ( typeIndex < 0 || typeIndex >= m_components.Length() )
	{
		return 0;
	}
	return m_components.GetValue( typeIndex ).Length();
}

const Component& Registry::GetComponentByIndex( int32_t typeIndex, uint32_t componentIndex ) const
{
	AE_ASSERT( typeIndex >= 0 && typeIndex < m_components.Length() );
	return *m_components.GetValue( typeIndex ).GetValue( componentIndex );
}

Component& Registry::GetComponentByIndex( int32_t typeIndex, uint32_t componentIndex )
{
	AE_ASSERT( typeIndex >= 0 && typeIndex < m_components.Length() );
	return *m_components.GetValue( typeIndex ).GetValue( componentIndex );
}

void Registry::Destroy( Entity entity )
{
	AE_ASSERT_MSG( !m_destroying, "Recursive destruction of objects or components is not supported" );
	m_destroying = true;

	// Get components each loop because m_components could grow at any iteration
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		Component* c;
		ae::Map< Entity, Component* >* components = &m_components.GetValue( i );
		if ( components->Remove( entity, &c ) )
		{
			if( m_onDestroyFn )
			{
				m_onDestroyFn( m_onDestroyUserData, c );
			}
			c->~Component();
			ae::Free( c );
		}
	}
	const char* name = GetNameByEntity( entity );
	if ( name && name[ 0 ] )
	{
		m_entityNames.Remove( name );
	}

	m_destroying = false;
}

void Registry::DestroyComponent( Component* component )
{
	if( component )
	{
		AE_ASSERT_MSG( !m_destroying, "Recursive destruction of objects or components is not supported" );
		m_destroying = true;

		const ae::TypeId typeId = ae::GetObjectTypeId( component );
		const ae::Entity entity = component->GetEntity();
		const bool removeSuccess = m_components.Get( typeId ).Remove( entity );
		AE_ASSERT( removeSuccess );

		component->~Component();
		ae::Free( component );

		m_destroying = false;
	}
}

void Registry::Clear()
{
	AE_ASSERT_MSG( !m_destroying, "Recursive destruction of objects or components is not supported" );
	m_destroying = true;

	// Get components each loop because m_components could grow at any iteration
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		for ( uint32_t j = 0; j < m_components.GetValue( i ).Length(); j++ )
		{
			Component* c = m_components.GetValue( i ).GetValue( j );
			c->~Component();
			ae::Free( c );
		}
	}
	m_components.Clear();
	m_lastEntity = kInvalidEntity;
	m_entityNames.Clear();

	m_destroying = false;
}

} // End ae namespace
