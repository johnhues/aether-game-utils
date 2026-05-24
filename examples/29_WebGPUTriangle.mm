//------------------------------------------------------------------------------
// 29_WebGPUTriangle.mm
//------------------------------------------------------------------------------
// Proof that the raw WebGPU C API (webgpu.h) is portable across:
//   - iOS via wgpu-native (Metal backend, prebuilt static lib)
//   - Web via emdawnwebgpu (Emscripten port)
//
// Renders a colored triangle on both platforms with the same shared code path.
// No ae::Window / ae::GraphicsDevice / ae::Shader / ae::VertexBuffer — raw
// WebGPU plus the bare minimum of UIKit (iOS) and emscripten loop glue (web).
//------------------------------------------------------------------------------
#include "aether.h"
#include <webgpu/webgpu.h>
#if _AE_IOS_
	#import <UIKit/UIKit.h>
	#import <QuartzCore/CAMetalLayer.h>
	#import <QuartzCore/CADisplayLink.h>
#endif
#if _AE_EMSCRIPTEN_
	#include <emscripten.h>
	#include <emscripten/html5.h>
#endif

//------------------------------------------------------------------------------
// WebGPU helpers
//------------------------------------------------------------------------------
static WGPUStringView WgpuStr( const char* s )
{
	WGPUStringView v;
	v.data = s;
	v.length = WGPU_STRLEN;
	return v;
}

//------------------------------------------------------------------------------
// Vertex data
//------------------------------------------------------------------------------
struct Vertex
{
	float pos[ 2 ];
	float color[ 4 ];
};

static const Vertex kTriangleVerts[] = {
	{ { -0.5f, -0.4f }, { 1.00f, 0.31f, 0.40f, 1.0f } }, // PicoRed-ish
	{ {  0.5f, -0.4f }, { 0.10f, 0.69f, 0.30f, 1.0f } }, // PicoGreen-ish
	{ {  0.0f,  0.5f }, { 0.16f, 0.42f, 0.85f, 1.0f } }, // PicoBlue-ish
};

static const char* kTriangleWgsl = R"(
struct VsOut {
	@builtin(position) pos : vec4f,
	@location(0) color : vec4f,
};
@vertex fn vs_main( @location(0) pos : vec2f, @location(1) color : vec4f ) -> VsOut
{
	var o : VsOut;
	o.pos = vec4f( pos, 0.0, 1.0 );
	o.color = color;
	return o;
}
@fragment fn fs_main( in : VsOut ) -> @location(0) vec4f
{
	return in.color;
}
)";

//------------------------------------------------------------------------------
// AppState
//------------------------------------------------------------------------------
struct AppState
{
	WGPUInstance instance = nullptr;
	WGPUSurface surface = nullptr;
	WGPUAdapter adapter = nullptr;
	WGPUDevice device = nullptr;
	WGPUQueue queue = nullptr;
	WGPURenderPipeline pipeline = nullptr;
	WGPUBuffer vertexBuffer = nullptr;
	WGPUTextureFormat surfaceFormat = WGPUTextureFormat_BGRA8Unorm;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t frame = 0;
	bool adapterRequested = false;
	bool deviceRequested = false;
	bool surfaceConfigured = false;
	bool pipelineBuilt = false;
};

static AppState s_app;

