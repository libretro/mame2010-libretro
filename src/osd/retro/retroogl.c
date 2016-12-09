
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static struct retro_hw_render_callback hw_render;

#include "glsym/glsym.h"
static GLuint prog;
static GLuint vbo;

struct frame
{
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
   GLuint tex;
#if !defined(HAVE_OPENGLES)
   GLuint pbo;
#endif
#endif
};
static struct frame frames[1];

static GLint vertex_loc;
static GLint tex_loc;

static const GLfloat vertex_data[] = {

      -1, -1, 0, 0,
       1, -1, 1, 0,
      -1,  1, 0, 1,
       1,  1, 1, 1,
};

static const char *vertex_shader[] = {
      "attribute vec2 aVertex;\n",
      "attribute vec2 aTexCoord;\n",
      "varying vec2 vTex;\n",
      "void main() { gl_Position = vec4(aVertex, 0.0, 1.0); vTex = aTexCoord; }\n",
};


static const char *fragment_shader[] = {
      "#ifdef GL_ES\n",
      "precision mediump float;\n",
      "#endif\n",
      "varying vec2 vTex;\n",
      "uniform sampler2D sTex0;\n",
#ifdef GLES
      "void main() { gl_FragColor = texture2D(sTex0, vTex); }\n",
};
      // Get format as GL_RGBA/GL_UNSIGNED_BYTE. Assume little endian, so we get ARGB -> BGRA byte order, and we have to swizzle to .BGR.
#else
      "void main() { gl_FragColor = texture2D(sTex0, vTex); }\n",
};
#endif


static void compile_program(void)
{
   prog = glCreateProgram();
   GLuint vert = glCreateShader(GL_VERTEX_SHADER);
   GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

   glShaderSource(vert, ARRAY_SIZE(vertex_shader), vertex_shader, 0);
   glShaderSource(frag, ARRAY_SIZE(fragment_shader), fragment_shader, 0);
   glCompileShader(vert);
   glCompileShader(frag);

   glAttachShader(prog, vert);
   glAttachShader(prog, frag);
   glLinkProgram(prog);
   glDeleteShader(vert);
   glDeleteShader(frag);
}

static void setup_texture(void)
{

   glUniform1i(glGetUniformLocation(prog, "sTex0"), 0);

   vertex_loc = glGetAttribLocation(prog, "aVertex");
   tex_loc = glGetAttribLocation(prog, "aTexCoord");


 for (unsigned i = 0; i < 1; i++)
   {
      glGenTextures(1, &frames[i].tex);

      glBindTexture(GL_TEXTURE_2D, frames[i].tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifndef HAVE_OPENGLES
      glGenBuffers(1, &frames[i].pbo);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, frames[i].pbo);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, 1024 * 768 * 4, NULL, GL_STREAM_DRAW);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif
   }


}
static void bind_texture(void)
{
   glBindTexture(GL_TEXTURE_2D, 0);
}

static void setup_vao(void)
{
   glUseProgram(prog);

   setup_texture();

   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   bind_texture();

   glUseProgram(0);
}

static void context_reset(void)
{
   fprintf(stderr, "Context reset!\n");
   rglgen_resolve_symbols(hw_render.get_proc_address);
   compile_program();
   setup_vao();
}

static void context_destroy(void)
{
   fprintf(stderr, "Context destroy!\n");
   glDeleteBuffers(1, &vbo);
   vbo = 0;
   glDeleteProgram(prog);
   prog = 0;
}

static void do_gl2d(){


      glBindTexture(GL_TEXTURE_2D, frames[0].tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                  rtwi, rthe, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, videoBuffer/*NULL*/);
      glBindTexture(GL_TEXTURE_2D, 0);

      glBindFramebuffer(GL_FRAMEBUFFER, hw_render.get_current_framebuffer());
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, rtwi, rthe/*1024, 768)*/);
      glUseProgram(prog);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, frames[0].tex);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (const GLvoid*)(0 * sizeof(GLfloat)));
      glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(GLfloat), (const GLvoid*)(2 * sizeof(GLfloat)));
      glEnableVertexAttribArray(vertex_loc);
      glEnableVertexAttribArray(tex_loc);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      glDisableVertexAttribArray(vertex_loc);
      glDisableVertexAttribArray(tex_loc);

      glUseProgram(0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);

      video_cb(RETRO_HW_FRAME_BUFFER_VALID, rtwi, rthe, topw << PITCH);

}

