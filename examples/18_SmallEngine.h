//------------------------------------------------------------------------------
//! 18_SmallEngine.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and /or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "ae/Editor.h"
#include "ae/Entity.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_SMALL_ENGINE = "small_engine";

//------------------------------------------------------------------------------
// Component
//------------------------------------------------------------------------------
class Component : public ae::Inheritor< ae::Component, Component >
{
public:
	virtual void Initialize( class SmallEngine* engine ) {}
	virtual void Terminate( class SmallEngine* engine ) {}
	virtual void Update( class SmallEngine* engine ) {}
	virtual void Render( class SmallEngine* engine ) {}
	bool initialized = false;
};

//------------------------------------------------------------------------------
// SmallEngine
//------------------------------------------------------------------------------
class SmallEngine
{
public:
	~SmallEngine();

	// System
	bool Initialize( int argc, char* argv[] );
	void Run();
	ae::Registry registry = TAG_SMALL_ENGINE;
	ae::Editor editor = TAG_SMALL_ENGINE;
	ae::Window window;
	ae::Input input;
	ae::GraphicsDevice gfx;
	ae::TimeStep timeStep;
	ae::FileSystem fs;
	ae::DebugLines debugLines = TAG_SMALL_ENGINE;

	// Resources
	struct MeshResource
	{
		ae::VertexBuffer vertexData;
		ae::CollisionMesh<> collision = TAG_SMALL_ENGINE;
	};
	const struct MeshResource* GetMeshResource( const char* name );
	ae::Map< ae::Str128, MeshResource* > meshResources = TAG_SMALL_ENGINE;
	ae::Texture2D defaultTexture;
	ae::Shader meshShader;

	// Rendering
	void GetUniforms( ae::UniformList* uniformList );
	ae::Vec3 cameraPos = ae::Vec3( 10.0f );
	ae::Vec3 cameraDir = ae::Vec3( -1.0f ).SafeNormalizeCopy();
	ae::Color skyColor = ae::Color::PicoBlue();
	ae::Matrix4 worldToView = ae::Matrix4::Identity();
	ae::Matrix4 viewToProj = ae::Matrix4::Identity();
	ae::Matrix4 worldToProj = ae::Matrix4::Identity();
};
