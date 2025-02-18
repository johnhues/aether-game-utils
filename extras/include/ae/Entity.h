//------------------------------------------------------------------------------
// Entity.h
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
#ifndef ENTITY_H
#define ENTITY_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::Entity
//------------------------------------------------------------------------------
typedef uint32_t Entity;
const Entity kInvalidEntity = 0;

//------------------------------------------------------------------------------
// ae::Component
//------------------------------------------------------------------------------
class Component : public ae::Inheritor< ae::Object, Component >
{
public:
	Entity GetEntity() const { return m_entity; }
	const char* GetEntityName() const;
	
	Component& GetComponent( const ae::ClassType* type );
	template < typename T > T& GetComponent();
	const Component& GetComponent( const ae::ClassType* type ) const;
	template < typename T > const T& GetComponent() const;
	
	Component* TryGetComponent( const ae::ClassType* type );
	template < typename T > T* TryGetComponent();
	const Component* TryGetComponent( const ae::ClassType* type ) const;
	template < typename T > const T* TryGetComponent() const;
	
private:
	friend class Registry;
	class Registry* m_reg = nullptr;
	Entity m_entity = kInvalidEntity;
};

//------------------------------------------------------------------------------
// ae::Registry
//------------------------------------------------------------------------------
class Registry
{
public:
	Registry( const ae::Tag& tag );
	~Registry();
	void SetOnCreateFn( void* userData, void(*fn)(void*, Component*) );
	void SetOnDestroyFn( void* userData, void(*fn)(void*, Component*) );
	
	// Creation
	Entity CreateEntity( const char* name = "" );
	Entity CreateEntity( Entity entity, const char* name = "" );
	//! Returns nullptr if the entity/component pair already exists or if the type is not default constructible
	Component* AddComponent( Entity entity, const char* typeName );
	//! Returns nullptr if the entity/component pair already exists or if the type is not default constructible
	Component* AddComponent( Entity entity, const ae::ClassType* type );
	//! Returns nullptr if the entity/component pair already exists or if the type is not default constructible
	template < typename T > T* AddComponent( Entity entity );
	
	// Get component
	Component& GetComponent( Entity entity, const ae::ClassType* type );
	Component* TryGetComponent( Entity entity, const ae::ClassType* type );
	Component& GetComponent( const char* name, const ae::ClassType* type );
	Component* TryGetComponent( const char* name, const ae::ClassType* type );
	template < typename T > T& GetComponent( Entity entity );
	template < typename T > T* TryGetComponent( Entity entity );
	template < typename T > T& GetComponent( const char* entityName );
	template < typename T > T* TryGetComponent( const char* entityName );
	
	// Const get component
	const Component& GetComponent( Entity entity, const ae::ClassType* type ) const;
	const Component* TryGetComponent( Entity entity, const ae::ClassType* type ) const;
	const Component& GetComponent( const char* name, const ae::ClassType* type ) const;
	const Component* TryGetComponent( const char* name, const ae::ClassType* type ) const;
	template < typename T > const T& GetComponent( Entity entity ) const;
	template < typename T > const T* TryGetComponent( Entity entity ) const;
	template < typename T > const T& GetComponent( const char* entityName ) const;
	template < typename T > const T* TryGetComponent( const char* entityName ) const;
	
	// Templated iteration over component type
	template < typename T > uint32_t GetComponentCount() const;
	template < typename T > Entity GetEntityByIndex( uint32_t index );
	template < typename T > T& GetComponentByIndex( uint32_t index );
	template < typename T > const T& GetComponentByIndex( uint32_t index ) const;
	
	// Entity names
	Entity GetEntityByName( const char* name ) const;
	const char* GetNameByEntity( Entity entity ) const;
	void SetEntityName( Entity entity, const char* name );
	