//------------------------------------------------------------------------------
// Pipeline + buffer setup (runs once after device arrives)
//------------------------------------------------------------------------------
static void BuildPipelineAndBuffer( AppState* s )
{
	WGPUShaderSourceWGSL wgslDesc = WGPU_SHADER_SOURCE_WGSL_INIT;
	wgslDesc.code = WgpuStr( kTriangleWgsl );

	WGPUShaderModuleDescriptor smDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
	smDesc.nextInChain = &wgslDesc.chain;
	smDesc.label = WgpuStr( "triangle.wgsl" );
	const WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule( s->device, &smDesc );

	WGPUVertexAttribute attrs[ 2 ];
	attrs[ 0 ] = WGPU_VERTEX_ATTRIBUTE_INIT;
	attrs[ 0 ].format = WGPUVertexFormat_Float32x2;
	attrs[ 0 ].offset = offsetof( Vertex, pos );
	attrs[ 0 ].shaderLocation = 0;
	attrs[ 1 ] = WGPU_VERTEX_ATTRIBUTE_INIT;
	attrs[ 1 ].format = WGPUVertexFormat_Float32x4;
	attrs[ 1 ].offset = offsetof( Vertex, color );
	attrs[ 1 ].shaderLocation = 1;

	WGPUVertexBufferLayout vbLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
	vbLayout.stepMode = WGPUVertexStepMode_Vertex;
	vbLayout.arrayStride = sizeof( Vertex );
	vbLayout.attributeCount = 2;
	vbLayout.attributes = attrs;

	WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
	colorTarget.format = s->surfaceFormat;
	colorTarget.writeMask = WGPUColorWriteMask_All;

	WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
	fragment.module = shaderModule;
	fragment.entryPoint = WgpuStr( "fs_main" );
	fragment.targetCount = 1;
	fragment.targets = &colorTarget;

	WGPURenderPipelineDescriptor pipeDesc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
	pipeDesc.label = WgpuStr( "triangle.pipeline" );
	pipeDesc.vertex.module = shaderModule;
	pipeDesc.vertex.entryPoint = WgpuStr( "vs_main" );
	pipeDesc.vertex.bufferCount = 1;
	pipeDesc.vertex.buffers = &vbLayout;
	pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	pipeDesc.primitive.frontFace = WGPUFrontFace_CCW;
	pipeDesc.primitive.cullMode = WGPUCullMode_None;
	pipeDesc.multisample.count = 1;
	pipeDesc.multisample.mask = 0xFFFFFFFF;
	pipeDesc.fragment = &fragment;
	s->pipeline = wgpuDeviceCreateRenderPipeline( s->device, &pipeDesc );

	wgpuShaderModuleRelease( shaderModule );

	WGPUBufferDescriptor bufDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
	bufDesc.label = WgpuStr( "triangle.vbuf" );
	bufDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	bufDesc.size = sizeof( kTriangleVerts );
	s->vertexBuffer = wgpuDeviceCreateBuffer( s->device, &bufDesc );
	wgpuQueueWriteBuffer( s->queue, s->vertexBuffer, 0, kTriangleVerts, sizeof( kTriangleVerts ) );

	s->pipelineBuilt = true;
	AE_LOG( "[frame #] WebGPU pipeline + vertex buffer ready", (int)s->frame );
}

//------------------------------------------------------------------------------
// Surface configuration
//------------------------------------------------------------------------------
static void ConfigureSurface( AppState* s )
{
	WGPUSurfaceCapabilities caps = WGPU_SURFACE_CAPABILITIES_INIT;
	wgpuSurfaceGetCapabilities( s->surface, s->adapter, &caps );
	WGPUTextureFormat preferred = WGPUTextureFormat_BGRA8Unorm;
	if( caps.formatCount > 0 && caps.formats )
	{
		preferred = caps.formats[ 0 ];
	}
	s->surfaceFormat = preferred;
	wgpuSurfaceCapabilitiesFreeMembers( caps );

	WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
	config.device = s->device;
	config.format = s->surfaceFormat;
	config.usage = WGPUTextureUsage_RenderAttachment;
	config.width = s->width;
	config.height = s->height;
	config.presentMode = WGPUPresentMode_Fifo;
	config.alphaMode = WGPUCompositeAlphaMode_Auto;
	wgpuSurfaceConfigure( s->surface, &config );
	s->surfaceConfigured = true;
	AE_LOG( "[frame #] surface configured w=# h=#", (int)s->frame, (int)s->width, (int)s->height );
}

//------------------------------------------------------------------------------
// Async callbacks
//------------------------------------------------------------------------------
static void OnDeviceReady( WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* ud1, void* /*ud2*/ )
{
	AppState* s = (AppState*)ud1;
	if( status != WGPURequestDeviceStatus_Success )
	{
		AE_LOG( "[frame #] device request FAILED", (int)s->frame );
		return;
	}
	s->device = device;
	s->queue = wgpuDeviceGetQueue( device );
	AE_LOG( "[frame #] WebGPU device ready", (int)s->frame );
}

