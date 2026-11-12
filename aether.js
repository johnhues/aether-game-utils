//------------------------------------------------------------------------------
// aether.js
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
// Runs programs built with aether.h and wasi-sdk in a browser. Host this file
// on the same origin as the page that imports it. Usage:
//
//   import * as aether from './aether.js';
//   const game = await aether.instantiate( 'game.wasm', document.querySelector( 'canvas' ) );
//
// An instance only touches the canvas it is given, see "DOM ownership" in the
// Web documentation.
//------------------------------------------------------------------------------

/**
 * The import ABI version this file provides. A program records the version it
 * was built for in an 'aether_abi' custom section, and instantiate() rejects a
 * mismatch before linking anything. Programs only import from these modules,
 * and instantiate() reports any other import as a missing definition:
 * - `ae`: aether's host functions
 * - `gl`: OpenGL ES 3 functions, provided with WebGL 2
 * - `al`: OpenAL 1.1 functions, provided with WebAudio
 * - `webgpu`: webgpu.h functions, provided with WebGPU. @NOTE: Placeholder.
 *   Only what aether's examples use.
 * - `wasi_snapshot_preview1`: The WASI system interface
 * - Any module the page passes in InstantiateOptions.imports
 */
export const abiVersion = 1;

/**
 * Loads, links and starts an aether program on a canvas. Runs the program's
 * main(), which hands control to the page by calling ae::Application(). The
 * returned instance's update loop is already running.
 * @param {string|URL} wasm The program. Instances of the same program share
 *   one download and compile.
 * @param {HTMLCanvasElement} canvas Where the program draws and receives
 *   input. The instance does not touch any other element.
 * @param {InstantiateOptions} [options]
 * @returns {Promise<AetherInstance>} Rejects if the program fails to load or
 *   compile, or halts while starting.
 */
