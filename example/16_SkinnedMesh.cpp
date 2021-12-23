//------------------------------------------------------------------------------
// 16_SkinnedMesh.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#include "ae/aether.h"
#include "ofbx.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::Keyframe struct
//------------------------------------------------------------------------------
struct Keyframe
{
	ae::Matrix4 GetLocalTransform() const;
	Keyframe Lerp( const Keyframe& target, float t ) const;
	
	ae::Vec3 position = ae::Vec3( 0.0f );
	ae::Quaternion rotation = ae::Quaternion::Identity();
	ae::Vec3 scale = ae::Vec3( 1.0f );
};

//------------------------------------------------------------------------------
// ae::Animation class
//------------------------------------------------------------------------------
class Animation
{
public:
	Animation( const ae::Tag& tag ) : keyframes( tag ) {}
	ae::Keyframe GetKeyframeByTime( const char* boneName, float time ) const;
	ae::Keyframe GetKeyframeByPercent( const char* boneName, float percent ) const;
	void AnimateByTime( class Skeleton* target, float time, float strength, const class Bone** mask, uint32_t maskCount ) const;
	void AnimateByPercent( class Skeleton* target, float percent, float strength, const class Bone** mask, uint32_t maskCount ) const;
	
	float duration = 0.0f;
	bool loop = false;
	ae::Map< ae::Str64, ae::Array< ae::Keyframe > > keyframes; // @TODO: boneKeyframes. Maybe private
};

//------------------------------------------------------------------------------
// ae::Bone struct
//------------------------------------------------------------------------------
struct Bone
{
	ae::Str64 name;
	uint32_t index = 0;
	ae::Matrix4 transform = ae::Matrix4::Identity();
	ae::Matrix4 localTransform = ae::Matrix4::Identity();
	Bone* firstChild = nullptr;
	Bone* nextSibling = nullptr;
	Bone* parent = nullptr;
};

//------------------------------------------------------------------------------
// ae::Skeleton class
//------------------------------------------------------------------------------
class Skeleton
{
public:
	Skeleton( const ae::Tag& tag ) : m_bones( tag ) {}
	void Initialize( uint32_t maxBones );
	void Initialize( const Skeleton* otherPose );
	const Bone* AddBone( const Bone* parent, const char* name, const ae::Matrix4& localTransform );
	void SetLocalTransforms( const Bone** targets, const ae::Matrix4* localTransforms, uint32_t count );
	void SetLocalTransform( const Bone* target, const ae::Matrix4& localTransform );
	
	const Bone* GetRoot() const;
	const Bone* GetBoneByName( const char* name ) const;
	const Bone* GetBoneByIndex( uint32_t index ) const;
	const Bone* GetBones() const;
	uint32_t GetBoneCount() const;
	
private:
	ae::Array< ae::Bone > m_bones;
};

//------------------------------------------------------------------------------
// ae::Skin class
//------------------------------------------------------------------------------
class Skin
{
public:
	struct Vertex
	{
		ae::Vec3 position;
		ae::Vec3 normal;
		uint16_t bones[ 4 ];
		uint8_t weights[ 4 ] = { 0 };
	};
	
	Skin( const ae::Tag& tag ) : m_bindPose( tag ), m_verts( tag ), m_invBindPoses( tag ) {}
	void Initialize( const Skeleton& bindPose, const ae::Skin::Vertex* vertices, uint32_t vertexCount );
	
	const class Skeleton* GetBindPose() const;
	const ae::Matrix4& GetInvBindPose( const char* name ) const;
	
	void ApplyPoseToMesh( const Skeleton* pose, float* positions, float* normals, uint32_t positionStride, uint32_t normalStride, uint32_t count ) const;
	
private:
	Skeleton m_bindPose;
	ae::Map< ae::Str64, ae::Matrix4 > m_invBindPoses;
	ae::Array< Vertex > m_verts;
};

//------------------------------------------------------------------------------
// ae::Keyframe member functions
//------------------------------------------------------------------------------
ae::Matrix4 Keyframe::GetLocalTransform() const
{
	ae::Matrix4 rot = ae::Matrix4::Identity();
	rot.SetRotation( rotation );
	return ae::Matrix4::Translation( position ) * rot * ae::Matrix4::Scaling( scale );
}

Keyframe Keyframe::Lerp( const Keyframe& target, float t ) const
{
	Keyframe result;
	result.position = position.Lerp( target.position, t );
	result.rotation = rotation.Nlerp( target.rotation, t );
	result.scale = scale.Lerp( target.scale, t );
	return result;
}

