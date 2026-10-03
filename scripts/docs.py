#!/usr/bin/env python3
"""
docs.py [preview|publish|report]

Generates documentation from aether.h. Run without arguments for a menu that
asks what you are generating, walks you through it, then asks again until you
answer q. Every answer is one key; ctrl+c quits.

The repository Doxyfile is the single source of settings; this script layers
per-task overrides (output location, HTML vs XML, warning log) on top of a copy
in build_docs/, so the checked-in Doxyfile never has to be edited to preview
something. Everything generated lands in build_docs/, which .gitignore covers.

Tasks:
  preview   builds the site into build_docs/html and serves it
  publish   assembles the site into build_docs/publish, nothing served
  report    ranks what is still undocumented

CLI: python3 scripts/docs.py [preview|publish|report] [--no-open] [--yes]
Naming a task runs it once and exits, skipping the menu.
"""
import argparse
import contextlib
import functools
import html
import http.server
import json
import os
import re
import shutil
import socket
import subprocess
import sys
from collections import Counter
from pathlib import Path

try:
    import fcntl
except ImportError:
    # Windows has no flock; two runs at once are then the caller's problem.
    fcntl = None

try:
    import termios
    import tty
except ImportError:
    # Windows has no POSIX terminal control; the prompt falls back to input().
    termios = None

REPO_ROOT = Path( __file__ ).resolve().parent.parent
DOXYFILE  = REPO_ROOT / "Doxyfile"
BUILD_DIR = REPO_ROOT / "build_docs"
DOCS_DIR  = REPO_ROOT / "docs"
LOG_FILE  = BUILD_DIR / "doxygen.log"

BUILD_CONFIGS = [ "Release", "RelWithDebInfo", "Debug" ]

# Separate ports: a published site is checked against the preview, not
# instead of it.
PREVIEW_PORT = 8765
PUBLISH_PORT = 8766

TASKS = {
    "preview": (
        "Preview build",
        "builds into build_docs/html and serves it"
    ),
    "publish": (
        "Publish build",
        "assembles the site into build_docs/publish, nothing served"
    ),
    "report": (
        "Coverage report",
        "what is still undocumented, ranked"
    ),
}


#------------------------------------------------------------------------------
# Output helpers
#------------------------------------------------------------------------------
def is_tty():
    return sys.stdout.isatty() and os.environ.get( "TERM" ) != "dumb"


def bold( text ):
    return f"\033[1m{text}\033[0m" if is_tty() else text


def dim( text ):
    return f"\033[2m{text}\033[0m" if is_tty() else text


def rule():
    print( dim( "─" * 78 ) )


def heading( text ):
    rule()
    print( " " + bold( text ) )
    rule()


def step( text ):
    print( f"\n{bold( '›' )} {text}" )


def note( text ):
    print( f"  {dim( text )}" )


#------------------------------------------------------------------------------
# Prompt
#------------------------------------------------------------------------------
class Quit( Exception ):
    """Raised by the prompt when the session is over."""


ESCAPE = "\x1b"


def read_key( prompt ):
    """Reads one keypress, no return needed. Gives "" for return and for keys
    this menu has no use for, and ESCAPE for escape. Raises Quit on ctrl+c and
    ctrl+d."""
    sys.stdout.write( prompt )
    sys.stdout.flush()
    if termios is None or not sys.stdin.isatty():
        try:
            answer = input()
        except ( EOFError, KeyboardInterrupt ):
            raise Quit()
        return answer.strip()[ :1 ]

    fd = sys.stdin.fileno()
    saved = termios.tcgetattr( fd )
    try:
        tty.setraw( fd )
        key = os.read( fd, 8 ).decode( "utf-8", "ignore" )
    finally:
        termios.tcsetattr( fd, termios.TCSADRAIN, saved )
    if key in ( "\x03", "\x04" ):
        print()
        raise Quit()
    if key[ 0 ] == ESCAPE:
        print()
        # Escape alone, rather than the sequence an arrow or function key sends.
        return ESCAPE if len( key ) == 1 else ""
    if not key or key in ( "\r", "\n" ):
        print()
        return ""
    print( key[ 0 ] )
    return key[ 0 ]


#------------------------------------------------------------------------------
# Tool discovery
#------------------------------------------------------------------------------
def tool_version( name, args ):
    path = shutil.which( name )
    if not path:
        return None, None
    try:
        result = subprocess.run(
            [ path ] + args, capture_output=True, text=True, timeout=20
        )
        version = ( result.stdout or result.stderr ).strip().split( "\n" )[ 0 ]
    except Exception:
        version = ""
    return path, version


