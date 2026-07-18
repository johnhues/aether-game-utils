//------------------------------------------------------------------------------
// IKTest.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2026 John Hughes
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
#include "IKTest.h"

//------------------------------------------------------------------------------
// Authoring helpers
//------------------------------------------------------------------------------
namespace
{

// Targets within this fraction of a chain's full extension are in a boundary
// band where FABRIK converges slowly; effector and straighten checks skip it.
const float kReachableBand = 0.97f;
const float kUnreachableBand = 1.03f;

const ae::Bone* AddBoneWorld( ae::Skeleton* skeleton, const ae::Bone* parent, const char* name, ae::Vec3 position, ae::Quaternion rotation = ae::Quaternion::Identity() )
{
	const ae::Matrix4 boneToModel = rotation.GetTransformMatrix().SetTranslation( position );
	const ae::Matrix4 parentToChild = parent->modelToBone * boneToModel;
	return skeleton->AddBone( parent, name, parentToChild );
}

uint32_t Idx( const IKScenario& scenario, const char* name )
{
	const ae::Bone* bone = scenario.bindPose.GetBoneByName( name );
	AE_ASSERT_MSG( bone, "Scenario '#' has no bone '#'", scenario.name, name );
	return bone->index;
}

float Cycle01( uint32_t frame, uint32_t frameCount )
{
	return frameCount ? ( frame % frameCount ) / (float)frameCount : 0.0f;
}

float PingPong01( float t01 )
{
	return 1.0f - ae::Abs( 1.0f - 2.0f * t01 );
}

ae::Vec3 CirclePath( ae::Vec3 center, ae::Vec3 u, ae::Vec3 v, float radius, float t01 )
{
	const float angle = t01 * ae::Pi * 2.0f;
	return center + ( u * ae::Cos( angle ) + v * ae::Sin( angle ) ) * radius;
}

float QuaternionAngleDiff( const ae::Quaternion& a, const ae::Quaternion& b )
{
	const ae::Quaternion delta = a.GetInverse() * b;
	ae::Vec3 axis;
	float angle;
	delta.GetAxisAngle( &axis, &angle );
	angle = ae::Abs( angle );
	if( angle > ae::Pi )
	{
		angle = ae::Pi * 2.0f - angle;
	}
	return angle;
}

void GetSubtreeMask( const ae::Skeleton& pose, uint32_t rootBoneIndex, ae::Array< bool >* maskOut )
{
	maskOut->Clear();
	for( uint32_t i = 0; i < pose.GetBoneCount(); i++ )
	{
		maskOut->Append( false );
	}
	auto markIter = [&]( auto&& markIter, const ae::Bone* bone ) -> void
	{
		( *maskOut )[ bone->index ] = true;
		for( const ae::Bone* child = bone->firstChild; child; child = child->nextSibling )
		{
			markIter( markIter, child );
		}
	};
	markIter( markIter, pose.GetBoneByIndex( rootBoneIndex ) );
}

void SolveFrame( const ae::Tag& tag, const IKScenario& scenario, uint32_t frame, uint32_t iterationCount, const ae::Skeleton& inputPose, ae::Skeleton* solvedOut, ae::DebugLines* debugLines, const ae::Matrix4& debugModelToWorld, float debugJointScale )
{
	ae::Map< uint32_t, ae::Vec3 > targets = tag;
	ae::Map< uint32_t, ae::Quaternion > orientations = tag;
	if( scenario.targetFn )
	{
		scenario.targetFn( scenario, frame, &targets, &orientations );
	}
	ae::IK ik = tag;
	ik.rootBoneIndex = scenario.rootBoneIndex;
	ik.targets = targets;
	ik.targetOrientations = orientations;
	ik.distanceConstraints = scenario.distanceConstraints;
	ik.rotationConstraints = scenario.rotationConstraints;
	ik.bindPose = &scenario.bindPose;
	ik.pose.Initialize( &inputPose );
	ik.debugLines = debugLines;
	ik.debugModelToWorld = debugModelToWorld;
	ik.debugJointScale = debugJointScale;
	ik.Run( iterationCount, solvedOut );
}

} // namespace

//------------------------------------------------------------------------------
// IKScenario
//------------------------------------------------------------------------------
IKScenario::IKScenario( const ae::Tag& tag ) :
	bindPose( tag ),
	rotationConstraints( tag ),
	distanceConstraints( tag ),
	staticBones( tag ),
	mirrorPairs( tag )
{}

