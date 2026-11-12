//------------------------------------------------------------------------------
// check.js
//------------------------------------------------------------------------------
// Loads every example in a web build in headless Chrome and checks each one
// starts on aether.js, keeps rendering, and reports no exceptions, console
// errors or halts. Needs Node 22+ and Chrome or Chromium, found from $CHROME,
// Playwright's browsers in $PLAYWRIGHT_BROWSERS_PATH, the macOS app, the PATH,
// or a Chromium flatpak.
//
// Usage: node test/web/check.js <dir> [name...] [--gpu] [--expect-log text]
//   <dir>             eg. build_wasi/examples/Release, or one page's directory
//   name              only these examples, default every directory with an index.html
//   --gpu             use the GPU, default SwiftShader
//   --expect-log text for pages not run by aether.js: passes once the page
//                     logs text, instead of checking its aether.js instance
//------------------------------------------------------------------------------
"use strict";
const fs = require( "fs" );
const path = require( "path" );
const http = require( "http" );
const os = require( "os" );
const { spawn } = require( "child_process" );

const kSettleMs = 6000; // Time each example runs before it is measured
const kReplyMs = 60000; // Time Chrome has to answer each DevTools call
const kTypes = { ".html": "text/html", ".js": "text/javascript", ".wasm": "application/wasm", ".png": "image/png", ".json": "application/json", ".webmanifest": "application/manifest+json" };

const args = process.argv.slice( 2 );
const gpu = args.includes( "--gpu" );
const expectIndex = args.indexOf( "--expect-log" );
const expectLog = expectIndex >= 0 ? args[ expectIndex + 1 ] : null;
const [ dir, ...only ] = args.filter( ( a, i ) => a !== "--gpu" && ( expectIndex < 0 || ( i !== expectIndex && i !== expectIndex + 1 ) ) );
const root = dir && path.resolve( dir );
if( !root || !fs.existsSync( root ) || ( expectIndex >= 0 && !expectLog ) )
{
	console.error( "Usage: node test/web/check.js <dir> [name...] [--gpu] [--expect-log text]" );
	process.exit( 2 );
}
const names = only.length ? only
	: fs.existsSync( path.join( root, "index.html" ) ) ? [ "." ]
	: fs.readdirSync( root ).filter( ( n ) => fs.existsSync( path.join( root, n, "index.html" ) ) ).sort();

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