export async function instantiate( wasm, canvas, options = {} )
{
	const url = new URL( wasm, document.baseURI ).href;
	const print = options.print || console.log;
	const onProgress = options.onProgress || function() {};
	const program = await compileProgram( url, onProgress );
	const [ abi ] = WebAssembly.Module.customSections( program.module, "aether_abi" );
	const version = abi ? Number( new TextDecoder().decode( abi ) ) : 0;
	if( version !== abiVersion )
	{
		throw new Error( "aether.js: " + url + ( abi ? " was built for ABI " + version : " was not built with aether.h and wasi-sdk" ) + ", this aether.js provides " + abiVersion );
	}
	const wanted = WebAssembly.Module.imports( program.module );

	// The canvas has one context, WebGL only once the program draws with it
	let gl = null;
	const webgl = function()
	{
		if( !gl )
		{
			gl = canvas.getContext( "webgl2", { alpha: false, antialias: true, depth: true, stencil: false, premultipliedAlpha: true } );
			if( !gl ) { throw new Error( "aether.js: WebGL 2 is not available" ); }
		}
		return gl;
	};
	let exports = null;
	let halted = false;
	let terminated = false;
	let running = false;
	let frame = 0;
	const stats = { frames: 0, busyMs: 0, draws: 0, last: performance.now() };
	const listeners = [];
	const reads = new Map(); // ae::File pointer -> AbortController

	const halt = function( reason )
	{
		if( !halted )
		{
			halted = true;
			running = false;
			cancelAnimationFrame( frame );
			if( options.onHalt ) { options.onHalt( reason ); }
		}
	};
	// Calls into the program, halting it if it traps or exits
	const call = function( name, ...args )
	{
		if( halted || terminated ) { return 0; }
		try
		{
			return exports[ name ]( ...args );
		}
		catch( error )
		{
			halt( error instanceof ProgramExit ? "exited with " + error.code : String( error ) );
			return 0;
		}
	};

	const memory = function() { return exports.memory.buffer; };
	const string = function( ptr ) { return readString( memory(), ptr ); };
	const audio = alImports( memory );
	const gpu = webgpuImports( canvas, {
		memory: memory,
		call: call,
		// Calls a function pointer, which needs the program linked with --export-table
		callback: function( index, ...args )
		{
			if( halted || terminated ) { return; }
			try
			{
				exports.__indirect_function_table.get( index )( ...args );
			}
			catch( error )
			{
				halt( error instanceof ProgramExit ? "exited with " + error.code : String( error ) );
			}
		},
	} );
	const imports = {
		wasi_snapshot_preview1: wasiImports( memory, print, url ),
		ae: {
			_ae_CanvasWidth: function() { return Math.round( canvas.getBoundingClientRect().width ); },
			_ae_CanvasHeight: function() { return Math.round( canvas.getBoundingClientRect().height ); },
			_ae_CanvasScale: function() { return window.devicePixelRatio || 1; },
			_ae_SetCanvasSize: function( width, height ) { canvas.width = width; canvas.height = height; },
			_ae_RunHostLoop: function() { throw hostLoop; },
			_ae_SetTitle: function( ptr ) { document.title = string( ptr ); },
			_ae_Fullscreen: function()
			{
				const element = document.fullscreenElement || document.webkitFullscreenElement;
				return ( element && element.contains( canvas ) ) ? 1 : 0;
			},
			_ae_SetFullscreen: function( fullscreen )
			{
				const request = fullscreen
					? ( canvas.requestFullscreen || canvas.webkitRequestFullscreen ).call( canvas )
					: ( document.exitFullscreen || document.webkitExitFullscreen ).call( document );
				if( request && request.catch ) { request.catch( function() {} ); }
			},
			_ae_RequestPointerLock: function()
			{
				const request = canvas.requestPointerLock();
				if( request && request.catch ) { request.catch( function() {} ); }
			},
			_ae_ExitPointerLock: function()
			{
				if( document.pointerLockElement === canvas ) { document.exitPointerLock(); }
			},
			_ae_SetCursorHidden: function( hidden )
			{
				const cursor = hidden ? "none" : "auto";
				if( canvas.style.cursor !== cursor ) { canvas.style.cursor = cursor; }
			},
			_ae_GamepadCount: function() { return navigator.getGamepads ? navigator.getGamepads().length : 0; },
			_ae_Gamepad: function( index, axesPtr, valuesPtr, pressedPtr )
			{
				const pad = navigator.getGamepads()[ index ];
				if( !pad || !pad.connected ) { return 0; }
				const dv = new DataView( memory() );
				let pressed = 0;
				for( let i = 0; i < 4; i++ ) { dv.setFloat32( axesPtr + i * 4, pad.axes[ i ] || 0, true ); }
				for( let i = 0; i < 17; i++ )
				{
					const button = pad.buttons[ i ];
					dv.setFloat32( valuesPtr + i * 4, button ? button.value : 0, true );
					if( button && button.pressed ) { pressed |= 1 << i; }
				}
				dv.setUint32( pressedPtr, pressed >>> 0, true );
				return 1;
			},
			_ae_ReadFile: function( urlPtr, file, timeoutMs )
			{
				const fileUrl = new URL( string( urlPtr ), document.baseURI ).href;
				const controller = new AbortController();
				let timedOut = false;
				const timer = timeoutMs ? setTimeout( function() { timedOut = true; controller.abort(); }, timeoutMs ) : 0;
				reads.set( file, controller );
				( async function()
				{
					let status = 0;
					let bytes = null;
					try
					{
						const response = await fetch( fileUrl, { signal: controller.signal } );
						status = response.status;
						if( response.ok ) { bytes = await readResponse( response, fileUrl, onProgress ); }
					}
					catch( error ) {}
					clearTimeout( timer );
					if( reads.get( file ) !== controller ) { return; } // Canceled
					reads.delete( file );
					if( bytes )
					{
						const ptr = call( "_ae_OnFileLoaded", file, bytes.length );
						if( ptr ) { new Uint8Array( memory(), ptr, bytes.length ).set( bytes ); }
					}
					else
					{
						onProgress( fileUrl, 0, 0, true );
						call( "_ae_OnFileFailed", file, status, timedOut ? 1 : 0 );
					}
				} )();
			},
			_ae_CancelFile: function( file )
			{
				const controller = reads.get( file );
				if( controller )
				{
					reads.delete( file );
					controller.abort();
				}
			},
			_ae_PageUrl: function( ptr, size ) { writeString( memory(), document.baseURI, ptr, size ); },
		},
		gl: glImports( webgl, memory, stats ),
		al: audio.imports,
		webgpu: gpu.imports,
	};
	for( const name in options.imports || {} )
	{
		if( imports[ name ] )
		{
			throw new Error( "aether.js: the import module '" + name + "' is provided by aether.js" );
		}
		imports[ name ] = options.imports[ name ];
	}
	for( const i of wanted )
	{
		if( !imports[ i.module ] || !imports[ i.module ][ i.name ] )
		{
			throw new Error( "aether.js: the program imports " + i.module + "." + i.name + ", which is not provided. Is it missing a definition?" );
		}
	}
	exports = ( await WebAssembly.instantiate( program.module, imports ) ).exports;

	// main() runs until ae::Application() hands control over by unwinding
	exports._initialize();
	try
	{
		const code = exports._ae_Main();
		throw new Error( "aether.js: main() returned " + code + " without starting ae::Application()" );
	}
	catch( error )
	{
		if( error !== hostLoop )
		{
			halt( error instanceof ProgramExit ? "exited with " + error.code : String( error ) );
			throw error;
		}
	}

	const update = function()
	{
		if( !running ) { return; }
		frame = requestAnimationFrame( update );
		const start = performance.now();
		const keepGoing = call( "_ae_Update" );
		stats.busyMs += performance.now() - start;
		stats.frames++;
		if( !keepGoing && !halted )
		{
			instance.terminate();
		}
	};
	const pause = function()
	{
		running = false;
		cancelAnimationFrame( frame );
	};
	const resume = function()
	{
		if( !running && !halted && !terminated )
		{
			running = true;
			frame = requestAnimationFrame( update );
		}
	};

	// Input, from this canvas only
	const listen = function( target, type, fn, opts )
	{
		target.addEventListener( type, fn, opts );
		listeners.push( [ target, type, fn, opts ] );
	};
	if( !canvas.hasAttribute( "tabindex" ) ) { canvas.tabIndex = -1; }
	canvas.setAttribute( "data-ae-canvas", "true" );
	if( !document.getElementById( "ae-canvas-style" ) )
	{
		// Zero specificity, so page styles take precedence
		const style = document.createElement( "style" );
		style.id = "ae-canvas-style";
		style.textContent = ":where( canvas[data-ae-canvas]:focus ) { outline: none; }";
		document.head.appendChild( style );
	}
	const local = function( e )
	{
		const rect = canvas.getBoundingClientRect();
		return [ e.clientX - rect.left, e.clientY - rect.top ];
	};
	const mouse = function( e )
	{
		const [ x, y ] = local( e );
		const activation = ( e.type === "mousedown" || e.type === "mouseup" ) ? 1 : 0;
		call( "_ae_OnMouse", x, y, e.movementX || 0, e.movementY || 0, e.buttons, activation );
		e.preventDefault();
	};
	for( const type of [ "mousedown", "mouseup", "mousemove", "mouseenter", "mouseleave" ] )
	{
		listen( canvas, type, mouse );
	}
	// The mouse handlers prevent the default mousedown focus, so focus explicitly.
	// Drags that leave the canvas keep reporting through pointer capture.
	listen( canvas, "pointerdown", function( e )
	{
		canvas.setPointerCapture( e.pointerId );
		canvas.focus( { preventScroll: true } );
	} );
	listen( document, "pointerlockchange", function() { call( "_ae_OnPointerLock", document.pointerLockElement === canvas ? 1 : 0, 0 ); } );
	listen( document, "pointerlockerror", function() { call( "_ae_OnPointerLock", 0, 1 ); } );
	listen( canvas, "contextmenu", function( e ) { e.preventDefault(); } );
	const key = function( e )
	{
		call( "_ae_OnKey", e.keyCode, e.type === "keydown" ? 1 : 0 );
		e.preventDefault();
	};
	listen( canvas, "keydown", key );
	listen( canvas, "keyup", key );
	listen( canvas, "focus", function() { call( "_ae_OnFocus", 1 ); } );
	listen( canvas, "blur", function() { call( "_ae_OnFocus", 0 ); } );
	// Audio only starts from a user activation
	for( const type of [ "pointerdown", "keydown", "touchend" ] )
	{
		listen( canvas, type, audio.unlock );
	}
	listen( canvas, "wheel", function( e )
	{
		// A trackpad's legacy delta is -3 * delta, truncated by WebKit
		const legacy = e.wheelDeltaY || e.wheelDeltaX;
		const delta = e.wheelDeltaY ? e.deltaY : e.deltaX;
		const trackpad = legacy ? ( Math.abs( legacy + 3 * delta ) < 1 ) : ( e.deltaMode === 0 );
		call( "_ae_OnWheel", e.deltaX, e.deltaY, e.deltaMode, e.ctrlKey ? 1 : 0, trackpad ? 1 : 0 );
		e.preventDefault();
	}, { passive: false } );
	// Safari reports a trackpad pinch as gesture events, sent on as the pinch
	// wheel event. Touchscreen gestures are left to the touch events.
	let gestureScale = 1;
	let touches = 0;
	listen( canvas, "gesturestart", function( e ) { e.preventDefault(); gestureScale = 1; }, { passive: false } );
	listen( canvas, "gesturechange", function( e )
	{
		e.preventDefault();
		if( !touches ) { call( "_ae_OnWheel", 0, -Math.log( e.scale / gestureScale ) * 100, 0, 1, 0 ); }
		gestureScale = e.scale;
	}, { passive: false } );
	const touch = function( phase )
	{
		return function( e )
		{
			touches = e.targetTouches.length;
			for( const t of e.changedTouches )
			{
				const [ x, y ] = local( t );
				call( "_ae_OnTouch", phase, t.identifier, x, y );
			}
			e.preventDefault();
		};
	};
	// @TODO: Every touch is captured, so a page embedding the canvas can't
	// scroll over it. ae::Input should let the program declare, each frame
	// (immediate mode), rectangles that capture scrolls per direction, and let
	// the rest propagate like a scrollable sub-frame in a web page.
	listen( canvas, "touchstart", touch( 0 ), { passive: false } );
	listen( canvas, "touchmove", touch( 1 ), { passive: false } );
	listen( canvas, "touchend", touch( 2 ), { passive: false } );
	listen( canvas, "touchcancel", touch( 3 ), { passive: false } );
	// The page being hidden may be the last chance to save, see ae::ApplicationParams::suspend
	listen( document, "visibilitychange", function()
	{
		if( document.hidden )
		{
			call( "_ae_Suspend" );
			pause();
		}
		else
		{
			call( "_ae_Resume" );
			resume();
		}
	} );
	if( document.hasFocus() && ( !document.activeElement || document.activeElement === document.body ) )
	{
		canvas.focus( { preventScroll: true } );
	}
	if( document.hasFocus() && document.activeElement === canvas )
	{
		call( "_ae_OnFocus", 1 );
	}

	const instance = {
		pause: pause,
		resume: resume,
		terminate: function()
		{
			if( terminated ) { return; }
			call( "_ae_Terminate" );
			terminated = true;
			pause();
			listeners.forEach( function( [ target, type, fn, opts ] ) { target.removeEventListener( type, fn, opts ); } );
			listeners.length = 0;
			reads.forEach( function( controller ) { controller.abort(); } );
			reads.clear();
			const lose = gl && gl.getExtension( "WEBGL_lose_context" );
			if( lose ) { lose.loseContext(); }
			audio.close();
			exports = null;
			instance._exports = null;
		},
		stats: function()
		{
			const now = performance.now();
			const elapsed = ( now - stats.last ) / 1000;
			const result = {
				fps: stats.frames / elapsed,
				frameMs: stats.frames ? stats.busyMs / stats.frames : 0,
				draws: stats.frames ? stats.draws / stats.frames : 0,
				api: gl ? "WebGL 2" : ( gpu.active() ? "WebGPU" : "" ),
				renderer: gl ? gl.getParameter( gl.RENDERER ) : "",
				width: canvas.width,
				height: canvas.height,
				pixelRatio: window.devicePixelRatio || 1,
				heapBytes: exports ? memory().byteLength : 0,
				wasmBytes: program.bytes,
			};
			stats.frames = 0;
			stats.busyMs = 0;
			stats.draws = 0;
			stats.last = now;
			return result;
		},
		// @NOTE: Placeholder. The program's exports, until calls between
		// instances have an API.
		_exports: exports,
	};
	if( Symbol.dispose )
	{
		instance[ Symbol.dispose ] = instance.terminate;
	}
	resume();
	return instance;
}

