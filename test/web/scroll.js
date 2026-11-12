//------------------------------------------------------------------------------
// scroll.js
//------------------------------------------------------------------------------
// Flings a docs site's main page up and down in headless Chrome and checks the
// live examples it passes are not loaded, then pauses on examples just long
// enough for each to start loading before leaving, then stops on the last one.
// Out of view frames get no animation frames, as in WebKit. Fails when an
// example's frame is removed before its program starts, which kills iPadOS
// Safari, when the last one is slow to get ready, or on an exception, console
// error or crash. Needs Node 22+ and Chrome or Chromium, found as in check.js.
//
// Usage: node test/web/scroll.js <dir> [--gpu]
//   <dir>  a site from 'scripts/docs.py publish', eg. build_docs/publish
//   --gpu  use the GPU, default SwiftShader
//------------------------------------------------------------------------------
"use strict";
const fs = require( "fs" );
const path = require( "path" );
const http = require( "http" );
const os = require( "os" );
const { spawn } = require( "child_process" );

const kPasses = 3; // Top to bottom and back
const kStepPx = 400;
const kStepMs = 30; // About 13000 pixels per second
const kMaxFlingLoads = 2; // Example loads allowed while flinging
const kReadyMs = 30000; // Time the last example has to get ready, before giving up
const kLastReadyMs = 6000; // Time the last example may take, measured from scrolling to it
const kVisits = 6; // Examples left mid-load
const kVisitMs = 700; // Longer than doxygen-demo.js waits before loading
const kAwayMs = 300;
const kResponsiveMs = 2000; // Longest round trip into the page
const kReplyMs = 60000; // Time Chrome has to answer each DevTools call
const kTypes = { ".html": "text/html", ".js": "text/javascript", ".css": "text/css", ".svg": "image/svg+xml", ".wasm": "application/wasm", ".png": "image/png", ".json": "application/json", ".webmanifest": "application/manifest+json" };

const args = process.argv.slice( 2 );
const gpu = args.includes( "--gpu" );
const dir = args.find( ( a ) => a !== "--gpu" );
const root = dir && path.resolve( dir );
if( !root || !fs.existsSync( path.join( root, "index.html" ) ) )
{
	console.error( "Usage: node test/web/scroll.js <dir> [--gpu]" );
	process.exit( 2 );
}

// The browser to run, see the header
function findChrome()
{
	if( process.env.CHROME ) { return process.env.CHROME; }
	const browsers = process.env.PLAYWRIGHT_BROWSERS_PATH;
	if( browsers && fs.existsSync( browsers ) )
	{
		for( const dir of fs.readdirSync( browsers ).filter( ( d ) => d.startsWith( "chromium-" ) ).sort().reverse() )
		{
			for( const sub of fs.readdirSync( path.join( browsers, dir ) ).filter( ( d ) => d.startsWith( "chrome-linux" ) ) )
			{
				const exe = path.join( browsers, dir, sub, "chrome" );
				if( fs.existsSync( exe ) ) { return exe; }
			}
		}
	}
	const mac = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
	if( fs.existsSync( mac ) ) { return mac; }
	for( const name of [ "google-chrome", "google-chrome-stable", "chromium", "chromium-browser" ] )
	{
		for( const dir of ( process.env.PATH || "" ).split( path.delimiter ).filter( Boolean ) )
		{
			const exe = path.join( dir, name );
			if( fs.existsSync( exe ) ) { return exe; }
		}
	}
	// Flathub's Chromium, the usual source on Linux distributions that have no
	// deb, through the launcher flatpak exports for it
	for( const dir of [ path.join( os.homedir(), ".local/share/flatpak/exports/bin" ), "/var/lib/flatpak/exports/bin" ] )
	{
		for( const id of [ "org.chromium.Chromium", "io.github.ungoogled_software.ungoogled_chromium" ] )
		{
			const exe = path.join( dir, id );
			if( fs.existsSync( exe ) ) { return exe; }
		}
	}
	throw new Error( "No Chrome found, set CHROME" );
}

