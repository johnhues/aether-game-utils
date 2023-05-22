//------------------------------------------------------------------------------
// Resource.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2023 John Hughes
// Created by John Hughes on 5/16/23.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/Resource.h"

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
// @TODO: Support registering classes in namespaces
//AE_REGISTER_CLASS( ae::Resource );
int force_link_aeResource = 0;
template <> const char* ae::_TypeName< ae::Resource >::Get() { return "ae::Resource"; }
template <> void ae::_DefineType< ae::Resource >( ae::Type *type, uint32_t index ) { type->Init< ae::Resource >( "ae::Resource", index ); }
static ae::_TypeCreator< ae::Resource > ae_type_creator_aeResource( "ae::Resource" );

//------------------------------------------------------------------------------
// Resource member functions
//------------------------------------------------------------------------------
ae::ResourceManager::ResourceManager( const ae::Tag& tag ) :
	m_tag( tag ),
	m_resources( tag )
{}

ae::ResourceManager::~ResourceManager()
{
	Terminate();
}

void ae::ResourceManager::Initialize( ae::FileSystem* fileSystem )
{
	m_fs = fileSystem;
}

void ae::ResourceManager::Terminate()
{
	if ( m_fs )
	{
		for ( const auto& resource : m_resources )
		{
			const ae::File* file = resource.value->m_file;
			resource.value->~Resource();
			ae::Free( resource.value );
			m_fs->Destroy( file );
		}
		m_resources.Clear();
		m_fs = nullptr;
	}
	else
	{
		AE_ASSERT( !m_resources.Length() );
	}
}

bool ae::ResourceManager::Add( const char* type, const char* name, ae::FileSystem::Root rootDir, const char* filePath )
{
	if ( ae::Resource* resource = m_Add( type, name ) )
	{
		resource->m_file = m_fs->Read( rootDir, filePath, 1.0f );
		AE_ASSERT( resource->m_file );
		return true;
	}
	return false;
}

bool ae::ResourceManager::Add( const char* type, const char* name, const char* filePath )
{
	if ( ae::Resource* resource = m_Add( type, name ) )
	{
		resource->m_file = m_fs->Read( filePath, 1.0f );
		AE_ASSERT( resource->m_file );
		return true;
	}
	return false;
}

ae::Resource* ae::ResourceManager::m_Add( const char* type, const char* name )
{
	ae::Str64 key( name );
	if ( m_resources.Get( key, nullptr ) )
	{
		AE_WARN( "Resource '#' already exists", name );
		return nullptr;
	}
	const ae::Type* resourceType = ae::GetTypeByName( type );
	if ( !resourceType )
	{
		return nullptr;
	}
	ae::Resource* resource = (ae::Resource*)ae::Allocate( m_tag, resourceType->GetSize(), resourceType->GetAlignment() );
	resourceType->New( resource );
	m_resources.Set( key, resource );
	return resource;
}


bool ae::ResourceManager::Load()
{
	bool allLoaded = true;
	for ( const auto& resource : m_resources )
	{
		const ae::File* file = resource.value->GetFile();
		if ( !resource.value->IsLoaded() && ( !file || file->GetStatus() == ae::File::Status::Success ) )
		{
			resource.value->m_isLoaded = resource.value->Load();
		}

		if ( !resource.value->IsLoaded() )
		{
			allLoaded = false;
		}
	}
	return allLoaded;
}