/**
 * @typedef {Object} InstantiateOptions
 * @property {( text: string ) => void} [print] Receives the program's log
 *   output. Defaults to console.log.
 * @property {( url: string, loaded: number, total: number, finished: boolean ) => void} [onProgress]
 *   Reports each download, the program's own wasm and every file it reads
 *   with ae::FileSystem. total is 0 when the server does not say.
 * @property {( reason: string ) => void} [onHalt] Called once if the program
 *   stops on its own, eg. by trapping.
 * @property {Object<string, Object<string, Function>>} [imports] Extra import
 *   modules, for functions a program declares with AE_WASM_IMPORT(). Must not
 *   name a module aether.js provides. @NOTE: Placeholder.
 */

/**
 * A running program, see instantiate().
 * @typedef {Object} AetherInstance
 * @property {() => void} pause Stops calling the program's update until
 *   resume(). Does not call the program's suspend.
 * @property {() => void} resume Continues calling update after pause().
 * @property {() => void} terminate Calls the program's terminate, then frees
 *   its graphics, audio and listeners. The instance is unusable afterwards and
 *   its memory is freed once the page drops it.
 * @property {() => void} [Symbol.dispose] Calls terminate(), so a page can
 *   write `using game = await aether.instantiate( ... )`.
 * @property {() => AetherStats} stats Measurements since the previous call.
 */

/**
 * @typedef {Object} AetherStats
 * @property {number} fps Update calls per second.
 * @property {number} frameMs CPU time per update, in milliseconds.
 * @property {number} draws Draw calls per update, WebGL only.
 * @property {string} api "WebGL 2", "WebGPU", or empty without graphics.
 * @property {string} renderer The GPU reported by WebGL.
 * @property {number} width Drawing buffer width in pixels.
 * @property {number} height Drawing buffer height in pixels.
 * @property {number} pixelRatio The display's device pixel ratio.
 * @property {number} heapBytes Size of the program's memory.
 * @property {number} wasmBytes Size of the program's wasm.
 */

//------------------------------------------------------------------------------
// Internal
//------------------------------------------------------------------------------
const programs = new Map(); // url -> Promise of a compiled program
// Thrown by the ae._ae_RunHostLoop import, unwinding main() back into the host
const hostLoop = { aether: "host loop" };
// Thrown by the wasi proc_exit import
class ProgramExit
{
	constructor( code ) { this.code = code; }
}

// Downloads and compiles a program once per url, reporting download progress.
function compileProgram( url, onProgress )
{
	if( !programs.has( url ) )
	{
		programs.set( url, ( async function()
		{
			const response = await fetch( url );
			if( !response.ok )
			{
				throw new Error( "aether.js: " + url + " HTTP " + response.status );
			}
			const bytes = await readResponse( response, url, onProgress );
			return {
				module: await WebAssembly.compile( bytes ),
				bytes: bytes.length,
			};
		} )() );
	}
	return programs.get( url );
}

// The body of a successful response, reporting progress as it arrives
async function readResponse( response, url, onProgress )
{
	// Compressed responses report the compressed size, so no total
	const total = response.headers.get( "Content-Encoding" ) ? 0 : Number( response.headers.get( "Content-Length" ) ) || 0;
	const reader = response.body.getReader();
	const chunks = [];
	let loaded = 0;
	for( ;; )
	{
		const chunk = await reader.read();
		if( chunk.done ) { break; }
		chunks.push( chunk.value );
		loaded += chunk.value.length;
		onProgress( url, loaded, total, false );
	}
	const bytes = new Uint8Array( loaded );
	let offset = 0;
	chunks.forEach( function( c ) { bytes.set( c, offset ); offset += c.length; } );
	onProgress( url, loaded, total, true );
	return bytes;
}

// The UTF-8 string at ptr, ending at a terminator or after length bytes
function readString( buffer, ptr, length = -1 )
{
	const bytes = new Uint8Array( buffer );
	let end = ptr;
	while( length < 0 ? bytes[ end ] : end < ptr + length ) { end++; }
	return new TextDecoder().decode( bytes.subarray( ptr, end ) );
}

// Writes text and a terminator at ptr, truncated to fit size bytes, and
// returns the length written without the terminator
function writeString( buffer, text, ptr, size )
{
	if( size <= 0 ) { return 0; }
	const encoded = new TextEncoder().encode( text ).subarray( 0, size - 1 );
	const bytes = new Uint8Array( buffer );
	bytes.set( encoded, ptr );
	bytes[ ptr + encoded.length ] = 0;
	return encoded.length;
}

