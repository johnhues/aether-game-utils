//------------------------------------------------------------------------------
// 06_Triangle.mm
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
#include "aether.h"
#if _AE_IOS_
#import <QuartzCore/CADisplayLink.h>
#import <UIKit/UIKit.h>

extern "C" void* g_eaglLayer = nullptr; // declared extern in aether.h; defined here
#endif

//------------------------------------------------------------------------------
// ae::Application
//------------------------------------------------------------------------------
namespace ae {

struct Application
{
	ae::Function< void(), 128 > Initialize;
	ae::Function< bool(), 128 > Update;
	ae::Function< int32_t(), 128 > Terminate;

	static Application* s_app;

	int32_t Run( int argc, char* argv[] )
	{
#if _AE_IOS_
		s_app = this;
		@autoreleasepool
		{
			return UIApplicationMain( argc, argv, nil, @"AETriangleAppDelegate" );
		}
#else
		(void)argc;
		(void)argv;
		Initialize();
#if _AE_EMSCRIPTEN_
		emscripten_set_main_loop_arg( []( void* fn ) { ( *(decltype( Update )*)fn )(); }, &Update, 0, 1 );
#else
		while( Update() )
		{
		}
#endif
		return Terminate();
#endif
	}
};
Application* Application::s_app = nullptr;

} // namespace ae

#if _AE_IOS_
@interface AETriangleEAGLView : UIView
@end
@implementation AETriangleEAGLView
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}
- (instancetype)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if( self )
	{
		CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
		layer.opaque = YES;
		layer.contentsScale = [UIScreen mainScreen].scale;
		layer.drawableProperties = @{
			kEAGLDrawablePropertyRetainedBacking : @YES,
			kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8,
		};
	}
	return self;
}
@end

@interface AETriangleViewController : UIViewController
@property ( nonatomic, strong ) AETriangleEAGLView* glView;
@end
@implementation AETriangleViewController
- (void)loadView
{
	self.glView = [[AETriangleEAGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];
	self.view = self.glView;
}
- (BOOL)prefersStatusBarHidden
{
	return YES;
}
@end

@interface AETriangleAppDelegate : UIResponder < UIApplicationDelegate >
@property ( nonatomic, strong ) UIWindow* window;
@property ( nonatomic, strong ) AETriangleViewController* viewController;
@property ( nonatomic, strong ) CADisplayLink* displayLink;
@end
@implementation AETriangleAppDelegate
- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)opts
{
	self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	self.viewController = [[AETriangleViewController alloc] init];
	self.window.rootViewController = self.viewController;
	[self.window makeKeyAndVisible];
	[self.viewController.view layoutIfNeeded];
	// Set the layer so window.Initialize() can attach its renderbuffer.
	// aether.h spins ≤1 s waiting for this; here it is already set before
	// Initialize() runs, so the spin exits on the first iteration.
	g_eaglLayer = (__bridge void*)self.viewController.glView.layer;
	ae::Application::s_app->Initialize();
	self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector( tick: )];
	[self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
	return YES;
}
- (void)tick:(CADisplayLink*)link
{
	if( !ae::Application::s_app->Update() )
	{
		[link invalidate];
		const int32_t termResult = ae::Application::s_app->Terminate();
		exit( termResult );
	}
}
@end
#endif // _AE_IOS_

//------------------------------------------------------------------------------
// Triangle
//------------------------------------------------------------------------------
struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
};

const Vertex kTriangleVerts[] = {
	{ ae::Vec4( -0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA() },
	{ ae::Vec4( 0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
	{ ae::Vec4( 0.0f, 0.4f, 0.0f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
};

const uint16_t kTriangleIndices[] = { 0, 1, 2 };

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::VertexBuffer vertexData;
	ae::Vec3 pos = ae::Vec3( 0.0f );
	float scale = 1.0f;
	float rotation = 0.0f;

	ae::Application app;
	app.Initialize = [ & ]()
	{
		AE_LOG( "Initialize (debug #)", (int)_AE_DEBUG_ );
		window.Initialize( 1280, 720, false, true, true );
		window.SetTitle( "triangle" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );

		const char* vertShader = R"(
			AE_UNIFORM mat4 u_modelToNdc;
			AE_IN_HIGHP vec4 a_position;
			AE_IN_HIGHP vec4 a_color;
			AE_OUT_HIGHP vec4 v_color;
			void main()
			{
				v_color = a_color;
				gl_Position = u_modelToNdc * a_position;
			})";
		const char* fragShader = R"(
			AE_IN_HIGHP vec4 v_color;
			void main()
			{
				AE_COLOR = v_color;
			})";
		shader.Initialize( vertShader, fragShader, nullptr, 0 );

		vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
		vertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( Vertex, pos ) );
		vertexData.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( Vertex, color ) );
		vertexData.UploadVertices( 0, kTriangleVerts, countof( kTriangleVerts ) );
		vertexData.UploadIndices( 0, kTriangleIndices, countof( kTriangleIndices ) );
	};
	app.Update = [ & ]() -> bool
	{
		input.Pump();
		rotation += timeStep.GetDt();

		if( input.GetMousePressLeft() )
		{
			input.SetMouseCaptured( !input.GetMouseCaptured() );
		}
		if( input.GetPress( ae::Key::Escape ) )
		{
			input.SetMouseCaptured( false );
		}

		ae::Vec3 dir( 0.0f );
		if( input.Get( ae::Key::Up ) )
		{
			dir.y += 1.0f;
		}
		if( input.Get( ae::Key::Down ) )
		{
			dir.y -= 1.0f;
		}
		if( input.Get( ae::Key::Left ) )
		{
			dir.x -= 1.0f;
		}
		if( input.Get( ae::Key::Right ) )
		{
			dir.x += 1.0f;
		}
		dir.SafeNormalize();
		pos += dir * 0.01f;

		if( input.GetMouseCaptured() )
		{
			pos.x += input.mouse.movement.x * 0.001f;
			pos.y += input.mouse.movement.y * 0.001f;
		}
		else if( input.mouse.usingTouch )
		{
			pos.x += input.mouse.scrollMomentum.x * 0.01f;
			pos.y += input.mouse.scrollMomentum.y * -0.01f;
		}
		else
		{
			scale += input.mouse.scrollMomentum.y * 0.01f;
			scale = ae::Clip( scale, 0.1f, 2.0f );
		}

		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );

		ae::Matrix4 transform = ae::Matrix4::Translation( pos );
		transform *= ae::Matrix4::RotationY( rotation );
		transform *= ae::Matrix4::Scaling( ae::Vec3( scale / render.GetAspectRatio(), scale, scale ) );

		ae::UniformList uniformList;
		uniformList.Set( "u_modelToNdc", transform );
		vertexData.Bind( &shader, uniformList );
		vertexData.Draw();

		render.Present();
		timeStep.Tick();

		return !input.quit;
	};
	app.Terminate = [ & ]() -> int32_t
	{
		vertexData.Terminate();
		shader.Terminate();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};
	return app.Run( argc, argv );
}
