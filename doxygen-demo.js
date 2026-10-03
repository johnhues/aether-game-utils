// Live example frames. A page declares one with
// <div class="ae-demo" data-demo="examples/<name>"></div>, holding whatever
// stands in for it where scripts do not run, such as a screenshot on GitHub.
// The example runs as soon as the page loads, but takes the mouse and keyboard
// only while it is armed, so the page still scrolls under the cursor.
( function()
{
	"use strict";

	function runtime( demo )
	{
		const frame = demo.querySelector( "iframe" );
		try
		{
			return frame && frame.contentWindow && frame.contentWindow.Module;
		}
		catch( error )
		{
			return null;
		}
	}

	function running( demo, run )
	{
		const module = runtime( demo );
		if( !module || !module.pauseMainLoop || !module.resumeMainLoop )
		{
			return false;
		}
		if( run )
		{
			module.resumeMainLoop();
		}
		else
		{
			module.pauseMainLoop();
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

	function load( demo )
	{
		const frame = document.createElement( "iframe" );
		frame.src = demo.dataset.demo + "/index.html";
		frame.loading = "lazy";
		frame.allow = "autoplay; fullscreen; gamepad; clipboard-write";
		frame.addEventListener( "load", function()
		{
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
		demo.appendChild( frame );
	}

	document.addEventListener( "DOMContentLoaded", function()
	{
		const demos = document.querySelectorAll( ".ae-demo[data-demo]" );
		demos.forEach( function( demo )
		{
			demo.replaceChildren();
			load( demo );
			const button = document.createElement( "button" );
			button.className = "ae-demo-play";
			button.textContent = "▶ Play";
			button.addEventListener( "click", function()
			{
				running( demo, true );
				arm( demo, true );
			} );
			demo.appendChild( button );
		} );

		if( demos.length )
		{
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