// The part of OpenAL 1.1 that aether uses, on one AudioContext per instance.
// Sources are unpositioned, and the context starts on the first unlock().
function alImports( memory )
{
	const kPlaying = 0x1012;
	const kStopped = 0x1014;
	let context = null;
	let master = null;
	const buffers = [ null ]; // id -> AudioBuffer
	const sources = [ null ]; // id -> source state
	const stop = function( s )
	{
		if( s.node )
		{
			s.node.onended = null;
			s.node.stop();
			s.node.disconnect();
			s.node = null;
		}
	};
	const imports = {
		alcOpenDevice: function() { return 1; },
		alcCloseDevice: function() { return 1; },
		alcCreateContext: function()
		{
			if( !context )
			{
				context = new AudioContext();
				master = context.createGain();
				master.connect( context.destination );
			}
			return 1;
		},
		alcDestroyContext: function() {},
		alcMakeContextCurrent: function() { return 1; },
		alcGetCurrentContext: function() { return context ? 1 : 0; },
		alcGetContextsDevice: function() { return 1; },
		alGetError: function() { return 0; },
		alGenBuffers: function( n, ptr )
		{
			for( let i = 0; i < n; i++ )
			{
				new DataView( memory() ).setUint32( ptr + i * 4, buffers.length, true );
				buffers.push( null );
			}
		},
		alDeleteBuffers: function( n, ptr )
		{
			for( let i = 0; i < n; i++ ) { buffers[ new DataView( memory() ).getUint32( ptr + i * 4, true ) ] = null; }
		},
		alBufferData: function( id, format, ptr, size, frequency )
		{
			const channels = ( format === 0x1102 || format === 0x1103 ) ? 2 : 1;
			const sampleBytes = ( format === 0x1101 || format === 0x1103 ) ? 2 : 1;
			const frames = Math.floor( size / ( channels * sampleBytes ) );
			const buffer = context.createBuffer( channels, Math.max( 1, frames ), frequency );
			const dv = new DataView( memory(), ptr, size );
			for( let c = 0; c < channels; c++ )
			{
				const out = buffer.getChannelData( c );
				for( let f = 0; f < frames; f++ )
				{
					const offset = ( f * channels + c ) * sampleBytes;
					out[ f ] = ( sampleBytes === 2 ) ? dv.getInt16( offset, true ) / 32768 : ( dv.getUint8( offset ) - 128 ) / 128;
				}
			}
			buffers[ id ] = buffer;
		},
		alGenSources: function( n, ptr )
		{
			for( let i = 0; i < n; i++ )
			{
				const gain = context.createGain();
				gain.connect( master );
				new DataView( memory() ).setUint32( ptr + i * 4, sources.length, true );
				sources.push( { gain: gain, node: null, buffer: 0, loop: false, pitch: 1, start: 0 } );
			}
		},
		alDeleteSources: function( n, ptr )
		{
			for( let i = 0; i < n; i++ )
			{
				const id = new DataView( memory() ).getUint32( ptr + i * 4, true );
				const s = sources[ id ];
				if( s )
				{
					stop( s );
					s.gain.disconnect();
					sources[ id ] = null;
				}
			}
		},
		alSourcef: function( id, param, value )
		{
			const s = sources[ id ];
			if( !s ) { return; }
			if( param === 0x100A ) { s.gain.gain.value = value; } // AL_GAIN
			else if( param === 0x1003 ) // AL_PITCH
			{
				s.pitch = value;
				if( s.node ) { s.node.playbackRate.value = value; }
			}
		},
		alSource3f: function() {},
		alSourcei: function( id, param, value )
		{
			const s = sources[ id ];
			if( !s ) { return; }
			if( param === 0x1007 ) // AL_LOOPING
			{
				s.loop = !!value;
				if( s.node ) { s.node.loop = s.loop; }
			}
			else if( param === 0x1009 ) { s.buffer = value; } // AL_BUFFER
		},
		alGetSourcef: function( id, param, ptr )
		{
			const s = sources[ id ];
			const buffer = s && s.node && buffers[ s.buffer ];
			const offset = buffer ? ( ( context.currentTime - s.start ) * s.pitch ) % buffer.duration : 0;
			new DataView( memory() ).setFloat32( ptr, param === 0x1024 ? offset : 0, true ); // AL_SEC_OFFSET
		},
		alGetSourcei: function( id, param, ptr )
		{
			const s = sources[ id ];
			const value = ( param === 0x1010 ) ? ( s && s.node ? kPlaying : kStopped ) : 0; // AL_SOURCE_STATE
			new DataView( memory() ).setInt32( ptr, value, true );
		},
		alSourcePlay: function( id )
		{
			const s = sources[ id ];
			if( !s || !buffers[ s.buffer ] ) { return; }
			stop( s );
			const node = context.createBufferSource();
			node.buffer = buffers[ s.buffer ];
			node.loop = s.loop;
			node.playbackRate.value = s.pitch;
			node.connect( s.gain );
			node.onended = function() { if( s.node === node ) { s.node = null; } };
			node.start();
			s.node = node;
			s.start = context.currentTime;
		},
		alSourceStop: function( id ) { if( sources[ id ] ) { stop( sources[ id ] ); } },
		alListenerf: function( param, value ) { if( param === 0x100A && master ) { master.gain.value = value; } },
		alListener3f: function() {},
		alListenerfv: function() {},
	};
	return {
		imports: imports,
		unlock: function() { if( context && context.state === "suspended" ) { context.resume(); } },
		close: function() { if( context && context.state !== "closed" ) { context.close(); } },
	};
}

// The part of WASI preview 1 that aether uses: output, clocks, randomness, the
// program's url as its only argument, and no environment, input or files.
function wasiImports( memory, print, url )
{
	const argument = new TextEncoder().encode( url + "\0" );
	const kSuccess = 0;
	const kBadFile = 8;
	const kNoEntry = 44;
	const lines = { 1: "", 2: "" };
	const view = function() { return new DataView( memory() ); };
	const empty = function( countPtr, sizePtr )
	{
		view().setUint32( countPtr, 0, true );
		view().setUint32( sizePtr, 0, true );
		return kSuccess;
	};
	return {
		args_sizes_get: function( countPtr, sizePtr )
		{
			view().setUint32( countPtr, 1, true );
			view().setUint32( sizePtr, argument.length, true );
			return kSuccess;
		},
		args_get: function( argvPtr, bufferPtr )
		{
			view().setUint32( argvPtr, bufferPtr, true );
			new Uint8Array( memory(), bufferPtr, argument.length ).set( argument );
			return kSuccess;
		},
		environ_sizes_get: empty,
		environ_get: function() { return kSuccess; },
		clock_time_get: function( id, precision, timePtr )
		{
			const ns = ( id === 0 ) ? BigInt( Date.now() ) * 1000000n : BigInt( Math.round( performance.now() * 1000000 ) );
			view().setBigUint64( timePtr, ns, true );
			return kSuccess;
		},
		random_get: function( ptr, length )
		{
			for( let offset = 0; offset < length; offset += 65536 )
			{
				crypto.getRandomValues( new Uint8Array( memory(), ptr + offset, Math.min( 65536, length - offset ) ) );
			}
			return kSuccess;
		},
		fd_write: function( fd, iovs, iovsLength, writtenPtr )
		{
			const dv = view();
			let written = 0;
			let text = "";
			for( let i = 0; i < iovsLength; i++ )
			{
				const ptr = dv.getUint32( iovs + i * 8, true );
				const length = dv.getUint32( iovs + i * 8 + 4, true );
				text += new TextDecoder().decode( new Uint8Array( memory(), ptr, length ) );
				written += length;
			}
			if( lines[ fd ] !== undefined )
			{
				const all = ( lines[ fd ] + text ).split( "\n" );
				lines[ fd ] = all.pop();
				all.forEach( function( line ) { print( line ); } );
			}
			dv.setUint32( writtenPtr, written, true );
			return kSuccess;
		},
		fd_read: function( fd, iovs, iovsLength, readPtr ) { view().setUint32( readPtr, 0, true ); return kSuccess; },
		fd_close: function() { return kBadFile; },
		fd_fdstat_set_flags: function() { return kBadFile; },
		path_open: function() { return kNoEntry; },
		fd_seek: function() { return kBadFile; },
		fd_fdstat_get: function( fd, statPtr )
		{
			if( fd > 2 ) { return kBadFile; }
			new Uint8Array( memory(), statPtr, 24 ).fill( 0 );
			view().setUint8( statPtr, 2 ); // Character device
			return kSuccess;
		},
		fd_prestat_get: function() { return kBadFile; },
		fd_prestat_dir_name: function() { return kBadFile; },
		proc_exit: function( code ) { throw new ProgramExit( code ); },
	};
}