	// Iterate over all entities/components
	uint32_t GetTypeCount() const;
	const ae::ClassType* GetTypeByIndex( uint32_t typeIndex ) const;
	int32_t GetTypeIndexByType( const ae::ClassType* type ) const;
	template < typename T > int32_t GetTypeIndexByType() const;
	uint32_t GetComponentCountByIndex( int32_t typeIndex ) const;
	const Component& GetComponentByIndex( int32_t typeIndex, uint32_t componentIndex ) const;
	Component& GetComponentByIndex( int32_t typeIndex, uint32_t componentIndex );
	template < typename T, typename Fn > uint32_t CallFn( Fn fn );
	template < typename T, typename Fn > uint32_t CallFn( Fn fn ) const;
	template < typename T, typename Fn > uint32_t CallFn( Entity entity, Fn fn );
	template < typename T, typename Fn > uint32_t CallFn( Entity entity, Fn fn ) const;

	// Removal
	void Destroy( Entity entity );
	void DestroyComponent( Component* component );
	void Clear();

private:
	const ae::Tag m_tag;
	Entity m_lastEntity = kInvalidEntity;
	ae::Map< ae::Str16, Entity > m_entityNames;
	ae::Map< ae::TypeId, ae::Map< Entity, Component* > > m_components;
	void(*m_onCreateFn)(void*, Component*) = nullptr;
	void* m_onCreateUserData = nullptr;
	void(*m_onDestroyFn)(void*, Component*) = nullptr;
	void* m_onDestroyUserData = nullptr;
	bool m_destroying = false;
};

//------------------------------------------------------------------------------
// ae::Component member functions
//------------------------------------------------------------------------------
template < typename T >
T& Component::GetComponent()
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	AE_ASSERT( m_reg );
	T* t = m_reg->TryGetComponent< T >( m_entity );
	if ( !t )
	{
		AE_FAIL_MSG( "Component '#' has no sibling '#'", ae::GetClassTypeFromObject( this )->GetName(), ae::GetClassType< T >()->GetName() );
	}
	return *t;
}

template < typename T >
T* Component::TryGetComponent()
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent< T >( m_entity );
}

template < typename T >
const T& Component::GetComponent() const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	AE_ASSERT( m_reg );
	T* t = m_reg->TryGetComponent< T >( m_entity );
	if ( !t )
	{
		AE_FAIL_MSG( "Component '#' has no sibling '#'", ae::GetClassTypeFromObject( this )->GetName(), ae::GetClassType< T >()->GetName() );
	}
	return *t;
}

template < typename T >
const T* Component::TryGetComponent() const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	AE_ASSERT( m_reg );
	return m_reg->TryGetComponent< T >( m_entity );
}

//------------------------------------------------------------------------------
// ae::Registry member functions
//------------------------------------------------------------------------------
template< typename T >
T* Registry::AddComponent( Entity entity )
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const ae::ClassType* type = ae::GetClassType< T >();
	AE_ASSERT( type );
	return (T*)AddComponent( entity, type );
}

template< typename T >
T& Registry::GetComponent( Entity entity )
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	T* t = TryGetComponent< T >( entity );
	AE_ASSERT( t );
	return *t;
}

template < typename T >
T* Registry::TryGetComponent( Entity entity )
{
	return const_cast< T* >( const_cast< const Registry* >( this )->TryGetComponent< T >( entity ) );
}

template< typename T >
T& Registry::GetComponent( const char* entityName )
{
	return const_cast< T& >( const_cast< const Registry* >( this )->GetComponent< T >( entityName ) );
}

template< typename T >
T* Registry::TryGetComponent( const char* entityName )
{
	if( !entityName[ 0 ] )
	{
		return nullptr;
	}
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	return TryGetComponent< T >( m_entityNames.Get( entityName, kInvalidEntity ) );
}

template< typename T >
const T& Registry::GetComponent( Entity entity ) const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const T* t = TryGetComponent< T >( entity );
	AE_ASSERT( t );
	return *t;
}

template< typename T >
const T* Registry::TryGetComponent( Entity entity ) const
{
	if( !entity )
	{
		return nullptr;
	}
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const ae::ClassType* type = ae::GetClassType< T >();
	AE_ASSERT_MSG( type, "No registered type" );
	if ( const ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() ) )
	{
		return static_cast< T* >( components->Get( entity, nullptr ) );
	}
	return nullptr;
}