//------------------------------------------------------------------------------
// ae::Animation member functions
//------------------------------------------------------------------------------
ae::Keyframe Animation::GetKeyframeByTime( const char* boneName, float time ) const
{
	return GetKeyframeByPercent( boneName, ae::Delerp( 0.0f, duration, time ) );
}

ae::Keyframe Animation::GetKeyframeByPercent( const char* boneName, float percent ) const
{
	const ae::Array< ae::Keyframe >* boneKeyframes = keyframes.TryGet( boneName );
	if ( !boneKeyframes || !boneKeyframes->Length() )
	{
		return ae::Keyframe();
	}
	percent = loop ? ae::Mod( percent, 1.0f ) : ae::Clip01( percent );
	float f = boneKeyframes->Length() * percent;
	uint32_t f0 = (uint32_t)f;
	uint32_t f1 = ( f0 + 1 );
	f0 = loop ? ( f0 % boneKeyframes->Length() ) : ae::Clip( f0, 0u, boneKeyframes->Length() - 1 );
	f1 = loop ? ( f1 % boneKeyframes->Length() ) : ae::Clip( f1, 0u, boneKeyframes->Length() - 1 );
	return (*boneKeyframes)[ f0 ].Lerp( (*boneKeyframes)[ f1 ], ae::Clip01( f - f0 ) );
}

void Animation::AnimateByTime( class Skeleton* target, float time, float strength, const ae::Bone** mask, uint32_t maskCount ) const
{
	AnimateByPercent( target, ae::Delerp( 0.0f, duration, time ), strength, mask, maskCount );
}

void Animation::AnimateByPercent( class Skeleton* target, float percent, float strength, const ae::Bone** mask, uint32_t maskCount ) const
{
	ae::Array< const ae::Bone* > tempBones = AE_ALLOC_TAG_FIXME; // @TODO: Allocate once in Animation class
	ae::Array< ae::Matrix4 > temp = AE_ALLOC_TAG_FIXME; // @TODO: Allocate once in Animation class
	tempBones.Reserve( target->GetBoneCount() );
	temp.Reserve( target->GetBoneCount() );
	
	strength = ae::Clip01( strength );
	const ae::Bone** maskEnd = mask + maskCount;
	
	for ( uint32_t i = 0; i < target->GetBoneCount(); i++ )
	{
		const ae::Bone* bone = target->GetBoneByIndex( i );
		AE_ASSERT( bone->index == i );
		AE_ASSERT( bone > bone->parent );
		
		float keyStrength = strength;
		bool found = ( std::find( mask, maskEnd, bone ) != maskEnd );
		if ( found )
		{
			keyStrength = 0.0f;
		}
		
		tempBones.Append( bone );
		ae::Keyframe keyframe = GetKeyframeByPercent( bone->name.c_str(), percent );
		if ( keyStrength < 1.0f )
		{
			const ae::Matrix4 current = bone->localTransform;
			const ae::Vec3 currTranslation = current.GetTranslation();
			const ae::Quaternion currRotation = current.GetRotation();
			const ae::Vec3 currScale = current.GetScale();
			keyframe.position = currTranslation.Lerp( keyframe.position, keyStrength );
			keyframe.rotation = currRotation.Nlerp( keyframe.rotation, keyStrength );
			keyframe.scale = currScale.Lerp( keyframe.scale, keyStrength );
		}
		temp.Append( keyframe.GetLocalTransform() );
	}
	target->SetLocalTransforms( tempBones.Begin(), temp.Begin(), target->GetBoneCount() );
}

//------------------------------------------------------------------------------
// ae::Skeleton member functions
//------------------------------------------------------------------------------
void Skeleton::Initialize( uint32_t maxBones )
{
	m_bones.Clear();
	m_bones.Reserve( maxBones );
	
	Bone* bone = &m_bones.Append( {} );
	bone->name = "root";
	bone->index = 0;
	bone->transform = ae::Matrix4::Identity();
	bone->localTransform = ae::Matrix4::Identity();
	bone->parent = nullptr;
}

void Skeleton::Initialize( const Skeleton* otherPose )
{
	Initialize( otherPose->GetBoneCount() );
	
	const void* beginCheck = m_bones.Begin();
	for ( uint32_t i = 1; i < otherPose->m_bones.Length(); i++ ) // Skip root
	{
		const ae::Bone& otherBone = otherPose->m_bones[ i ];
		const ae::Bone* parent = &m_bones[ otherBone.parent->index ];
		AddBone( parent, otherBone.name.c_str(), otherBone.localTransform );
	}
	AE_ASSERT( beginCheck == m_bones.Begin() );
}