// OpenGL ES 3 on WebGL 2. Objects are referred to by integer ids, and
// pointers are offsets into the program's memory.
function glImports( context, memory, stats )
{
	const objects = [ null ]; // id -> WebGL object, 0 is none
	const locations = []; // id -> WebGLUniformLocation
	let unpackAlignment = 4;
	let unpackRowLength = 0;
	const add = function( object )
	{
		object.aeId = objects.length;
		objects.push( object );
		return object.aeId;
	};
	const get = function( id ) { return objects[ id ] || null; };
	const u8 = function() { return new Uint8Array( memory() ); };
	const dv = function() { return new DataView( memory() ); };
	const string = function( ptr, length ) { return readString( memory(), ptr, length ); };
	const writeLog = function( text, ptr, bufSize, lengthPtr )
	{
		const length = writeString( memory(), text, ptr, bufSize );
		if( lengthPtr ) { dv().setInt32( lengthPtr, length, true ); }
	};
	// A typed view of count elements at ptr, copied when ptr is not aligned for the type
	const typed = function( Type, ptr, count )
	{
		if( ptr % Type.BYTES_PER_ELEMENT === 0 ) { return new Type( memory(), ptr, count ); }
		return new Type( memory().slice( ptr, ptr + count * Type.BYTES_PER_ELEMENT ) );
	};
	const gen = function( create )
	{
		return function( n, idsPtr )
		{
			for( let i = 0; i < n; i++ ) { dv().setUint32( idsPtr + i * 4, add( create() ), true ); }
		};
	};
	const del = function( destroy )
	{
		return function( n, idsPtr )
		{
			for( let i = 0; i < n; i++ )
			{
				const id = dv().getUint32( idsPtr + i * 4, true );
				if( objects[ id ] ) { destroy( objects[ id ] ); objects[ id ] = null; }
			}
		};
	};
	const channels = { 0x1908: 4, 0x1907: 3, 0x8227: 2, 0x1903: 1, 0x8D94: 1, 0x1902: 1, 0x84F9: 1, 0x1909: 1, 0x190A: 2, 0x1906: 1, 0x8D99: 4 };
	// The pixels of a width by height image at ptr, as the array type WebGL expects
	const pixels = function( width, height, format, type, ptr )
	{
		if( !ptr ) { return null; }
		let Type = Uint8Array;
		let bytes = channels[ format ] || 4;
		if( type === 0x1406 ) { Type = Float32Array; bytes *= 4; } // FLOAT
		else if( type === 0x140B || type === 0x1403 ) { Type = Uint16Array; bytes *= 2; } // HALF_FLOAT, UNSIGNED_SHORT
		else if( type === 0x1405 || type === 0x84FA ) { Type = Uint32Array; bytes *= 4; } // UNSIGNED_INT, UNSIGNED_INT_24_8
		else if( type === 0x8363 || type === 0x8033 || type === 0x8034 ) { Type = Uint16Array; bytes = 2; } // Packed 16 bit
		const row = Math.ceil( ( unpackRowLength || width ) * bytes / unpackAlignment ) * unpackAlignment;
		const size = height ? row * ( height - 1 ) + width * bytes : 0;
		return typed( Type, ptr, size / Type.BYTES_PER_ELEMENT );
	};
	const draw = function( fn )
	{
		return function( ...args ) { stats.draws++; return fn( ...args ); };
	};
	const programInfo = function( program, pname )
	{
		const p = get( program );
		if( pname === 0x8B84 ) { return ( context().getProgramInfoLog( p ) || "" ).length + 1; } // INFO_LOG_LENGTH
		if( pname === 0x8B8A || pname === 0x8B87 ) // ACTIVE_ATTRIBUTE_MAX_LENGTH, ACTIVE_UNIFORM_MAX_LENGTH
		{
			const attributes = pname === 0x8B8A;
			const count = context().getProgramParameter( p, attributes ? context().ACTIVE_ATTRIBUTES : context().ACTIVE_UNIFORMS );
			let longest = 0;
			for( let i = 0; i < count; i++ )
			{
				const info = attributes ? context().getActiveAttrib( p, i ) : context().getActiveUniform( p, i );
				longest = Math.max( longest, info.name.length + 1 );
			}
			return longest;
		}
		return Number( context().getProgramParameter( p, pname ) );
	};
	const writeActive = function( info, bufSize, lengthPtr, sizePtr, typePtr, namePtr )
	{
		writeLog( info.name, namePtr, bufSize, lengthPtr );
		dv().setInt32( sizePtr, info.size, true );
		dv().setUint32( typePtr, info.type, true );
	};
	return {
		glClear: function( mask ) { context().clear( mask ); },
		glClearColor: function( r, g, b, a ) { context().clearColor( r, g, b, a ); },
		glClearDepthf: function( depth ) { context().clearDepth( depth ); },
		glViewport: function( x, y, width, height ) { context().viewport( x, y, width, height ); },
		glEnable: function( cap ) { context().enable( cap ); },
		glDisable: function( cap ) { context().disable( cap ); },
		glDepthMask: function( flag ) { context().depthMask( !!flag ); },
		glDepthFunc: function( func ) { context().depthFunc( func ); },
		glBlendFunc: function( s, d ) { context().blendFunc( s, d ); },
		glBlendFuncSeparate: function( sRgb, dRgb, sA, dA ) { context().blendFuncSeparate( sRgb, dRgb, sA, dA ); },
		glFrontFace: function( mode ) { context().frontFace( mode ); },
		glGetError: function() { return context().getError(); },
		glGetIntegerv: function( pname, dataPtr )
		{
			// OpenGL ES 3 queries WebGL leaves out
			const es = { 0x821B: 3, 0x821C: 0 }; // MAJOR_VERSION, MINOR_VERSION
			const value = ( pname in es ) ? es[ pname ] : context().getParameter( pname );
			if( value && typeof value === "object" && "length" in value )
			{
				for( let i = 0; i < value.length; i++ ) { dv().setInt32( dataPtr + i * 4, Number( value[ i ] ), true ); }
			}
			else
			{
				dv().setInt32( dataPtr, value && typeof value === "object" ? ( value.aeId || 0 ) : Number( value || 0 ), true );
			}
		},
		glPixelStorei: function( pname, param )
		{
			if( pname === 0x0CF5 ) { unpackAlignment = param; } // UNPACK_ALIGNMENT
			if( pname === 0x0CF2 ) { unpackRowLength = param; } // UNPACK_ROW_LENGTH
			context().pixelStorei( pname, param );
		},
		glPolygonMode: function() {}, // Not in WebGL, aether asserts wireframe is off
		glGetString: function() { return 0; },
		glIsEnabled: function( cap ) { return context().isEnabled( cap ) ? 1 : 0; },
		glScissor: function( x, y, width, height ) { context().scissor( x, y, width, height ); },
		glBlendEquation: function( mode ) { context().blendEquation( mode ); },
		glBlendEquationSeparate: function( modeRgb, modeAlpha ) { context().blendEquationSeparate( modeRgb, modeAlpha ); },
		glDebugMessageCallback: function() {},
		glTextureBarrierNV: function() {},
		glActiveTexture: function( texture ) { context().activeTexture( texture ); },
		glBindTexture: function( target, id ) { context().bindTexture( target, get( id ) ); },
		glGenTextures: gen( function() { return context().createTexture(); } ),
		glDeleteTextures: del( function( t ) { context().deleteTexture( t ); } ),
		glTexParameteri: function( target, pname, param ) { context().texParameteri( target, pname, param ); },
		glTexImage2D: function( target, level, internalFormat, width, height, border, format, type, ptr )
		{
			context().texImage2D( target, level, internalFormat, width, height, border, format, type, pixels( width, height, format, type, ptr ) );
		},
		glTexSubImage2D: function( target, level, x, y, width, height, format, type, ptr )
		{
			context().texSubImage2D( target, level, x, y, width, height, format, type, pixels( width, height, format, type, ptr ) );
		},
		glTexStorage2D: function( target, levels, internalFormat, width, height ) { context().texStorage2D( target, levels, internalFormat, width, height ); },
		glGenerateMipmap: function( target ) { context().generateMipmap( target ); },
		glCreateShader: function( type ) { return add( context().createShader( type ) ); },
		glShaderSource: function( shader, count, stringsPtr, lengthsPtr )
		{
			let source = "";
			for( let i = 0; i < count; i++ )
			{
				const length = lengthsPtr ? dv().getInt32( lengthsPtr + i * 4, true ) : -1;
				source += string( dv().getUint32( stringsPtr + i * 4, true ), length );
			}
			context().shaderSource( get( shader ), source );
		},
		glCompileShader: function( shader ) { context().compileShader( get( shader ) ); },
		glGetShaderiv: function( shader, pname, paramsPtr )
		{
			const s = get( shader );
			const value = ( pname === 0x8B84 ) ? ( context().getShaderInfoLog( s ) || "" ).length + 1 : Number( context().getShaderParameter( s, pname ) );
			dv().setInt32( paramsPtr, value, true );
		},
		glGetShaderInfoLog: function( shader, bufSize, lengthPtr, logPtr ) { writeLog( context().getShaderInfoLog( get( shader ) ) || "", logPtr, bufSize, lengthPtr ); },
		glDetachShader: function( program, shader ) { context().detachShader( get( program ), get( shader ) ); },
		glIsProgram: function( program ) { return context().isProgram( get( program ) ) ? 1 : 0; },
		glDeleteShader: function( shader ) { context().deleteShader( get( shader ) ); objects[ shader ] = null; },
		glCreateProgram: function() { return add( context().createProgram() ); },
		glAttachShader: function( program, shader ) { context().attachShader( get( program ), get( shader ) ); },
		glLinkProgram: function( program ) { context().linkProgram( get( program ) ); },
		glGetProgramiv: function( program, pname, paramsPtr ) { dv().setInt32( paramsPtr, programInfo( program, pname ), true ); },
		glGetProgramInfoLog: function( program, bufSize, lengthPtr, logPtr ) { writeLog( context().getProgramInfoLog( get( program ) ) || "", logPtr, bufSize, lengthPtr ); },
		glGetActiveAttrib: function( program, index, bufSize, lengthPtr, sizePtr, typePtr, namePtr ) { writeActive( context().getActiveAttrib( get( program ), index ), bufSize, lengthPtr, sizePtr, typePtr, namePtr ); },
		glGetActiveUniform: function( program, index, bufSize, lengthPtr, sizePtr, typePtr, namePtr ) { writeActive( context().getActiveUniform( get( program ), index ), bufSize, lengthPtr, sizePtr, typePtr, namePtr ); },
		glGetAttribLocation: function( program, namePtr ) { return context().getAttribLocation( get( program ), string( namePtr ) ); },
		glGetUniformLocation: function( program, namePtr )
		{
			const location = context().getUniformLocation( get( program ), string( namePtr ) );
			if( !location ) { return -1; }
			locations.push( location );
			return locations.length - 1;
		},
		glDeleteProgram: function( program ) { context().deleteProgram( get( program ) ); objects[ program ] = null; },
		glUseProgram: function( program ) { context().useProgram( get( program ) ); },
		glUniform1i: function( location, v ) { context().uniform1i( locations[ location ] || null, v ); },
		glUniform1fv: function( location, count, ptr ) { context().uniform1fv( locations[ location ] || null, typed( Float32Array, ptr, count ) ); },
		glUniform2fv: function( location, count, ptr ) { context().uniform2fv( locations[ location ] || null, typed( Float32Array, ptr, count * 2 ) ); },
		glUniform3fv: function( location, count, ptr ) { context().uniform3fv( locations[ location ] || null, typed( Float32Array, ptr, count * 3 ) ); },
		glUniform4fv: function( location, count, ptr ) { context().uniform4fv( locations[ location ] || null, typed( Float32Array, ptr, count * 4 ) ); },
		glUniformMatrix4fv: function( location, count, transpose, ptr ) { context().uniformMatrix4fv( locations[ location ] || null, !!transpose, typed( Float32Array, ptr, count * 16 ) ); },
		glBindFramebuffer: function( target, id ) { context().bindFramebuffer( target, get( id ) ); },
		glGenFramebuffers: gen( function() { return context().createFramebuffer(); } ),
		glDeleteFramebuffers: del( function( f ) { context().deleteFramebuffer( f ); } ),
		glFramebufferTexture2D: function( target, attachment, texTarget, texture, level ) { context().framebufferTexture2D( target, attachment, texTarget, get( texture ), level ); },
		glCheckFramebufferStatus: function( target ) { return context().checkFramebufferStatus( target ); },
		glDrawBuffers: function( n, ptr ) { context().drawBuffers( Array.from( typed( Uint32Array, ptr, n ) ) ); },
		glGenVertexArrays: gen( function() { return context().createVertexArray(); } ),
		glBindVertexArray: function( id ) { context().bindVertexArray( get( id ) ); },
		glDeleteVertexArrays: del( function( v ) { context().deleteVertexArray( v ); } ),
		glGenBuffers: gen( function() { return context().createBuffer(); } ),
		glBindBuffer: function( target, id ) { context().bindBuffer( target, get( id ) ); },
		glDeleteBuffers: del( function( b ) { context().deleteBuffer( b ); } ),
		glBufferData: function( target, size, ptr, usage )
		{
			if( ptr ) { context().bufferData( target, u8().subarray( ptr, ptr + size ), usage ); }
			else { context().bufferData( target, size, usage ); }
		},
		glBufferSubData: function( target, offset, size, ptr ) { context().bufferSubData( target, offset, u8().subarray( ptr, ptr + size ) ); },
		glEnableVertexAttribArray: function( index ) { context().enableVertexAttribArray( index ); },
		glDisableVertexAttribArray: function( index ) { context().disableVertexAttribArray( index ); },
		glGetVertexAttribiv: function( index, pname, ptr )
		{
			const value = context().getVertexAttrib( index, pname );
			dv().setInt32( ptr, value && typeof value === "object" ? ( value.aeId || 0 ) : Number( value ), true );
		},
		glGetVertexAttribPointerv: function( index, pname, ptr ) { dv().setUint32( ptr, context().getVertexAttribOffset( index, pname ), true ); },
		glGenSamplers: gen( function() { return context().createSampler(); } ),
		glDeleteSamplers: del( function( s ) { context().deleteSampler( s ); } ),
		glBindSampler: function( unit, id ) { context().bindSampler( unit, get( id ) ); },
		glSamplerParameteri: function( id, pname, param ) { context().samplerParameteri( get( id ), pname, param ); },
		glVertexAttribPointer: function( index, size, type, normalized, stride, offset ) { context().vertexAttribPointer( index, size, type, !!normalized, stride, offset ); },
		glVertexAttribDivisor: function( index, divisor ) { context().vertexAttribDivisor( index, divisor ); },
		glDrawArrays: draw( function( mode, first, count ) { context().drawArrays( mode, first, count ); } ),
		glDrawElements: draw( function( mode, count, type, offset ) { context().drawElements( mode, count, type, offset ); } ),
		glDrawArraysInstanced: draw( function( mode, first, count, instances ) { context().drawArraysInstanced( mode, first, count, instances ); } ),
		glDrawElementsInstanced: draw( function( mode, count, type, offset, instances ) { context().drawElementsInstanced( mode, count, type, offset, instances ); } ),
	};
}