//------------------------------------------------------------------------------
// IKEvaluateFrame
//------------------------------------------------------------------------------
IKFrameReport IKEvaluateFrame( const IKScenario& scenario, uint32_t frame, const ae::Skeleton& inputPose, const ae::Skeleton& solvedPose, const ae::Skeleton* prevSolvedPose )
{
	const ae::Tag tag = "ikframe";
	const ae::Skeleton& bind = scenario.bindPose;
	IKFrameReport report;
	float worstRatio = 1.0f;
	auto consider = [&]( float error, float tolerance, float* fieldOut, const char* checkName, const char* boneName )
	{
		if( error > *fieldOut )
		{
			*fieldOut = error;
		}
		if( tolerance >= 0.0f && error > tolerance )
		{
			report.pass = false;
			const float ratio = ( tolerance > 0.0f ) ? ( error / tolerance ) : INFINITY;
			if( ratio > worstRatio )
			{
				worstRatio = ratio;
				report.worstCheck = ae::Str128::Format( "# '#' error # (tolerance #) frame #", checkName, boneName, error, tolerance, frame );
			}
		}
	};

	ae::Map< uint32_t, ae::Vec3 > targets = tag;
	ae::Map< uint32_t, ae::Quaternion > orientations = tag;
	if( scenario.targetFn )
	{
		scenario.targetFn( scenario, frame, &targets, &orientations );
	}
	ae::Array< bool > subtree = tag;
	GetSubtreeMask( solvedPose, scenario.rootBoneIndex, &subtree );

	// Bone lengths vs the bind pose. The solved root bone is pinned or
	// targeted rather than length-constrained, and bones outside the solve
	// subtree pass through, so both are skipped.
	for( uint32_t i = 1; i < solvedPose.GetBoneCount(); i++ )
	{
		if( i == scenario.rootBoneIndex || !subtree[ i ] )
		{
			continue;
		}
		const ae::Bone* solvedBone = solvedPose.GetBoneByIndex( i );
		const ae::Bone* bindBone = bind.GetBoneByIndex( i );
		const float bindLength = ( bindBone->boneToModel.GetTranslation() - bindBone->parent->boneToModel.GetTranslation() ).Length();
		if( bindLength < 0.0001f )
		{
			continue;
		}
		const float solvedLength = ( solvedBone->boneToModel.GetTranslation() - solvedBone->parent->boneToModel.GetTranslation() ).Length();
		const float error = ae::Abs( solvedLength - bindLength ) / bindLength;
		consider( error, scenario.boneLengthTolerance, &report.maxBoneLengthError, "bone length", solvedBone->name.c_str() );
	}

	// Distance constraints. Rest lengths derive from the solve-input pose,
	// matching ae::IK::Run().
	for( const ae::IKDistanceConstraint& constraint : scenario.distanceConstraints )
	{
		if( (uint32_t)constraint.idx0 >= solvedPose.GetBoneCount() || (uint32_t)constraint.idx1 >= solvedPose.GetBoneCount() )
		{
			continue;
		}
		const ae::Vec3 input0 = inputPose.GetBoneByIndex( constraint.idx0 )->boneToModel.GetTranslation();
		const ae::Vec3 input1 = inputPose.GetBoneByIndex( constraint.idx1 )->boneToModel.GetTranslation();
		const float restLength = ( input0 - input1 ).Length();
		if( restLength < 0.0001f )
		{
			continue;
		}
		const float minLength = restLength * ae::Clip01( constraint.maxCompression01 );
		const float maxLength = restLength * ae::Max( 1.0f, constraint.maxStretch1N );
		const ae::Vec3 solved0 = solvedPose.GetBoneByIndex( constraint.idx0 )->boneToModel.GetTranslation();
		const ae::Vec3 solved1 = solvedPose.GetBoneByIndex( constraint.idx1 )->boneToModel.GetTranslation();
		const float distance = ( solved0 - solved1 ).Length();
		const float error = ae::Max( minLength - distance, distance - maxLength, 0.0f ) / restLength;
		const ae::Str128 pairName = ae::Str128::Format( "#-#", solvedPose.GetBoneByIndex( constraint.idx0 )->name, solvedPose.GetBoneByIndex( constraint.idx1 )->name );
		consider( error, scenario.distanceConstraintTolerance, &report.maxDistanceConstraintError, "distance constraint", pairName.c_str() );
	}

	// Rotation limits, re-derived from the solved joint positions the same way
	// ae::IK::Run() derives its reference frames from the bind pose.
	for( const auto& pair : scenario.rotationConstraints )
	{
		const uint32_t childIndex = pair.key;
		const ae::IKRotationConstraint& constraint = pair.value;
		const ae::Bone* solvedChild = solvedPose.GetBoneByIndex( childIndex );
		const ae::Bone* solvedParent = solvedChild->parent;
		if( !solvedParent || !solvedParent->parent )
		{
			continue; // The solver only clips joints with a grandparent
		}
		const ae::Bone* solvedGp = solvedParent->parent;
		const ae::Bone* bindChild = bind.GetBoneByIndex( childIndex );
		const ae::Bone* bindParent = bindChild->parent;
		const ae::Bone* bindGp = bindParent->parent;
		const ae::Vec3 bindIncoming = ( bindParent->boneToModel.GetTranslation() - bindGp->boneToModel.GetTranslation() ).SafeNormalizeCopy();
		const ae::Quaternion parentBindRot = bindParent->boneToModel.GetRotation();
		const ae::Vec3 childBindDirWorld = ( bindChild->boneToModel.GetTranslation() - bindParent->boneToModel.GetTranslation() ).SafeNormalizeCopy();
		const ae::Vec3 primary = parentBindRot.GetInverse().Rotate( childBindDirWorld );
		ae::Vec3 basisX, basisY;
		ae::IK::GetLimitBasis( primary, &basisX, &basisY );
		const ae::Vec3 currentIncoming = ( solvedParent->boneToModel.GetTranslation() - solvedGp->boneToModel.GetTranslation() ).SafeNormalizeCopy();
		const ae::Quaternion refOri = bindIncoming.RotationTo( currentIncoming ) * parentBindRot;
		const ae::Vec3 childDir = refOri.GetInverse().Rotate( ( solvedChild->boneToModel.GetTranslation() - solvedParent->boneToModel.GetTranslation() ).SafeNormalizeCopy() );
		const float q[ 4 ] =
		{
			ae::Tan( ae::Clip( constraint.rotationLimits[ 0 ], 0.01f, ae::HalfPi - 0.01f ) ),
			ae::Tan( ae::Clip( constraint.rotationLimits[ 1 ], 0.01f, ae::HalfPi - 0.01f ) ),
			ae::Tan( ae::Clip( constraint.rotationLimits[ 2 ], 0.01f, ae::HalfPi - 0.01f ) ),
			ae::Tan( ae::Clip( constraint.rotationLimits[ 3 ], 0.01f, ae::HalfPi - 0.01f ) )
		};
		float error = 0.0f;
		const float p = childDir.Dot( primary );
		if( p <= 0.001f )
		{
			// Behind the limit plane: past 90 degrees of swing in every case
			const float maxLimit = ae::Max( constraint.rotationLimits[ 0 ], constraint.rotationLimits[ 1 ], ae::Max( constraint.rotationLimits[ 2 ], constraint.rotationLimits[ 3 ] ) );
			error = ae::Acos( ae::Clip( p, -1.0f, 1.0f ) ) - maxLimit;
		}
		else
		{
			const ae::Vec2 p2 = ae::Vec2( childDir.Dot( basisX ), childDir.Dot( basisY ) ) / p;
			const float halfX = ( p2.x >= 0.0f ) ? q[ 0 ] : q[ 2 ];
			const float halfY = ( p2.y >= 0.0f ) ? q[ 1 ] : q[ 3 ];
			const float inside = ( p2.x * p2.x ) / ( halfX * halfX ) + ( p2.y * p2.y ) / ( halfY * halfY );
			if( inside > 1.0f )
			{
				const ae::Vec2 edge = ae::IK::GetNearestPointOnEllipse( ae::Vec2( halfX, halfY ), ae::Vec2( 0.0f ), p2 );
				const ae::Vec3 clippedDir = ( basisX * edge.x + basisY * edge.y + primary ).SafeNormalizeCopy();
				error = childDir.GetAngleBetween( clippedDir );
			}
		}
		consider( ae::Max( error, 0.0f ), scenario.rotationLimitTolerance, &report.maxRotationLimitError, "rotation limit", solvedChild->name.c_str() );
	}

	// Twist limits, measured as rotation around the bone direction relative to
	// the parent-space bind orientation. Reported separately: twist clamping
	// is currently disabled inside ae::IK::Run().
	for( const auto& pair : scenario.rotationConstraints )
	{
		const uint32_t childIndex = pair.key;
		const ae::IKRotationConstraint& constraint = pair.value;
		const ae::Bone* solvedChild = solvedPose.GetBoneByIndex( childIndex );
		const ae::Bone* solvedParent = solvedChild->parent;
		if( !solvedParent )
		{
			continue;
		}
		const ae::Bone* bindChild = bind.GetBoneByIndex( childIndex );
		const ae::Bone* bindParent = bindChild->parent;
		const ae::Quaternion bindLocalRot = bindParent->boneToModel.GetRotation().GetInverse() * bindChild->boneToModel.GetRotation();
		const ae::Vec3 childBindDirWorld = ( bindChild->boneToModel.GetTranslation() - bindParent->boneToModel.GetTranslation() ).SafeNormalizeCopy();
		const ae::Vec3 selfBindDir = bindChild->boneToModel.GetRotation().GetInverse().Rotate( childBindDirWorld );
		const ae::Quaternion ref = solvedParent->boneToModel.GetRotation() * bindLocalRot;
		const ae::Quaternion delta = ref.GetInverse() * solvedChild->boneToModel.GetRotation();
		ae::Quaternion twist;
		delta.GetTwistSwing( selfBindDir, &twist, nullptr );
		ae::Vec3 twistAxis;
		float twistAngle;
		twist.GetAxisAngle( &twistAxis, &twistAngle );
		if( twistAxis.Dot( selfBindDir ) < 0.0f )
		{
			twistAngle = -twistAngle;
		}
		while( twistAngle > ae::Pi ) { twistAngle -= ae::Pi * 2.0f; }
		while( twistAngle < -ae::Pi ) { twistAngle += ae::Pi * 2.0f; }
		const float error = ae::Max( twistAngle - constraint.twistLimits[ 1 ], constraint.twistLimits[ 0 ] - twistAngle, 0.0f );
		if( error > report.maxTwistLimitError )
		{
			report.maxTwistLimitError = error;
		}
		if( scenario.twistLimitTolerance >= 0.0f && error > scenario.twistLimitTolerance )
		{
			report.twistPass = false;
			if( report.worstCheck.Empty() )
			{
				report.worstCheck = ae::Str128::Format( "twist limit '#' error # (tolerance #) frame #", solvedChild->name, error, scenario.twistLimitTolerance, frame );
			}
		}
	}

	// Position targets. A chain is only expected to reach a target when the
	// target is comfortably within the chain's full extension (or the solve
	// root is free to translate); comfortably beyond it, an opted-in scenario
	// expects the chain to straighten toward the target.
	const ae::Vec3* rootTarget = targets.TryGet( scenario.rootBoneIndex );
	const bool rootPinned = ( solvedPose.GetBoneByIndex( scenario.rootBoneIndex )->parent != solvedPose.GetRoot() );
	for( const auto& target : targets )
	{
		const uint32_t boneIndex = target.key;
		if( boneIndex >= solvedPose.GetBoneCount() || !subtree[ boneIndex ] )
		{
			continue;
		}
		const ae::Bone* solvedBone = solvedPose.GetBoneByIndex( boneIndex );
		float chainLength = 0.0f;
		if( boneIndex != scenario.rootBoneIndex )
		{
			const ae::Bone* walk = bind.GetBoneByIndex( boneIndex );
			while( walk && walk->index != scenario.rootBoneIndex )
			{
				chainLength += ( walk->boneToModel.GetTranslation() - walk->parent->boneToModel.GetTranslation() ).Length();
				walk = walk->parent;
			}
		}
		ae::Vec3 anchor;
		bool anchored = true;
		if( boneIndex == scenario.rootBoneIndex )
		{
			anchor = target.value;
		}
		else if( rootTarget )
		{
			anchor = *rootTarget;
		}
		else if( rootPinned )
		{
			anchor = inputPose.GetBoneByIndex( scenario.rootBoneIndex )->boneToModel.GetTranslation();
		}
		else
		{
			anchored = false; // Free root: the whole chain can translate to reach
		}
		const float targetDistance = anchored ? ( target.value - anchor ).Length() : 0.0f;
		if( !anchored || targetDistance <= chainLength * kReachableBand || boneIndex == scenario.rootBoneIndex )
		{
			const float error = ( solvedBone->boneToModel.GetTranslation() - target.value ).Length();
			consider( error, scenario.effectorTolerance, &report.maxEffectorError, "effector", solvedBone->name.c_str() );
		}
		else if( scenario.checkStraighten && targetDistance >= chainLength * kUnreachableBand )
		{
			const ae::Vec3 expected = anchor + ( target.value - anchor ).SafeNormalizeCopy() * chainLength;
			const float error = ( solvedBone->boneToModel.GetTranslation() - expected ).Length();
			consider( error, scenario.effectorTolerance, &report.maxEffectorError, "straighten", solvedBone->name.c_str() );
		}
	}

	// Orientation targets
	for( const auto& target : orientations )
	{
		if( target.key >= solvedPose.GetBoneCount() || !subtree[ target.key ] )
		{
			continue;
		}
		const ae::Bone* solvedBone = solvedPose.GetBoneByIndex( target.key );
		const float error = QuaternionAngleDiff( solvedBone->boneToModel.GetRotation(), target.value );
		consider( error, scenario.effectorOriTolerance, &report.maxEffectorOriError, "effector orientation", solvedBone->name.c_str() );
	}

	// Bones expected to hold their solve-input position
	for( const uint32_t boneIndex : scenario.staticBones )
	{
		const float error = ( solvedPose.GetBoneByIndex( boneIndex )->boneToModel.GetTranslation() - inputPose.GetBoneByIndex( boneIndex )->boneToModel.GetTranslation() ).Length();
		consider( error, scenario.staticDriftTolerance, &report.maxStaticDrift, "static drift", solvedPose.GetBoneByIndex( boneIndex )->name.c_str() );
	}

	// Frame-to-frame continuity. Targets move smoothly, so joints teleporting
	// or orientations flipping between consecutive frames indicate spin or
	// quadrant instability even when each individual pose looks valid.
	if( prevSolvedPose )
	{
		for( uint32_t i = 1; i < solvedPose.GetBoneCount(); i++ )
		{
			if( !subtree[ i ] )
			{
				continue;
			}
			const ae::Bone* solvedBone = solvedPose.GetBoneByIndex( i );
			const ae::Bone* prevBone = prevSolvedPose->GetBoneByIndex( i );
			const float positionDelta = ( solvedBone->boneToModel.GetTranslation() - prevBone->boneToModel.GetTranslation() ).Length();
			consider( positionDelta, scenario.continuityTolerance, &report.maxContinuityDelta, "continuity", solvedBone->name.c_str() );
			const float orientationDelta = QuaternionAngleDiff( solvedBone->boneToModel.GetRotation(), prevBone->boneToModel.GetRotation() );
			consider( orientationDelta, scenario.continuityOriTolerance, &report.maxContinuityOriDelta, "orientation continuity", solvedBone->name.c_str() );
		}
	}

	// Mirrored bone pairs must land mirrored across the model yz plane
	for( const ae::Int2 pair : scenario.mirrorPairs )
	{
		const ae::Vec3 left = solvedPose.GetBoneByIndex( pair.x )->boneToModel.GetTranslation();
		const ae::Vec3 right = solvedPose.GetBoneByIndex( pair.y )->boneToModel.GetTranslation();
		const float error = ( ae::Vec3( -left.x, left.y, left.z ) - right ).Length();
		const ae::Str128 pairName = ae::Str128::Format( "#-#", solvedPose.GetBoneByIndex( pair.x )->name, solvedPose.GetBoneByIndex( pair.y )->name );
		consider( error, scenario.mirrorTolerance, &report.maxMirrorError, "mirror", pairName.c_str() );
	}

	return report;
}