const Bone* Skeleton::AddBone( const Bone* _parent, const char* name, const ae::Matrix4& localTransform )
{
	Bone* parent = const_cast< Bone* >( _parent );
	AE_ASSERT_MSG( m_bones.Size(), "Must call ae::Skeleton::Initialize() before calling ae::Skeleton::AddBone()" );
	AE_ASSERT_MSG( m_bones.Begin() <= parent && parent < m_bones.End(), "ae::Bones must have a parent from the same ae::Skeleton" );
	if ( !parent || m_bones.Length() == m_bones.Size() )
	{
		return nullptr;
	}
#if _AE_DEBUG_
	Bone* beginCheck = m_bones.Begin();
#endif
	Bone* bone = &m_bones.Append( {} );
#if _AE_DEBUG_
	AE_ASSERT( beginCheck == m_bones.Begin() );
#endif

	bone->name = name;
	bone->index = m_bones.Length() - 1;
	bone->transform = parent->transform * localTransform;
	bone->localTransform = localTransform;
	bone->parent = parent;
	
	Bone** children = &parent->firstChild;
	while ( *children )
	{
		children = &(*children)->nextSibling;
	}
	*children = bone;
	
	return bone;
}

void Skeleton::SetLocalTransforms( const Bone** targets, const ae::Matrix4* localTransforms, uint32_t count )
{
	if ( !count )
	{
		return;
	}
	
	for ( uint32_t i = 0; i < count; i++ )
	{
		ae::Bone* bone = const_cast< ae::Bone* >( targets[ i ] );
		AE_ASSERT_MSG( bone, "Null bone passed to skeleton when setting transforms" );
		AE_ASSERT_MSG( m_bones.Begin() <= bone && bone < m_bones.End(), "Transform target '#' is not part of this skeleton", bone->name );
		bone->localTransform = localTransforms[ i ];
	}
	
	m_bones[ 0 ].transform = m_bones[ 0 ].localTransform;
	for ( uint32_t i = 1; i < m_bones.Length(); i++ )
	{
		ae::Bone* bone = &m_bones[ i ];
		AE_ASSERT( bone->parent );
		AE_ASSERT( bone->parent < bone );
		bone->transform = bone->parent->transform * bone->localTransform;
	}
}

void Skeleton::SetLocalTransform( const Bone* target, const ae::Matrix4& localTransform )
{
	SetLocalTransforms( &target, &localTransform, 1 );
}

const Bone* Skeleton::GetRoot() const
{
	return m_bones.Begin();
}

const Bone* Skeleton::GetBoneByName( const char* name ) const
{
	int32_t idx = m_bones.FindFn( [ name ]( const Bone& b ){ return b.name == name; } );
	return ( idx >= 0 ) ? &m_bones[ idx ] : nullptr;
}

const Bone* Skeleton::GetBoneByIndex( uint32_t index ) const
{
#if _AE_DEBUG_
	AE_ASSERT( m_bones[ index ].index == index );
#endif
	return &m_bones[ index ];
}

const Bone* Skeleton::GetBones() const
{
	return m_bones.Begin();
}

uint32_t Skeleton::GetBoneCount() const
{
	return m_bones.Length();
}

//------------------------------------------------------------------------------
// ae::Skin member functions
//------------------------------------------------------------------------------
void Skin::Initialize( const Skeleton& bindPose, const ae::Skin::Vertex* vertices, uint32_t vertexCount )
{
	AE_ASSERT( bindPose.GetBoneCount() );
	m_bindPose.Initialize( &bindPose );
	
	m_invBindPoses.Clear();
	m_invBindPoses.Reserve( m_bindPose.GetBoneCount() );
	for ( uint32_t i = 0; i < m_bindPose.GetBoneCount(); i++ )
	{
		const ae::Bone* bone = m_bindPose.GetBoneByIndex( i );
		m_invBindPoses.Set( bone->name, bone->transform.GetInverse() );
	}
	
	m_verts.Clear();
	m_verts.Append( vertices, vertexCount );
}

const Skeleton* Skin::GetBindPose() const
{
	return &m_bindPose;
}

const ae::Matrix4& Skin::GetInvBindPose( const char* name ) const
{
	return m_invBindPoses.Get( name, ae::Matrix4::Identity() );
}