// The part of webgpu.h (webgpu-headers 7d3186c, as used by wgpu-native v29)
// that aether's examples use. Objects are integer ids. A surface descriptor
// without a source is the instance's canvas. host provides memory(), call()
// for exports and callback() for function pointers.
function webgpuImports( canvas, host )
{
	const kStrlen = 0xFFFFFFFF;
	const objects = [ null ]; // id -> GPU object, 0 is none
	let context = null;
	let surfaceTextureId = 0;
	const add = function( object ) { objects.push( object ); return objects.length - 1; };
	const get = function( id ) { return objects[ id ] || undefined; };
	const release = function( id ) { if( id > 1 ) { objects[ id ] = null; } };
	const dv = function() { return new DataView( host.memory() ); };
	const u32 = function( ptr ) { return dv().getUint32( ptr, true ); };
	const u64 = function( ptr ) { return Number( dv().getBigUint64( ptr, true ) ); };
	// A WGPUStringView, or undefined when null
	const view = function( ptr )
	{
		const data = u32( ptr );
		const length = u32( ptr + 4 );
		if( !data ) { return undefined; }
		return readString( host.memory(), data, length === kStrlen ? -1 : length );
	};
	const unsupported = function( what ) { throw new Error( "aether.js: webgpu " + what + " is not supported" ); };
	const textureFormats = [ undefined, "r8unorm", "r8snorm", "r8uint", "r8sint", "r16unorm", "r16snorm", "r16uint", "r16sint", "r16float",
		"rg8unorm", "rg8snorm", "rg8uint", "rg8sint", "r32float", "r32uint", "r32sint", "rg16unorm", "rg16snorm", "rg16uint", "rg16sint",
		"rg16float", "rgba8unorm", "rgba8unorm-srgb", "rgba8snorm", "rgba8uint", "rgba8sint", "bgra8unorm", "bgra8unorm-srgb",
		"rgb10a2uint", "rgb10a2unorm", "rg11b10ufloat", "rgb9e5ufloat", "rg32float", "rg32uint", "rg32sint", "rgba16unorm",
		"rgba16snorm", "rgba16uint", "rgba16sint", "rgba16float", "rgba32float", "rgba32uint", "rgba32sint", "stencil8",
		"depth16unorm", "depth24plus", "depth24plus-stencil8", "depth32float", "depth32float-stencil8" ];
	const vertexFormats = [ undefined, "uint8", "uint8x2", "uint8x4", "sint8", "sint8x2", "sint8x4", "unorm8", "unorm8x2", "unorm8x4",
		"snorm8", "snorm8x2", "snorm8x4", "uint16", "uint16x2", "uint16x4", "sint16", "sint16x2", "sint16x4", "unorm16", "unorm16x2",
		"unorm16x4", "snorm16", "snorm16x2", "snorm16x4", "float16", "float16x2", "float16x4", "float32", "float32x2", "float32x3",
		"float32x4", "uint32", "uint32x2", "uint32x3", "uint32x4", "sint32", "sint32x2", "sint32x3", "sint32x4", "unorm10-10-10-2",
		"unorm8x4-bgra" ];
	const topologies = [ undefined, "point-list", "line-list", "line-strip", "triangle-list", "triangle-strip" ];
	const indexFormats = [ undefined, "uint16", "uint32" ];
	const frontFaces = [ undefined, "ccw", "cw" ];
	const cullModes = [ undefined, "none", "front", "back" ];
	const loadOps = [ undefined, "load", "clear" ];
	const storeOps = [ undefined, "store", "discard" ];
	const blendOperations = [ undefined, "add", "subtract", "reverse-subtract", "min", "max" ];
	const blendFactors = [ undefined, "zero", "one", "src", "one-minus-src", "src-alpha", "one-minus-src-alpha", "dst", "one-minus-dst",
		"dst-alpha", "one-minus-dst-alpha", "src-alpha-saturated", "constant", "one-minus-constant", "src1", "one-minus-src1",
		"src1-alpha", "one-minus-src1-alpha" ];
	const alphaModes = [ "opaque", "opaque", "premultiplied", "opaque", "opaque" ];
	const stepModes = [ undefined, "vertex", "instance" ];
	const viewDimensions = [ undefined, "1d", "2d", "2d-array", "cube", "cube-array", "3d" ];
	const aspects = [ undefined, "all", "stencil-only", "depth-only" ];
	const blendComponent = function( ptr )
	{
		return { operation: blendOperations[ u32( ptr ) ], srcFactor: blendFactors[ u32( ptr + 4 ) ], dstFactor: blendFactors[ u32( ptr + 8 ) ] };
	};
	// Writes a WGPUStringView of text into memory from _ae_Alloc(), freed by the returned function
	const message = function( text )
	{
		const bytes = new TextEncoder().encode( text || "" );
		const ptr = host.call( "_ae_Alloc", 8 + bytes.length );
		new Uint8Array( host.memory(), ptr + 8, bytes.length ).set( bytes );
		dv().setUint32( ptr, ptr + 8, true );
		dv().setUint32( ptr + 4, bytes.length, true );
		return { ptr: ptr, free: function() { host.call( "_ae_Free", ptr ); } };
	};
	// Calls the callback of a WGPURequest*CallbackInfo at infoPtr with the result of promise
	const request = function( promise, infoPtr )
	{
		const callback = u32( infoPtr + 8 );
		const userdata1 = u32( infoPtr + 12 );
		const userdata2 = u32( infoPtr + 16 );
		promise.then( function( object )
		{
			const text = message( object ? "" : "Unavailable" );
			host.callback( callback, object ? 1 : 3, object ? add( object ) : 0, text.ptr, userdata1, userdata2 );
			text.free();
		}, function( error )
		{
			const text = message( String( error ) );
			host.callback( callback, 3, 0, text.ptr, userdata1, userdata2 );
			text.free();
		} );
		return 0n;
	};
	const imports = {
		wgpuCreateInstance: function() { return navigator.gpu ? add( navigator.gpu ) : 0; },
		wgpuInstanceRelease: release,
		wgpuInstanceCreateSurface: function( instance, descPtr )
		{
			if( u32( descPtr ) ) { unsupported( "surface sources" ); }
			context = canvas.getContext( "webgpu" );
			return context ? add( context ) : 0;
		},
		wgpuSurfaceRelease: release,
		wgpuInstanceRequestAdapter: function( instance, optionsPtr, infoPtr )
		{
			const preference = optionsPtr ? [ undefined, "low-power", "high-performance" ][ u32( optionsPtr + 8 ) ] : undefined;
			return request( navigator.gpu.requestAdapter( { powerPreference: preference } ), infoPtr );
		},
		wgpuAdapterRelease: release,
		wgpuAdapterRequestDevice: function( adapter, descPtr, infoPtr ) { return request( get( adapter ).requestDevice(), infoPtr ); },
		wgpuDeviceRelease: release,
		wgpuDeviceGetQueue: function( device ) { return add( get( device ).queue ); },
		wgpuQueueRelease: release,
		wgpuSurfaceGetCapabilities: function( surface, adapter, capsPtr )
		{
			const format = textureFormats.indexOf( navigator.gpu.getPreferredCanvasFormat() );
			const formats = host.call( "_ae_Alloc", 4 );
			dv().setUint32( formats, format, true );
			new Uint8Array( host.memory(), capsPtr + 8, 32 ).fill( 0 );
			dv().setBigUint64( capsPtr + 8, 0x10n, true ); // RenderAttachment
			dv().setUint32( capsPtr + 16, 1, true );
			dv().setUint32( capsPtr + 20, formats, true );
			return 1;
		},
		wgpuSurfaceCapabilitiesFreeMembers: function( capsPtr )
		{
			[ 20, 28, 36 ].forEach( function( offset ) { if( u32( capsPtr + offset ) ) { host.call( "_ae_Free", u32( capsPtr + offset ) ); } } );
		},
		wgpuSurfaceConfigure: function( surface, configPtr )
		{
			canvas.width = u32( configPtr + 24 );
			canvas.height = u32( configPtr + 28 );
			get( surface ).configure( {
				device: get( u32( configPtr + 4 ) ),
				format: textureFormats[ u32( configPtr + 8 ) ],
				usage: u64( configPtr + 16 ),
				alphaMode: alphaModes[ u32( configPtr + 40 ) ],
			} );
		},
		wgpuSurfaceGetCurrentTexture: function( surface, outPtr )
		{
			// The canvas has one current texture, so its id is reused
			const texture = get( surface ).getCurrentTexture();
			if( surfaceTextureId ) { objects[ surfaceTextureId ] = texture; }
			else { surfaceTextureId = add( texture ); }
			dv().setUint32( outPtr + 4, surfaceTextureId, true );
			dv().setUint32( outPtr + 8, 1, true ); // SuccessOptimal
		},
		wgpuSurfacePresent: function() { return 1; }, // The browser presents when the task returns
		wgpuTextureRelease: function( id ) { if( id !== surfaceTextureId ) { release( id ); } },
		wgpuTextureCreateView: function( texture, descPtr )
		{
			const desc = {};
			if( descPtr )
			{
				desc.label = view( descPtr + 4 );
				desc.format = textureFormats[ u32( descPtr + 12 ) ];
				desc.dimension = viewDimensions[ u32( descPtr + 16 ) ];
				desc.baseMipLevel = u32( descPtr + 20 );
				if( u32( descPtr + 24 ) !== kStrlen ) { desc.mipLevelCount = u32( descPtr + 24 ); }
				desc.baseArrayLayer = u32( descPtr + 28 );
				if( u32( descPtr + 32 ) !== kStrlen ) { desc.arrayLayerCount = u32( descPtr + 32 ); }
				desc.aspect = aspects[ u32( descPtr + 36 ) ];
			}
			return add( get( texture ).createView( desc ) );
		},
		wgpuTextureViewRelease: release,
		wgpuDeviceCreateShaderModule: function( device, descPtr )
		{
			const chain = u32( descPtr );
			if( !chain || u32( chain + 4 ) !== 2 ) { unsupported( "shader sources other than WGSL" ); }
			return add( get( device ).createShaderModule( { label: view( descPtr + 4 ), code: view( chain + 8 ) } ) );
		},
		wgpuShaderModuleRelease: release,
		wgpuDeviceCreateRenderPipeline: function( device, descPtr )
		{
			if( u32( descPtr + 72 ) ) { unsupported( "depth stencil state" ); }
			const buffers = [];
			for( let i = 0; i < u32( descPtr + 16 + 24 ); i++ )
			{
				const layout = u32( descPtr + 16 + 28 ) + i * 24;
				const attributes = [];
				for( let j = 0; j < u32( layout + 16 ); j++ )
				{
					const a = u32( layout + 20 ) + j * 24;
					attributes.push( { format: vertexFormats[ u32( a + 4 ) ], offset: u64( a + 8 ), shaderLocation: u32( a + 16 ) } );
				}
				buffers.push( { stepMode: stepModes[ u32( layout + 4 ) ], arrayStride: u64( layout + 8 ), attributes: attributes } );
			}
			const desc = {
				label: view( descPtr + 4 ),
				layout: u32( descPtr + 12 ) ? get( u32( descPtr + 12 ) ) : "auto",
				vertex: { module: get( u32( descPtr + 16 + 4 ) ), entryPoint: view( descPtr + 16 + 8 ), buffers: buffers },
				primitive: {
					topology: topologies[ u32( descPtr + 48 + 4 ) ],
					stripIndexFormat: indexFormats[ u32( descPtr + 48 + 8 ) ],
					frontFace: frontFaces[ u32( descPtr + 48 + 12 ) ],
					cullMode: cullModes[ u32( descPtr + 48 + 16 ) ],
				},
				multisample: { count: u32( descPtr + 76 + 4 ) || 1, mask: u32( descPtr + 76 + 8 ), alphaToCoverageEnabled: !!u32( descPtr + 76 + 12 ) },
			};
			const fragment = u32( descPtr + 92 );
			if( fragment )
			{
				const targets = [];
				for( let i = 0; i < u32( fragment + 24 ); i++ )
				{
					const t = u32( fragment + 28 ) + i * 24;
					const blend = u32( t + 8 );
					targets.push( {
						format: textureFormats[ u32( t + 4 ) ],
						blend: blend ? { color: blendComponent( blend ), alpha: blendComponent( blend + 12 ) } : undefined,
						writeMask: u64( t + 16 ),
					} );
				}
				desc.fragment = { module: get( u32( fragment + 4 ) ), entryPoint: view( fragment + 8 ), targets: targets };
			}
			return add( get( device ).createRenderPipeline( desc ) );
		},
		wgpuRenderPipelineRelease: release,
		wgpuDeviceCreateBuffer: function( device, descPtr )
		{
			return add( get( device ).createBuffer( {
				label: view( descPtr + 4 ),
				usage: u64( descPtr + 16 ),
				size: u64( descPtr + 24 ),
				mappedAtCreation: !!u32( descPtr + 32 ),
			} ) );
		},
		wgpuBufferRelease: function( id ) { if( get( id ) ) { get( id ).destroy(); } release( id ); },
		wgpuQueueWriteBuffer: function( queue, buffer, offset, dataPtr, size )
		{
			get( queue ).writeBuffer( get( buffer ), Number( offset ), new Uint8Array( host.memory(), dataPtr, size ).slice() );
		},
		wgpuDeviceCreateCommandEncoder: function( device ) { return add( get( device ).createCommandEncoder() ); },
		wgpuCommandEncoderRelease: release,
		wgpuCommandEncoderBeginRenderPass: function( encoder, descPtr )
		{
			if( u32( descPtr + 20 ) ) { unsupported( "depth stencil attachments" ); }
			const colorAttachments = [];
			for( let i = 0; i < u32( descPtr + 12 ); i++ )
			{
				const a = u32( descPtr + 16 ) + i * 56;
				const depthSlice = u32( a + 8 );
				const d = dv();
				colorAttachments.push( {
					view: get( u32( a + 4 ) ),
					depthSlice: depthSlice === kStrlen ? undefined : depthSlice,
					resolveTarget: get( u32( a + 12 ) ),
					loadOp: loadOps[ u32( a + 16 ) ],
					storeOp: storeOps[ u32( a + 20 ) ],
					clearValue: [ d.getFloat64( a + 24, true ), d.getFloat64( a + 32, true ), d.getFloat64( a + 40, true ), d.getFloat64( a + 48, true ) ],
				} );
			}
			return add( get( encoder ).beginRenderPass( { label: view( descPtr + 4 ), colorAttachments: colorAttachments } ) );
		},
		wgpuRenderPassEncoderSetPipeline: function( pass, pipeline ) { get( pass ).setPipeline( get( pipeline ) ); },
		wgpuRenderPassEncoderSetVertexBuffer: function( pass, slot, buffer, offset, size )
		{
			get( pass ).setVertexBuffer( slot, get( buffer ), Number( offset ), size === 0xFFFFFFFFFFFFFFFFn ? undefined : Number( size ) );
		},
		wgpuRenderPassEncoderDraw: function( pass, vertexCount, instanceCount, firstVertex, firstInstance )
		{
			get( pass ).draw( vertexCount, instanceCount, firstVertex, firstInstance );
		},
		wgpuRenderPassEncoderEnd: function( pass ) { get( pass ).end(); },
		wgpuRenderPassEncoderRelease: release,
		wgpuCommandEncoderFinish: function( encoder ) { return add( get( encoder ).finish() ); },
		wgpuCommandBufferRelease: release,
		wgpuQueueSubmit: function( queue, count, ptr )
		{
			const buffers = [];
			for( let i = 0; i < count; i++ ) { buffers.push( get( u32( ptr + i * 4 ) ) ); }
			get( queue ).submit( buffers );
		},
	};
	return {
		imports: imports,
		active: function() { return !!context; },
	};
}
