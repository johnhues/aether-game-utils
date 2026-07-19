//------------------------------------------------------------------------------
// IKTest.h
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
#ifndef IKTEST_H
#define IKTEST_H
#include "aether.h"

//------------------------------------------------------------------------------
// IKScenario
//------------------------------------------------------------------------------
//! A hand-authored skeleton plus scripted targets and invariant tolerances.
//! Target paths are pure functions of frame index so a failure at a given
//! scenario and frame reproduces identically in the catch2 tests and in
//! examples/30_IKGallery.cpp. Negative tolerances disable a check.
struct IKScenario
{
	IKScenario( const ae::Tag& tag );

	//! Fills the world-space targets for the given frame
	typedef void ( *TargetFn )( const IKScenario& scenario, uint32_t frame, ae::Map< uint32_t, ae::Vec3 >* targetsOut, ae::Map< uint32_t, ae::Quaternion >* orientationsOut );

	ae::Str64 name;
	ae::Str256 description;
	uint32_t frameCount = 240;
	uint32_t rootBoneIndex = 1;
	//! Rigid model transform applied to the input pose root when a solve
	//! starts from the bind pose, the way a game poses a character in the
	//! world while ae::IK::bindPose stays in model space. Targets returned by
	//! targetFn must be in this transformed space.
	ae::Matrix4 inputTransform = ae::Matrix4::Identity();
	ae::Skeleton bindPose;
	ae::Map< uint32_t, ae::IKRotationConstraint > rotationConstraints;
	ae::Array< ae::IKDistanceConstraint > distanceConstraints;
	TargetFn targetFn = nullptr;

	float boneLengthTolerance = 0.005f; //!< Relative to bind bone length
	float distanceConstraintTolerance = 0.05f; //!< Relative to constraint rest length
	float rotationLimitTolerance = 0.02f; //!< Radians beyond the limit ellipse
	float twistLimitTolerance = 0.02f; //!< Radians beyond the twist limits
	float effectorTolerance = 0.02f; //!< World units, reachable position targets only
	float effectorOriTolerance = 0.05f; //!< Radians, orientation targets only
	float staticDriftTolerance = 0.01f; //!< World units, bones listed in staticBones
	float mirrorTolerance = 0.01f; //!< World units, bone pairs listed in mirrorPairs
	float continuityTolerance = 0.06f; //!< World units of joint movement between consecutive frames
	float continuityOriTolerance = 0.5f; //!< Radians of joint rotation between consecutive frames
	bool checkStraighten = false; //!< Unreachable position targets fully extend their chain
	bool checkConvergenceTrend = true; //!< Rotation-limited scenarios opt out; clipping can trade effector error between iterations
	ae::Array< uint32_t > staticBones; //!< Bones expected to stay at their solve-input position
	ae::Array< ae::Int2 > mirrorPairs; //!< Bone index pairs expected to mirror across the model yz plane
};

//! Number of hand-authored scenarios available to IKScenarioBuild()
uint32_t IKScenarioCount();
//! Builds the scenario at \p index. \p scenarioOut should be freshly
//! constructed or previously built by this function.
void IKScenarioBuild( uint32_t index, IKScenario* scenarioOut );

//------------------------------------------------------------------------------
// IKFrameReport
//------------------------------------------------------------------------------
//! Invariant measurements for a single solved frame. Error fields are zero
//! when a check passes or is disabled. \p pass covers every enabled check
//! except twist limits, which are reported separately by \p twistPass because
//! twist enforcement is currently disabled in ae::IK::Run().
struct IKFrameReport
{
	float maxBoneLengthError = 0.0f;
	float maxDistanceConstraintError = 0.0f;
	float maxRotationLimitError = 0.0f;
	float maxTwistLimitError = 0.0f;
	float maxEffectorError = 0.0f;
	float maxEffectorOriError = 0.0f;
	float maxStaticDrift = 0.0f;
	float maxContinuityDelta = 0.0f;
	float maxContinuityOriDelta = 0.0f;
	float maxMirrorError = 0.0f;
	ae::Str128 worstCheck; //!< Description of the worst failing check, if any
	bool pass = true;
	bool twistPass = true;
};

//! Evaluates all enabled invariants for a solved frame. \p inputPose is the
//! pose the solve started from (distance constraint rest lengths and static
//! bone positions are measured against it, matching ae::IK::Run()).
//! \p prevSolvedPose may be null; when given it must be the solved result of
//! frame - 1 with identical settings, enabling the continuity checks.
IKFrameReport IKEvaluateFrame( const IKScenario& scenario, uint32_t frame, const ae::Skeleton& inputPose, const ae::Skeleton& solvedPose, const ae::Skeleton* prevSolvedPose );

//! Solves \p frame from \p inputPose at 1, 2, 4 and 8 iterations and returns
//! true if the max effector error is non-increasing within \p slack at each
//! step. On failure \p messageOut describes the regression.
bool IKConvergenceTrend( const ae::Tag& tag, const IKScenario& scenario, uint32_t frame, const ae::Skeleton& inputPose, float slack, ae::Str256* messageOut );

//------------------------------------------------------------------------------
// IKScenarioRunner
//------------------------------------------------------------------------------
//! Steps a scenario frame by frame, feeding either the bind pose or the
//! previous solved pose into each solve, and evaluates invariants.
enum class IKStartMode
{
	FromBind, //!< Stateless: every solve starts at the bind pose
	FromPrevious //!< Warm start: every solve starts at the last solved pose
};

class IKScenarioRunner
{
public:
	IKScenarioRunner( const ae::Tag& tag, const IKScenario* scenario );
	void Reset();
	//! Solves \p frame and returns the invariant report. Continuity checks
	//! only run when \p frame directly follows the previously stepped frame.
	//! Debug params are forwarded to ae::IK for examples/30_IKGallery.cpp.
	IKFrameReport Step( uint32_t frame, uint32_t iterationCount, IKStartMode startMode, ae::DebugLines* debugLines = nullptr, const ae::Matrix4& debugModelToWorld = ae::Matrix4::Identity(), float debugJointScale = 0.1f );
	const ae::Skeleton& SolvedPose() const { return m_solvedPose; }
	const ae::Skeleton& InputPose() const { return m_inputPose; }
	const IKScenario* Scenario() const { return m_scenario; }

private:
	const ae::Tag m_tag;
	const IKScenario* m_scenario;
	ae::Skeleton m_inputPose;
	ae::Skeleton m_solvedPose;
	ae::Skeleton m_prevSolvedPose;
	bool m_hasSolved = false;
	uint32_t m_lastFrame = 0;
};

#endif
