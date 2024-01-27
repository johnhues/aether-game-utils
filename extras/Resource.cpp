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
AE_REGISTER_CLASS( ae, Resource );

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
	if( ae::Resource* resource = m_Register( type, name ) )
	{
		resource->m_file = m_fs->Read( rootDir, filePath, 1.0f );
		AE_ASSERT( resource->m_file );
		AE_INFO( "Queuing load '#'...", resource->m_file->GetUrl() );
		return true;
	}
	return false;
}

bool ae::ResourceManager::Add( const char* type, const char* name, const char* filePath )
{
	return Add( type, name, ae::FileSystem::Root::Data, filePath );
}

void ae::ResourceManager::Reload( const ae::Resource* _resource )
{
	ae::Resource* resource = const_cast< ae::Resource* >( _resource );
	resource->m_isLoaded = false;
	if( resource->m_file )
	{
		const ae::Str256 url = resource->m_file->GetUrl();
		m_fs->Destroy( resource->m_file );
		resource->m_file = m_fs->Read( url.c_str(), 1.0f );
		AE_ASSERT( resource->m_file );
	}
}

bool ae::ResourceManager::Load()
{
	bool allLoaded = true;
	for( const auto& resource : m_resources )
	{
		const ae::File* file = resource.value->GetFile();
		if( !resource.value->IsLoaded() && ( !file || file->GetStatus() == ae::File::Status::Success ) )
		{
			if( file )
			{
				AE_INFO( "Loading '#'...", file->GetUrl() );
			}
			resource.value->m_isLoaded = resource.value->Load();
			if( file )
			{
				if( resource.value->m_isLoaded )
				{
					AE_INFO( "Loaded '#'", file->GetUrl() );
				}
				else
				{
					AE_WARN( "Failed to load '#'", file->GetUrl() );
				}
			}
		}

		if ( !resource.value->IsLoaded() )
		{
			allLoaded = false;
		}
	}
	return allLoaded;
}

bool ae::ResourceManager::AnyPendingLoad() const
{
	for( const auto& resource : m_resources )
	{
		if ( !resource.value->IsLoaded() )
		{
			return true;
		}
	}
	return false;
}

uint32_t ae::ResourceManager::GetCount() const
{
	return m_resources.Length();
}

void ae::ResourceManager::HotLoad()
{
	ae::ResourceManager temp = m_tag;
	memcpy( this, &temp, sizeof(void*) );
	for( const auto& resource : m_resources )
	{
		const ae::Type* type = ae::GetTypeFromObject( resource.value );
		type->PatchVTable( resource.value );
	}
}

ae::Resource* ae::ResourceManager::m_Register( const char* type, const char* name )
{
	ae::Str64 key( name );
	if ( m_resources.Get( key, nullptr ) )
	{
		AE_FAIL_MSG( "Resource '#' already exists", name );
		return nullptr;
	}
	const ae::Type* resourceType = ae::GetTypeByName( type );
	if ( !resourceType )
	{
		AE_FAIL_MSG( "Unknown resource type '#'", type );
		return nullptr;
	}
	ae::Resource* resource = (ae::Resource*)ae::Allocate( m_tag, resourceType->GetSize(), resourceType->GetAlignment() );
	resourceType->New( resource );
	m_resources.Set( key, resource );
	return resource;
}