def print_tools():
    doxygen_path, doxygen_version = tool_version( "doxygen", [ "--version" ] )
    rows = [
        (
            "doxygen",
            doxygen_version if doxygen_path else "not found",
            doxygen_path or "required: brew install doxygen"
        ),
    ]
    for name, state, hint in rows:
        print( f" {name:<9} {state:<14} {dim( hint )}" )
    return doxygen_path is not None


#------------------------------------------------------------------------------
# Remembered choices
#------------------------------------------------------------------------------
# The live example build is remembered by the link itself; everything else that
# should outlast a run is kept here. build_docs/ is gitignored, so these are
# per checkout.
SETTINGS = BUILD_DIR / "settings.json"


def remembered( key, default ):
    try:
        return json.loads( SETTINGS.read_text() ).get( key, default )
    except ( OSError, ValueError ):
        return default


def remember( key, value ):
    try:
        settings = json.loads( SETTINGS.read_text() )
    except ( OSError, ValueError ):
        settings = {}
    settings[ key ] = value
    BUILD_DIR.mkdir( parents=True, exist_ok=True )
    SETTINGS.write_text( json.dumps( settings, indent=1 ) + "\n" )


#------------------------------------------------------------------------------
# Doxygen
#------------------------------------------------------------------------------
@contextlib.contextmanager
def generating():
    """Holds the build directory for one run. Doxygen reads and writes the same
    files every time, and two runs at once crash each other in its parser."""
    BUILD_DIR.mkdir( parents=True, exist_ok=True )
    lock = BUILD_DIR / ".lock"
    if fcntl is None:
        yield
        return
    with open( lock, "w" ) as handle:
        try:
            fcntl.flock( handle, fcntl.LOCK_EX | fcntl.LOCK_NB )
        except OSError:
            note( "another docs run is generating, waiting for it to finish" )
            fcntl.flock( handle, fcntl.LOCK_EX )
        try:
            yield
        finally:
            fcntl.flock( handle, fcntl.LOCK_UN )



def write_doxyfile( name, overrides ):
    """Copies the repository Doxyfile and appends overrides; later tags win."""
    BUILD_DIR.mkdir( parents=True, exist_ok=True )
    config = BUILD_DIR / f"Doxyfile.{name}"
    lines = [ DOXYFILE.read_text(), "\n# Appended by scripts/docs.py\n" ]
    for tag, value in overrides.items():
        lines.append( f"{tag:<22} = {value}\n" )
    config.write_text( "".join( lines ) )
    return config


def run_doxygen( doxyfile ):
    print( dim( f"  doxygen {doxyfile.relative_to( REPO_ROOT )}" ) )
    result = subprocess.run(
        [ "doxygen", str( doxyfile ) ],
        cwd=REPO_ROOT, capture_output=True, text=True
    )
    if result.returncode < 0:
        sys.exit( f"ERROR: doxygen was killed by signal {-result.returncode}. "
                  f"That is a crash inside doxygen, not a problem with the "
                  f"configuration; running it again usually works." )
    if result.returncode != 0:
        print( result.stderr.strip()[ -2000: ] )
        sys.exit( f"ERROR: doxygen exited {result.returncode}" )
    obsolete = len( re.findall( r"has become obsolete", result.stderr ) )
    if obsolete:
        note( f"{obsolete} Doxyfile tags are obsolete for this doxygen "
              f"version (doxygen -u Doxyfile updates them)" )


def default_layout():
    """Writes doxygen's built-in layout and returns its path. An empty
    LAYOUT_FILE does not reach it: doxygen reads DoxygenLayout.xml from the
    working directory whenever one is there."""
    path = BUILD_DIR / "DoxygenLayout.default.xml"
    if not path.exists():
        BUILD_DIR.mkdir( parents=True, exist_ok=True )
        subprocess.run(
            [ "doxygen", "-l", str( path ) ],
            cwd=REPO_ROOT, capture_output=True, text=True
        )
    return path


def read_log():
    return LOG_FILE.read_text().split( "\n" ) if LOG_FILE.exists() else []


def summarize_log():
    lines = read_log()
    undocumented = [ l for l in lines if "is not documented" in l ]
    unknown = [ l for l in lines if "Found unknown command" in l ]
    other = len( lines ) - len( undocumented ) - len( unknown ) - 1
    print( f"  {len( undocumented ):>5} undocumented members and classes" )
    print( f"  {len( unknown ):>5} unknown commands (@TODO style tags, see ALIASES)" )
    print( f"  {max( other, 0 ):>5} other warnings" )
    note( f"full log: {LOG_FILE.relative_to( REPO_ROOT )}" )