void Skin::ApplyPoseToMesh( const Skeleton* pose, float* positions, float* normals, uint32_t positionStride, uint32_t normalStride, uint32_t count ) const
{
	AE_ASSERT_MSG( count == m_verts.Length(), "Given mesh data does not match skin vertex count" );
	AE_ASSERT_MSG( m_bindPose.GetBoneCount() == pose->GetBoneCount(), "Given ae::Skeleton pose does not match bind pose hierarchy" );
	for ( uint32_t i = 0; i < count; i++ )
	{
		ae::Vec3 pos( 0.0f );
		const ae::Skin::Vertex& skinVert = m_verts[ i ];
		for ( uint32_t j = 0; j < 4; j++ )
		{
			const ae::Bone* bone = pose->GetBoneByIndex( skinVert.bones[ j ] );
			const ae::Bone* bindPoseBone = m_bindPose.GetBoneByIndex( skinVert.bones[ j ] );
			if ( bone->parent ) { AE_ASSERT_MSG( bone->parent->index == bindPoseBone->parent->index, "Given ae::Skeleton pose does not match bind pose hierarchy" ); }
			else { AE_ASSERT_MSG( !bindPoseBone->parent, "Given ae::Skeleton pose does not match bind pose hierarchy" ); }
			
			ae::Matrix4 transform = bone->transform * m_invBindPoses.GetValue( skinVert.bones[ j ] );
			float weight = skinVert.weights[ j ] / 256.0f;
			pos += ( transform * ae::Vec4( skinVert.position, 1.0f ) ).GetXYZ() * weight;
		}
		
		float* p = (float*)( (uint8_t*)positions + ( i * positionStride ) );
		float* n = (float*)( (uint8_t*)normals + ( i * normalStride ) );
		p[ 0 ] = pos.x;
		p[ 1 ] = pos.y;
		p[ 2 ] = pos.z;
		n[ 0 ] = skinVert.normal.x;
		n[ 1 ] = skinVert.normal.y;
		n[ 2 ] = skinVert.normal.z;
	}
}

} // ae namespace end

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";

struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 normal;
	ae::Vec4 color;
	ae::Vec2 uv;
};

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static ae::Matrix4 ofbxToAe( const ofbx::Matrix& m )
{
	ae::Matrix4 result;
	for ( uint32_t i = 0; i < 16; i++ )
	{
		result.data[ i ] = m.m[ i ];
	}
	return result;
}

static ae::Matrix4 getBindPoseMatrix( const ofbx::Skin* skin, const ofbx::Object* node )
{
	if (!skin) return ofbxToAe( node->getGlobalTransform() );

	for (int i = 0, c = skin->getClusterCount(); i < c; ++i)
	{
		const ofbx::Cluster* cluster = skin->getCluster(i);
		if (cluster->getLink() == node)
		{
			return ofbxToAe( cluster->getTransformLinkMatrix() );
		}
	}
	return ofbxToAe( node->getGlobalTransform() );
}