//------------------------------------------------------------------------------
// IKConvergenceTrend
//------------------------------------------------------------------------------
bool IKConvergenceTrend( const ae::Tag& tag, const IKScenario& scenario, uint32_t frame, const ae::Skeleton& inputPose, float slack, ae::Str256* messageOut )
{
	const uint32_t iterationCounts[] = { 1, 2, 4, 8 };
	float prevError = INFINITY;
	uint32_t prevIterations = 0;
	for( const uint32_t iterations : iterationCounts )
	{
		ae::Skeleton solved = tag;
		SolveFrame( tag, scenario, frame, iterations, inputPose, &solved, nullptr, ae::Matrix4::Identity(), 0.1f );
		const IKFrameReport report = IKEvaluateFrame( scenario, frame, inputPose, solved, nullptr );
		if( report.maxEffectorError > prevError + slack )
		{
			if( messageOut )
			{
				*messageOut = ae::Str256::Format( "'#' frame #: effector error # at # iterations vs # at # iterations", scenario.name, frame, report.maxEffectorError, iterations, prevError, prevIterations );
			}
			return false;
		}
		prevError = report.maxEffectorError;
		prevIterations = iterations;
	}
	return true;
}

//------------------------------------------------------------------------------
// IKScenarioRunner
//------------------------------------------------------------------------------
IKScenarioRunner::IKScenarioRunner( const ae::Tag& tag, const IKScenario* scenario ) :
	m_tag( tag ),
	m_scenario( scenario ),
	m_inputPose( tag ),
	m_solvedPose( tag ),
	m_prevSolvedPose( tag )
{
	Reset();
}

void IKScenarioRunner::Reset()
{
	m_hasSolved = false;
	m_lastFrame = 0;
	if( !m_scenario->bindPose.GetBoneCount() )
	{
		return; // Not built yet; call Reset() again after IKScenarioBuild()
	}
	m_inputPose.Initialize( &m_scenario->bindPose );
	m_solvedPose.Initialize( &m_scenario->bindPose );
	m_prevSolvedPose.Initialize( &m_scenario->bindPose );
}

IKFrameReport IKScenarioRunner::Step( uint32_t frame, uint32_t iterationCount, IKStartMode startMode, ae::DebugLines* debugLines, const ae::Matrix4& debugModelToWorld, float debugJointScale )
{
	const bool continuous = m_hasSolved && ( frame == m_lastFrame + 1 );
	if( m_hasSolved )
	{
		m_prevSolvedPose.Initialize( &m_solvedPose );
	}
	if( startMode == IKStartMode::FromBind || !m_hasSolved )
	{
		m_inputPose.Initialize( &m_scenario->bindPose );
	}
	else
	{
		m_inputPose.Initialize( &m_solvedPose );
	}
	SolveFrame( m_tag, *m_scenario, frame, iterationCount, m_inputPose, &m_solvedPose, debugLines, debugModelToWorld, debugJointScale );
	m_lastFrame = frame;
	m_hasSolved = true;
	return IKEvaluateFrame( *m_scenario, frame, m_inputPose, m_solvedPose, continuous ? &m_prevSolvedPose : nullptr );
}

//------------------------------------------------------------------------------
// Scenario builders
//------------------------------------------------------------------------------
namespace
{

void ResetScenario( IKScenario* s )
{
	const IKScenario defaults = ae::Tag( "ikreset" );
	s->name = "";
	s->description = "";
	s->frameCount = defaults.frameCount;
	s->rootBoneIndex = defaults.rootBoneIndex;
	s->rotationConstraints.Clear();
	s->distanceConstraints.Clear();
	s->targetFn = nullptr;
	s->boneLengthTolerance = defaults.boneLengthTolerance;
	s->distanceConstraintTolerance = defaults.distanceConstraintTolerance;
	s->rotationLimitTolerance = defaults.rotationLimitTolerance;
	s->twistLimitTolerance = defaults.twistLimitTolerance;
	s->effectorTolerance = defaults.effectorTolerance;
	s->effectorOriTolerance = defaults.effectorOriTolerance;
	s->staticDriftTolerance = defaults.staticDriftTolerance;
	s->mirrorTolerance = defaults.mirrorTolerance;
	s->continuityTolerance = defaults.continuityTolerance;
	s->continuityOriTolerance = defaults.continuityOriTolerance;
	s->checkStraighten = defaults.checkStraighten;
	s->checkConvergenceTrend = defaults.checkConvergenceTrend;
	s->staticBones.Clear();
	s->mirrorPairs.Clear();
}

void BuildTwoBoneReach( IKScenario* s )
{
	s->name = "TwoBoneReach";
	s->description = "Planar two bone arm sweeping a reachable circular target from a pinned shoulder";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.85f ) );
	const ae::Bone* shoulder = AddBoneWorld( &s->bindPose, mount, "shoulder", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* elbow = AddBoneWorld( &s->bindPose, shoulder, "elbow", ae::Vec3( 0.4f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, elbow, "hand", ae::Vec3( 0.8f, 0.0f, 1.0f ) );
	s->rootBoneIndex = shoulder->index;
	s->staticBones.Append( shoulder->index );
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		// The circle sits off the bind chain axis: a target crossing that
		// line flips which side the elbow folds to, an inherent cold-start
		// discontinuity covered separately by ColinearReach
		const float t = Cycle01( frame, s.frameCount );
		targets->Set( Idx( s, "hand" ), CirclePath( ae::Vec3( 0.45f, 0.15f, 1.0f ), ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.25f, t ) );
	};
}