// Serves root over http on an unused port
function serve()
{
	return new Promise( function( resolve )
	{
		const server = http.createServer( function( req, res )
		{
			const file = path.join( root, decodeURIComponent( new URL( req.url, "http://x" ).pathname ) );
			if( !file.startsWith( path.resolve( root ) ) || !fs.existsSync( file ) || fs.statSync( file ).isDirectory() )
			{
				res.writeHead( 404 );
				res.end();
				return;
			}
			res.writeHead( 200, { "Content-Type": kTypes[ path.extname( file ) ] || "application/octet-stream" } );
			fs.createReadStream( file ).pipe( res );
		} );
		server.listen( 0, "127.0.0.1", function() { resolve( server ); } );
	} );
}

// Starts Chrome at a tablet's size and resolves with its DevTools websocket url
function launch( exe, profile )
{
	const flags = gpu ? [ "--use-angle=metal", "--enable-gpu", "--ignore-gpu-blocklist" ] : [ "--use-angle=swiftshader", "--enable-unsafe-swiftshader", "--use-webgpu-adapter=swiftshader" ];
	// --password-store=basic: Linux otherwise encrypts cookies with a key from
	// the desktop keyring, and while it is locked every page load waits for it
	const chrome = spawn( exe, [ "--headless=new", "--no-sandbox", "--remote-debugging-port=0", "--user-data-dir=" + profile,
		"--password-store=basic", "--enable-unsafe-webgpu", "--autoplay-policy=no-user-gesture-required", "--window-size=834,1194", ...flags, "about:blank" ],
		{ stdio: [ "ignore", "ignore", "pipe" ], detached: true } );
	// Chrome outlives this process unless killed, so a failure or a signal
	// (eg. from timeout) would leave it running. It has its own process group,
	// killed whole: a launcher such as a flatpak's does not pass signals on.
	process.on( "exit", function()
	{
		try { process.kill( -chrome.pid, "SIGTERM" ); } catch( e ) {}
		fs.rmSync( profile, { recursive: true, force: true } );
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
			if( m ) { resolve( { chrome: chrome, url: m[ 1 ] } ); }
		} );
		chrome.on( "exit", function( code ) { reject( new Error( "Chrome exited " + code + ": " + err.slice( -500 ) ) ); } );
	} );
}

// A DevTools protocol session over one websocket
function connect( url )
{
	const ws = new WebSocket( url );
	let id = 0;
	const pending = new Map();
	const handlers = [];
	ws.onmessage = function( m )
	{
		const msg = JSON.parse( m.data );
		if( msg.id && pending.has( msg.id ) )
		{
			pending.get( msg.id )( msg );
			pending.delete( msg.id );
		}
		else { handlers.forEach( function( h ) { h( msg ); } ); }
	};
	return new Promise( function( resolve )
	{
		ws.onopen = function()
		{
			resolve( {
				// Rejects if Chrome does not answer, so a hung page fails the
				// check instead of stalling it
				send: function( method, params, sessionId )
				{
					return new Promise( function( resolve, reject )
					{
						id++;
						const timer = setTimeout( function() { reject( new Error( "No reply from Chrome to " + method + " within " + kReplyMs / 1000 + "s" ) ); }, kReplyMs );
						pending.set( id, function( msg ) { clearTimeout( timer ); resolve( msg ); } );
						ws.send( JSON.stringify( { id, method, params, sessionId } ) );
					} );
				},
				on: function( h ) { handlers.push( h ); },
				close: function() { ws.close(); },
			} );
		};
	} );
}

const sleep = ( ms ) => new Promise( ( r ) => setTimeout( r, ms ) );

