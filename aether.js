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
// Runs programs built with aether.h in a browser. Host this file on the same
// origin as the page that imports it. Usage:
//
//   import * as aether from './aether.js';
//   const game = await aether.instantiate( 'game.wasm', document.querySelector( 'canvas' ) );
//
// An instance only touches the canvas it is given, see "DOM ownership" in the
// Web documentation.
//
// @NOTE: Placeholder. Programs are currently built with Emscripten, so this
// file loads the Emscripten module next to the wasm ( game.wasm -> game.js )
// and adapts it to this API. The API is the contract, its implementation will
// be replaced by a host for programs built with wasi-sdk.
//------------------------------------------------------------------------------

/**
 * The import ABI version this file provides. A program records the version it
 * was built for in an 'aether_abi' custom section, and instantiate() rejects a
 * mismatch before linking anything. Programs only import from these modules,
 * and instantiate() reports any other import as a missing definition:
 * - `ae`: aether's host functions
 * - `env`: OpenGL ES 3 functions, provided with WebGL 2. These move to a `gl`
 *   module before 1.0.
 * - `wasi_snapshot_preview1`: The WASI system interface
 *
 * @NOTE: Placeholder. Not checked until programs are built with wasi-sdk.
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
	const factory = ( await import( url.replace( /\.wasm$/, ".js" ) ) ).default;

	const stats = { frames: 0, busyMs: 0, draws: 0, start: 0, last: performance.now() };
	const audioContexts = [];
	let halted = false;
	let module = null;
	const halt = function( reason )
	{
		if( !halted )
		{
			halted = true;
			if( options.onHalt ) { options.onHalt( reason ); }
		}
	};

	// Audio contexts the program creates while starting are its own, so
	// terminate() can close them.
	const restoreAudio = trackAudioContexts( audioContexts );
	try
	{
		module = await factory( {
			canvas: canvas,
			print: print,
			printErr: print,
			aeFileProgress: function( fileUrl, loaded, total, finished )
			{
				onProgress( fileUrl, loaded, total, finished );
			},
			onAbort: function( what ) { halt( String( what || "aborted" ) ); },
			preMainLoop: function() { stats.start = performance.now(); },
			postMainLoop: function()
			{
				stats.busyMs += performance.now() - stats.start;
				stats.frames++;
			},
			instantiateWasm: function( imports, receive )
			{
				WebAssembly.instantiate( program.module, imports ).then(
					function( instance ) { receive( instance, program.module ); },
					function( error ) { halt( String( error ) ); }
				);
				return {};
			},
		} );
	}
	finally
	{
		restoreAudio();
	}
	if( halted )
	{
		throw new Error( "aether.js: the program halted while starting" );
	}

	const gl = module.ctx || null;
	if( gl )
	{
		countDraws( gl, stats );
	}
	const gpu = !gl && canvas.getContext( "webgpu" );

	const instance = {
		pause: function() { if( module ) { module.pauseMainLoop(); } },
		resume: function() { if( module ) { module.resumeMainLoop(); } },
		terminate: function()
		{
			if( !module ) { return; }
			module.pauseMainLoop();
			const lose = gl && gl.getExtension( "WEBGL_lose_context" );
			if( lose ) { lose.loseContext(); }
			audioContexts.forEach( function( context ) { if( context.state !== "closed" ) { context.close(); } } );
			audioContexts.length = 0;
			module = null;
			instance._module = null;
		},
		stats: function()
		{
			const now = performance.now();
			const elapsed = ( now - stats.last ) / 1000;
			const result = {
				fps: stats.frames / elapsed,
				frameMs: stats.frames ? stats.busyMs / stats.frames : 0,
				draws: stats.frames ? stats.draws / stats.frames : 0,
				api: gl ? "WebGL 2" : ( gpu ? "WebGPU" : "" ),
				renderer: gl ? gl.getParameter( gl.RENDERER ) : "",
				width: canvas.width,
				height: canvas.height,
				pixelRatio: window.devicePixelRatio || 1,
				heapBytes: module ? module.HEAPU8.buffer.byteLength : 0,
				wasmBytes: program.bytes,
			};
			stats.frames = 0;
			stats.busyMs = 0;
			stats.draws = 0;
			stats.last = now;
			return result;
		},
		// @NOTE: Internal. The underlying Emscripten module, until twin calls have an API.
		_module: module,
	};
	if( Symbol.dispose )
	{
		instance[ Symbol.dispose ] = instance.terminate;
	}
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
 */

/**
 * A running program, see instantiate().
 * @typedef {Object} AetherInstance
 * @property {() => void} pause Stops calling the program's update until
 *   resume(). Does not call the program's suspend.
 * @property {() => void} resume Continues calling update after pause().
 * @property {() => void} terminate Stops the program and frees its graphics
 *   and audio. The instance is unusable afterwards and its memory is freed
 *   once the page drops it. @NOTE: Placeholder. Does not yet call the program's
 *   terminate, or remove its listeners.
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
			return {
				module: await WebAssembly.compile( bytes ),
				bytes: loaded,
			};
		} )() );
	}
	return programs.get( url );
}

// Counts draw calls on one context, without touching WebGL2RenderingContext.
function countDraws( gl, stats )
{
	[ "drawArrays", "drawElements", "drawArraysInstanced", "drawElementsInstanced", "drawRangeElements" ].forEach( function( name )
	{
		const original = gl[ name ];
		if( original )
		{
			gl[ name ] = function() { stats.draws++; return original.apply( gl, arguments ); };
		}
	} );
}

// Records audio contexts created until the returned function is called.
function trackAudioContexts( list )
{
	const restore = [];
	[ "AudioContext", "webkitAudioContext" ].forEach( function( name )
	{
		const Original = window[ name ];
		if( !Original ) { return; }
		const Tracked = function( ...args )
		{
			const context = new Original( ...args );
			list.push( context );
			return context;
		};
		Tracked.prototype = Original.prototype;
		window[ name ] = Tracked;
		restore.push( function() { window[ name ] = Original; } );
	} );
	return function() { restore.forEach( function( r ) { r(); } ); };
}