template< typename T >
const T& Registry::GetComponent( const char* entityName ) const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	AE_ASSERT( entityName && entityName[ 0 ] );
	const T* t = TryGetComponent< T >( entityName );
	if ( !t )
	{
		AE_ASSERT_MSG( GetEntityByName( entityName ) != kInvalidEntity, "No entity named '#'", entityName );
		AE_ASSERT_MSG( t, "No component '#' attached to entity '#'", ae::GetClassType< T >()->GetName(), entityName );
	}
	return *t;
}

template< typename T >
const T* Registry::TryGetComponent( const char* entityName ) const
{
	if( !entityName[ 0 ] )
	{
		return nullptr;
	}
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	return TryGetComponent< T >( m_entityNames.Get( entityName, kInvalidEntity ) );
}

template < typename T >
uint32_t Registry::GetComponentCount() const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const ae::ClassType* type = ae::GetClassType< T >();
	AE_ASSERT_MSG( type, "No registered type" );
	const ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() );
	return components ? components->Length() : 0;
}

template < typename T >
Entity Registry::GetEntityByIndex( uint32_t index )
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const ae::ClassType* type = ae::GetClassType< T >();
	AE_ASSERT_MSG( type, "No registered type" );
	const ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() );
	AE_ASSERT_MSG( "No components of type '#'", type->GetName() );
	return components->GetKey( index );
}

template < typename T >
T& Registry::GetComponentByIndex( uint32_t index )
{
	return const_cast< T& >( const_cast< const Registry* >( this )->GetComponentByIndex< T >( index ) );
}

template < typename T >
const T& Registry::GetComponentByIndex( uint32_t index ) const
{
	AE_STATIC_ASSERT( (std::is_base_of< Component, T >::value) );
	const ae::ClassType* type = ae::GetClassType< T >();
	AE_ASSERT_MSG( type, "No registered type" );
	const ae::Map< Entity, Component* >* components = m_components.TryGet( type->GetId() );
	AE_ASSERT_MSG( "No components of type '#'", type->GetName() );
	return *(const T*)components->GetValue( index );
}

template < typename T >
int32_t Registry::GetTypeIndexByType() const
{
	return GetTypeIndexByType( ae::GetClassType< T >() );
}

template < typename T, typename Fn >
uint32_t Registry::CallFn( Fn fn )
{
	uint32_t result = 0;
	const ae::ClassType* type = ae::GetClassType< T >();
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		const ae::ClassType* componentType = ae::GetClassTypeById( m_components.GetKey( i ) );
		if ( componentType->IsType( type ) )
		{
			// Get components each loop because m_components could grow at any iteration
			for ( uint32_t j = 0; j < m_components.GetValue( i ).Length(); j++ )
			{
				fn( (T*)m_components.GetValue( i ).GetValue( j ) );
				result++;
			}
		}
	}
	return result;
}

template < typename T, typename Fn >
uint32_t Registry::CallFn( Fn fn ) const
{
	uint32_t result = 0;
	const ae::ClassType* type = ae::GetClassType< T >();
	for ( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		const ae::ClassType* componentType = ae::GetClassTypeById( m_components.GetKey( i ) );
		if ( componentType->IsType( type ) )
		{
			// Get components each loop because m_components could grow at any iteration
			for ( uint32_t j = 0; j < m_components.GetValue( i ).Length(); j++ )
			{
				fn( (const T*)m_components.GetValue( i ).GetValue( j ) );
				result++;
			}
		}
	}
	return result;
}

template < typename T, typename Fn >
uint32_t Registry::CallFn( Entity entity, Fn fn )
{
	uint32_t result = 0;
	for( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		const ae::Map< Entity, Component* >* components = &m_components.GetValue( i );
		if( Component* _c = components->Get( entity, nullptr ) )
		{
			if( T* c = ae::Cast< T >( _c ) )
			{
				fn( c );
				result++;
			}
		}
	}
	return result;
}

template < typename T, typename Fn >
uint32_t Registry::CallFn( Entity entity, Fn fn ) const
{
	uint32_t result = 0;
	for( uint32_t i = 0; i < m_components.Length(); i++ )
	{
		const ae::Map< Entity, Component* >* components = &m_components.GetValue( i );
		if( Component* _c = components->Get( entity, nullptr ) )
		{
			if( const T* c = ae::Cast< T >( _c ) )
			{
				fn( c );
				result++;
			}
		}
	}
	return result;
}

} // End ae namespace

#endif
