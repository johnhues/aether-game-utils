// Live example frames. A page declares one with
// <div class="ae-demo" data-demo="examples/<name>"></div>, holding whatever
// stands in for it where scripts do not run, such as a screenshot on GitHub.
// The example runs as soon as the page loads, but takes the mouse and keyboard
// only while it is armed, so the page still scrolls under the cursor.
( function()
{
	"use strict";

	// The running example, see aether.js. The example's page sets it once the
	// program has started.
	function runtime( demo )
	{
		const frame = demo.querySelector( "iframe" );
		try
		{
			return frame && frame.contentWindow && frame.contentWindow.aeInstance;
		}
		catch( error )
		{
			return null;
		}
	}

	function running( demo, run )
	{
		const instance = runtime( demo );
		if( !instance )
		{
			return false;
		}
		if( run )
		{
			instance.resume();
		}
		else
		{
			instance.pause();
		}
		demo.classList.toggle( "paused", !run );
		return true;
	}

	function arm( demo, armed )
	{
		demo.classList.toggle( "armed", armed );
		const frame = demo.querySelector( "iframe" );
		if( frame && armed )
		{
			// Focusing the frame rather than its window, so the browser does
			// not scroll it into view and move the page under the reader.
			frame.focus( { preventScroll: true } );
			frame.contentWindow.focus();
		}
		else if( !armed )
		{
			window.focus();
		}
	}

	function release( demo )
	{
		arm( demo, false );
		running( demo, false );
	}

	// The bottom strip saying a frame is waiting its turn or starting; the
	// example's own status replaces it once its page has loaded.
	function label( demo, text )
	{
		const status = demo.querySelector( ".ae-demo-status" );
		status.textContent = text;
		status.hidden = !text;
	}

	function load( demo )
	{
		const frame = document.createElement( "iframe" );
		frame.src = demo.dataset.demo + "/index.html";
		label( demo, "Loading" );
		frame.allow = "autoplay; fullscreen; gamepad; clipboard-write";
		frame.addEventListener( "load", function()
		{
			label( demo, "" );
			// An armed frame holds the keyboard, so Escape arrives in the
			// example's document rather than this one.
			try
			{
				frame.contentDocument.addEventListener( "keydown", function( event )
				{
					if( event.key === "Escape" )
					{
						release( demo );
					}
				} );
			}
			catch( error ) {}
		} );
		demo.insertBefore( frame, demo.firstChild );
	}

	// A frame out of view is stopped and removed. Its graphics and audio are
	// released immediately, and its wasm memory is freed once collected.
	function unload( demo )
	{
		const frame = demo.querySelector( "iframe" );
		if( frame )
		{
			if( demo.classList.contains( "armed" ) )
			{
				arm( demo, false );
			}
			const instance = runtime( demo );
			if( instance )
			{
				instance.terminate();
			}
			demo.classList.remove( "paused", "ready" );
			frame.remove();
			label( demo, "" );
		}
	}

	// The line under a frame, measured by the example's aeInstance.stats().
	function size( bytes )
	{
		return bytes >= 1048576 ? ( bytes / 1048576 ).toFixed( 1 ) + " MB" : Math.ceil( bytes / 1024 ) + " KB";
	}

	function showStats( demo, stats )
	{
		let read = null;
		try
		{
			const instance = runtime( demo );
			read = instance && instance.stats();
		}
		catch( error ) {}
		const items = [];
		function add( value, unit, title )
		{
			items.push( [ value, unit, title ] );
		}
		if( read )
		{
			if( demo.classList.contains( "paused" ) || !read.fps )
			{
				add( "paused", "", "The main loop is not running" );
			}
			else
			{
				add( read.fps.toFixed( 0 ), "fps", "Frames per second" );
				add( read.frameMs.toFixed( 1 ), "ms", "Main loop CPU time per frame" );
				if( read.api )
				{
					add( Math.round( read.draws ), "draws", "Draw calls per frame" );
				}
			}
			if( read.api )
			{
				add( read.api, read.renderer, "Graphics API and renderer" );
			}
			if( read.width )
			{
				add( read.width + "×" + read.height, "@" + read.pixelRatio + "x", "Drawing buffer size and device pixel ratio" );
			}
			if( read.heapBytes )
			{
				add( size( read.heapBytes ), "heap", "WebAssembly memory" );
			}
			if( read.downloadBytes )
			{
				add( size( read.downloadBytes ), "downloaded", "The wasm and data files, excluding scripts" );
			}
			if( read.readyMs )
			{
				add( ( read.readyMs / 1000 ).toFixed( 1 ), "s to ready", "Time from opening the example until it was ready to play" );
			}
		}
		stats.replaceChildren();
		items.forEach( function( item )
		{
			const span = document.createElement( "span" );
			span.title = item[ 2 ];
			const value = document.createElement( "b" );
			value.textContent = item[ 0 ];
			span.appendChild( value );
			if( item[ 1 ] )
			{
				span.appendChild( document.createTextNode( " " + item[ 1 ] ) );
			}
			stats.appendChild( span );
		} );
	}

	document.addEventListener( "DOMContentLoaded", function()
	{
		const demos = document.querySelectorAll( ".ae-demo[data-demo]" );
		demos.forEach( function( demo )
		{
			demo.replaceChildren();
			const button = document.createElement( "button" );
			button.className = "ae-demo-play";
			button.textContent = "▶ Play";
			button.addEventListener( "click", function()
			{
				running( demo, true );
				arm( demo, true );
			} );
			demo.appendChild( button );
			// iOS zooms the page on a quick second tap, which here is game input
			let lastTouchEnd = -Infinity;
			demo.addEventListener( "touchend", function( event )
			{
				if( event.timeStamp - lastTouchEnd < 350 )
				{
					event.preventDefault();
				}
				lastTouchEnd = event.timeStamp;
			}, { passive: false } );
			const status = document.createElement( "div" );
			status.className = "ae-demo-status";
			status.hidden = true;
			demo.appendChild( status );
			const stats = document.createElement( "div" );
			stats.className = "ae-demo-stats";
			demo.after( stats );
		} );
		if( demos.length )
		{
			// Leaving the page releases every frame now rather than at garbage
			// collection. A page restored from the back-forward cache starts over.
			window.addEventListener( "pagehide", function()
			{
				demos.forEach( unload );
			} );
			window.addEventListener( "pageshow", function( event )
			{
				if( event.persisted )
				{
					location.reload();
				}
			} );
			setInterval( function()
			{
				demos.forEach( function( demo ) { showStats( demo, demo.nextElementSibling ); } );
			}, 500 );
		}

		// Frames in view load one at a time, nearest to its middle first, until
		// each reports it is ready or halted.
		const wanted = new Set();
		let loading = null;
		function loadNext()
		{
			if( loading )
			{
				return;
			}
			const middle = window.innerHeight / 2;
			let nearestDistance = Infinity;
			wanted.forEach( function( demo )
			{
				const rect = demo.getBoundingClientRect();
				const distance = Math.abs( rect.top + rect.height / 2 - middle );
				if( !demo.querySelector( "iframe" ) && distance < nearestDistance )
				{
					loading = demo;
					nearestDistance = distance;
				}
			} );
			if( loading )
			{
				load( loading );
			}
		}

		if( "IntersectionObserver" in window )
		{
			const observer = new IntersectionObserver( function( entries )
			{
				entries.forEach( function( entry )
				{
					if( entry.isIntersecting )
					{
						wanted.add( entry.target );
						if( !entry.target.querySelector( "iframe" ) )
						{
							label( entry.target, "Queued" );
						}
					}
					else
					{
						wanted.delete( entry.target );
						if( loading === entry.target )
						{
							loading = null;
						}
						unload( entry.target );
					}
				} );
				loadNext();
			} );
			demos.forEach( function( demo ) { observer.observe( demo ); } );
		}
		else
		{
			demos.forEach( load );
		}

		if( demos.length )
		{
			// A frame says when it has finished loading, which shows its play
			// button, or that it halted. Either lets the next frame load.
			window.addEventListener( "message", function( event )
			{
				const state = event.data && event.data.aeDemo;
				if( state !== "ready" && state !== "halted" )
				{
					return;
				}
				demos.forEach( function( demo )
				{
					const frame = demo.querySelector( "iframe" );
					if( frame && frame.contentWindow === event.source )
					{
						demo.classList.toggle( "ready", state === "ready" );
						if( loading === demo )
						{
							loading = null;
						}
					}
				} );
				loadNext();
			} );
			document.addEventListener( "pointerdown", function( event )
			{
				demos.forEach( function( demo )
				{
					if( !demo.contains( event.target ) )
					{
						arm( demo, false );
					}
				} );
			} );
			document.addEventListener( "keydown", function( event )
			{
				if( event.key === "Escape" )
				{
					// Escape stops the example as well as releasing it; a
					// click elsewhere only takes the input back.
					demos.forEach( release );
				}
			} );
		}
	} );
} )();