# Doxygen writes these index pages whether or not the layout links them.
# DoxygenLayout.xml offers the class index and aether.h only, so the rest are
# removed after generation. doxygen_crawl.html goes with them, it is a list of
# links to every page including these.
UNLISTED = (
    "annotated.html", "annotated_dup.js",
    "hierarchy.html", "hierarchy.js", "inherits.html",
    "files.html", "files_dup.js",
    "functions*.html", "functions*.js",
    "doxygen_crawl.html",
)


def block_end( text, start ):
    """Index just past the div opened at that index."""
    depth = 0
    for tag in re.finditer( r"<div\b|</div>", text[ start: ] ):
        depth += 1 if tag.group( 0 ).startswith( "<div" ) else -1
        if depth == 0:
            return start + tag.end()
    return len( text )


def drop_undocumented_details( text ):
    """Removes the detail blocks doxygen writes for members carrying no
    comment. EXTRACT_ALL is on so that every member reaches the member list,
    and it writes a block for each of them whether there is anything to say."""
    for item in reversed( list( re.finditer( r'<div class="memitem">', text ) ) ):
        end = block_end( text, item.start() )
        doc = re.search( r'<div class="memdoc">(.*)', text[ item.start():end ], re.S )
        if doc and re.sub( r"<[^>]+>", "", doc.group( 1 ) ).strip():
            continue
        title = text.rfind( '<h2 class="memtitle">', 0, item.start() )
        anchor = text.rfind( "<a id=", 0, title if title >= 0 else item.start() )
        start = anchor if anchor >= 0 else ( title if title >= 0 else item.start() )
        text = text[ :start ] + text[ end: ]
    return text


def first_sentence( page_text, anchor ):
    """The opening sentence of the member documented at that anchor, as text."""
    at = page_text.find( f'id="{anchor}"' )
    doc = page_text.find( '<div class="memdoc">', at ) if at >= 0 else -1
    if doc < 0:
        return ""
    para = re.search( r"<p>(.*?)</p>", page_text[ doc:doc + 8000 ], re.S )
    if not para:
        return ""
    plain = html.unescape( re.sub( r"<[^>]+>", "", para.group( 1 ) ) )
    plain = re.sub( r"\s+", " ", plain ).strip()
    stop = re.search( r"\.(\s|$)", plain )
    return html.escape( plain[ :stop.end() ].strip() if stop else plain )


def inline_member_lists( html_dir ):
    """Moves each class's member list onto the class page and deletes the page
    it lived on. The column naming the class repeats the page title, so the
    signature keeps the row and the member's first sentence follows it."""
    moved = 0
    for source in sorted( html_dir.glob( "*-members.html" ) ):
        page = html_dir / ( source.name[ :-len( "-members.html" ) ] + ".html" )
        if not page.exists():
            continue
        table = re.search(
            r'<table class="directory">.*?</table>', source.read_text(), re.S
        )
        if table:
            text = drop_undocumented_details( page.read_text() )
            rows = []
            for row in re.finditer( r'<tr class="([^"]*)">(.*?)</tr>', table.group( 0 ), re.S ):
                cells = re.findall( r'<td class="entry">(.*?)</td>', row.group( 2 ), re.S )
                if not cells:
                    continue
                # Members of this class are on this page now, so their links
                # are anchors. Undocumented ones carry no link, and doxygen
                # names the defining class after them, which the page title
                # already says.
                entry = cells[ 0 ].replace( f'href="{page.name}#', 'href="#' )
                entry = re.sub( r"\s*\(defined in .*\)\s*$", "", entry, flags=re.S )
                anchor = re.search( r'href="#([^"]+)"', entry )
                if anchor and f'id="{anchor.group( 1 )}"' not in text:
                    # Nothing to link to: the member carries no comment.
                    entry = re.sub( r"<a [^>]*>(.*?)</a>", r"\1", entry, count=1, flags=re.S )
                    anchor = None
                brief = first_sentence( text, anchor.group( 1 ) ) if anchor else ""
                rows.append(
                    f'<tr class="{row.group( 1 )}"><td class="entry">{entry}</td>'
                    f'<td class="desc">{brief}</td></tr>'
                )
            block = (
                '<h2 class="groupheader">Public Members</h2>\n'
                '<table class="directory">\n' + "\n".join( rows ) + "\n</table>\n"
            ) if rows else ""
            # The description leads the page, so the list goes after it: the
            # first heading that is not the description, or the end of the
            # contents when the class carries nothing else.
            cut = None
            for at in re.finditer(
                r'<table class="memberdecls">'
                r'|(?:<a name="doc-[^"]*"[^>]*></a>\s*)?'
                r'<h2[^>]*class="(?:groupheader|memtitle)"[^>]*>(?P<title>.*?)</h2>',
                text, re.S
            ):
                if ( at.group( "title" ) or "" ).strip() == "Description":
                    continue
                cut = at.start()
                break
            if cut is None:
                cut = text.index( '</div><!-- contents -->' )
            page.write_text( text[ :cut ] + block + text[ cut: ] )
        source.unlink()
        moved += 1

    # The link to the page that is now inlined, the separator it followed, and
    # the summary line when the link was all it held.
    def tidy_summary( match ):
        inner = re.sub( r"(?:\s|&#124;)+$", "", match.group( 1 ) )
        return f'<div class="summary">{inner}</div>' if inner.strip() else ""

    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        stripped = re.sub(
            r'\s*<a href="[^"]*-members\.html">[^<]*</a>', "", text
        )
        stripped = re.sub(
            r'<div class="summary">(.*?)</div>', tidy_summary, stripped, flags=re.S
        )
        if stripped != text:
            page.write_text( stripped )
    return moved


