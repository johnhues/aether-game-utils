//------------------------------------------------------------------------------
// editor.js
//------------------------------------------------------------------------------
// Opens ae::Editor from the 18_SmallEngine web build in headless Chrome and
// checks editing round trips. Asserts on the editor's log, so the build must
// log debug messages, eg. Release or Debug. Needs Node 22+ and Chrome or
// Chromium, found like check.js.
//
// Usage: node test/web/editor.js <dir> [--swiftshader]
//   <dir>          the page's directory, eg. build_wasi/examples/Release/18_SmallEngine
//   --swiftshader  render with SwiftShader, the default everywhere but macOS
//------------------------------------------------------------------------------
"use strict";
const fs = require( "fs" );
const path = require( "path" );
const http = require( "http" );
const os = require( "os" );
const zlib = require( "zlib" );
const { spawn } = require( "child_process" );

const kWidth = 1600; // Editor on the left half, game on the right
const kHeight = 700;
const kTimeoutMs = 30000; // Time to connect, load every mesh, or answer a DevTools call
const kSlowFiles = [ "bunny.obj", "tall_tree.obj" ]; // Served late, so BlobCube.obj's components resolve while these still wait
const kSlowMs = 2000;
const kDragGrowthBytes = 16 << 20;
const kTypes = { ".html": "text/html", ".js": "text/javascript", ".wasm": "application/wasm", ".png": "image/png", ".json": "application/json", ".webmanifest": "application/manifest+json" };

const args = process.argv.slice( 2 );
const gpu = process.platform === "darwin" && !args.includes( "--swiftshader" );
const dir = args.filter( ( a ) => a !== "--swiftshader" )[ 0 ];
const root = dir && path.resolve( dir );
if( !root || !fs.existsSync( path.join( root, "index.html" ) ) )
{
	console.error( "Usage: node test/web/editor.js <dir> [--swiftshader]" );
	process.exit( 2 );
}

