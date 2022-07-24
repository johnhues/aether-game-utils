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
#include "loaders.h"
#include "Editor.h"
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

bool ofbxLoadMesh( const ae::Tag& tag, const uint8_t* fileData, uint32_t fileDataLen, const VertexLoaderHelper& vertexHelper, const LoadMeshParams& params )
{
	ofbx::IScene* scene = ofbx::load( (ofbx::u8*)fileData, fileDataLen, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE );
	if ( !scene )
	{
		return false;
	}
	
	uint32_t meshCount = scene->getMeshCount();
	
	uint32_t totalVerts = 0;
	uint32_t totalIndices = 0;
	for ( uint32_t i = 0; i < meshCount; i++ )
	{
		const ofbx::Mesh* mesh = scene->getMesh( i );
		const ofbx::Geometry* geo = mesh->getGeometry();
		totalVerts += geo->getVertexCount();
		totalIndices += geo->getIndexCount();
	}
	
	uint32_t indexOffset = 0;
	ae::Array< uint8_t > vertexBuffer( tag, totalVerts * vertexHelper.size );
	ae::Array< uint32_t > indices( tag, totalIndices );
	for ( uint32_t i = 0; i < meshCount; i++ )
	{
		const ofbx::Mesh* mesh = scene->getMesh( i );
		const ofbx::Geometry* geo = mesh->getGeometry();
		ae::Matrix4 localToWorld = params.transform * ofbxToAe( mesh->getGlobalTransform() );
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
			
			uint8_t vertex[ 128 ];
			AE_ASSERT( vertexHelper.size <= sizeof(vertex) );
			vertexHelper.SetPosition( vertex, 0, localToWorld * p );
			vertexHelper.SetNormal( vertex, 0, ae::Vec4( 0.0f ) );
			vertexHelper.SetColor( vertex, 0, color.GetLinearRGBA() );
			vertexHelper.SetUV( vertex, 0, uv );
			vertexBuffer.Append( vertex, vertexHelper.size );
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
			ae::Vec4 normal = ( normalMatrix * ae::Vec4( n.x, n.y, n.z, 0.0f ) ).SafeNormalizeCopy();
			vertexHelper.SetNormal( vertexBuffer.Begin(), index, normal );
		}
		
		indexOffset += vertexCount;
	}
	
	if ( params.vertexData )
	{
		params.vertexData->Initialize( vertexHelper.size, sizeof(uint32_t),
			totalVerts, indices.Length(),
			ae::Vertex::Primitive::Triangle,
			ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
		if ( vertexHelper.posOffset >= 0 ) { params.vertexData->AddAttribute( vertexHelper.posAttrib, vertexHelper.posComponents, ae::Vertex::Type::Float, vertexHelper.posOffset ); }
		if ( vertexHelper.normalOffset >= 0 ) { params.vertexData->AddAttribute( vertexHelper.normalAttrib, vertexHelper.normalComponents, ae::Vertex::Type::Float, vertexHelper.normalOffset ); }
		if ( vertexHelper.colorOffset >= 0 ) { params.vertexData->AddAttribute( vertexHelper.colorAttrib, vertexHelper.colorComponents, ae::Vertex::Type::Float, vertexHelper.colorOffset ); }
		if ( vertexHelper.uvOffset >= 0 ) { params.vertexData->AddAttribute( vertexHelper.uvAttrib, vertexHelper.uvComponents, ae::Vertex::Type::Float, vertexHelper.uvOffset ); }
		params.vertexData->SetVertices( vertexBuffer.Begin(), totalVerts );
		params.vertexData->SetIndices( indices.Begin(), indices.Length() );
		params.vertexData->Upload();
	}
	
	if ( params.collisionMesh )
	{
		ae::CollisionMesh::Params collisionParams;
		collisionParams.positionCount = totalVerts;
		collisionParams.positions = vertexHelper.GetPosition( vertexBuffer.Begin(), 0 ).data;
		collisionParams.positionStride = vertexHelper.size;
		collisionParams.indices = indices.Begin();
		collisionParams.indexCount = indices.Length();
		collisionParams.indexSize = sizeof(uint32_t);
		params.collisionMesh->Load( collisionParams );
	}
	
	if ( params.editorMesh )
	{
		params.editorMesh->verts.Reserve( totalVerts );
		for ( uint32_t i = 0; i < totalVerts; i++ )
		{
			params.editorMesh->verts.Append( vertexHelper.GetPosition( vertexBuffer.Begin(), i ).GetXYZ() );
		}
		params.editorMesh->indices.Append( indices.Begin(), indices.Length() );
	}
	
	return true;
}