def drop_class_keyword( html_dir ):
    """Removes the kind from a class listing. The heading above the table
    already says these are classes, and the name links to a page that names the
    kind again."""
    keyword = re.compile(
        r'<td class="memItemLeft">(?:class|struct|union)\s*(?:&#160;)?</td>'
    )
    dropped = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        trimmed, count = keyword.subn( '<td class="memItemLeft"></td>', text )
        if count:
            page.write_text( trimmed )
            dropped += count
    return dropped


def drop_index_intro( html_dir ):
    """Removes the sentence doxygen writes above an index listing. The page
    title says what the list is, and the layout's intro attribute only replaces
    the sentence, never drops it."""
    intro = re.compile(
        r'<div class="textblock">Here is a list of all [^<]*</div>'
    )
    dropped = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        trimmed, count = intro.subn( "", text )
        if count:
            page.write_text( trimmed )
            dropped += 1
    return dropped


def drop_breadcrumb( html_dir ):
    """Removes the navigation path above a page. It leads to the namespace,
    which the site does not list."""
    trail = re.compile( r'<div id="nav-path" class="navpath">.*?</div>\s*', re.S )
    dropped = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        trimmed, count = trail.subn( "", text )
        if count:
            page.write_text( trimmed )
            dropped += 1
    return dropped


def drop_description_heading( html_dir ):
    """Removes the heading over a class description. The page title and its
    group already name what follows, so the heading reads as a second title."""
    heading = re.compile(
        r'<h2 id="header-details" class="groupheader">Description</h2>\s*'
    )
    dropped = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        trimmed, count = heading.subn( "", text )
        if count:
            page.write_text( trimmed )
            dropped += 1
    return dropped


def remove_unlisted( html_dir ):
    """Deletes the index pages nothing in the site links to."""
    removed = 0
    for pattern in UNLISTED:
        for path in sorted( html_dir.glob( pattern ) ):
            path.unlink()
            removed += 1
    return removed


def open_path( path, allowed ):
    """Opens a built site, unless asked not to or nobody is watching."""
    if not allowed or not is_tty():
        return
    opener = "open" if sys.platform == "darwin" else "xdg-open"
    if shutil.which( opener ):
        subprocess.run( [ opener, str( path ) ] )


#------------------------------------------------------------------------------
# Preview server
#------------------------------------------------------------------------------
class NoStoreHandler( http.server.SimpleHTTPRequestHandler ):
    """Serves the site without caching. A preview is regenerated in place, so a
    cached stylesheet or script shows the previous run's site."""

    def end_headers( self ):
        self.send_header( "Cache-Control", "no-store" )
        super().end_headers()

    def log_message( self, *args ):
        pass