void BuildColinearReach( IKScenario* s )
{
	BuildTwoBoneReach( s );
	s->name = "ColinearReach";
	s->description = "Target slides along the exact bind chain axis, always closer than full extension; the classic FABRIK fold singularity";
	s->effectorTolerance = 0.03f; // Deep folds converge slower than open reaches
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = PingPong01( Cycle01( frame, s.frameCount ) );
		targets->Set( Idx( s, "hand" ), ae::Vec3( ae::Lerp( 0.3f, 0.75f, t ), 0.0f, 1.0f ) );
	};
}

void BuildTwoBoneOriented( IKScenario* s )
{
	BuildTwoBoneReach( s );
	s->name = "TwoBoneOriented";
	s->description = "TwoBoneReach with arbitrary non-identity bind orientations; catches constraint frame derivation bugs";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.85f ), ae::Quaternion( ae::Vec3( 0.0f, 1.0f, 0.0f ), 0.6f ) );
	const ae::Bone* shoulder = AddBoneWorld( &s->bindPose, mount, "shoulder", ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Quaternion( ae::Vec3( 0.3f, 0.8f, 0.5f ).SafeNormalizeCopy(), 1.2f ) );
	const ae::Bone* elbow = AddBoneWorld( &s->bindPose, shoulder, "elbow", ae::Vec3( 0.4f, 0.0f, 1.0f ), ae::Quaternion( ae::Vec3( -0.5f, 0.2f, 0.8f ).SafeNormalizeCopy(), -0.8f ) );
	AddBoneWorld( &s->bindPose, elbow, "hand", ae::Vec3( 0.8f, 0.0f, 1.0f ), ae::Quaternion( ae::Vec3( 1.0f, 1.0f, -1.0f ).SafeNormalizeCopy(), 2.0f ) );
}

void BuildThreeBoneUnreachable( IKScenario* s )
{
	s->name = "ThreeBoneUnreachable";
	s->description = "Three bone chain with a target sweeping far out of reach; the chain must straighten toward it";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.7f ) );
	const ae::Bone* base = AddBoneWorld( &s->bindPose, mount, "base", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* j1 = AddBoneWorld( &s->bindPose, base, "j1", ae::Vec3( 0.3f, 0.0f, 1.0f ) );
	const ae::Bone* j2 = AddBoneWorld( &s->bindPose, j1, "j2", ae::Vec3( 0.6f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, j2, "j3", ae::Vec3( 0.9f, 0.0f, 1.0f ) );
	s->rootBoneIndex = base->index;
	s->staticBones.Append( base->index );
	s->checkStraighten = true;
	s->effectorTolerance = 0.03f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = PingPong01( Cycle01( frame, s.frameCount ) );
		targets->Set( Idx( s, "j3" ), ae::Lerp( ae::Vec3( 0.5f, 0.0f, 1.1f ), ae::Vec3( 2.2f, 0.6f, 1.4f ), t ) );
	};
}

void BuildRotationLimitQuadrants( IKScenario* s )
{
	s->name = "RotationLimitQuadrants";
	s->description = "Target sweeps a cone crossing every limit ellipse quadrant; solved joints must stay inside their asymmetric limits";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.3f ) );
	const ae::Bone* base = AddBoneWorld( &s->bindPose, mount, "base", ae::Vec3( 0.0f, 0.0f, 0.5f ) );
	const ae::Bone* j1 = AddBoneWorld( &s->bindPose, base, "j1", ae::Vec3( 0.0f, 0.0f, 0.9f ) );
	AddBoneWorld( &s->bindPose, j1, "j2", ae::Vec3( 0.0f, 0.0f, 1.3f ) );
	s->rootBoneIndex = base->index;
	s->staticBones.Append( base->index );
	s->rotationConstraints.Set( Idx( *s, "j1" ), { .rotationLimits = { 0.15f, 0.15f, 0.15f, 0.15f } } );
	s->rotationConstraints.Set( Idx( *s, "j2" ), { .rotationLimits = { 0.2f, 0.5f, 0.35f, 0.65f } } );
	s->effectorTolerance = -1.0f; // Limits intentionally prevent reaching
	s->checkConvergenceTrend = false;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		targets->Set( Idx( s, "j2" ), CirclePath( ae::Vec3( 0.0f, 0.0f, 1.15f ), ae::Vec3( 1.0f, 0.0f, 0.0f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), 0.35f, t ) );
	};
}

void BuildTwistChain( IKScenario* s )
{
	s->name = "TwistChain";
	s->description = "Orientation target twists the leaf of a vertical chain far past the per-joint twist limits";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.2f ) );
	const ae::Bone* b0 = AddBoneWorld( &s->bindPose, mount, "b0", ae::Vec3( 0.0f, 0.0f, 0.4f ) );
	const ae::Bone* b1 = AddBoneWorld( &s->bindPose, b0, "b1", ae::Vec3( 0.0f, 0.0f, 0.7f ) );
	const ae::Bone* b2 = AddBoneWorld( &s->bindPose, b1, "b2", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, b2, "b3", ae::Vec3( 0.0f, 0.0f, 1.3f ) );
	s->rootBoneIndex = b0->index;
	const ae::IKRotationConstraint twistLimited = { .rotationLimits = { 1.0f, 1.0f, 1.0f, 1.0f }, .twistLimits = { -0.3f, 0.3f } };
	s->rotationConstraints.Set( Idx( *s, "b1" ), twistLimited );
	s->rotationConstraints.Set( Idx( *s, "b2" ), twistLimited );
	s->rotationConstraints.Set( Idx( *s, "b3" ), twistLimited );
	s->effectorOriTolerance = -1.0f; // Twist limits should clip the demanded orientation
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* orientations )
	{
		const float t = Cycle01( frame, s.frameCount );
		const float angle = ae::Sin( t * ae::Pi * 2.0f ) * 2.5f;
		targets->Set( Idx( s, "b3" ), ae::Vec3( 0.0f, 0.0f, 1.3f ) );
		orientations->Set( Idx( s, "b3" ), ae::Quaternion( ae::Vec3( 0.0f, 0.0f, 1.0f ), angle ) );
	};
}

