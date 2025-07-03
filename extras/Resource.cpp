//------------------------------------------------------------------------------
// Resource.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "ae/Resource.h"

//------------------------------------------------------------------------------
// Registration
//------------------------------------------------------------------------------
AE_REGISTER_NAMESPACECLASS( (ae, Resource) );

//------------------------------------------------------------------------------
// Resource member functions
//------------------------------------------------------------------------------
ae::ResourceManager::ResourceManager( const ae::Tag& tag ) :
	m_tag( tag ),
	m_files( tag ),
	m_resources( tag )
{}

ae::ResourceManager::~ResourceManager()
{
	AE_DEBUG_ASSERT( !m_resources.Length() );
	Terminate();
}

void ae::ResourceManager::Initialize( ae::FileSystem* fileSystem )
{
	m_fs = fileSystem;
}

void ae::ResourceManager::Terminate()
{
	if( m_fs )
	{
		for( const auto& resource : m_resources )
		{
			const ae::Object* params = resource.value->m_params;
			resource.value->~Resource();
			ae::Free( resource.value );
			ae::Delete( params );
		}
		m_resources.Clear();
		for( const auto& file : m_files )
		{
			m_fs->Destroy( file.key );
			ae::Delete( file.value );
		}
		m_files.Clear();
		m_fs = nullptr;
	}
	AE_ASSERT( !m_resources.Length() );
}

bool ae::ResourceManager::Add( const char* typeName, ResourceId id, ae::FileSystem::Root rootDir, const char* filePath )
{
	const ae::ClassType* type = ae::GetClassTypeByName( typeName );
	if( !type )
	{
		AE_FAIL_MSG( "Unknown resource type '#'", typeName );
		return false;
	}
	if( m_Register( type, id, rootDir, filePath ) )
	{
		AE_INFO( "Queuing load '#'...", filePath );
		return true;
	}
	return false;
}

bool ae::ResourceManager::Add( const char* type, ResourceId id, const char* filePath )
{
	return Add( type, id, ae::FileSystem::Root::Data, filePath );
}

void ae::ResourceManager::Reload( const ae::Resource* _resource )
{
	ae::Resource* resource = const_cast< ae::Resource* >( _resource );
	if( resource->m_path.Empty() )
	{
		m_resourcesToLoad.Append( resource->m_node );
	}
	else
	{
		const ae::File* file = m_fs->Read( resource->m_path.c_str(), 1.0f );
		AE_ASSERT( file );
		FileInfo* fileInfo = m_files.Get( file, nullptr );
		if( !fileInfo )
		{
			fileInfo = m_files.Set( file, ae::New< FileInfo >( m_tag ) );
		}
		fileInfo->resources.Append( resource->m_node );
	}
}

bool ae::ResourceManager::Load()
{
	for( ae::Resource* r = m_resourcesToLoad.GetFirst(); r; r = r->m_node.GetNext() )
	{
		const ae::ClassType* type = ae::GetClassTypeFromObject( r );
		r->m_isLoaded = r->Load( nullptr );
		if( r->m_isLoaded )
		{
			AE_INFO( "Loaded #::#", type->GetName(), r->GetId() );
		}
		else
		{
			AE_WARN( "Failed to load #::#", type->GetName(), r->GetId() );
		}
	}
	m_resourcesToLoad.Clear();

	for( int32_t fileIdx = m_files.Length() - 1; fileIdx >= 0; fileIdx-- )
	{
		const ae::File* file = m_files.GetKey( fileIdx );
		if( file->GetStatus() == ae::File::Status::Success )
		{
			bool allLoaded = true;
			AE_INFO( "Loading '#'...", file->GetUrl() );
			FileInfo* fileInfo = m_files.GetValue( fileIdx );
			for( ae::Resource* r = fileInfo->resources.GetFirst(); r; r = r->m_node.GetNext() )
			{
				const ae::ClassType* type = ae::GetClassTypeFromObject( r );
				r->m_isLoaded = r->Load( file );
				if( r->m_isLoaded )
				{
					AE_INFO( "\t#::#", type->GetName(), r->GetId() );
				}
				else
				{
					AE_WARN( "\tFailed to load #::#", type->GetName(), r->GetId() );
					allLoaded = false;
				}
			}
			if( allLoaded )
			{
				AE_INFO( "\tSuccess", file->GetUrl() );
			}
			else
			{
				AE_WARN( "\tFailed to load '#'", file->GetUrl() );
			}
			m_fs->Destroy( file );
			m_files.RemoveIndex( fileIdx );
			ae::Delete( fileInfo );
		}
	}
	return !(bool)m_files.Length();
}

bool ae::ResourceManager::AnyPendingLoad() const
{
	for( const auto& resource : m_resources )
	{
		if( !resource.value->IsLoaded() )
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
		const ae::ClassType* type = ae::GetClassTypeFromObject( resource.value );
		type->PatchVTable( resource.value );
	}
}

ae::Resource* ae::ResourceManager::m_Register( const ae::ClassType* resourceType, ResourceId id, ae::FileSystem::Root rootDir, const char* filePath )
{
	if( m_resources.Get( id, nullptr ) )
	{
		AE_FAIL_MSG( "Resource '#' already exists", id );
		return nullptr;
	}

	ae::Resource* resource = (ae::Resource*)ae::Allocate( m_tag, resourceType->GetSize(), resourceType->GetAlignment() );
	resourceType->New( resource );
	resource->m_id = id;
	if( filePath[ 0 ] )
	{
		m_fs->GetAbsolutePath( rootDir, filePath, &resource->m_path );
	}
	m_resources.Set( id, resource );
	Reload( resource );

	return resource;
}