bool LoadFBX( ae::FileSystem* fileSystem, const char* fileName, ae::VertexData* vertexData, ae::Skin* skinOut, ae::Animation* animOut )
{
	uint32_t fileSize = fileSystem->GetSize( fileName );
	if ( !fileSize )
	{
		return false;
	}
	ae::Scratch< uint8_t > fileData( TAG_ALL, fileSize );
	if ( fileSize != fileSystem->Read( fileName, fileData.Data(), fileSize ) )
	{
		return false;
	}
	ofbx::IScene* scene = ofbx::load( (ofbx::u8*)fileData.Data(), fileSize, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE );
	if ( !scene )
	{
		return false;
	}
	
	// Scene globals
	const ofbx::GlobalSettings* ofbxSettings = scene->getGlobalSettings();
	const float frameRate = scene->getSceneFrameRate();
	const float startTime = ofbxSettings->TimeSpanStart;
	const float endTime = ofbxSettings->TimeSpanStop;
	
	// Find skin
	const ofbx::Skin* ofbxSkin = nullptr;
	for ( uint32_t i = 0; i < scene->getAllObjectCount(); i++ )
	{
		const ofbx::Object* ofbxObj = scene->getAllObjects()[ i ];
		if ( ofbxObj->getType() == ofbx::Object::Type::SKIN )
		{
			ofbxSkin = dynamic_cast< const ofbx::Skin* >( ofbxObj );
			AE_ASSERT( ofbxSkin );
			break;
		}
	}
	AE_ASSERT( ofbxSkin );
	
	// Find the root node of the skinned meshes skeleton (the parent of root bone)
	const ofbx::Object* ofbxSkeletonRoot = nullptr;
	{
		AE_ASSERT( ofbxSkin->getClusterCount() );
		const ofbx::Cluster* cluster = ofbxSkin->getCluster( 1 );
		ofbxSkeletonRoot = cluster->getLink();
		AE_ASSERT( ofbxSkeletonRoot );
		AE_ASSERT( ofbxSkeletonRoot->getType() == ofbx::Object::Type::LIMB_NODE );
		while ( true )
		{
			const ofbx::Object* ofbxParent = ofbxSkeletonRoot->getParent();
			if ( ofbxParent && ofbxParent->getType() == ofbx::Object::Type::LIMB_NODE )
			{
				ofbxSkeletonRoot = ofbxParent;
				continue;
			}
			break;
		}
		AE_ASSERT( ofbxSkeletonRoot );
		ofbxSkeletonRoot = ofbxSkeletonRoot->getParent(); // Not an actual bone, scene root or something
		AE_ASSERT( ofbxSkeletonRoot );
	}
	
	// Skeleton
	ae::Skeleton bindPose = TAG_ALL;
	bindPose.Initialize( 32 ); // @TODO: Count ofbxSkeletonRoot bones
	ae::Array< const ofbx::Object* > sceneBones = TAG_ALL;
	sceneBones.Append( ofbxSkeletonRoot );
	std::function< void( const ofbx::Object*, const ae::Bone* ) > skeletonBuilderFn = [ & ]( const ofbx::Object* ofbxParent, const ae::Bone* parent )
	{
		int32_t i = 0;
		ae::Matrix4 parentInverseWorldTransform = getBindPoseMatrix( ofbxSkin, ofbxParent ).GetInverse();
		while ( const ofbx::Object* ofbxBone = ofbxParent->resolveObjectLink( i ) )
		{
			if ( ofbxBone->getType() == ofbx::Object::Type::LIMB_NODE )
			{
				ae::Matrix4 worldTransform = getBindPoseMatrix( ofbxSkin, ofbxBone );
				ae::Matrix4 transform = parentInverseWorldTransform * worldTransform;
				const ae::Bone* bone = bindPose.AddBone( parent, ofbxBone->name, transform );
				sceneBones.Append( ofbxBone );
				skeletonBuilderFn( ofbxBone, bone );
			}
			i++;
		}
	};
	skeletonBuilderFn( sceneBones[ 0 ], bindPose.GetRoot() );
	
	// Mesh
	// @TODO: Get mesh from ofbxSkin
	uint32_t meshCount = scene->getMeshCount();
	AE_ASSERT( meshCount == 1 );

	uint32_t totalVerts = 0;
	uint32_t totalIndices = 0;
	for ( uint32_t i = 0; i < meshCount; i++ )
	{
		const ofbx::Mesh* mesh = scene->getMesh( i );
		const ofbx::Geometry* geo = mesh->getGeometry();
		totalVerts += geo->getVertexCount();
		totalIndices += geo->getIndexCount();
	}
	
	ae::Array< ae::Skin::Vertex > skinVerts = TAG_ALL;
	skinVerts.Reserve( totalVerts );
	for ( uint32_t i = 0; i < totalVerts; i++ )
	{
		skinVerts.Append( {} );
	}

	uint32_t indexOffset = 0;
	ae::Array< Vertex > vertices( TAG_ALL, totalVerts );
	ae::Array< uint32_t > indices( TAG_ALL, totalIndices );
	for ( uint32_t i = 0; i < meshCount; i++ )
	{
		const ofbx::Mesh* mesh = scene->getMesh( i );
		const ofbx::Geometry* geo = mesh->getGeometry();
		ae::Matrix4 localToWorld = ofbxToAe( mesh->getGlobalTransform() );
		ae::Matrix4 normalMatrix = localToWorld.GetNormalMatrix();

		uint32_t vertexCount = geo->getVertexCount();
		const ofbx::Vec3* meshVerts = geo->getVertices();
		const ofbx::Vec3* meshNormals = geo->getNormals();
		const ofbx::Vec4* meshColors = geo->getColors();
		const ofbx::Vec2* meshUvs = geo->getUVs();
		for ( uint32_t j = 0; j < vertexCount; j++ )
		{
			ofbx::Vec3 p0 = meshVerts[ j ];
			ae::Vec4 p( p0.x, p0.y, p0.z, 1.0f );
			ae::Color color = meshColors ? ae::Color::SRGBA( (float)meshColors[ j ].x, (float)meshColors[ j ].y, (float)meshColors[ j ].z, (float)meshColors[ j ].w ) : ae::Color::White();
			ae::Vec2 uv = meshUvs ? ae::Vec2( meshUvs[ j ].x, meshUvs[ j ].y ) : ae::Vec2( 0.0f );

			Vertex v;
			v.pos = p;
			v.pos = localToWorld * v.pos;
			v.normal = ae::Vec4( 0.0f );
			v.color = color.GetLinearRGBA();
			v.uv = uv;
			vertices.Append( v );
			
			skinVerts[ j ].position = v.pos.GetXYZ();
		}

		uint32_t indexCount = geo->getIndexCount();
		const int32_t* meshIndices = geo->getFaceIndices();
		for ( uint32_t j = 0; j < indexCount; j++ )
		{
			int32_t index = ( meshIndices[ j ] < 0 ) ? ( -meshIndices[ j ] - 1 ) : meshIndices[ j ];
			AE_ASSERT( index < vertexCount );
			index += indexOffset;
			indices.Append( index );

			ofbx::Vec3 n = meshNormals[ j ];
			Vertex& v = vertices[ index ];
			v.normal.x = n.x;
			v.normal.y = n.y;
			v.normal.z = n.z;
			v.normal.w = 0.0f;
			v.normal = normalMatrix * v.normal;
			v.normal.SafeNormalize();
			
			skinVerts[ index ].normal = v.normal.GetXYZ();
		}

		indexOffset += vertexCount;
	}
	
	// Skin
	for (int i = 0, c = ofbxSkin->getClusterCount(); i < c; ++i)
	{
		const ofbx::Cluster* cluster = ofbxSkin->getCluster(i);
		uint32_t indexCount = cluster->getIndicesCount();
		if ( !indexCount )
		{
			continue;
		}
		
		const ofbx::Object* ofbxBone = cluster->getLink();
		int32_t boneIndex = sceneBones.Find( ofbxBone );
#if _AE_DEBUG_
		const ae::Bone* bone = bindPose.GetBoneByIndex( boneIndex );
		AE_ASSERT( bone );
		AE_ASSERT( bone->name == ofbxBone->name );
#endif

		const int* clusterIndices = cluster->getIndices();
		const double* clusterWeights = cluster->getWeights();
		for ( uint32_t j = 0; j < indexCount; j++ )
		{
			ae::Skin::Vertex* vertex = &skinVerts[ clusterIndices[ j ] ];
			int32_t weightIdx = 0;
			while ( vertex->weights[ weightIdx ] )
			{
				weightIdx++;
			}
			AE_ASSERT_MSG( weightIdx < countof( vertex->weights ), "Export FBX with skin weight limit set to 4" );
			vertex->bones[ weightIdx ] = boneIndex;
			vertex->weights[ weightIdx ] = ae::Clip( (int)( clusterWeights[ j ] * 256.5 ), 0, 255 );
			// Fix up weights so they total 255
			if ( weightIdx == 3 )
			{
				int32_t total = 0;
				uint8_t* greatest = nullptr;
				for ( uint32_t i = 0; i < 4; i++ )
				{
					total += vertex->weights[ i ];
					if ( !greatest || *greatest < vertex->weights[ i ] )
					{
						greatest = &vertex->weights[ i ];
					}
				}
#if _AE_DEBUG_
				AE_ASSERT_MSG( total > 128, "Skin weights missing" );
#endif
				if ( total != 255 )
				{
					*greatest += ( 255 - total );
				}
#if _AE_DEBUG_
				total = 0;
				for ( uint32_t i = 0; i < 4; i++ )
				{
					total += vertex->weights[ i ];
				}
				AE_ASSERT( total == 255 );
#endif
			}
		}
	}
	
	// Finalize skin
	skinOut->Initialize( bindPose, skinVerts.Begin(), skinVerts.Length() );
	
	// Animations
	if ( scene->getAnimationStackCount() )
	{
		const ofbx::AnimationStack* animStack = scene->getAnimationStack( 0 );
		const ofbx::AnimationLayer* animLayer = animStack->getLayer( 0 );
		if ( animLayer )
		{
			for ( uint32_t i = 1; i < bindPose.GetBoneCount(); i++ )
			{
				const ae::Bone* bone = bindPose.GetBoneByIndex( i );
				const ofbx::Object* ofbxBone = sceneBones[ i ];
				AE_ASSERT( bone->name == ofbxBone->name );

				const ofbx::AnimationCurveNode* tCurveNode = animLayer->getCurveNode( *ofbxBone, "Lcl Translation" );
				const ofbx::AnimationCurveNode* rCurveNode = animLayer->getCurveNode( *ofbxBone, "Lcl Rotation" );
				const ofbx::AnimationCurveNode* sCurveNode = animLayer->getCurveNode( *ofbxBone, "Lcl Scaling" );

				const ofbx::AnimationCurve* curves[] =
				{
					tCurveNode ? tCurveNode->getCurve( 1 ) : nullptr,
					rCurveNode ? rCurveNode->getCurve( 1 ) : nullptr,
					sCurveNode ? sCurveNode->getCurve( 1 ) : nullptr
				};

				bool hasKeyframes = false;
				for ( uint32_t j = 0; j < countof( curves ); j++ )
				{
					const ofbx::AnimationCurve* curve = curves[ j ];
					if ( !curve )
					{
						continue;
					}
					if ( curve->getKeyCount() )
					{
						hasKeyframes = true;
						break;
					}
				}

				if ( hasKeyframes )
				{
					animOut->duration = ( endTime - startTime );

					ae::Array< ae::Keyframe >& boneKeyframes = animOut->keyframes.Set( bone->name, TAG_ALL );

					// The following is weird because when you select an animation frame window in Maya it always shows an extra frame
					uint32_t sampleCount = ae::Round( animOut->duration * frameRate );
					sampleCount++; // If an animation has one keyframe at frame 0 and the last one at 48, it actually has 49 frames
					boneKeyframes.Clear();
					boneKeyframes.Reserve( sampleCount );
					for ( uint32_t j = 0; j < sampleCount; j++ )
					{
						// Subtract 1 from sample count so last frame doesn't count towards final length
						// ie. A 2 second animation playing at 2 frames per second should have 5 frames, at times: 0, 0.5, 1.0, 1.5, 2
						float t = ( sampleCount > 1 ) ? startTime + ( j / ( sampleCount - 1.0f ) ) * animOut->duration : 0.0f;
						ofbx::Vec3 posFrame = { 0.0, 0.0, 0.0 };
						ofbx::Vec3 rotFrame = { 0.0, 0.0, 0.0 };
						ofbx::Vec3 scaleFrame = { 1.0, 1.0, 1.0 };
						if ( tCurveNode ) { posFrame = tCurveNode->getNodeLocalTransform( t ); }
						if ( rCurveNode ) { rotFrame = rCurveNode->getNodeLocalTransform( t ); }
						if ( sCurveNode ) { scaleFrame = sCurveNode->getNodeLocalTransform( t ); }

						ae::Keyframe keyframe;
						ae::Matrix4 animTransform = ofbxToAe( ofbxBone->evalLocal( posFrame, rotFrame, scaleFrame ) );
						if ( j == 0 )
						{
							// @HACK: Should have this bind pose skeleton in advance
							ae::Matrix4 parentTransform = bone->parent ? bone->parent->transform : ae::Matrix4::Identity();
							((ae::Bone*)bone)->localTransform = animTransform;
							((ae::Bone*)bone)->transform = parentTransform * animTransform;
						}
						ae::Matrix4 offsetTransform = animTransform;
						keyframe.position = offsetTransform.GetTranslation();
						keyframe.rotation = offsetTransform.GetRotation();
						keyframe.scale = offsetTransform.GetScale();
						//AE_INFO( "f:# p:# r:X s:#", i, keyframe.position, keyframe.scale );
						boneKeyframes.Append( keyframe );
					}
				}
			}
		}
	}
	
	vertexData->Initialize( sizeof(Vertex), sizeof(uint32_t),
		vertices.Length(), indices.Length(),
		ae::VertexData::Primitive::Triangle,
		ae::VertexData::Usage::Dynamic, ae::VertexData::Usage::Static );
	vertexData->AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
	vertexData->AddAttribute( "a_normal", 4, ae::VertexData::Type::Float, offsetof( Vertex, normal ) );
	vertexData->AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
	vertexData->AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof( Vertex, uv ) );
	vertexData->SetVertices( vertices.Begin(), vertices.Length() );
	vertexData->SetIndices( indices.Begin(), indices.Length() );
	
	return true;
}

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_UNIFORM vec4 u_color;\
	AE_IN_HIGHP vec4 a_position;\
	AE_IN_HIGHP vec4 a_color;\
	AE_IN_HIGHP vec2 a_uv;\
	AE_OUT_HIGHP vec4 v_color;\
	AE_OUT_HIGHP vec2 v_uv;\
	void main()\
	{\
		v_color = a_color * u_color;\
		v_uv = a_uv;\
		gl_Position = u_worldToProj * a_position;\
	}";