// Flings the main page, filling result. Its problems are empty when it passes.
async function check( cdp, base, result )
{
	const { result: { targetId } } = await cdp.send( "Target.createTarget", { url: "about:blank" } );
	const { result: { sessionId } } = await cdp.send( "Target.attachToTarget", { targetId, flatten: true } );
	const problems = result.problems;
	const loads = result.loads;
	const text = ( a ) => a.value !== undefined ? String( a.value ) : ( a.description || "" );
	cdp.on( function( msg )
	{
		if( msg.sessionId !== sessionId ) { return; }
		if( msg.method === "Inspector.targetCrashed" ) { problems.push( "page crashed" ); }
		else if( msg.method === "Network.requestWillBeSent" )
		{
			const m = /\/examples\/([^/]+)\/index\.wasm$/.exec( msg.params.request.url );
			if( m ) { loads.push( m[ 1 ] ); }
		}
		else if( msg.method === "Runtime.exceptionThrown" )
		{
			const d = msg.params.exceptionDetails;
			problems.push( "exception: " + ( d.exception && d.exception.description || d.text ).split( "\n" )[ 0 ] );
		}
		else if( msg.method === "Runtime.consoleAPICalled" )
		{
			const line = msg.params.args.map( text ).join( " " );
			if( /\[(error|fatal)\]/.test( line ) ) { problems.push( "aether log: " + line.trim().slice( 0, 300 ) ); }
			if( msg.params.type === "error" ) { problems.push( "console error: " + line.slice( 0, 300 ) ); }
		}
		else if( msg.method === "Log.entryAdded" && msg.params.entry.level === "error" )
		{
			const e = msg.params.entry;
			// The browser asks for one on its own, and the site has none
			if( e.url && e.url.endsWith( "/favicon.ico" ) ) { return; }
			problems.push( e.source + " error: " + e.text.slice( 0, 300 ) + ( e.url ? " (" + e.url + ")" : "" ) );
		}
	} );
	await cdp.send( "Inspector.enable", {}, sessionId );
	await cdp.send( "Network.enable", {}, sessionId );
	await cdp.send( "Runtime.enable", {}, sessionId );
	await cdp.send( "Log.enable", {}, sessionId );
	await cdp.send( "Page.enable", {}, sessionId );
	// An example's frame holds animation frames while out of view, as WebKit
	// does. The main page counts frames removed before they posted any
	// { aeDemo } report, seen ahead of the page's scripts so a report is
	// counted before a removal it causes.
	await cdp.send( "Page.addScriptToEvaluateOnNewDocument", { source: "( function() { if( window.top !== window ) {" +
		"const raf = window.requestAnimationFrame.bind( window ); const held = [];" +
		"const inView = function() { const f = window.frameElement; const r = f ? f.getBoundingClientRect() : null; return !r || ( r.bottom > 0 && r.top < window.parent.innerHeight ); };" +
		"window.requestAnimationFrame = function( fn ) { if( inView() ) { return raf( fn ); } held.push( fn ); return 0; };" +
		"setInterval( function() { if( held.length && inView() ) { held.splice( 0 ).forEach( function( fn ) { raf( fn ); } ); } }, 100 );" +
		"return; }" +
		"window.aeEarlyRemovals = []; const windows = new WeakMap(); const reported = new WeakSet();" +
		"window.addEventListener( 'message', function( e ) { if( e.data && e.data.aeDemo ) { reported.add( e.source ); } } );" +
		"document.addEventListener( 'DOMContentLoaded', function() { new MutationObserver( function( list ) { list.forEach( function( m ) {" +
		"m.addedNodes.forEach( function( n ) { if( n.tagName === 'IFRAME' ) { windows.set( n, n.contentWindow ); } } );" +
		"m.removedNodes.forEach( function( n ) { if( n.tagName === 'IFRAME' && !reported.has( windows.get( n ) ) ) { window.aeEarlyRemovals.push( n.src.replace( /.*examples\\/|\\/index\\.html/g, '' ) ); } } );" +
		"} ); } ).observe( document.body, { childList: true, subtree: true } ); } ); } )()" }, sessionId );
	await cdp.send( "Page.navigate", { url: base + "index.html" }, sessionId );
	await sleep( 3000 );
	const evaluate = async function( expression )
	{
		const start = Date.now();
		const reply = await Promise.race( [ cdp.send( "Runtime.evaluate", { expression, returnByValue: true }, sessionId ), sleep( kResponsiveMs * 5 ).then( () => null ) ] );
		const ms = Date.now() - start;
		if( !reply ) { throw new Error( "page stopped responding" ); }
		if( ms > kResponsiveMs ) { problems.push( "page took " + ms + " ms to respond" ); }
		return reply.result.result.value;
	};

	const demos = result.demos = await evaluate( "document.querySelectorAll( '.ae-demo[data-demo]' ).length" );
	const height = await evaluate( "document.documentElement.scrollHeight - window.innerHeight" );
	if( demos < 2 ) { problems.push( "only " + demos + " live examples on the page" ); }

	result.flingStart = loads.length;
	for( let pass = 0; pass < kPasses; pass++ )
	{
		for( let y = 0; y <= height; y += kStepPx ) { await evaluate( "window.scrollTo( 0, " + y + " )" ); await sleep( kStepMs ); }
		for( let y = height; y >= 0; y -= kStepPx ) { await evaluate( "window.scrollTo( 0, " + y + " )" ); await sleep( kStepMs ); }
	}
	result.flingEnd = loads.length;
	const flung = loads.slice( result.flingStart );
	if( flung.length > kMaxFlingLoads ) { problems.push( flung.length + " examples loaded while flinging, at most " + kMaxFlingLoads + ": " + flung.join( " " ) ); }

	// Pauses on examples until each starts loading, then leaves
	result.visitStart = loads.length;
	for( let i = 0; i < kVisits; i++ )
	{
		await evaluate( "document.querySelectorAll( '.ae-demo[data-demo]' )[ " + i + " * 3 % " + demos + " ].scrollIntoView( { block: 'center' } )" );
		await sleep( kVisitMs );
		await evaluate( "window.scrollBy( 0, window.innerHeight * 2 )" );
		await sleep( kAwayMs );
	}
	result.visits = loads.length - result.visitStart;
	if( !result.visits ) { problems.push( "no example started loading while paused on it" ); }

	// Stops on the last example straight after, which has to get ready without
	// waiting on the ones left behind
	const target = result.target = await evaluate( "( function() { const d = document.querySelectorAll( '.ae-demo[data-demo]' ); const t = d[ d.length - 1 ]; t.scrollIntoView( { block: 'center' } ); return t.dataset.demo; } )()" );
	const expression = "( function() { const t = document.querySelector( '.ae-demo[data-demo=\"" + target + "\"]' ); return t.classList.contains( 'ready' ) ? 'ready' : ''; } )()";
	let state = "";
	const arrived = Date.now();
	while( !state && Date.now() - arrived < kReadyMs ) { await sleep( 100 ); state = await evaluate( expression ); }
	result.lastMs = Date.now() - arrived;
	if( !state ) { problems.push( target + " was not ready " + ( kReadyMs / 1000 ) + " s after scrolling stopped on it" ); }
	else if( result.lastMs > kLastReadyMs ) { problems.push( target + " took " + result.lastMs + " ms to get ready, at most " + kLastReadyMs ); }
	result.frames = await evaluate( "document.querySelectorAll( '.ae-demo iframe' ).length" );
	const early = await evaluate( "window.aeEarlyRemovals.join( ' ' )" );
	if( early ) { problems.push( "removed before starting: " + early ); }
	await cdp.send( "Target.closeTarget", { targetId } );
}