static void OnAdapterReady( WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* ud1, void* /*ud2*/ )
{
	AppState* s = (AppState*)ud1;
	if( status != WGPURequestAdapterStatus_Success )
	{
		AE_LOG( "[frame #] adapter request FAILED", (int)s->frame );
		return;
	}
	s->adapter = adapter;
	AE_LOG( "[frame #] WebGPU adapter ready", (int)s->frame );

	WGPUDeviceDescriptor devDesc = {};
	WGPURequestDeviceCallbackInfo cb = {};
	cb.mode = WGPUCallbackMode_AllowSpontaneous;
	cb.callback = OnDeviceReady;
	cb.userdata1 = s;
	wgpuAdapterRequestDevice( s->adapter, &devDesc, cb );
}

//------------------------------------------------------------------------------
// Surface creation (platform-specific glue, identical API after this point)
//------------------------------------------------------------------------------
#if _AE_IOS_
static void CreateSurfaceMetal( AppState* s, void* metalLayer )
{
	WGPUSurfaceSourceMetalLayer src = WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;
	src.layer = metalLayer;
	WGPUSurfaceDescriptor surfDesc = WGPU_SURFACE_DESCRIPTOR_INIT;
	surfDesc.nextInChain = &src.chain;
	s->surface = wgpuInstanceCreateSurface( s->instance, &surfDesc );
}
#endif
#if _AE_EMSCRIPTEN_
static void CreateSurfaceCanvas( AppState* s, const char* selector )
{
	WGPUEmscriptenSurfaceSourceCanvasHTMLSelector src = WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
	src.selector = WgpuStr( selector );
	WGPUSurfaceDescriptor surfDesc = WGPU_SURFACE_DESCRIPTOR_INIT;
	surfDesc.nextInChain = &src.chain;
	s->surface = wgpuInstanceCreateSurface( s->instance, &surfDesc );
}
#endif

//------------------------------------------------------------------------------
// Init: creates the instance and kicks off the async adapter/device chain.
//------------------------------------------------------------------------------
static void InitWebGPU( AppState* s, uint32_t width, uint32_t height )
{
	s->width = width;
	s->height = height;
	s->instance = wgpuCreateInstance( nullptr );
	if( !s->instance )
	{
		AE_LOG( "wgpuCreateInstance returned null" );
		return;
	}

	WGPURequestAdapterOptions opts = {};
	opts.powerPreference = WGPUPowerPreference_HighPerformance;
	opts.compatibleSurface = s->surface;
	WGPURequestAdapterCallbackInfo cb = {};
	cb.mode = WGPUCallbackMode_AllowSpontaneous;
	cb.callback = OnAdapterReady;
	cb.userdata1 = s;
	wgpuInstanceRequestAdapter( s->instance, &opts, cb );
	s->adapterRequested = true;
	AE_LOG( "[frame #] wgpuInstanceRequestAdapter dispatched", (int)s->frame );
}

