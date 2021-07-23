#include <emscripten.h> // For emscripten_get_device_pixel_ratio()
#include <emscripten/html5.h> // For Emscripten HTML5 WebGL context creation API
#include <webgl/webgl1.h> // For Emscripten WebGL API headers (see also webgl/webgl1_ext.h and webgl/webgl2.h)
#include <string.h> // For NULL and strcmp()
#include <assert.h> // For assert()
#include <math.h>
#include <memory.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "ae/aether.h"

typedef struct Texture
{
  // Image
  char *url;
  int w, h;

  GLuint texture;
} Texture;

extern "C"
{
  void load_texture_from_url(GLuint texture, const char *url, int *outWidth, int *outHeight);
}

#define WIDTH 1024
#define HEIGHT 768
#define MAX_TEXTURES 256
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext;
static GLuint quad, colorPos, matPos, solidColor;
static float pixelWidth, pixelHeight;
static Texture textures[MAX_TEXTURES] = {};

static GLuint compile_shader(GLenum shaderType, const char *src)
{
   GLuint shader = glCreateShader(shaderType);
   glShaderSource(shader, 1, &src, NULL);
   glCompileShader(shader);
   return shader;
}

static GLuint create_program(GLuint vertexShader, GLuint fragmentShader)
{
   GLuint program = glCreateProgram();
   glAttachShader(program, vertexShader);
   glAttachShader(program, fragmentShader);
   glBindAttribLocation(program, 0, "pos");
   glLinkProgram(program);
   glUseProgram(program);
   return program;
}

static GLuint create_texture()
{
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return texture;
}

static Texture *find_or_cache_url(const char *url)
{
  for(int i = 0; i < MAX_TEXTURES; ++i) // Naive O(n) lookup for tiny code size
    if (!strcmp(textures[i].url, url))
      return textures+i;
    else if (!textures[i].url)
    {
      textures[i].url = strdup(url);
      textures[i].texture = create_texture();
      load_texture_from_url(textures[i].texture, url, &textures[i].w, &textures[i].h);
      return textures+i;
    }
  return 0; // fail
}