void BuildMultiChildSpine( IKScenario* s )
{
	s->name = "MultiChildSpine";
	s->description = "Spine with two arms and a head circling out of phase; exercises multi-child averaging and spin stability at the chest";
	s->bindPose.Initialize( 12 );
	const ae::Bone* hips = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "hips", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* spine = AddBoneWorld( &s->bindPose, hips, "spine", ae::Vec3( 0.0f, 0.0f, 1.25f ) );
	const ae::Bone* chest = AddBoneWorld( &s->bindPose, spine, "chest", ae::Vec3( 0.0f, 0.0f, 1.5f ) );
	const ae::Bone* neck = AddBoneWorld( &s->bindPose, chest, "neck", ae::Vec3( 0.0f, 0.0f, 1.65f ) );
	AddBoneWorld( &s->bindPose, neck, "head", ae::Vec3( 0.0f, 0.0f, 1.8f ) );
	const ae::Bone* shoulderL = AddBoneWorld( &s->bindPose, chest, "shoulderL", ae::Vec3( 0.2f, 0.0f, 1.5f ) );
	AddBoneWorld( &s->bindPose, shoulderL, "handL", ae::Vec3( 0.55f, 0.0f, 1.5f ) );
	const ae::Bone* shoulderR = AddBoneWorld( &s->bindPose, chest, "shoulderR", ae::Vec3( -0.2f, 0.0f, 1.5f ) );
	AddBoneWorld( &s->bindPose, shoulderR, "handR", ae::Vec3( -0.55f, 0.0f, 1.5f ) );
	s->rootBoneIndex = hips->index;
	s->effectorTolerance = 0.04f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		targets->Set( Idx( s, "hips" ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		targets->Set( Idx( s, "handL" ), CirclePath( ae::Vec3( 0.45f, 0.0f, 1.5f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.2f, t ) );
		targets->Set( Idx( s, "handR" ), CirclePath( ae::Vec3( -0.45f, 0.0f, 1.5f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.2f, t + 0.5f ) );
		targets->Set( Idx( s, "head" ), ae::Vec3( 0.0f, 0.1f * ae::Sin( t * ae::Pi * 2.0f ), 1.8f ) );
	};
}

void BuildDistancePair( IKScenario* s )
{
	s->name = "DistancePair";
	s->description = "Shoulder pair locked by distance constraints while hand targets pull wide apart and cross over";
	s->bindPose.Initialize( 12 );
	const ae::Bone* hips = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "hips", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* spine = AddBoneWorld( &s->bindPose, hips, "spine", ae::Vec3( 0.0f, 0.0f, 1.25f ) );
	const ae::Bone* chest = AddBoneWorld( &s->bindPose, spine, "chest", ae::Vec3( 0.0f, 0.0f, 1.5f ) );
	const ae::Bone* shoulderL = AddBoneWorld( &s->bindPose, chest, "shoulderL", ae::Vec3( 0.2f, 0.0f, 1.5f ) );
	const ae::Bone* elbowL = AddBoneWorld( &s->bindPose, shoulderL, "elbowL", ae::Vec3( 0.45f, 0.0f, 1.5f ) );
	AddBoneWorld( &s->bindPose, elbowL, "handL", ae::Vec3( 0.7f, 0.0f, 1.5f ) );
	const ae::Bone* shoulderR = AddBoneWorld( &s->bindPose, chest, "shoulderR", ae::Vec3( -0.2f, 0.0f, 1.5f ) );
	const ae::Bone* elbowR = AddBoneWorld( &s->bindPose, shoulderR, "elbowR", ae::Vec3( -0.45f, 0.0f, 1.5f ) );
	AddBoneWorld( &s->bindPose, elbowR, "handR", ae::Vec3( -0.7f, 0.0f, 1.5f ) );
	s->rootBoneIndex = hips->index;
	s->distanceConstraints.Append( { (int32_t)Idx( *s, "shoulderL" ), (int32_t)Idx( *s, "shoulderR" ) } );
	s->distanceConstraints.Append( { (int32_t)Idx( *s, "spine" ), (int32_t)Idx( *s, "shoulderL" ), 0.95f, 1.1f } );
	s->distanceConstraints.Append( { (int32_t)Idx( *s, "spine" ), (int32_t)Idx( *s, "shoulderR" ), 0.95f, 1.1f } );
	s->effectorTolerance = 0.05f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = PingPong01( Cycle01( frame, s.frameCount ) );
		targets->Set( Idx( s, "hips" ), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		targets->Set( Idx( s, "handL" ), ae::Lerp( ae::Vec3( 0.7f, 0.0f, 1.5f ), ae::Vec3( -0.25f, 0.4f, 1.55f ), t ) );
		targets->Set( Idx( s, "handR" ), ae::Lerp( ae::Vec3( -0.7f, 0.0f, 1.5f ), ae::Vec3( 0.25f, 0.4f, 1.45f ), t ) );
	};
}

void BuildMidChainSteer( IKScenario* s )
{
	s->name = "MidChainSteer";
	s->description = "Mid-chain elbow target orbits the base-to-effector axis while the effector holds; both targets are exactly compatible";
	s->bindPose.Initialize( 8 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.7f ) );
	const ae::Bone* base = AddBoneWorld( &s->bindPose, mount, "base", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* j1 = AddBoneWorld( &s->bindPose, base, "j1", ae::Vec3( 0.3f, 0.0f, 1.0f ) );
	const ae::Bone* j2 = AddBoneWorld( &s->bindPose, j1, "j2", ae::Vec3( 0.6f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, j2, "j3", ae::Vec3( 0.9f, 0.0f, 1.0f ) );
	s->rootBoneIndex = base->index;
	s->staticBones.Append( base->index );
	s->effectorTolerance = 0.03f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		const ae::Vec3 basePos( 0.0f, 0.0f, 1.0f );
		const ae::Vec3 leafTarget( 0.55f, 0.0f, 1.25f );
		// The elbow circle where both chain segments keep their bind lengths
		const float len0 = 0.6f; // base to j2 through j1, fully extended
		const float len1 = 0.3f; // j2 to j3
		const ae::Vec3 toLeaf = leafTarget - basePos;
		const float d = toLeaf.Length();
		const ae::Vec3 axis = toLeaf / d;
		const float a = ( d * d + len0 * len0 - len1 * len1 ) / ( 2.0f * d );
		const float r = ae::Sqrt( ae::Max( 0.0f, len0 * len0 - a * a ) );
		ae::Vec3 u, v;
		ae::IK::GetLimitBasis( axis, &u, &v );
		const float angle = t * ae::Pi * 2.0f;
		targets->Set( Idx( s, "j3" ), leafTarget );
		targets->Set( Idx( s, "j2" ), basePos + axis * a + ( u * ae::Cos( angle ) + v * ae::Sin( angle ) ) * r );
	};
}

void BuildMirroredLimits( IKScenario* s )
{
	s->name = "MirroredLimits";
	s->description = "Symmetric rotation-limited arms tracking mirrored targets; solved poses must stay mirror images";
	s->bindPose.Initialize( 12 );
	const ae::Bone* mount = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "mount", ae::Vec3( 0.0f, 0.0f, 0.8f ) );
	const ae::Bone* chest = AddBoneWorld( &s->bindPose, mount, "chest", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, chest, "neck", ae::Vec3( 0.0f, 0.0f, 1.2f ) );
	const ae::Bone* shoulderL = AddBoneWorld( &s->bindPose, chest, "shoulderL", ae::Vec3( 0.2f, 0.0f, 1.0f ) );
	const ae::Bone* elbowL = AddBoneWorld( &s->bindPose, shoulderL, "elbowL", ae::Vec3( 0.5f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, elbowL, "handL", ae::Vec3( 0.8f, 0.0f, 1.0f ) );
	const ae::Bone* shoulderR = AddBoneWorld( &s->bindPose, chest, "shoulderR", ae::Vec3( -0.2f, 0.0f, 1.0f ) );
	const ae::Bone* elbowR = AddBoneWorld( &s->bindPose, shoulderR, "elbowR", ae::Vec3( -0.5f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, elbowR, "handR", ae::Vec3( -0.8f, 0.0f, 1.0f ) );
	s->rootBoneIndex = chest->index;
	// Mirror-fair limits: opposing quadrants match, so the ellipse is symmetric
	// regardless of which world axis GetLimitBasis() picks for each side
	const ae::IKRotationConstraint symmetric = { .rotationLimits = { 0.4f, 0.7f, 0.4f, 0.7f } };
	s->rotationConstraints.Set( Idx( *s, "elbowL" ), symmetric );
	s->rotationConstraints.Set( Idx( *s, "elbowR" ), symmetric );
	s->effectorTolerance = -1.0f; // Limits intentionally interfere
	s->checkConvergenceTrend = false;
	s->mirrorPairs.Append( ae::Int2( (int32_t)Idx( *s, "shoulderL" ), (int32_t)Idx( *s, "shoulderR" ) ) );
	s->mirrorPairs.Append( ae::Int2( (int32_t)Idx( *s, "elbowL" ), (int32_t)Idx( *s, "elbowR" ) ) );
	s->mirrorPairs.Append( ae::Int2( (int32_t)Idx( *s, "handL" ), (int32_t)Idx( *s, "handR" ) ) );
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		const ae::Vec3 left = CirclePath( ae::Vec3( 0.55f, 0.2f, 1.1f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.25f, t );
		targets->Set( Idx( s, "handL" ), left );
		targets->Set( Idx( s, "handR" ), ae::Vec3( -left.x, left.y, left.z ) );
	};
}

void BuildHumanoid( IKScenario* s )
{
	s->name = "Humanoid";
	s->description = "Minimal humanoid with the 17_IK distance constraint web; hands circle, head sways, feet stay planted";
	s->bindPose.Initialize( 24 );
	const ae::Bone* hips = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "hips", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	const ae::Bone* spine = AddBoneWorld( &s->bindPose, hips, "spine", ae::Vec3( 0.0f, 0.0f, 1.12f ) );
	const ae::Bone* spine1 = AddBoneWorld( &s->bindPose, spine, "spine1", ae::Vec3( 0.0f, 0.0f, 1.24f ) );
	const ae::Bone* spine2 = AddBoneWorld( &s->bindPose, spine1, "spine2", ae::Vec3( 0.0f, 0.0f, 1.36f ) );
	const ae::Bone* neck = AddBoneWorld( &s->bindPose, spine2, "neck", ae::Vec3( 0.0f, 0.0f, 1.48f ) );
	AddBoneWorld( &s->bindPose, neck, "head", ae::Vec3( 0.0f, 0.0f, 1.6f ) );
	const ae::Bone* shoulderL = AddBoneWorld( &s->bindPose, spine2, "shoulderL", ae::Vec3( 0.1f, 0.0f, 1.4f ) );
	const ae::Bone* armL = AddBoneWorld( &s->bindPose, shoulderL, "armL", ae::Vec3( 0.22f, 0.0f, 1.4f ) );
	const ae::Bone* foreArmL = AddBoneWorld( &s->bindPose, armL, "foreArmL", ae::Vec3( 0.45f, 0.0f, 1.4f ) );
	AddBoneWorld( &s->bindPose, foreArmL, "handL", ae::Vec3( 0.68f, 0.0f, 1.4f ) );
	const ae::Bone* shoulderR = AddBoneWorld( &s->bindPose, spine2, "shoulderR", ae::Vec3( -0.1f, 0.0f, 1.4f ) );
	const ae::Bone* armR = AddBoneWorld( &s->bindPose, shoulderR, "armR", ae::Vec3( -0.22f, 0.0f, 1.4f ) );
	const ae::Bone* foreArmR = AddBoneWorld( &s->bindPose, armR, "foreArmR", ae::Vec3( -0.45f, 0.0f, 1.4f ) );
	AddBoneWorld( &s->bindPose, foreArmR, "handR", ae::Vec3( -0.68f, 0.0f, 1.4f ) );
	const ae::Bone* upLegL = AddBoneWorld( &s->bindPose, hips, "upLegL", ae::Vec3( 0.09f, 0.0f, 0.95f ) );
	const ae::Bone* legL = AddBoneWorld( &s->bindPose, upLegL, "legL", ae::Vec3( 0.09f, 0.0f, 0.55f ) );
	AddBoneWorld( &s->bindPose, legL, "footL", ae::Vec3( 0.09f, 0.0f, 0.12f ) );
	const ae::Bone* upLegR = AddBoneWorld( &s->bindPose, hips, "upLegR", ae::Vec3( -0.09f, 0.0f, 0.95f ) );
	const ae::Bone* legR = AddBoneWorld( &s->bindPose, upLegR, "legR", ae::Vec3( -0.09f, 0.0f, 0.55f ) );
	AddBoneWorld( &s->bindPose, legR, "footR", ae::Vec3( -0.09f, 0.0f, 0.12f ) );
	s->rootBoneIndex = hips->index;
	auto constrain = [ s ]( const char* n0, const char* n1, float compression, float stretch )
	{
		s->distanceConstraints.Append( { (int32_t)Idx( *s, n0 ), (int32_t)Idx( *s, n1 ), compression, stretch } );
	};
	// Head
	constrain( "head", "shoulderL", 0.85f, 1.15f );
	constrain( "head", "shoulderR", 0.85f, 1.15f );
	constrain( "neck", "armL", 0.85f, 1.15f );
	constrain( "neck", "armR", 0.85f, 1.15f );
	// Collarbone
	constrain( "shoulderL", "shoulderR", 1.0f, 1.0f );
	constrain( "armL", "armR", 1.0f, 1.1f );
	constrain( "spine2", "armL", 1.0f, 1.25f );
	constrain( "spine2", "armR", 1.0f, 1.25f );
	// Spine
	constrain( "hips", "spine1", 1.0f, 1.05f );
	constrain( "spine", "spine2", 1.0f, 1.05f );
	constrain( "spine1", "neck", 1.0f, 1.05f );
	constrain( "spine2", "head", 1.0f, 1.05f );
	// Torso
	constrain( "armL", "upLegL", 0.9f, 1.1f );
	constrain( "armR", "upLegR", 0.9f, 1.1f );
	constrain( "spine1", "armL", 0.9f, 1.1f );
	constrain( "spine1", "armR", 0.9f, 1.1f );
	constrain( "spine2", "upLegL", 0.9f, 1.1f );
	constrain( "spine2", "upLegR", 0.9f, 1.1f );
	// Hips
	constrain( "upLegL", "upLegR", 1.0f, 1.0f );
	constrain( "spine", "upLegL", 1.0f, 1.2f );
	constrain( "spine", "upLegR", 1.0f, 1.2f );
	constrain( "spine1", "upLegL", 0.9f, 1.0f );
	constrain( "spine1", "upLegR", 0.9f, 1.0f );
	s->effectorTolerance = 0.05f;
	s->distanceConstraintTolerance = 0.08f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		const float sway = ae::Sin( t * ae::Pi * 2.0f );
		targets->Set( Idx( s, "hips" ), ae::Vec3( 0.05f * sway, 0.0f, 1.0f + 0.02f * ae::Sin( t * ae::Pi * 4.0f ) ) );
		targets->Set( Idx( s, "handL" ), CirclePath( ae::Vec3( 0.4f, 0.2f, 1.3f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.2f, t ) );
		targets->Set( Idx( s, "handR" ), CirclePath( ae::Vec3( -0.4f, 0.2f, 1.3f ), ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.0f, 1.0f ), 0.2f, t + 0.35f ) );
		targets->Set( Idx( s, "head" ), ae::Vec3( 0.06f * sway, 0.04f * ae::Cos( t * ae::Pi * 2.0f ), 1.6f ) );
		targets->Set( Idx( s, "footL" ), ae::Vec3( 0.09f, 0.0f, 0.12f ) );
		targets->Set( Idx( s, "footR" ), ae::Vec3( -0.09f, 0.0f, 0.12f ) );
	};
}

void BuildQuadruped( IKScenario* s )
{
	s->name = "Quadruped";
	s->description = "Horizontal spine on four trotting legs with shoulder/hip pair loops and side braces; the free body follows the feet";
	s->bindPose.Initialize( 24 );
	const ae::Bone* spineMid = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "spineMid", ae::Vec3( 0.0f, 0.0f, 0.55f ) );
	const ae::Bone* chest = AddBoneWorld( &s->bindPose, spineMid, "chest", ae::Vec3( 0.0f, 0.3f, 0.55f ) );
	const ae::Bone* neck = AddBoneWorld( &s->bindPose, chest, "neck", ae::Vec3( 0.0f, 0.5f, 0.65f ) );
	AddBoneWorld( &s->bindPose, neck, "head", ae::Vec3( 0.0f, 0.62f, 0.75f ) );
	const ae::Bone* rear = AddBoneWorld( &s->bindPose, spineMid, "rear", ae::Vec3( 0.0f, -0.3f, 0.55f ) );
	const ae::Bone* tail1 = AddBoneWorld( &s->bindPose, rear, "tail1", ae::Vec3( 0.0f, -0.5f, 0.6f ) );
	AddBoneWorld( &s->bindPose, tail1, "tail2", ae::Vec3( 0.0f, -0.65f, 0.62f ) );
	const char* legNames[ 4 ][ 3 ] = {
		{ "hipFL", "kneeFL", "footFL" },
		{ "hipFR", "kneeFR", "footFR" },
		{ "hipBL", "kneeBL", "footBL" },
		{ "hipBR", "kneeBR", "footBR" }
	};
	for( uint32_t i = 0; i < 4; i++ )
	{
		const float side = ( i % 2 == 0 ) ? 1.0f : -1.0f;
		const bool front = ( i < 2 );
		const float y = front ? 0.3f : -0.3f;
		const ae::Bone* attach = front ? chest : rear;
		const ae::Bone* hip = AddBoneWorld( &s->bindPose, attach, legNames[ i ][ 0 ], ae::Vec3( side * 0.12f, y, 0.5f ) );
		const ae::Bone* knee = AddBoneWorld( &s->bindPose, hip, legNames[ i ][ 1 ], ae::Vec3( side * 0.12f, y, 0.25f ) );
		AddBoneWorld( &s->bindPose, knee, legNames[ i ][ 2 ], ae::Vec3( side * 0.12f, y + 0.02f, 0.0f ) );
	}
	s->rootBoneIndex = spineMid->index;
	auto constrain = [ s ]( const char* n0, const char* n1, float compression, float stretch )
	{
		s->distanceConstraints.Append( { (int32_t)Idx( *s, n0 ), (int32_t)Idx( *s, n1 ), compression, stretch } );
	};
	constrain( "chest", "rear", 0.95f, 1.05f );
	constrain( "hipFL", "hipFR", 1.0f, 1.0f );
	constrain( "hipBL", "hipBR", 1.0f, 1.0f );
	constrain( "hipFL", "hipBL", 0.95f, 1.05f );
	constrain( "hipFR", "hipBR", 0.95f, 1.05f );
	constrain( "chest", "hipBL", 0.95f, 1.1f );
	constrain( "chest", "hipBR", 0.95f, 1.1f );
	s->effectorTolerance = 0.05f;
	s->distanceConstraintTolerance = 0.08f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		const char* feet[ 4 ] = { "footFL", "footFR", "footBL", "footBR" };
		const ae::Vec3 bindFeet[ 4 ] = {
			ae::Vec3( 0.12f, 0.32f, 0.0f ), ae::Vec3( -0.12f, 0.32f, 0.0f ),
			ae::Vec3( 0.12f, -0.28f, 0.0f ), ae::Vec3( -0.12f, -0.28f, 0.0f )
		};
		for( uint32_t i = 0; i < 4; i++ )
		{
			// Trot: diagonal pairs (FL,BR) and (FR,BL) step in opposite phase
			const float phase = ( ( i == 0 || i == 3 ) ? 0.0f : 0.5f );
			const float cycle = ( t * 2.0f + phase ) * ae::Pi * 2.0f;
			const float lift = ae::Max( 0.0f, ae::Sin( cycle ) ) * 0.07f;
			const float stride = ae::Cos( cycle ) * 0.06f;
			targets->Set( Idx( s, feet[ i ] ), bindFeet[ i ] + ae::Vec3( 0.0f, stride, lift ) );
		}
		targets->Set( Idx( s, "head" ), ae::Vec3( 0.03f * ae::Sin( t * ae::Pi * 4.0f ), 0.62f, 0.75f + 0.03f * ae::Sin( t * ae::Pi * 8.0f ) ) );
	};
}

void BuildBird( IKScenario* s )
{
	s->name = "Bird";
	s->description = "Perched bird flapping both wings with a pecking head; shoulder and leg pair loops keep the free body coherent";
	s->bindPose.Initialize( 20 );
	const ae::Bone* body = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "body", ae::Vec3( 0.0f, 0.0f, 1.0f ) );
	AddBoneWorld( &s->bindPose, body, "tail", ae::Vec3( 0.0f, -0.28f, 0.98f ) );
	const ae::Bone* neck1 = AddBoneWorld( &s->bindPose, body, "neck1", ae::Vec3( 0.0f, 0.15f, 1.08f ) );
	const ae::Bone* neck2 = AddBoneWorld( &s->bindPose, neck1, "neck2", ae::Vec3( 0.0f, 0.26f, 1.2f ) );
	AddBoneWorld( &s->bindPose, neck2, "head", ae::Vec3( 0.0f, 0.32f, 1.34f ) );
	const ae::Bone* shoulderL = AddBoneWorld( &s->bindPose, body, "shoulderL", ae::Vec3( 0.1f, 0.05f, 1.05f ) );
	const ae::Bone* wing1L = AddBoneWorld( &s->bindPose, shoulderL, "wing1L", ae::Vec3( 0.4f, 0.05f, 1.12f ) );
	const ae::Bone* wing2L = AddBoneWorld( &s->bindPose, wing1L, "wing2L", ae::Vec3( 0.75f, 0.05f, 1.12f ) );
	AddBoneWorld( &s->bindPose, wing2L, "tipL", ae::Vec3( 1.05f, 0.05f, 1.06f ) );
	const ae::Bone* shoulderR = AddBoneWorld( &s->bindPose, body, "shoulderR", ae::Vec3( -0.1f, 0.05f, 1.05f ) );
	const ae::Bone* wing1R = AddBoneWorld( &s->bindPose, shoulderR, "wing1R", ae::Vec3( -0.4f, 0.05f, 1.12f ) );
	const ae::Bone* wing2R = AddBoneWorld( &s->bindPose, wing1R, "wing2R", ae::Vec3( -0.75f, 0.05f, 1.12f ) );
	AddBoneWorld( &s->bindPose, wing2R, "tipR", ae::Vec3( -1.05f, 0.05f, 1.06f ) );
	const ae::Bone* legL = AddBoneWorld( &s->bindPose, body, "legL", ae::Vec3( 0.07f, -0.05f, 0.75f ) );
	AddBoneWorld( &s->bindPose, legL, "footL", ae::Vec3( 0.07f, -0.02f, 0.5f ) );
	const ae::Bone* legR = AddBoneWorld( &s->bindPose, body, "legR", ae::Vec3( -0.07f, -0.05f, 0.75f ) );
	AddBoneWorld( &s->bindPose, legR, "footR", ae::Vec3( -0.07f, -0.02f, 0.5f ) );
	s->rootBoneIndex = body->index;
	auto constrain = [ s ]( const char* n0, const char* n1, float compression, float stretch )
	{
		s->distanceConstraints.Append( { (int32_t)Idx( *s, n0 ), (int32_t)Idx( *s, n1 ), compression, stretch } );
	};
	constrain( "shoulderL", "shoulderR", 1.0f, 1.0f );
	constrain( "shoulderL", "tail", 0.9f, 1.1f );
	constrain( "shoulderR", "tail", 0.9f, 1.1f );
	constrain( "legL", "legR", 1.0f, 1.0f );
	constrain( "shoulderL", "legL", 0.9f, 1.1f );
	constrain( "shoulderR", "legR", 0.9f, 1.1f );
	s->effectorTolerance = 0.05f;
	s->distanceConstraintTolerance = 0.08f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		const float flap = ae::Sin( t * ae::Pi * 4.0f ); // Two flaps per loop
		const float tipZ = 1.06f + 0.4f * flap;
		const float tipX = 1.0f - 0.12f * ae::Max( 0.0f, flap );
		targets->Set( Idx( s, "tipL" ), ae::Vec3( tipX, 0.05f, tipZ ) );
		targets->Set( Idx( s, "tipR" ), ae::Vec3( -tipX, 0.05f, tipZ ) );
		targets->Set( Idx( s, "footL" ), ae::Vec3( 0.07f, -0.02f, 0.5f ) );
		targets->Set( Idx( s, "footR" ), ae::Vec3( -0.07f, -0.02f, 0.5f ) );
		const float peck = ae::Max( 0.0f, ae::Sin( t * ae::Pi * 6.0f ) );
		targets->Set( Idx( s, "head" ), ae::Vec3( 0.0f, 0.32f + 0.1f * peck, 1.34f - 0.12f * peck ) );
	};
}