//------------------------------------------------------------------------------
// Frame
//------------------------------------------------------------------------------
static bool Frame( AppState* s )
{
	if( !s->device )
	{
		s->frame++;
		return true;
	}
	if( !s->surfaceConfigured )
	{
		ConfigureSurface( s );
	}
	if( !s->pipelineBuilt )
	{
		BuildPipelineAndBuffer( s );
	}

	WGPUSurfaceTexture surfTex = WGPU_SURFACE_TEXTURE_INIT;
	wgpuSurfaceGetCurrentTexture( s->surface, &surfTex );
	if( surfTex.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal
		&& surfTex.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal )
	{
		AE_LOG( "[frame #] surface get current texture failed status=#", (int)s->frame, (int)surfTex.status );
		s->frame++;
		return true;
	}

	WGPUTextureViewDescriptor viewDesc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
	viewDesc.label = WgpuStr( "triangle.surfaceView" );
	viewDesc.format = s->surfaceFormat;
	viewDesc.dimension = WGPUTextureViewDimension_2D;
	viewDesc.mipLevelCount = 1;
	viewDesc.arrayLayerCount = 1;
	viewDesc.aspect = WGPUTextureAspect_All;
	const WGPUTextureView view = wgpuTextureCreateView( surfTex.texture, &viewDesc );

	WGPURenderPassColorAttachment colorAttach = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
	colorAttach.view = view;
	colorAttach.loadOp = WGPULoadOp_Clear;
	colorAttach.storeOp = WGPUStoreOp_Store;
	colorAttach.clearValue.r = 0.10;
	colorAttach.clearValue.g = 0.05;
	colorAttach.clearValue.b = 0.18;
	colorAttach.clearValue.a = 1.0;

	WGPURenderPassDescriptor passDesc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
	passDesc.label = WgpuStr( "triangle.pass" );
	passDesc.colorAttachmentCount = 1;
	passDesc.colorAttachments = &colorAttach;

	WGPUCommandEncoderDescriptor encDesc = {};
	const WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder( s->device, &encDesc );
	const WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass( enc, &passDesc );
	wgpuRenderPassEncoderSetPipeline( pass, s->pipeline );
	wgpuRenderPassEncoderSetVertexBuffer( pass, 0, s->vertexBuffer, 0, sizeof( kTriangleVerts ) );
	wgpuRenderPassEncoderDraw( pass, 3, 1, 0, 0 );
	wgpuRenderPassEncoderEnd( pass );
	wgpuRenderPassEncoderRelease( pass );

	WGPUCommandBufferDescriptor cmdDesc = {};
	const WGPUCommandBuffer cmd = wgpuCommandEncoderFinish( enc, &cmdDesc );
	wgpuQueueSubmit( s->queue, 1, &cmd );
	wgpuCommandBufferRelease( cmd );
	wgpuCommandEncoderRelease( enc );
	wgpuTextureViewRelease( view );

#if !_AE_EMSCRIPTEN_
	// On the web the browser presents automatically when the JS task returns.
	wgpuSurfacePresent( s->surface );
#endif
#if _AE_IOS_
	// wgpu-native returns the surface texture by ownership; release per frame.
	wgpuTextureRelease( surfTex.texture );
#endif

	s->frame++;
	return true;
}

//------------------------------------------------------------------------------
// Term
//------------------------------------------------------------------------------
static void TermWebGPU( AppState* s )
{
	if( s->vertexBuffer ) { wgpuBufferRelease( s->vertexBuffer ); s->vertexBuffer = nullptr; }
	if( s->pipeline ) { wgpuRenderPipelineRelease( s->pipeline ); s->pipeline = nullptr; }
	if( s->queue ) { wgpuQueueRelease( s->queue ); s->queue = nullptr; }
	if( s->device ) { wgpuDeviceRelease( s->device ); s->device = nullptr; }
	if( s->adapter ) { wgpuAdapterRelease( s->adapter ); s->adapter = nullptr; }
	if( s->surface ) { wgpuSurfaceRelease( s->surface ); s->surface = nullptr; }
	if( s->instance ) { wgpuInstanceRelease( s->instance ); s->instance = nullptr; }
}

//------------------------------------------------------------------------------
// iOS application glue
//------------------------------------------------------------------------------
#if _AE_IOS_
@interface AEWGPUTriangleView : UIView
@end
@implementation AEWGPUTriangleView
+ (Class)layerClass { return [ CAMetalLayer class ]; }
- (instancetype)initWithFrame:(CGRect)frame
{
	self = [ super initWithFrame:frame ];
	if( self )
	{
		CAMetalLayer* layer = (CAMetalLayer*)self.layer;
		layer.opaque = YES;
		layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
		layer.framebufferOnly = YES;
		layer.contentsScale = [ UIScreen mainScreen ].scale;
	}
	return self;
}
@end

@interface AEWGPUTriangleViewController : UIViewController
@property( nonatomic, strong ) AEWGPUTriangleView* metalView;
@end
@implementation AEWGPUTriangleViewController
- (void)loadView
{
	self.metalView = [ [ AEWGPUTriangleView alloc ] initWithFrame:[ UIScreen mainScreen ].bounds ];
	self.view = self.metalView;
}
- (BOOL)prefersStatusBarHidden { return YES; }
@end