// WebGL canvas clear
void clear_screen(float r, float g, float b, float a)
{
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void fill_textured_rectangle(float x0, float y0, float x1, float y1, float r, float g, float b, float a, GLuint texture)
{
  float mat[16] = { (x1-x0)*pixelWidth, 0, 0, 0, 0, (y1-y0)*pixelHeight, 0, 0, 0, 0, 1, 0, x0*pixelWidth-1.f, y0*pixelHeight-1.f, 0, 1};
  glUniformMatrix4fv(matPos, 1, 0, mat);
  glUniform4f(colorPos, r, g, b, a);
  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Fills a rectangle (x0,y0) -> (x1,y1) with given solid color rgba. Coordinates are in pixel units.
// Bottom-left corner of screen is (0,0). Top-right corner is (width-1,height-1) pixels.
void fill_solid_rectangle(float x0, float y0, float x1, float y1, float r, float g, float b, float a)
{
  fill_textured_rectangle(x0, y0, x1, y1, r, g, b, a, solidColor);
}

// Draws an image from given url to pixel coordinates x0,y0, applying uniform scaling factor scale, modulated with rgba. 
void fill_image(float x0, float y0, float scale, float r, float g, float b, float a, const char *url)
{
  Texture *t = find_or_cache_url(url);
  fill_textured_rectangle(x0, y0, x0 + t->w * scale, y0 + t->h * scale, r, g, b, a, t->texture);
}

class Game
{
public:
  ae::Vec2 charPos = ae::Vec2( 0.0f );
  ae::Vec2 charVel = ae::Vec2( 0.0f );

  void Initialize()
  {
    Validate();

    printf( "Initialize\n" );
    window.Initialize( WIDTH, HEIGHT, "WebGL Demo", true );
    gfx.Initialize( &window );
    input.Initialize( &window );

    Validate();
  }

  void Update( float dt )
  {
    Validate();

    // printf( "dt %.2f\n", dt );
    // printf( "update0 %.2f %.2f\n", charPos.x, charPos.y );
    input.Pump();

    char u = input.Get( ae::Key::Up );// != 0;
    char d = input.Get( ae::Key::Down );// != 0;
    char l = input.Get( ae::Key::Left );// != 0;
    char r = input.Get( ae::Key::Right );// != 0;
    // AE_INFO( "u:# d:# l:# r:#", (int)u, (int)d, (int)l, (int)r );
    // printf( "u:%d d:%d l:%d r:%d\n", (int)u, (int)d, (int)l, (int)r );

    float speed = 500.0f;

    ae::Vec2 accel( 0.0f );
    accel.x += input.Get( ae::Key::Right ) ? 1.0f : 0.0f;
    accel.x -= input.Get( ae::Key::Left ) ? 1.0f : 0.0f;
    accel.y += input.Get( ae::Key::Up ) ? 1.0f : 0.0f;
    accel.y -= input.Get( ae::Key::Down ) ? 1.0f : 0.0f;
    accel.SafeNormalize();
    accel *= speed;
    // printf( "accel %.2f %.2f\n", accel.x, accel.y );

    charVel = ae::DtLerp( charVel, 2.0f, dt, ae::Vec2( 0.0f ) );
    charVel += accel * dt;
    charPos += charVel * dt;
    // printf( "update1 %.2f %.2f\n", charPos.x, charPos.y );

    Validate();
  }

  void Render( float dt )
  {
    Validate();

    // printf( "render %.2f %.2f\n", charPos.x, charPos.y );
    clear_screen(0.1f, 0.2f, 0.3f, 1.f);
    fill_image(WIDTH-250.f, HEIGHT - 250.f, 2.f, 1.f, 1.f, 1.f, 1.f, "moon.png");
    ae::Vec2 min = charPos - ae::Vec2( 16.0f );
    ae::Vec2 max = charPos + ae::Vec2( 16.0f );
    fill_solid_rectangle( min.x, min.y, max.x, max.y, 1.f, 0.5f, 0.5f, 1.f );

    gfx.Activate();
    gfx.Clear( ae::Color::PicoDarkPurple() );
    gfx.Present();

    Validate();
  }

  void Validate()
  {
    AE_ASSERT( check0 == 0xCDCDCDCD );
    AE_ASSERT( check1 == 0xCDCDCDCD );
    AE_ASSERT( check2 == 0xCDCDCDCD );
    AE_ASSERT( check3 == 0xCDCDCDCD );
  }

  uint32_t check0 = 0xCDCDCDCD;
  ae::Window window;
  uint32_t check1 = 0xCDCDCDCD;
  ae::GraphicsDevice gfx;
  uint32_t check2 = 0xCDCDCDCD;
  ae::Input input;
  uint32_t check3 = 0xCDCDCDCD;
};

// Per-frame animation tick.
EM_BOOL draw_frame( double t, void* userData )
{
  static double prevT = t;
  float dt = ( t - prevT ) * 0.001f;
  prevT = t;

  Game* game = (Game*)userData;
  game->Validate();
  game->Update( dt );
  game->Render( dt );
  game->Validate();

  return EM_TRUE;
}

int main()
{
  Game* game = ae::New< Game >( "game" );
  game->Initialize();
  
  pixelWidth = 2.f / WIDTH;
  pixelHeight = 2.f / HEIGHT;

  static const char vertex_shader[] =
    "attribute vec4 pos;"
    "varying vec2 uv;"
    "uniform mat4 mat;"
    "void main(){"
      "uv=pos.xy;"
      "gl_Position=mat*pos;"
    "}";
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

  static const char fragment_shader[] =
    "precision lowp float;"
    "uniform sampler2D tex;"
    "varying vec2 uv;"
    "uniform vec4 color;"
    "void main(){"
      "gl_FragColor=color*texture2D(tex,uv);"
    "}";
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

  GLuint program = create_program(vs, fs);
  colorPos = glGetUniformLocation(program, "color");
  matPos = glGetUniformLocation(program, "mat");
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenBuffers(1, &quad);
  glBindBuffer(GL_ARRAY_BUFFER, quad);
  const float pos[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  solidColor = create_texture();
  unsigned int whitePixel = 0xFFFFFFFFu;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);

  game->Validate();
  emscripten_request_animation_frame_loop( &draw_frame, game );
}

#define AE_MAIN
#include "ae/aether.h"