void BuildSpider( IKScenario* s )
{
	s->name = "Spider";
	s->description = "Eight radial legs stepping in alternating tetrapod groups; a hip ring loop and abdomen braces hold the free body's shape";
	s->bindPose.Initialize( 32 );
	const ae::Bone* body = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "body", ae::Vec3( 0.0f, 0.0f, 0.35f ) );
	AddBoneWorld( &s->bindPose, body, "abdomen", ae::Vec3( 0.0f, -0.3f, 0.42f ) );
	AddBoneWorld( &s->bindPose, body, "head", ae::Vec3( 0.0f, 0.18f, 0.35f ) );
	for( uint32_t i = 0; i < 8; i++ )
	{
		const float angle = ae::Pi * 2.0f * ( 0.0625f + i / 8.0f ); // Evenly spaced, avoiding the y axis
		const ae::Vec2 dir( ae::Cos( angle ), ae::Sin( angle ) );
		const ae::Str32 hipName = ae::Str32::Format( "hip#", i );
		const ae::Str32 kneeName = ae::Str32::Format( "knee#", i );
		const ae::Str32 footName = ae::Str32::Format( "foot#", i );
		const ae::Bone* hip = AddBoneWorld( &s->bindPose, body, hipName.c_str(), ae::Vec3( dir.x * 0.12f, dir.y * 0.12f, 0.38f ) );
		const ae::Bone* knee = AddBoneWorld( &s->bindPose, hip, kneeName.c_str(), ae::Vec3( dir.x * 0.32f, dir.y * 0.32f, 0.5f ) );
		AddBoneWorld( &s->bindPose, knee, footName.c_str(), ae::Vec3( dir.x * 0.52f, dir.y * 0.52f, 0.0f ) );
	}
	s->rootBoneIndex = body->index;
	for( uint32_t i = 0; i < 8; i++ )
	{
		// Ring loop between adjacent hips maintains the radial layout
		const ae::Str32 hip0 = ae::Str32::Format( "hip#", i );
		const ae::Str32 hip1 = ae::Str32::Format( "hip#", ( i + 1 ) % 8 );
		s->distanceConstraints.Append( { (int32_t)Idx( *s, hip0.c_str() ), (int32_t)Idx( *s, hip1.c_str() ), 0.95f, 1.05f } );
	}
	s->distanceConstraints.Append( { (int32_t)Idx( *s, "hip2" ), (int32_t)Idx( *s, "abdomen" ), 0.9f, 1.1f } );
	s->distanceConstraints.Append( { (int32_t)Idx( *s, "hip5" ), (int32_t)Idx( *s, "abdomen" ), 0.9f, 1.1f } );
	s->effectorTolerance = 0.05f;
	s->distanceConstraintTolerance = 0.08f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		for( uint32_t i = 0; i < 8; i++ )
		{
			const float angle = ae::Pi * 2.0f * ( 0.0625f + i / 8.0f );
			const ae::Vec2 dir( ae::Cos( angle ), ae::Sin( angle ) );
			const ae::Vec3 bindFoot( dir.x * 0.52f, dir.y * 0.52f, 0.0f );
			// Alternating tetrapod gait: even and odd legs swing in opposite phase
			const float phase = ( i % 2 == 0 ) ? 0.0f : 0.5f;
			const float cycle = ( t * 2.0f + phase ) * ae::Pi * 2.0f;
			const float lift = ae::Max( 0.0f, ae::Sin( cycle ) ) * 0.05f;
			const float stride = ae::Cos( cycle ) * 0.05f;
			targets->Set( Idx( s, ae::Str32::Format( "foot#", i ).c_str() ), bindFoot + ae::Vec3( 0.0f, stride, lift ) );
		}
	};
}