def serve_site( html_dir, port ):
    """Serves the generated site on the given port and returns what is
    listening. A
    page that embeds a live example only runs over http; the browser refuses to
    fetch its wasm from a file:// page. The server outlives this script, and
    regenerating the site needs no restart."""
    with socket.socket( socket.AF_INET, socket.SOCK_STREAM ) as probe:
        if probe.connect_ex( ( "127.0.0.1", port ) ) == 0:
            return "an earlier preview is still serving it"
    process = subprocess.Popen(
        [ sys.executable, __file__, "--serve", str( html_dir ),
          "--port", str( port ) ],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        start_new_session=True
    )
    return f"serving as pid {process.pid}, kill it to stop"


#------------------------------------------------------------------------------
# Code blocks
#------------------------------------------------------------------------------
# A snippet keeps the indentation it has in its file, so a region taken from
# inside a function arrives a level or more deep. The shared indentation is
# dropped so every block reads from the left edge.
FRAGMENT = re.compile( r'<div class="fragment">(.*?)</div><!-- fragment -->', re.S )
CODE_LINE = re.compile(
    r'(<div class="line">(?:<a id="l\d+" name="l\d+"></a>)?'
    r'(?:<span class="lineno">[^<]*</span>)?)( *)'
)


def dedent_block( match ):
    lines = match.group( 1 ).split( "\n" )
    widths = []
    for line in lines:
        opening = CODE_LINE.match( line )
        if opening and line[ opening.end(): ] not in ( "", "</div>" ):
            widths.append( len( opening.group( 2 ) ) )
    shared = min( widths ) if widths else 0
    if not shared:
        return match.group( 0 )
    trimmed = []
    for line in lines:
        opening = CODE_LINE.match( line )
        if opening:
            line = (
                opening.group( 1 ) + opening.group( 2 )[ shared: ]
                + line[ opening.end(): ]
            )
        trimmed.append( line )
    return (
        '<div class="fragment">' + "\n".join( trimmed ) + '</div><!-- fragment -->'
    )


LINE_NUMBER = re.compile( r'(<span class="lineno">)([^<]*)(</span>)' )


def renumber_block( match ):
    """Numbers a code block from one. A snippet carries the line numbers of the
    file it was lifted from, which say nothing about the block."""
    body = match.group( 1 )
    numbers = LINE_NUMBER.findall( body )
    if not numbers or numbers[ 0 ][ 1 ].strip() in ( "", "1" ):
        return match.group( 0 )
    counter = iter( range( 1, len( numbers ) + 1 ) )
    body = LINE_NUMBER.sub(
        lambda m: m.group( 1 ) + str( next( counter ) ).rjust( len( m.group( 2 ) ) ) + m.group( 3 ),
        body
    )
    return '<div class="fragment">' + body + '</div><!-- fragment -->'


def renumber_fragments( html_dir ):
    """Numbers every code block from one; returns how many pages changed."""
    renumbered = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        if page.name.endswith( "_source.html" ):
            continue
        text = page.read_text()
        numbered = FRAGMENT.sub( renumber_block, text )
        if numbered != text:
            page.write_text( numbered )
            renumbered += 1
    return renumbered


def dedent_fragments( html_dir ):
    """Drops the indentation every line of a code block shares. The source
    listing is left alone; it is the file, indentation and all."""
    dedented = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        if page.name.endswith( "_source.html" ):
            continue
        text = page.read_text()
        trimmed = FRAGMENT.sub( dedent_block, text )
        if trimmed != text:
            page.write_text( trimmed )
            dedented += 1
    return dedented


#------------------------------------------------------------------------------
# Live examples
#------------------------------------------------------------------------------
def add_demo_script( html_dir ):
    """Loads doxygen-demo.js on every page holding a live example frame. The
    frames come from a doxygen alias and from raw HTML in README.md, so the
    script is attached here rather than at each site."""
    tag = '<script src="doxygen-demo.js"></script>'
    added = 0
    for page in sorted( html_dir.glob( "*.html" ) ):
        text = page.read_text()
        if 'class="ae-demo"' not in text or tag in text:
            continue
        page.write_text( text.replace( "</body>", f"{tag}\n</body>", 1 ) )
        added += 1
    return added



def emscripten_build_dir():
    """Where the emscripten preset writes, read from CMakePresets.json so a
    renamed build directory does not leave the preview pointing at nothing."""
    presets = REPO_ROOT / "CMakePresets.json"
    if presets.exists():
        import json
        for preset in json.loads( presets.read_text() ).get( "configurePresets", [] ):
            if preset.get( "name" ) == "emscripten" and preset.get( "binaryDir" ):
                return Path( preset[ "binaryDir" ].replace( "${sourceDir}", str( REPO_ROOT ) ) )
    return REPO_ROOT / "build_emscripten"