bool ofbxLoadSkinnedMesh( const ae::Tag& tag, const uint8_t* fileData, uint32_t fileDataLen, const VertexLoaderHelper& vertexHelper, ae::VertexData* vertexData, ae::Skin* skinOut, ae::Animation* animOut )
{
	ofbx::IScene* scene = ofbx::load( (ofbx::u8*)fileData, fileDataLen, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE );
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
	ae::Skeleton bindPose = tag;
	bindPose.Initialize( 32 ); // @TODO: Count ofbxSkeletonRoot bones
	ae::Array< const ofbx::Object* > sceneBones = tag;
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
	
	ae::Array< ae::Skin::Vertex > skinVerts = tag;
	skinVerts.Reserve( totalVerts );
	for ( uint32_t i = 0; i < totalVerts; i++ )
	{
		skinVerts.Append( {} );
	}

	uint32_t indexOffset = 0;
	ae::Array< uint8_t > vertexBuffer( tag, totalVerts * vertexHelper.size );
	ae::Array< uint32_t > indices( tag, totalIndices );
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

			uint8_t vertex[ 128 ];
			AE_ASSERT( vertexHelper.size <= sizeof(vertex) );
			vertexHelper.SetPosition( vertex, 0, localToWorld * p );
			vertexHelper.SetNormal( vertex, 0, ae::Vec4( 0.0f ) );
			vertexHelper.SetColor( vertex, 0, color.GetLinearRGBA() );
			vertexHelper.SetUV( vertex, 0, uv );
			vertexBuffer.Append( vertex, vertexHelper.size );
			
			skinVerts[ j ].position = p.GetXYZ();
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
			ae::Vec4 normal = ( normalMatrix * ae::Vec4( n.x, n.y, n.z, 0.0f ) ).SafeNormalizeCopy();
			vertexHelper.SetNormal( vertexBuffer.Begin(), index, normal );
			
			skinVerts[ index ].normal = normal.GetXYZ();
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
	if ( animOut && scene->getAnimationStackCount() )
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

					ae::Array< ae::Keyframe >& boneKeyframes = animOut->keyframes.Set( bone->name, tag );

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
						keyframe.translation = offsetTransform.GetTranslation();
						keyframe.rotation = offsetTransform.GetRotation();
						keyframe.scale = offsetTransform.GetScale();
						//AE_INFO( "f:# p:# r:X s:#", i, keyframe.position, keyframe.scale );
						boneKeyframes.Append( keyframe );
					}
				}
			}
		}
	}
	
	vertexData->Initialize( vertexHelper.size, sizeof(uint32_t),
		totalVerts, indices.Length(),
		ae::Vertex::Primitive::Triangle,
		ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Static );
	if ( vertexHelper.posOffset >= 0 ) { vertexData->AddAttribute( vertexHelper.posAttrib, vertexHelper.posComponents, ae::Vertex::Type::Float, vertexHelper.posOffset ); }
	if ( vertexHelper.normalOffset >= 0 ) { vertexData->AddAttribute( vertexHelper.normalAttrib, vertexHelper.normalComponents, ae::Vertex::Type::Float, vertexHelper.normalOffset ); }
	if ( vertexHelper.colorOffset >= 0 ) { vertexData->AddAttribute( vertexHelper.colorAttrib, vertexHelper.colorComponents, ae::Vertex::Type::Float, vertexHelper.colorOffset ); }
	if ( vertexHelper.uvOffset >= 0 ) { vertexData->AddAttribute( vertexHelper.uvAttrib, vertexHelper.uvComponents, ae::Vertex::Type::Float, vertexHelper.uvOffset ); }
	vertexData->SetVertices( vertexBuffer.Begin(), totalVerts );
	vertexData->SetIndices( indices.Begin(), indices.Length() );
	
	return true;
}

} // End ae namespace