// Starts Chrome and resolves with its DevTools websocket url
function launch( exe, profile )
{
	const flags = gpu ? [ "--use-angle=metal", "--enable-gpu", "--ignore-gpu-blocklist" ] : [ "--use-angle=swiftshader", "--enable-unsafe-swiftshader", "--use-webgpu-adapter=swiftshader" ];
	// --password-store=basic: Linux otherwise encrypts cookies with a key from
	// the desktop keyring, and while it is locked every page load waits for it
	const chrome = spawn( exe, [ "--headless=new", "--no-sandbox", "--remote-debugging-port=0", "--user-data-dir=" + profile,
		"--password-store=basic", "--enable-unsafe-webgpu", "--autoplay-policy=no-user-gesture-required", "--window-size=800,500", ...flags, "about:blank" ],
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

// Loads one page and returns its problems, empty when it passes
async function check( cdp, base, name )
{
	const { result: { targetId } } = await cdp.send( "Target.createTarget", { url: "about:blank" } );
	const { result: { sessionId } } = await cdp.send( "Target.attachToTarget", { targetId, flatten: true } );
	const problems = [];
	const logs = [];
	const text = ( a ) => a.value !== undefined ? String( a.value ) : ( a.description || "" );
	cdp.on( function( msg )
	{
		if( msg.sessionId !== sessionId ) { return; }
		if( msg.method === "Runtime.exceptionThrown" )
		{
			const d = msg.params.exceptionDetails;
			problems.push( "exception: " + ( d.exception && d.exception.description || d.text ).split( "\n" )[ 0 ] );
		}
		else if( msg.method === "Runtime.consoleAPICalled" )
		{
			const line = msg.params.args.map( text ).join( " " );
			logs.push( line );
			// aether's own log, eg. '[warn]  file.cpp:12: message'
			if( /\[(warn|error|fatal)\]/.test( line ) ) { problems.push( "aether log: " + line.trim().slice( 0, 300 ) ); }
			if( msg.params.type === "error" || msg.params.type === "warning" )
			{
				problems.push( "console " + msg.params.type + ": " + line.slice( 0, 300 ) );
			}
		}
		else if( msg.method === "Log.entryAdded" && ( msg.params.entry.level === "error" || msg.params.entry.level === "warning" ) )
		{
			// Performance notes from the GL driver under the browser, eg. SwiftShader readback
			if( /GL Driver Message \([^,]*, Performance,/.test( msg.params.entry.text ) ) { return; }
			const e = msg.params.entry;
			problems.push( e.source + " " + e.level + ": " + e.text.slice( 0, 300 ) + ( e.url ? " (" + e.url + ")" : "" ) );
		}
	} );
	await cdp.send( "Runtime.enable", {}, sessionId );
	await cdp.send( "Log.enable", {}, sessionId );
	await cdp.send( "Page.enable", {}, sessionId );
	await cdp.send( "Page.navigate", { url: base + name + "/index.html" }, sessionId );
	await sleep( kSettleMs );
	const evaluate = async ( expression ) => ( await cdp.send( "Runtime.evaluate", { expression, returnByValue: true }, sessionId ) ).result.result.value;
	let stats = null;
	if( expectLog )
	{
		if( !logs.some( ( l ) => l.includes( expectLog ) ) ) { problems.push( "never logged '" + expectLog + "'" ); }
	}
	else
	{
		const before = await evaluate( "window.aeInstance ? window.aeInstance.stats() : null" );
		await sleep( 1000 );
		stats = await evaluate( "window.aeInstance ? window.aeInstance.stats() : null" );
		const halted = await evaluate( "( function() { const s = document.getElementById( 'ae-status' ); return s && /Stopped/.test( s.textContent ) ? s.textContent.trim().slice( 0, 300 ) : ''; } )()" );
		if( !before ) { problems.push( "did not start" ); }
		if( halted ) { problems.push( "halted: " + halted.replace( /\s+/g, " " ) ); }
		if( stats && !( stats.fps > 0 ) ) { problems.push( "stopped rendering" ); }
	}
	await cdp.send( "Target.closeTarget", { targetId } );
	return { problems, stats };
}

( async function()
{
	const server = await serve();
	const base = "http://127.0.0.1:" + server.address().port + "/";
	const profile = fs.mkdtempSync( path.join( os.tmpdir(), "ae-web-check-" ) );
	const { chrome, url } = await launch( findChrome(), profile );
	const cdp = await connect( url );
	let failures = 0;
	console.log( "aether web pages, " + ( gpu ? "GPU" : "SwiftShader" ) );
	for( const name of names )
	{
		const { problems, stats } = await check( cdp, base, name );
		const shown = ( name === "." ) ? path.basename( root ) : name;
		const detail = stats ? ( stats.api || "no graphics" ) + ", " + stats.fps.toFixed( stats.fps < 10 ? 1 : 0 ) + " fps" : "";
		if( problems.length )
		{
			failures++;
			console.log( "FAIL  " + shown.padEnd( 24 ) + detail );
			problems.forEach( function( p ) { console.log( "      " + p ); } );
		}
		else { console.log( "pass  " + shown.padEnd( 24 ) + detail ); }
	}
	cdp.close();
	await new Promise( function( r ) { chrome.removeAllListeners( "exit" ); chrome.on( "exit", r ); process.kill( -chrome.pid, "SIGTERM" ); } );
	server.close();
	fs.rmSync( profile, { recursive: true, force: true, maxRetries: 5 } );
	if( failures )
	{
		console.log( failures + " example(s) failed" );
		process.exit( 1 );
	}
	process.exit( 0 );
} )().catch( function( e ) { console.error( e ); process.exit( 1 ); } );