void BuildCentipede( IKScenario* s )
{
	s->name = "Centipede";
	s->description = "Seventeen segment chain rooted mid-body with bracing loops every other segment; the head serpentines while the tail holds";
	s->bindPose.Initialize( 20 );
	// Authored from the middle outwards: seg8 is the root, seg7..seg0 run to
	// the head and seg9..seg16 run to the tail
	const ae::Bone* seg8 = AddBoneWorld( &s->bindPose, s->bindPose.GetRoot(), "seg8", ae::Vec3( 0.0f, 0.0f, 0.12f ) );
	const ae::Bone* parent = seg8;
	for( int32_t i = 7; i >= 0; i-- )
	{
		const float y = ( 8 - i ) * 0.1f;
		parent = AddBoneWorld( &s->bindPose, parent, ae::Str32::Format( "seg#", i ).c_str(), ae::Vec3( 0.0f, y, 0.12f ) );
	}
	parent = seg8;
	for( int32_t i = 9; i <= 16; i++ )
	{
		const float y = ( 8 - i ) * 0.1f;
		parent = AddBoneWorld( &s->bindPose, parent, ae::Str32::Format( "seg#", i ).c_str(), ae::Vec3( 0.0f, y, 0.12f ) );
	}
	s->rootBoneIndex = seg8->index;
	for( uint32_t i = 0; i + 2 <= 16; i++ )
	{
		const ae::Str32 seg0 = ae::Str32::Format( "seg#", i );
		const ae::Str32 seg2 = ae::Str32::Format( "seg#", i + 2 );
		s->distanceConstraints.Append( { (int32_t)Idx( *s, seg0.c_str() ), (int32_t)Idx( *s, seg2.c_str() ), 0.9f, 1.05f } );
	}
	s->effectorTolerance = 0.04f;
	s->distanceConstraintTolerance = 0.08f;
	s->targetFn = []( const IKScenario& s, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targets, ae::Map< uint32_t, ae::Quaternion >* )
	{
		const float t = Cycle01( frame, s.frameCount );
		targets->Set( Idx( s, "seg0" ), ae::Vec3( 0.3f * ae::Sin( t * ae::Pi * 2.0f ), 0.8f - 0.12f * ae::Sin( t * ae::Pi * 4.0f ), 0.12f ) );
		targets->Set( Idx( s, "seg16" ), ae::Vec3( 0.0f, -0.8f, 0.12f ) );
	};
}

