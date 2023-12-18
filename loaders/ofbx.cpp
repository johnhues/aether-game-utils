//------------------------------------------------------------------------------
// ofbx.cpp
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
#include "aether.h"
#include "ae/loaders.h"
#include "ofbx.h"
namespace ae {

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

struct FbxLoaderMeshInfo
{
	const ofbx::Mesh* mesh = nullptr;
	const ofbx::Object* rootJoint = nullptr;
	uint32_t boneCount = 0;
};

struct FbxLoaderImpl
{
	FbxLoaderImpl( const ae::Tag& tag ) : meshes( tag ) {}
	ofbx::IScene* scene;
	ae::Map< ae::Str128, FbxLoaderMeshInfo > meshes;
};

//------------------------------------------------------------------------------
// ae::FbxLoader member functions
//------------------------------------------------------------------------------
FbxLoader::FbxLoader( const ae::Tag& tag ) :
	m_tag( tag )
{}

FbxLoader::~FbxLoader()
{
	Terminate();
}

bool FbxLoader::Initialize( const void* fileData, uint32_t fileDataLen )
{
	Terminate();
	m_state = ae::New< FbxLoaderImpl >( m_tag, m_tag );
	m_state->scene = ofbx::load( (ofbx::u8*)fileData, fileDataLen, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE );
	if ( !m_state->scene )
	{
		Terminate();
		return false;
	}
	
	const uint32_t meshCount = m_state->scene->getMeshCount();
	for ( uint32_t i = 0; i < meshCount; i++ )
	{
		const ofbx::Mesh* ofbxMesh = m_state->scene->getMesh( i );
		FbxLoaderMeshInfo* info = &m_state->meshes.Set( ofbxMesh->name, {} );
		info->mesh = ofbxMesh;
		if ( const ofbx::Skin* ofbxSkin = ofbxMesh->getGeometry()->getSkin() )
		{
			AE_ASSERT( ofbxSkin->getClusterCount() );
			const ofbx::Object* ofbxSkeletonRoot = ofbxSkin->getCluster( 0 )->getLink();
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
			info->rootJoint = ofbxSkeletonRoot;
			
			auto countBonesFn = [&]( auto&& countBonesFn, const ofbx::Object* ofbxParent ) -> uint32_t
			{
				uint32_t result = 1;
				for ( int32_t i = 0; const ofbx::Object* ofbxBone = ofbxParent->resolveObjectLink( i ); i++ )
				{
					if ( ofbxBone->getType() == ofbx::Object::Type::LIMB_NODE )
					{
						result += countBonesFn( countBonesFn, ofbxBone );
					}
				}
				return result;
			};
			// An extra bone to maintain offset of hips etc
			info->boneCount = ( 1 + countBonesFn( countBonesFn, ofbxSkeletonRoot ) );
		}
	}
	
	return true;
}

void FbxLoader::Terminate()
{
	if ( m_state )
	{
		if ( m_state->scene )
		{
			m_state->scene->destroy();
		}
		ae::Delete( m_state );
		m_state = nullptr;
	}
}

uint32_t FbxLoader::GetMeshCount() const
{
	return m_state ? m_state->meshes.Length() : 0;
}

const char* FbxLoader::GetMeshName( uint32_t idx ) const
{
	return m_state ? m_state->meshes.GetKey( idx ).c_str() : "";
}

uint32_t FbxLoader::GetMeshVertexCount( uint32_t idx ) const
{
	return m_state ? m_state->meshes.GetValue( idx ).mesh->getGeometry()->getVertexCount() : 0;
}

uint32_t FbxLoader::GetMeshIndexCount( uint32_t idx ) const
{
	return m_state ? m_state->meshes.GetValue( idx ).mesh->getGeometry()->getIndexCount() : 0;
}

uint32_t FbxLoader::GetMeshBoneCount( uint32_t idx ) const
{
	return m_state ? m_state->meshes.GetValue( idx ).boneCount : 0;
}

uint32_t FbxLoader::GetMeshVertexCount( const char* name ) const
{
	if ( m_state )
	{
		if ( FbxLoaderMeshInfo* info = m_state->meshes.TryGet( name ) )
		{
			return info->mesh->getGeometry()->getVertexCount();
		}
	}
	return 0;
}

uint32_t FbxLoader::GetMeshIndexCount( const char* name ) const
{
	if ( m_state )
	{
		if ( FbxLoaderMeshInfo* info = m_state->meshes.TryGet( name ) )
		{
			return info->mesh->getGeometry()->getIndexCount();
		}
	}
	return 0;
}

uint32_t FbxLoader::GetMeshBoneCount( const char* name ) const
{
	if ( m_state )
	{
		if ( FbxLoaderMeshInfo* info = m_state->meshes.TryGet( name ) )
		{
			return info->boneCount;
		}
	}
	return 0;
}

bool FbxLoader::Load( const char* meshName, const ae::FbxLoaderParams& params ) const
{
	if ( !m_state ) { return false; }
	const FbxLoaderMeshInfo* info = m_state->meshes.TryGet( meshName );
	if ( !info ) { return false; }
	
	AE_ASSERT_MSG( params.descriptor.indexSize, "Must define index type size" );
	AE_ASSERT_MSG( params.descriptor.indexSize == sizeof(uint32_t), "TODO" );
	
	ofbx::IScene* ofbxScene = m_state->scene;
	AE_ASSERT( ofbxScene );
	const ofbx::GlobalSettings* ofbxSettings = ofbxScene->getGlobalSettings();
	const float frameRate = ofbxScene->getSceneFrameRate();
	const float startTime = ofbxSettings->TimeSpanStart;
	const float endTime = ofbxSettings->TimeSpanStop;
	const ofbx::Mesh* mesh = info->mesh;
	const ofbx::Geometry* geo = mesh->getGeometry();
	const ae::Matrix4 localToWorld = ofbxToAe( mesh->getGlobalTransform() );
	const ae::Matrix4 normalMatrix = localToWorld.GetNormalMatrix();
	const uint32_t vertexCount = geo->getVertexCount();
	const uint32_t indexCount = geo->getIndexCount();
	const ofbx::Vec3* meshVerts = geo->getVertices();
	const ofbx::Vec3* meshNormals = geo->getNormals();
	const ofbx::Vec4* meshColors = geo->getColors();
	const ofbx::Vec2* meshUvs = geo->getUVs();
	const int32_t* meshIndices = geo->getFaceIndices();
	if ( params.vertexOut && params.maxVerts < vertexCount ) { return false; }
	if ( params.indexOut && params.maxIndex < indexCount ) { return false; }
	const ofbx::Skin* ofbxSkin = info->mesh->getGeometry()->getSkin();
	const ofbx::Object* ofbxSkeletonRoot = info->rootJoint;
	if ( ofbxSkeletonRoot )
	{
		// Find the root node of the skinned meshes skeleton (the parent of root bone)
		// Not an actual bone, scene root or something
		ofbxSkeletonRoot = ofbxSkeletonRoot->getParent();
		AE_ASSERT( ofbxSkeletonRoot );
	}
	const uint32_t clusterCount = ofbxSkin ? ofbxSkin->getClusterCount() : 0;
	const uint32_t boneCount = info->boneCount;
	
	ae::Skeleton bindPose = m_tag;
	ae::Array< const ofbx::Object* > sceneBones = m_tag;
	if ( ofbxSkeletonRoot )
	{
		bindPose.Initialize( boneCount );
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
					const ae::Bone* bone = bindPose.AddBone( parent, ofbxBone->name, transform, {} );
					AE_ASSERT( bone );
					sceneBones.Append( ofbxBone );
					skeletonBuilderFn( ofbxBone, bone );
				}
				i++;
			}
		};
		skeletonBuilderFn( ofbxSkeletonRoot, bindPose.GetRoot() );
	}
	
	// Mesh
	ae::Array< uint8_t > vertexBuffer( m_tag, vertexCount * params.descriptor.vertexSize );
	ae::Array< uint32_t > indices( m_tag, indexCount );
	ae::Array< ae::Skin::Vertex > skinVerts( m_tag, {}, vertexCount );
	
	for ( uint32_t j = 0; j < vertexCount; j++ )
	{
		ofbx::Vec3 p0 = meshVerts[ j ];
		ae::Vec4 p = localToWorld * ae::Vec4( p0.x, p0.y, p0.z, 1.0f );
		ae::Color color = meshColors ? ae::Color::SRGBA( (float)meshColors[ j ].x, (float)meshColors[ j ].y, (float)meshColors[ j ].z, (float)meshColors[ j ].w ) : ae::Color::White();
		ae::Vec2 uv = meshUvs ? ae::Vec2( meshUvs[ j ].x, meshUvs[ j ].y ) : ae::Vec2( 0.0f );
		
		uint8_t vertex[ 128 ];
		AE_ASSERT( params.descriptor.vertexSize <= sizeof(vertex) );
		params.descriptor.SetPosition( vertex, 0, p );
		params.descriptor.SetNormal( vertex, 0, ae::Vec4( 0.0f ) );
		params.descriptor.SetColor( vertex, 0, color.GetLinearRGBA() );
		params.descriptor.SetUV( vertex, 0, uv );
		vertexBuffer.AppendArray( vertex, params.descriptor.vertexSize );
		
		skinVerts[ j ].position = p.GetXYZ();
	}
	
	for ( uint32_t j = 0; j < indexCount; j++ )
	{
		int32_t index = ( meshIndices[ j ] < 0 ) ? ( -meshIndices[ j ] - 1 ) : meshIndices[ j ];
		AE_ASSERT( index < vertexCount );
		indices.Append( index );
		
		ofbx::Vec3 n = meshNormals[ j ];
		ae::Vec4 normal = ( normalMatrix * ae::Vec4( n.x, n.y, n.z, 0.0f ) ).SafeNormalizeCopy();
		params.descriptor.SetNormal( vertexBuffer.Data(), index, normal );
		
		skinVerts[ index ].normal = normal.GetXYZ();
	}
	
	// Skin
	for ( uint32_t i = 0; i < clusterCount; i++ )
	{
		const ofbx::Cluster* cluster = ofbxSkin->getCluster( i );
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
			const uint8_t clusterWeight = ae::Clip( (int)( clusterWeights[ j ] * 256.5 ), 0, 255 );
			const uint32_t weightCount = [&](){ uint32_t c = 0; while ( c < kMaxSkinWeights && vertex->weights[ c ] ) { c++; } return c; }();
			
			if ( weightCount == kMaxSkinWeights )
			{
				// Replace the influence with the smallest contribution
				int32_t lowestIdx = 0;
				uint8_t lowestWeight = vertex->weights[ 0 ];
				for ( uint32_t i = 1; i < kMaxSkinWeights; i++ )
				{
					const uint8_t weight = vertex->weights[ i ];
					if ( lowestWeight > weight )
					{
						lowestIdx = i;
						lowestWeight = weight;
					}
				}
				vertex->bones[ lowestIdx ] = boneIndex;
				vertex->weights[ lowestIdx ] = clusterWeight;
			}
			else
			{
				vertex->bones[ weightCount ] = boneIndex;
				vertex->weights[ weightCount ] = ae::Clip( (int)( clusterWeights[ j ] * 256.5 ), 0, 255 );
			}
		}
	}
	// Fix up weights so they total 255
	for ( ae::Skin::Vertex& vertex : skinVerts )
	{
		int32_t total = 0;
		uint8_t* greatest = nullptr;
		for ( uint32_t i = 0; i < kMaxSkinWeights; i++ )
		{
			total += vertex.weights[ i ];
			if ( !greatest || *greatest < vertex.weights[ i ] )
			{
				greatest = &vertex.weights[ i ];
			}
		}
		if ( total != 255 )
		{
			// Bump up the contribution of the strongest influence to total uint8 max
			*greatest += ( 255 - total );
		}
#if _AE_DEBUG_
		total = 0;
		for ( uint32_t i = 0; i < kMaxSkinWeights; i++ )
		{
			total += vertex.weights[ i ];
		}
		AE_ASSERT( total == 255 );
#endif
	}
	
	// Finalize skin
	if ( params.skin )
	{
		params.skin->Initialize( bindPose, skinVerts.Data(), skinVerts.Length() );
	}
	
	// Animations
	if ( params.anim && ofbxScene->getAnimationStackCount() )
	{
		const ofbx::AnimationStack* animStack = ofbxScene->getAnimationStack( 0 );
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
					params.anim->duration = ( endTime - startTime );

					ae::Array< ae::Keyframe >& boneKeyframes = params.anim->keyframes.Set( bone->name, m_tag );

					// The following is weird because when you select an animation frame window in Maya it always shows an extra frame
					uint32_t sampleCount = ae::Round( params.anim->duration * frameRate );
					sampleCount++; // If an animation has one keyframe at frame 0 and the last one at 48, it actually has 49 frames
					boneKeyframes.Clear();
					boneKeyframes.Reserve( sampleCount );
					for ( uint32_t j = 0; j < sampleCount; j++ )
					{
						// Subtract 1 from sample count so last frame doesn't count towards final length
						// ie. A 2 second animation playing at 2 frames per second should have 5 frames, at times: 0, 0.5, 1.0, 1.5, 2
						float t = ( sampleCount > 1 ) ? startTime + ( j / ( sampleCount - 1.0f ) ) * params.anim->duration : 0.0f;
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
						keyframe.translation = offsetTransform.GetTranslation();
						keyframe.rotation = offsetTransform.GetRotation();
						keyframe.scale = offsetTransform.GetScale();
						boneKeyframes.Append( keyframe );
					}
				}
			}
		}
	}
	
	if ( params.vertexOut )
	{
		memcpy( params.vertexOut, vertexBuffer.Data(), vertexCount * params.descriptor.vertexSize );
	}
	if ( params.indexOut )
	{
		memcpy( params.indexOut, indices.Data(), indexCount * params.descriptor.indexSize );
	}
	
	if ( params.vertexData )
	{
		params.vertexData->Initialize( params.descriptor.vertexSize, sizeof(uint32_t),
			vertexCount, indices.Length(),
			ae::Vertex::Primitive::Triangle,
			ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Static );
		if ( params.descriptor.posOffset >= 0 ) { params.vertexData->AddAttribute( params.descriptor.posAttrib, params.descriptor.posComponents, ae::Vertex::Type::Float, params.descriptor.posOffset ); }
		if ( params.descriptor.normalOffset >= 0 ) { params.vertexData->AddAttribute( params.descriptor.normalAttrib, params.descriptor.normalComponents, ae::Vertex::Type::Float, params.descriptor.normalOffset ); }
		if ( params.descriptor.colorOffset >= 0 ) { params.vertexData->AddAttribute( params.descriptor.colorAttrib, params.descriptor.colorComponents, ae::Vertex::Type::Float, params.descriptor.colorOffset ); }
		if ( params.descriptor.uvOffset >= 0 ) { params.vertexData->AddAttribute( params.descriptor.uvAttrib, params.descriptor.uvComponents, ae::Vertex::Type::Float, params.descriptor.uvOffset ); }
		params.vertexData->UploadVertices( 0, vertexBuffer.Data(), vertexCount );
		params.vertexData->UploadIndices( 0, indices.Data(), indices.Length() );
	}

	if ( params.collisionMesh )
	{
		params.collisionMesh->AddIndexed(
			ae::Matrix4::Identity(),
			params.descriptor.GetPosition( vertexBuffer.Data(), 0 ).data,
			vertexCount,
			params.descriptor.vertexSize,
			indices.Data(),
			indices.Length(),
			sizeof(uint32_t)
		);
		params.collisionMesh->BuildBVH();
	}
	
	return true;
}

} // End ae namespace