@interface AEWGPUTriangleAppDelegate : UIResponder < UIApplicationDelegate >
@property( nonatomic, strong ) UIWindow* window;
@property( nonatomic, strong ) AEWGPUTriangleViewController* viewController;
@property( nonatomic, strong ) CADisplayLink* displayLink;
@end
@implementation AEWGPUTriangleAppDelegate
- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)opts
{
	self.window = [ [ UIWindow alloc ] initWithFrame:[ UIScreen mainScreen ].bounds ];
	self.viewController = [ [ AEWGPUTriangleViewController alloc ] init ];
	self.window.rootViewController = self.viewController;
	[ self.window makeKeyAndVisible ];
	[ self.viewController.view layoutIfNeeded ];

	CAMetalLayer* metalLayer = (CAMetalLayer*)self.viewController.metalView.layer;
	const CGSize size = self.viewController.metalView.bounds.size;
	const CGFloat scale = metalLayer.contentsScale;
	const uint32_t pw = (uint32_t)( size.width * scale );
	const uint32_t ph = (uint32_t)( size.height * scale );
	metalLayer.drawableSize = CGSizeMake( pw, ph );

	s_app.instance = wgpuCreateInstance( nullptr );
	CreateSurfaceMetal( &s_app, (__bridge void*)metalLayer );
	// Kick off async adapter request now that the surface exists.
	WGPURequestAdapterOptions adapterOpts = {};
	adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;
	adapterOpts.compatibleSurface = s_app.surface;
	WGPURequestAdapterCallbackInfo cb = {};
	cb.mode = WGPUCallbackMode_AllowSpontaneous;
	cb.callback = OnAdapterReady;
	cb.userdata1 = &s_app;
	wgpuInstanceRequestAdapter( s_app.instance, &adapterOpts, cb );
	s_app.adapterRequested = true;
	s_app.width = pw;
	s_app.height = ph;
	AE_LOG( "iOS WebGPU triangle init: # x #", (int)pw, (int)ph );

	self.displayLink = [ CADisplayLink displayLinkWithTarget:self selector:@selector(tick:) ];
	[ self.displayLink addToRunLoop:[ NSRunLoop mainRunLoop ] forMode:NSDefaultRunLoopMode ];
	return YES;
}
- (void)tick:(CADisplayLink*)link
{
	if( !Frame( &s_app ) )
	{
		[ link invalidate ];
		TermWebGPU( &s_app );
		exit( 0 );
	}
}
@end
#endif // _AE_IOS_

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
#if _AE_IOS_
	@autoreleasepool { UIApplicationMain( argc, argv, nil, @"AEWGPUTriangleAppDelegate" ); }
	return 0; // unreachable
#elif _AE_EMSCRIPTEN_
	const int canvasW = (int)EM_ASM_INT({ return Module.canvas ? Module.canvas.width : 1280; });
	const int canvasH = (int)EM_ASM_INT({ return Module.canvas ? Module.canvas.height : 720; });
	s_app.instance = wgpuCreateInstance( nullptr );
	CreateSurfaceCanvas( &s_app, "#canvas" );
	WGPURequestAdapterOptions opts = {};
	opts.powerPreference = WGPUPowerPreference_HighPerformance;
	opts.compatibleSurface = s_app.surface;
	WGPURequestAdapterCallbackInfo cb = {};
	cb.mode = WGPUCallbackMode_AllowSpontaneous;
	cb.callback = OnAdapterReady;
	cb.userdata1 = &s_app;
	wgpuInstanceRequestAdapter( s_app.instance, &opts, cb );
	s_app.adapterRequested = true;
	s_app.width = (uint32_t)canvasW;
	s_app.height = (uint32_t)canvasH;
	AE_LOG( "Emscripten WebGPU triangle init: # x #", canvasW, canvasH );
	emscripten_set_main_loop_arg( []( void* ud ) { Frame( (AppState*)ud ); }, &s_app, 0, 1 );
	return 0;
#else
	(void)argc; (void)argv;
	return 0; // Not built for desktop / Android.
#endif
}