( async function()
{
	const server = await serve();
	const base = "http://127.0.0.1:" + server.address().port + "/";
	const profile = fs.mkdtempSync( path.join( os.tmpdir(), "ae-web-scroll-" ) );
	const { chrome, url } = await launch( findChrome(), profile );
	const cdp = await connect( url );
	console.log( "docs scroll, " + ( gpu ? "GPU" : "SwiftShader" ) );
	// Example names in the order their wasm was requested, and the fling's span of them
	const result = { problems: [], loads: [], flingStart: 0, flingEnd: Infinity, visits: 0, demos: 0, target: "none", lastMs: 0, frames: 0 };
	try
	{
		await check( cdp, base, result );
	}
	catch( error )
	{
		result.problems.push( error.message );
	}
	cdp.close();
	await new Promise( function( r ) { chrome.removeAllListeners( "exit" ); chrome.on( "exit", r ); process.kill( -chrome.pid, "SIGTERM" ); } );
	server.close();
	fs.rmSync( profile, { recursive: true, force: true, maxRetries: 5 } );
	const flung = result.loads.slice( result.flingStart, result.flingEnd ).length;
	const detail = result.demos + " examples, " + flung + " loaded while flinging, " + result.visits + " left mid-load, " + result.frames + " running after stopping on " + result.target + " ready in " + result.lastMs + " ms";
	if( result.problems.length )
	{
		console.log( "FAIL  " + detail );
		result.problems.forEach( function( p ) { console.log( "      " + p ); } );
		process.exit( 1 );
	}
	console.log( "pass  " + detail );
	process.exit( 0 );
} )().catch( function( e ) { console.error( e ); process.exit( 1 ); } );