typedef void ( *BuildFn )( IKScenario* );
const BuildFn kScenarioBuilders[] =
{
	BuildTwoBoneReach,
	BuildTwoBoneOriented,
	BuildColinearReach,
	BuildThreeBoneUnreachable,
	BuildRotationLimitQuadrants,
	BuildTwistChain,
	BuildMultiChildSpine,
	BuildDistancePair,
	BuildMidChainSteer,
	BuildMirroredLimits,
	BuildHumanoid,
	BuildQuadruped,
	BuildBird,
	BuildSpider,
	BuildCentipede
};

} // namespace

uint32_t IKScenarioCount()
{
	return countof( kScenarioBuilders );
}

void IKScenarioBuild( uint32_t index, IKScenario* scenarioOut )
{
	AE_ASSERT( index < IKScenarioCount() );
	ResetScenario( scenarioOut );
	kScenarioBuilders[ index ]( scenarioOut );
}

//------------------------------------------------------------------------------
// Catch2 tests. Only compiled into the unit test target: the gallery example
// (examples/30_IKGallery.cpp) links this file without catch2 on its include
// path, so these sections vanish there.
//------------------------------------------------------------------------------
#if __has_include( <catch2/catch_test_macros.hpp> )
#include <catch2/catch_test_macros.hpp>

namespace
{
const ae::Tag kIKTestTag = "iktest";
const uint32_t kTestIterations = 8;
}

TEST_CASE( "ik scenario invariants hold when solving from the bind pose", "[ae::IK]" )
{
	for( uint32_t i = 0; i < IKScenarioCount(); i++ )
	{
		IKScenario scenario = kIKTestTag;
		IKScenarioBuild( i, &scenario );
		DYNAMIC_SECTION( scenario.name.c_str() )
		{
			IKScenarioRunner runner( kIKTestTag, &scenario );
			for( uint32_t frame = 0; frame < scenario.frameCount; frame++ )
			{
				const IKFrameReport report = runner.Step( frame, kTestIterations, IKStartMode::FromBind );
				INFO( scenario.name.c_str() << " frame " << frame << ": " << report.worstCheck.c_str() );
				REQUIRE( report.pass );
			}
		}
	}
}

// Twist clamping is currently commented out inside ae::IK::Run(), so twist
// violations are expected until it's re-enabled
TEST_CASE( "ik twist limits are respected", "[ae::IK][!mayfail]" )
{
	for( uint32_t i = 0; i < IKScenarioCount(); i++ )
	{
		IKScenario scenario = kIKTestTag;
		IKScenarioBuild( i, &scenario );
		if( !scenario.rotationConstraints.Length() || scenario.twistLimitTolerance < 0.0f )
		{
			continue;
		}
		DYNAMIC_SECTION( scenario.name.c_str() )
		{
			IKScenarioRunner runner( kIKTestTag, &scenario );
			for( uint32_t frame = 0; frame < scenario.frameCount; frame++ )
			{
				const IKFrameReport report = runner.Step( frame, kTestIterations, IKStartMode::FromBind );
				INFO( scenario.name.c_str() << " frame " << frame << ": max twist violation " << report.maxTwistLimitError );
				REQUIRE( report.twistPass );
			}
		}
	}
}

TEST_CASE( "ik effector error is non-increasing with iteration count", "[ae::IK]" )
{
	for( uint32_t i = 0; i < IKScenarioCount(); i++ )
	{
		IKScenario scenario = kIKTestTag;
		IKScenarioBuild( i, &scenario );
		if( !scenario.checkConvergenceTrend )
		{
			continue;
		}
		DYNAMIC_SECTION( scenario.name.c_str() )
		{
			ae::Skeleton input = kIKTestTag;
			input.Initialize( &scenario.bindPose );
			for( uint32_t frame = 0; frame < scenario.frameCount; frame += 16 )
			{
				ae::Str256 message;
				const bool trend = IKConvergenceTrend( kIKTestTag, scenario, frame, input, 0.005f, &message );
				INFO( message.c_str() );
				REQUIRE( trend );
			}
		}
	}
}

// Warm starting is the intended future direction for ae::IK but is not
// currently exercised by any example; failures here document the gap. Note
// that distance constraint rest lengths derive from the solve-input pose, so
// they drift when feeding solved poses back in.
TEST_CASE( "ik scenario invariants hold when solving from the previous pose", "[ae::IK][!mayfail]" )
{
	for( uint32_t i = 0; i < IKScenarioCount(); i++ )
	{
		IKScenario scenario = kIKTestTag;
		IKScenarioBuild( i, &scenario );
		DYNAMIC_SECTION( scenario.name.c_str() )
		{
			IKScenarioRunner runner( kIKTestTag, &scenario );
			for( uint32_t frame = 0; frame < scenario.frameCount; frame++ )
			{
				const IKFrameReport report = runner.Step( frame, kTestIterations, IKStartMode::FromPrevious );
				INFO( scenario.name.c_str() << " frame " << frame << ": " << report.worstCheck.c_str() );
				REQUIRE( report.pass );
			}
		}
	}
}

TEST_CASE( "ik limit basis is orthonormal and deterministic", "[ae::IK]" )
{
	const ae::Vec3 primaries[] =
	{
		ae::Vec3( 1.0f, 0.0f, 0.0f ),
		ae::Vec3( 0.0f, 0.0f, 1.0f ),
		ae::Vec3( 0.577f, 0.577f, 0.577f ),
		ae::Vec3( -0.2f, 0.9f, 0.4f ).SafeNormalizeCopy()
	};
	for( const ae::Vec3 primary : primaries )
	{
		ae::Vec3 x0, y0, x1, y1;
		ae::IK::GetLimitBasis( primary, &x0, &y0 );
		ae::IK::GetLimitBasis( primary, &x1, &y1 );
		REQUIRE( ae::Abs( x0.Length() - 1.0f ) < 0.001f );
		REQUIRE( ae::Abs( y0.Length() - 1.0f ) < 0.001f );
		REQUIRE( ae::Abs( x0.Dot( primary ) ) < 0.001f );
		REQUIRE( ae::Abs( y0.Dot( primary ) ) < 0.001f );
		REQUIRE( ae::Abs( x0.Dot( y0 ) ) < 0.001f );
		REQUIRE( ( x0 - x1 ).Length() < 0.0001f );
		REQUIRE( ( y0 - y1 ).Length() < 0.0001f );
	}
}

#endif