def linked_config( html_dir ):
    """Which build the examples are currently linked to, or None when they are
    copied or absent. The link is the setting, so a preview that is not told
    otherwise leaves it alone."""
    target = html_dir / "examples"
    if not target.is_symlink():
        return None
    name = Path( os.readlink( target ) ).name
    return name if name in BUILD_CONFIGS else None


DEMO_REFERENCE = re.compile( r'data-demo="examples/([^"]+)"' )


def referenced_examples( html_dir ):
    """The examples the generated pages embed, by directory name."""
    names = set()
    for page in html_dir.glob( "*.html" ):
        names.update( DEMO_REFERENCE.findall( page.read_text() ) )
    return names


def place_examples( html_dir, source, copy=False ):
    """Puts the built web examples where a demo frame looks for them, and says
    what it did. A config name links the directory the emscripten build writes
    to, so a rebuild needs no regeneration; "published" copies docs/examples,
    which is what the site ships."""
    target = html_dir / "examples"
    if target.is_symlink():
        target.unlink()
    elif target.is_dir():
        shutil.rmtree( target )

    named = source in BUILD_CONFIGS
    source = emscripten_build_dir() / "examples" / source if named else Path( source )
    if not source.is_dir():
        return f"none: {source} does not exist"
    built = len( [ p for p in source.iterdir() if p.is_dir() and any( p.iterdir() ) ] )
    if copy:
        # Publishing ships what the pages ask for. A link can carry everything
        # built, since trying a demo out comes before referencing it.
        wanted = referenced_examples( html_dir )
        target.mkdir( parents=True )
        for name in sorted( wanted ):
            if ( source / name ).is_dir():
                shutil.copytree( source / name, target / name )
        missing = sorted( name for name in wanted if not ( target / name ).exists() )
        copied_count = len( list( target.iterdir() ) )
        if missing:
            note( f"not built, so not published: {', '.join( missing )}" )
        return f"{copied_count} of {built} copied from {source}"
    target.symlink_to( os.path.relpath( source, html_dir ) )
    return f"{built} linked from {shorten( source )}"


#------------------------------------------------------------------------------
# Tasks
#------------------------------------------------------------------------------
def shorten( path ):
    """A path relative to the repository when it sits inside it."""
    try:
        return path.relative_to( REPO_ROOT )
    except ValueError:
        return path


def finish_site( html_dir, args ):
    """Turns doxygen's output into the site: the examples, the script that runs
    them, and the page edits. Shared by the preview and the published build."""
    pages = len( list( html_dir.glob( "*.html" ) ) )
    print( f"  {pages} pages in {shorten( html_dir )}" )
    source = args.config_dir or args.config or linked_config( html_dir ) or "Release"
    print( f"  live examples: {place_examples( html_dir, source, args.publishing )}" )
    scripted = add_demo_script( html_dir )
    if scripted:
        print( f"  {scripted} pages run a live example" )
    renumbered = renumber_fragments( html_dir )
    if renumbered:
        print( f"  {renumbered} pages with code blocks numbered from one" )
    dedented = dedent_fragments( html_dir )
    if dedented:
        print( f"  {dedented} pages with code blocks moved to the left edge" )
    inlined = inline_member_lists( html_dir )
    if inlined:
        print( f"  {inlined} member lists moved onto their class pages" )
    dropped = drop_description_heading( html_dir )
    if dropped:
        print( f"  {dropped} description headings removed" )
    keywords = drop_class_keyword( html_dir )
    if keywords:
        print( f"  {keywords} class rows lost their kind" )
    intros = drop_index_intro( html_dir )
    if intros:
        print( f"  {intros} index pages lost their preamble" )
    trails = drop_breadcrumb( html_dir )
    if trails:
        print( f"  {trails} navigation paths removed" )
    unlisted = remove_unlisted( html_dir )
    if unlisted:
        print( f"  {unlisted} unlisted index pages removed" )