const char* kFragShader = "\
	AE_UNIFORM sampler2D u_tex;\
	AE_IN_HIGHP vec4 v_color;\
	AE_IN_HIGHP vec2 v_uv;\
	void main()\
	{\
		AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
	}";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_INFO( "Initialize" );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::FileSystem fileSystem;
	ae::DebugCamera camera;
	ae::DebugLines debugLines;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "cube" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "johnhues", "16_SkinnedMesh" );
	camera.Initialize( ae::Axis::Y, ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.4f, 3.5f ) );
	camera.SetDistanceLimits( 1.0f, 25.0f );
	debugLines.Initialize( 4096 );

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetBlending( true );
	shader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

	ae::Texture2D texture;
	{
		ae::TargaFile targaFile = TAG_ALL;
		uint32_t fileSize = fileSystem.GetSize( "character.tga" );
		AE_ASSERT( fileSize );
		ae::Scratch< uint8_t > fileData( TAG_ALL, fileSize );
		fileSystem.Read( "character.tga", fileData.Data(), fileData.Length() );
		targaFile.Load( fileData.Data(), fileData.Length() );
		texture.Initialize( targaFile.textureParams );
	}
	
	ae::Skin skin = TAG_ALL;
	ae::Animation anim = TAG_ALL;
	ae::VertexData vertexData;
	LoadFBX( &fileSystem, "character.fbx", &vertexData, &skin, &anim );
	anim.loop = true;
	
	double animTime = 0.0;
	
	AE_INFO( "Run" );
	while ( !input.quit )
	{
		input.Pump();
		camera.Update( &input, timeStep.GetDt() );
		
		animTime += timeStep.GetDt() * 0.01;
		
		// Update skeleton
		ae::Skeleton currentPose = TAG_ALL;
		currentPose.Initialize( skin.GetBindPose() );
		
		// Settings update
		static int32_t s_strength10 = 10;
		if ( input.Get( ae::Key::Minus ) && !input.GetPrev( ae::Key::Minus ) )
		{
			s_strength10--;
		}
		if ( input.Get( ae::Key::Equals ) && !input.GetPrev( ae::Key::Equals ) )
		{
			s_strength10++;
		}
		static int32_t s_strength10Prev = -1;
		if ( s_strength10Prev != s_strength10 )
		{
			s_strength10 = ae::Clip( s_strength10, 0, 10 );
			AE_INFO( "Anim blend strength: #", s_strength10 / 10.0f );
			s_strength10Prev = s_strength10;
		}
		ae::Array< const ae::Bone* > mask = TAG_ALL;
		static int32_t s_maskCountPrev = -1;
		if ( input.Get( ae::Key::Space ) )
		{
			std::function< void( const ae::Bone* ) > maskFn = [&]( const ae::Bone* bone )
			{
				if ( bone )
				{
					mask.Append( bone );
					for ( bone = bone->firstChild; bone; bone = bone->nextSibling )
					{
						maskFn( bone );
					}
				}
			};
			mask.Append( currentPose.GetBoneByName( "QuickRigCharacter_Hips" ) );
			maskFn( currentPose.GetBoneByName( "QuickRigCharacter_LeftUpLeg" ) );
			maskFn( currentPose.GetBoneByName( "QuickRigCharacter_RightUpLeg" ) );
		}
		if ( s_maskCountPrev != mask.Length() )
		{
			AE_INFO( "Animation Mask" );
			for ( const ae::Bone* b : mask )
			{
				AE_INFO( "\t#", b->name );
			}
			if ( !mask.Length() )
			{
				AE_INFO( "\tNone" );
			}
			s_maskCountPrev = mask.Length();
		}
		
		// Update mesh
		anim.AnimateByTime( &currentPose, animTime, s_strength10 / 10.0f, mask.Begin(), mask.Length() );
		Vertex* meshVerts = vertexData.GetWritableVertices< Vertex >();
		skin.ApplyPoseToMesh( &currentPose, meshVerts->pos.data, meshVerts->normal.data, sizeof(Vertex), sizeof(Vertex), vertexData.GetVertexCount() );
		vertexData.Upload();
		
		// Debug
		for ( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
		{
			const ae::Bone* bone = currentPose.GetBoneByIndex( i );
			const ae::Bone* parent = bone->parent;
			if ( parent )
			{
				debugLines.AddLine( parent->transform.GetTranslation(), bone->transform.GetTranslation(), ae::Color::Red() );
				debugLines.AddOBB( bone->transform * ae::Matrix4::Scaling( 0.1f ), ae::Color::Red() );
			}
		}
		
		// Start frame
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		// Render mesh
		ae::UniformList uniformList;
		uniformList.Set( "u_worldToProj", worldToProj );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		uniformList.Set( "u_tex", &texture );
		vertexData.Render( &shader, uniformList );
		
		// Frame end
		debugLines.Render( worldToProj );
		render.Present();
		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
