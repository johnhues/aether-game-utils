//------------------------------------------------------------------------------
// 28_WebGPUAsync.cpp
//------------------------------------------------------------------------------
// Proof that browser WebGPU adapter+device acquisition is unavoidably async.
// Logs which frame each callback completes on; once the device is ready, an
// empty command buffer is submitted every frame to verify the device works.
//------------------------------------------------------------------------------
#if defined( __EMSCRIPTEN__ )

#include <cstdio>
#include <cstdint>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>

//------------------------------------------------------------------------------
// State
//------------------------------------------------------------------------------
struct AsyncState
{
	WGPUInstance instance = nullptr;
	WGPUAdapter adapter = nullptr;
	WGPUDevice device = nullptr;
	WGPUQueue queue = nullptr;
	uint32_t frame = 0;
	uint32_t adapterRequestedFrame = UINT32_MAX;
	uint32_t adapterReadyFrame = UINT32_MAX;
	uint32_t deviceRequestedFrame = UINT32_MAX;
	uint32_t deviceReadyFrame = UINT32_MAX;
	bool adapterRequested = false;
	bool deviceRequested = false;
	bool emptySubmitLogged = false;
};

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static const char* StringViewOrEmpty( WGPUStringView v )
{
	return ( v.data && v.length ) ? v.data : "";
}

//------------------------------------------------------------------------------
// Callbacks (modern Dawn webgpu.h signature: status, handle, message, ud1, ud2)
//------------------------------------------------------------------------------
static void OnDeviceReady( WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* /*userdata2*/ )
{
	AsyncState* s = (AsyncState*)userdata1;
	if( status != WGPURequestDeviceStatus_Success )
	{
		printf( "[frame %u] device request FAILED: %s\n", s->frame, StringViewOrEmpty( message ) );
		return;
	}
	s->device = device;
	s->queue = wgpuDeviceGetQueue( device );
	s->deviceReadyFrame = s->frame;
	printf( "[frame %u] device ready (requested on frame %u)\n", s->frame, s->deviceRequestedFrame );
}

static void OnAdapterReady( WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* /*userdata2*/ )
{
	AsyncState* s = (AsyncState*)userdata1;
	if( status != WGPURequestAdapterStatus_Success )
	{
		printf( "[frame %u] adapter request FAILED: %s\n", s->frame, StringViewOrEmpty( message ) );
		return;
	}
	s->adapter = adapter;
	s->adapterReadyFrame = s->frame;
	printf( "[frame %u] adapter ready (requested on frame %u)\n", s->frame, s->adapterRequestedFrame );
}

//------------------------------------------------------------------------------
// Frame
//------------------------------------------------------------------------------
static void Frame( void* userdata )
{
	AsyncState* s = (AsyncState*)userdata;
	const uint32_t f = s->frame;

	if( !s->adapterRequested )
	{
		s->adapterRequested = true;
		s->adapterRequestedFrame = f;
		printf( "[frame %u] wgpuInstanceRequestAdapter() called -- returning to event loop\n", f );
		WGPURequestAdapterOptions opts = {};
		WGPURequestAdapterCallbackInfo cbInfo = {};
		cbInfo.mode = WGPUCallbackMode_AllowSpontaneous;
		cbInfo.callback = OnAdapterReady;
		cbInfo.userdata1 = s;
		wgpuInstanceRequestAdapter( s->instance, &opts, cbInfo );
	}
	else if( !s->adapter )
	{
		printf( "[frame %u] waiting for adapter...\n", f );
	}
	else if( !s->deviceRequested )
	{
		s->deviceRequested = true;
		s->deviceRequestedFrame = f;
		printf( "[frame %u] wgpuAdapterRequestDevice() called -- returning to event loop\n", f );
		WGPUDeviceDescriptor desc = {};
		WGPURequestDeviceCallbackInfo cbInfo = {};
		cbInfo.mode = WGPUCallbackMode_AllowSpontaneous;
		cbInfo.callback = OnDeviceReady;
		cbInfo.userdata1 = s;
		wgpuAdapterRequestDevice( s->adapter, &desc, cbInfo );
	}
	else if( !s->device )
	{
		printf( "[frame %u] waiting for device...\n", f );
	}
	else
	{
		// Device is live -- submit an empty command buffer each frame to prove
		// it works. By the time we get here, main() returned long ago.
		WGPUCommandEncoderDescriptor encDesc = {};
		WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder( s->device, &encDesc );
		WGPUCommandBufferDescriptor cmdDesc = {};
		WGPUCommandBuffer cmd = wgpuCommandEncoderFinish( enc, &cmdDesc );
		wgpuQueueSubmit( s->queue, 1, &cmd );
		wgpuCommandBufferRelease( cmd );
		wgpuCommandEncoderRelease( enc );
		if( !s->emptySubmitLogged )
		{
			s->emptySubmitLogged = true;
			printf( "[frame %u] empty command buffer submitted -- device is live\n", f );
			printf( "  PROOF SUMMARY:\n" );
			printf( "    adapter requested on frame %u, ready on frame %u (+%u)\n",
				s->adapterRequestedFrame, s->adapterReadyFrame,
				s->adapterReadyFrame - s->adapterRequestedFrame );
			printf( "    device  requested on frame %u, ready on frame %u (+%u)\n",
				s->deviceRequestedFrame, s->deviceReadyFrame,
				s->deviceReadyFrame - s->deviceRequestedFrame );
			printf( "    => sync Init() pattern is impossible: control must yield to the event loop\n" );
		}
	}

	s->frame++;
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
static AsyncState s_state;

int main( int argc, char* argv[] )
{
	printf( "28_WebGPUAsync: proving WGPU adapter/device acquisition cannot be synchronous in the browser\n" );

	// wgpuCreateInstance with a null descriptor returns the emdawnwebgpu
	// sentinel WGPUInstance handle -- there is no real JS-side instance, the
	// request* calls dispatch directly against navigator.gpu.
	s_state.instance = wgpuCreateInstance( nullptr );
	if( !s_state.instance )
	{
		printf( "wgpuCreateInstance returned null -- WebGPU not available in this browser\n" );
		return 1;
	}

	emscripten_set_main_loop_arg( Frame, &s_state, 0, 1 );
	return 0;
}

#else
int main( int /*argc*/, char* /*argv*/[] ) { return 0; }
#endif