def task_publish( args ):
    """Assembles the site for publishing: the examples the pages reference are
    copied in rather than linked, and nothing is served."""
    heading( "Site" )
    args.publishing = True
    html_dir = Path( args.out ).resolve() if args.out else BUILD_DIR / "publish"
    if html_dir.exists():
        shutil.rmtree( html_dir )
    with generating():
        step( "Generating HTML" )
        doxyfile = write_doxyfile( "publish", {
            "OUTPUT_DIRECTORY": html_dir.parent,
            "HTML_OUTPUT": html_dir.name,
            "GENERATE_HTML": "YES",
            "GENERATE_XML": "NO",
            "QUIET": "YES",
            "WARN_LOGFILE": LOG_FILE,
        } )
        run_doxygen( doxyfile )
        finish_site( html_dir, args )

    step( "Warnings" )
    summarize_log()

    step( "Serving" )
    port = args.port or PUBLISH_PORT
    url = f"http://127.0.0.1:{port}/index.html"
    print( f"  {url}" )
    note( serve_site( html_dir, port ) )

    step( "Next" )
    note( f"upload {shorten( html_dir )}" )
    open_path( url, not args.no_open and remembered( "open", True ) )


def task_preview( args ):
    heading( "Preview build" )
    args.publishing = False
    html_dir = Path( args.out ).resolve() if args.out else BUILD_DIR / "html"
    if not args.config:
        args.config = linked_config( html_dir ) or "Release"
    if args.clean and html_dir.exists():
        # Doxygen leaves a page it no longer generates where it is, and the
        # preview serves it like any other.
        shutil.rmtree( html_dir )
        note( "cleared the previous site, every graph is drawn again" )
    with generating():
        step( "Generating HTML" )
        overrides = {
            "OUTPUT_DIRECTORY": BUILD_DIR,
            "HTML_OUTPUT": "html",
            "GENERATE_HTML": "YES",
            "GENERATE_XML": "NO",
            "QUIET": "YES",
            "WARN_LOGFILE": LOG_FILE,
        }
        doxyfile = write_doxyfile( "preview", overrides )
        run_doxygen( doxyfile )
        finish_site( html_dir, args )

    step( "Warnings" )
    summarize_log()

    step( "Serving" )
    url = f"http://127.0.0.1:{args.port or PREVIEW_PORT}/index.html"
    print( f"  {url}" )
    note( serve_site( html_dir, args.port or PREVIEW_PORT ) )

    step( "Next" )
    note( "'report' ranks what is still undocumented" )
    note( "edit doxygen-custom.css to restyle, DoxygenLayout.xml to rename tabs" )
    open_path( url, not args.no_open and remembered( "open", True ) )


def task_report( args ):
    heading( "Coverage report" )
    with generating():
        step( "Generating XML to collect warnings" )
        doxyfile = write_doxyfile( "report", {
            "OUTPUT_DIRECTORY": BUILD_DIR,
            "XML_OUTPUT": "xml",
            "GENERATE_XML": "YES",
            "GENERATE_HTML": "NO",
            "QUIET": "YES",
            "WARN_LOGFILE": LOG_FILE,
            # Doxygen warns about an undocumented member only where it writes one
            # out, and the site layout writes no declaration lists, so the coverage
            # run reads through the stock layout to reach the whole API.
            "LAYOUT_FILE": default_layout(),
        } )
        run_doxygen( doxyfile )

        lines = read_log()
    step( "Totals" )
    summarize_log()

    compounds = sorted( set(
        re.findall( r"Compound (ae::[\w:]+) is not documented", "\n".join( lines ) )
    ) )
    members = Counter(
        re.findall( r"of class (ae::[\w:]+) is not documented", "\n".join( lines ) )
    )

    step( f"Classes with no description ({len( compounds )})" )
    source = ( REPO_ROOT / "aether.h" ).read_text()

    def mentions( name ):
        # Whole word match on the unqualified name, so ae::Type does not also
        # count ae::TypeId and ae::ClassType.
        return len( re.findall(
            r"\b" + re.escape( name.replace( "ae::", "" ) ) + r"\b", source
        ) )

    ranked = sorted( compounds, key=mentions, reverse=True )
    for name in ranked[ :10 ]:
        print( f"  {mentions( name ):>5} mentions in aether.h   {name}" )
    if len( ranked ) > 10:
        note( f"...and {len( ranked ) - 10} more" )

    step( "Classes with the most undocumented members" )
    for name, count in members.most_common( 10 ):
        print( f"  {count:>5} members              {name}" )

    step( "Next" )
    note( "a one line //! above a class fills its row in every list it appears in" )
    note( "see AGENTS.md 'Documentation' for the doxygen conventions" )