// The browser to run, see check.js
function findChrome()
{
	if( process.env.CHROME ) { return process.env.CHROME; }
	const candidates = [ "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "/Applications/Chromium.app/Contents/MacOS/Chromium" ];
	for( const name of [ "google-chrome", "google-chrome-stable", "chromium", "chromium-browser" ] )
	{
		( process.env.PATH || "" ).split( path.delimiter ).filter( Boolean ).forEach( ( d ) => candidates.push( path.join( d, name ) ) );
	}
	const exe = candidates.find( ( c ) => fs.existsSync( c ) );
	if( !exe ) { throw new Error( "No Chrome found, set CHROME" ); }
	return exe;
}

// Serves root read only on an unused port, holding back kSlowFiles
function serve()
{
	return new Promise( function( resolve )
	{
		const server = http.createServer( function( req, res )
		{
			const file = path.join( root, decodeURIComponent( new URL( req.url, "http://x" ).pathname ) );
			if( req.method !== "GET" || !file.startsWith( root ) || !fs.existsSync( file ) || fs.statSync( file ).isDirectory() )
			{
				res.writeHead( req.method === "GET" ? 404 : 405 );
				res.end();
				return;
			}
			const send = function()
			{
				res.writeHead( 200, { "Content-Type": kTypes[ path.extname( file ) ] || "application/octet-stream", "Cache-Control": "no-cache" } );
				fs.createReadStream( file ).pipe( res );
			};
			if( kSlowFiles.includes( path.basename( file ) ) ) { setTimeout( send, kSlowMs ); }
			else { send(); }
		} );
		server.listen( 0, "127.0.0.1", function() { resolve( server ); } );
	} );
}

// Starts Chrome and resolves with its DevTools websocket url
function launch( exe, profile )
{
	const flags = gpu ? [ "--use-angle=metal", "--enable-gpu", "--ignore-gpu-blocklist" ] : [ "--use-angle=swiftshader", "--enable-unsafe-swiftshader" ];
	const chrome = spawn( exe, [ "--headless=new", "--no-sandbox", "--remote-debugging-port=0", "--user-data-dir=" + profile,
		"--password-store=basic", "--window-size=" + kWidth + "," + kHeight, ...flags, "about:blank" ],
		{ stdio: [ "ignore", "ignore", "pipe" ], detached: true } );
	process.on( "exit", function()
	{
		try { process.kill( -chrome.pid, "SIGTERM" ); } catch( e ) {}
		fs.rmSync( profile, { recursive: true, force: true, maxRetries: 5 } );
	} );
	process.on( "SIGINT", function() { process.exit( 130 ); } );
	process.on( "SIGTERM", function() { process.exit( 143 ); } );
	return new Promise( function( resolve, reject )
	{
		let err = "";
		chrome.stderr.on( "data", function( d )
		{
			err += d;
			const m = /DevTools listening on (ws:\/\/\S+)/.exec( err );
			if( m ) { resolve( m[ 1 ] ); }
		} );
		chrome.on( "exit", function( code ) { reject( new Error( "Chrome exited " + code + ": " + err.slice( -500 ) ) ); } );
	} );
}

// A DevTools protocol session with one page
async function connect( url )
{
	const ws = new WebSocket( url );
	let id = 0;
	const pending = new Map();
	const handlers = [];
	ws.onmessage = function( m )
	{
		const msg = JSON.parse( m.data );
		if( msg.id && pending.has( msg.id ) ) { pending.get( msg.id )( msg ); pending.delete( msg.id ); }
		else { handlers.forEach( function( h ) { h( msg ); } ); }
	};
	await new Promise( function( r ) { ws.onopen = r; } );
	let sessionId;
	const send = function( method, params )
	{
		return new Promise( function( resolve, reject )
		{
			id++;
			const timer = setTimeout( function() { reject( new Error( "No reply from Chrome to " + method ) ); }, kTimeoutMs );
			pending.set( id, function( msg ) { clearTimeout( timer ); resolve( msg.result || {} ); } );
			ws.send( JSON.stringify( { id, method, params, sessionId } ) );
		} );
	};
	const { targetId } = await send( "Target.createTarget", { url: "about:blank" } );
	sessionId = ( await send( "Target.attachToTarget", { targetId, flatten: true } ) ).sessionId;
	return { send, on: function( h ) { handlers.push( h ); }, close: function() { ws.close(); } };
}

const sleep = ( ms ) => new Promise( ( r ) => setTimeout( r, ms ) );

// Decodes an 8-bit RGB or RGBA png into { width, height, channels, data }
function decodePng( png )
{
	let pos = 8;
	let width = 0, height = 0, channels = 0;
	const idat = [];
	while( pos < png.length )
	{
		const length = png.readUInt32BE( pos );
		const type = png.toString( "ascii", pos + 4, pos + 8 );
		const body = png.subarray( pos + 8, pos + 8 + length );
		if( type === "IHDR" ) { width = body.readUInt32BE( 0 ); height = body.readUInt32BE( 4 ); channels = ( body[ 9 ] === 6 ) ? 4 : 3; }
		else if( type === "IDAT" ) { idat.push( body ); }
		pos += 12 + length;
	}
	const raw = zlib.inflateSync( Buffer.concat( idat ) );
	const stride = width * channels;
	const data = Buffer.alloc( stride * height );
	for( let y = 0; y < height; y++ )
	{
		const filter = raw[ y * ( stride + 1 ) ];
		for( let x = 0; x < stride; x++ )
		{
			const v = raw[ y * ( stride + 1 ) + 1 + x ];
			const a = ( x >= channels ) ? data[ y * stride + x - channels ] : 0;
			const b = y ? data[ ( y - 1 ) * stride + x ] : 0;
			const c = ( x >= channels && y ) ? data[ ( y - 1 ) * stride + x - channels ] : 0;
			const p = a + b - c;
			const paeth = ( Math.abs( p - a ) <= Math.abs( p - b ) && Math.abs( p - a ) <= Math.abs( p - c ) ) ? a : ( Math.abs( p - b ) <= Math.abs( p - c ) ? b : c );
			data[ y * stride + x ] = ( v + [ 0, a, b, ( a + b ) >> 1, paeth ][ filter ] ) & 0xff;
		}
	}
	return { width, height, channels, data };
}

( async function()
{
	const level = JSON.parse( fs.readFileSync( path.join( root, "data", "example.level" ), "utf8" ) );
	const expected = new Map(); // Entity -> level position of each object with a Mesh
	for( const object of level.objects )
	{
		if( !object.components || !object.components.Mesh ) { continue; }
		const m = object.transform.split( " " ).map( Number );
		expected.set( object.id, [ m[ 12 ], m[ 13 ], m[ 14 ] ] );
	}

	const server = await serve();
	const profile = fs.mkdtempSync( path.join( os.tmpdir(), "ae-web-editor-" ) );
	const cdp = await connect( await launch( findChrome(), profile ) );
	const logs = [];
	const exceptions = [];
	cdp.on( function( msg )
	{
		if( msg.method === "Runtime.consoleAPICalled" ) { logs.push( msg.params.args.map( ( a ) => String( a.value !== undefined ? a.value : a.description ) ).join( " " ) ); }
		else if( msg.method === "Runtime.exceptionThrown" ) { exceptions.push( String( msg.params.exceptionDetails.exception && msg.params.exceptionDetails.exception.description || msg.params.exceptionDetails.text ).split( "\n" )[ 0 ] ); }
	} );
	await cdp.send( "Runtime.enable" );
	await cdp.send( "Page.enable" );
	await cdp.send( "Emulation.setDeviceMetricsOverride", { width: kWidth, height: kHeight, deviceScaleFactor: 1, mobile: false } );
	await cdp.send( "Page.navigate", { url: "http://127.0.0.1:" + server.address().port + "/index.html" } );

	const evaluate = async ( expression ) => ( await cdp.send( "Runtime.evaluate", { expression, returnByValue: true } ) ).result.value;
	const halted = () => evaluate( "( function() { const s = document.getElementById( 'ae-status' ); return s && /Stopped/.test( s.textContent ) ? s.textContent.trim().replace( /\\s+/g, ' ' ).slice( 0, 300 ) : ''; } )()" );
	const mouse = ( type, x, y, buttons ) => cdp.send( "Input.dispatchMouseEvent", { type, x, y, button: ( type === "mouseMoved" && !buttons ) ? "none" : "left", buttons, clickCount: 1 } );
	const click = async ( x, y ) => { await mouse( "mouseMoved", x, y, 0 ); await mouse( "mousePressed", x, y, 1 ); await sleep( 80 ); await mouse( "mouseReleased", x, y, 0 ); };
	const keyEvent = ( type, code, key, vk, modifiers ) => cdp.send( "Input.dispatchKeyEvent", { type, code, key, windowsVirtualKeyCode: vk, modifiers: modifiers || 0 } );
	const press = async ( code, key, vk ) => { await keyEvent( "keyDown", code, key, vk ); await sleep( 100 ); await keyEvent( "keyUp", code, key, vk ); await sleep( 100 ); };
	const until = async ( fn, what ) =>
	{
		for( const end = Date.now() + kTimeoutMs; Date.now() < end; await sleep( 200 ) ) { if( fn() ) { return; } }
		throw new Error( "timed out waiting for " + what );
	};
	const count = ( text ) => logs.filter( ( l ) => l.includes( text ) ).length;
	// Each entity's logged mesh positions in order, from MeshEditorPlugin
	const positions = function()
	{
		const result = new Map();
		for( const line of logs )
		{
			const m = /Set component reference->\S+ entity:(\d+) .* position:(\S+) (\S+) (\S+)/.exec( line );
			if( !m ) { continue; }
			const entity = Number( m[ 1 ] );
			if( !result.has( entity ) ) { result.set( entity, [] ); }
			result.get( entity ).push( [ Number( m[ 2 ] ), Number( m[ 3 ] ), Number( m[ 4 ] ) ] );
		}
		return result;
	};
	const distance = ( a, b ) => Math.hypot( a[ 0 ] - b[ 0 ], a[ 1 ] - b[ 1 ], a[ 2 ] - b[ 2 ] );

	const results = [];
	const check = async function( name, fn )
	{
		const problems = [];
		try { await fn( problems ); }
		catch( e ) { problems.push( e.message ); }
		let stopped = "";
		try { stopped = await halted(); }
		catch( e ) { stopped = e.message; }
		if( stopped ) { problems.push( "stopped: " + stopped ); }
		exceptions.splice( 0 ).forEach( ( e ) => problems.push( "exception: " + e ) );
		results.push( { name, problems } );
		console.log( ( problems.length ? "FAIL  " : "pass  " ) + name );
		problems.slice( 0, 8 ).forEach( ( p ) => console.log( "      " + p ) );
		if( stopped ) { throw new Error( "stopped" ); }
	};

	console.log( "ae::Editor on the web, " + ( gpu ? "GPU" : "SwiftShader" ) );
	try
	{
		await check( "editor launches and connects", async function()
		{
			await until( () => count( "Loaded level" ), "the game to load its level" );
			// Clicking the game captures the cursor, so it is focused directly
			await evaluate( "document.getElementById( 'canvas' ).focus()" );
			await press( "Backquote", "`", 192 );
			await until( () => count( "ae::Editor client connected" ), "the editor to connect" );
		} );

		let start;
		await check( "every mesh placed at its object", async function( problems )
		{
			await until( () => { const p = positions(); return [ ...expected.keys() ].every( ( e ) => p.has( e ) ); }, expected.size + " mesh instances" );
			start = new Map( [ ...positions() ].map( ( [ e, list ] ) => [ e, list[ list.length - 1 ] ] ) );
			for( const [ entity, position ] of expected )
			{
				if( distance( start.get( entity ), position ) > 0.01 ) { problems.push( "entity " + entity + " at " + start.get( entity ).join( " " ) + ", expected " + position.join( " " ) ); }
			}
		} );

		await check( "dragging everything moves each object once", async function( problems )
		{
			await click( kWidth / 4, 120 );
			await press( "KeyA", "a", 65 );
			await press( "KeyW", "w", 87 );
			await sleep( 1000 );
			// The translate gizmo's blue z axis, the only saturated blue in the editor
			const png = Buffer.from( ( await cdp.send( "Page.captureScreenshot", { format: "png" } ) ).data, "base64" );
			const shot = decodePng( png );
			const blue = [];
			for( let y = 20; y < shot.height; y++ )
			{
				for( let x = 0; x < kWidth / 2 - 10; x++ )
				{
					const i = ( y * shot.width + x ) * shot.channels;
					if( shot.data[ i + 2 ] > 180 && shot.data[ i ] < 90 && shot.data[ i + 1 ] < 90 ) { blue.push( [ x, y ] ); }
				}
			}
			if( blue.length < 10 ) { throw new Error( "no gizmo found" ); }
			blue.sort( ( a, b ) => a[ 1 ] - b[ 1 ] );
			const [ x, y ] = blue[ blue.length >> 1 ];
			const before = logs.length;
			const heapBytes = () => evaluate( "window.aeInstance.allStats().reduce( function( total, s ) { return total + s.heapBytes; }, 0 )" );
			const heapBefore = await heapBytes();
			await mouse( "mouseMoved", x - 30, y + 40, 0 );
			await sleep( 200 );
			await mouse( "mouseMoved", x, y, 0 );
			await sleep( 300 );
			await mouse( "mousePressed", x, y, 1 );
			await sleep( 200 );
			for( let i = 1; i <= 30; i++ )
			{
				await mouse( "mouseMoved", x, y - 4 * i, 1 );
				await sleep( 100 );
			}
			await mouse( "mouseReleased", x, y - 120, 0 );
			await sleep( 1000 );
			if( !logs.slice( before ).some( ( l ) => l.includes( "Set component reference" ) ) ) { throw new Error( "the drag moved nothing" ); }
			// Undo history merges each object's changes during one drag
			const growth = await heapBytes() - heapBefore;
			if( growth > kDragGrowthBytes ) { problems.push( "memory grew " + ( growth >> 20 ) + " MB during the drag" ); }
			const moved = positions();
			const offsets = [ ...expected.keys() ].map( ( e ) => { const list = moved.get( e ); const p = list[ list.length - 1 ]; const s = start.get( e ); return [ e, [ p[ 0 ] - s[ 0 ], p[ 1 ] - s[ 1 ], p[ 2 ] - s[ 2 ] ] ]; } );
			const reference = offsets[ 0 ][ 1 ];
			if( distance( reference, [ 0, 0, 0 ] ) < 0.1 ) { problems.push( "entity " + offsets[ 0 ][ 0 ] + " did not move" ); }
			for( const [ entity, offset ] of offsets )
			{
				if( distance( offset, reference ) > 0.05 ) { problems.push( "entity " + entity + " moved " + offset.map( ( v ) => v.toFixed( 3 ) ).join( " " ) + ", others " + reference.map( ( v ) => v.toFixed( 3 ) ).join( " " ) ); }
			}
		} );

		await check( "one undo restores every object", async function( problems )
		{
			const undos = count( "Command: Undo" );
			await keyEvent( "rawKeyDown", "MetaLeft", "Meta", 91, 4 );
			await keyEvent( "rawKeyDown", "KeyZ", "z", 90, 4 );
			await sleep( 100 );
			await keyEvent( "keyUp", "KeyZ", "z", 90, 4 );
			await keyEvent( "keyUp", "MetaLeft", "Meta", 91, 0 );
			await sleep( 1500 );
			if( count( "Command: Undo" ) !== undos + 1 ) { problems.push( ( count( "Command: Undo" ) - undos ) + " undos, expected 1" ); }
			const undone = positions();
			for( const [ entity, position ] of expected )
			{
				const list = undone.get( entity );
				if( distance( list[ list.length - 1 ], position ) > 0.01 ) { problems.push( "entity " + entity + " at " + list[ list.length - 1 ].join( " " ) + ", expected " + position.join( " " ) ); }
			}
		} );

		await check( "Command held: a key without keyup presses once", async function( problems )
		{
			// macOS browsers drop the keyup of keys released while Command is held
			const undos = count( "Command: Undo" );
			await keyEvent( "rawKeyDown", "MetaLeft", "Meta", 91, 4 );
			await keyEvent( "rawKeyDown", "KeyZ", "z", 90, 4 );
			await sleep( 1000 );
			await keyEvent( "keyUp", "MetaLeft", "Meta", 91, 0 );
			await sleep( 500 );
			if( count( "Command: Undo" ) !== undos + 1 ) { problems.push( ( count( "Command: Undo" ) - undos ) + " undos, expected 1" ); }
		} );
	}
	catch( e ) { if( e.message !== "stopped" ) { throw e; } } // Later checks need a running page

	cdp.close();
	server.close();
	const failures = results.filter( ( r ) => r.problems.length ).length;
	if( failures ) { console.log( failures + " check(s) failed" ); }
	process.exit( failures ? 1 : 0 );
} )().catch( function( e ) { console.error( e ); process.exit( 1 ); } );