#------------------------------------------------------------------------------
# Menu
#------------------------------------------------------------------------------
def choose_config( args ):
    """Sets which web build the next preview's live examples come from. Escape,
    return and q all keep the current one."""
    print( "\n " + bold( "Config" ) + "\n" )
    for i, name in enumerate( BUILD_CONFIGS, 1 ):
        where = f"linked from {emscripten_build_dir().name}/examples/{name}"
        current = args.config or linked_config( BUILD_DIR / "html" ) or "Release"
        print( f"   {i}  {'*' if name == current else ' '} {name:<15} {dim( where )}" )
    print( f"\n   q  {'Back':<17} {dim( 'or escape' )}" )
    while True:
        choice = read_key( "\n > " )
        if not choice or choice in ( "q", ESCAPE ):
            return
        if choice.isdigit() and 1 <= int( choice ) <= len( BUILD_CONFIGS ):
            args.config = BUILD_CONFIGS[ int( choice ) - 1 ]
        else:
            print( dim( "   Pick a number, or q to go back." ) )
            continue
        note( f"{args.config} on the next preview" )
        return


def choose_task( first, args ):
    """Returns a task key. Raises Quit."""
    while True:
        print( "\n " + bold(
            "What are you generating?" if first else "What next?"
        ) + "\n" )
        keys = list( TASKS )
        for i, key in enumerate( keys, 1 ):
            title, description = TASKS[ key ]
            print( f"   {i}  {title:<17} {dim( description )}" )
        shown = args.config or linked_config( BUILD_DIR / "html" ) or "Release"
        print( f"\n   c  {'Config':<17} {dim( shown )}" )
        opens = "open in browser" if remembered( "open", True ) else "browser disabled"
        print( f"   b  {'On build':<17} {dim( opens )}" )
        print( f"   q  {'Quit':<17} {dim( 'or escape' )}" )
        while True:
            choice = read_key( "\n > " ).lower()
            if choice == "q" or choice == ESCAPE:
                raise Quit()
            if choice == "c":
                choose_config( args )
                break
            if choice == "b":
                remember( "open", not remembered( "open", True ) )
                break
            if choice in keys:
                return choice
            if choice.isdigit() and 1 <= int( choice ) <= len( keys ):
                return keys[ int( choice ) - 1 ]
            if choice:
                print( dim( "   Pick a number, a task name, c, b, or q." ) )
        first = False


def run_task( task, args ):
    {
        "preview": task_preview,
        "publish": task_publish,
        "report": task_report
    }[ task ]( args )


def menu( args ):
    """Runs tasks until the prompt quits. A task that gives up returns here."""
    first = True
    while True:
        try:
            run_task( choose_task( first, args ), args )
        except Quit:
            return
        except KeyboardInterrupt:
            print()
        except SystemExit as error:
            if isinstance( error.code, str ):
                print( f"\n{error.code}" )
        first = False


def main():
    parser = argparse.ArgumentParser(
        description="Generate aether-game-utils documentation.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="\n".join(
            f"  {key:<9} {TASKS[ key ][ 1 ]}" for key in TASKS
        )
    )
    parser.add_argument(
        "task", nargs="?", choices=list( TASKS ),
        help="task to run once; omit for a menu that repeats until q"
    )
    parser.add_argument(
        "--no-open", action="store_true", help="do not open the result"
    )
    parser.add_argument(
        "--yes", action="store_true", help="do not ask before overwriting docs/"
    )
    parser.add_argument(
        "--port", type=int, default=None,
        help=f"port to serve on; preview default: {PREVIEW_PORT}, publish default: {PUBLISH_PORT}"
    )
    parser.add_argument(
        "--config", default=None,
        choices=BUILD_CONFIGS,
        help="which web build the live examples come from; "
             "defaults to the one they are already linked to"
    )
    parser.add_argument(
        "--config-dir", metavar="DIR",
        help="a build config's example directory, in place of --config"
    )
    parser.add_argument(
        "--out", metavar="DIR",
        help="where the site is written; defaults to build_docs/html for "
             "preview and build_docs/publish for publish"
    )
    parser.add_argument(
        "--clean", action="store_true",
        help="delete the generated site first, dropping stale pages"
    )
    parser.add_argument(
        "--serve", metavar="DIR", help=argparse.SUPPRESS
    )
    args = parser.parse_args()

    if args.serve:
        handler = functools.partial( NoStoreHandler, directory=args.serve )
        http.server.ThreadingHTTPServer(
            ( "127.0.0.1", args.port or PREVIEW_PORT ), handler ).serve_forever()
        return

    heading( "aether-game-utils documentation" )
    if not print_tools():
        sys.exit( "\nERROR: doxygen is required; install it with: brew install doxygen" )

    if args.task:
        run_task( args.task, args )
    else:
        menu( args )


main()
