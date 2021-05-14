/* Copyright 2013-2018 the Alfalfa authors
                       and the Massachusetts Institute of Technology

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

      1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#pragma once

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <string>
#include <vector>

class GLFWContext
{
  static void error_callback( const int, const char* const description );

public:
  GLFWContext();
  ~GLFWContext();

  /* forbid copy */
  GLFWContext( const GLFWContext& other ) = delete;
  GLFWContext& operator=( const GLFWContext& other ) = delete;
};

void glCheck( const std::string& where, bool ignore = false );

class Window
{
  struct Deleter
  {
    void operator()( GLFWwindow* x ) const;
  };
  std::unique_ptr<GLFWwindow, Deleter> window_;

public:
  Window( const unsigned int width,
          const unsigned int height,
          const std::string& title,
          const bool fullscreen = false );
  void make_context_current();
  bool should_close() const { return glfwWindowShouldClose( window_.get() ); }
  void swap_buffers() { glfwSwapBuffers( window_.get() ); }
  void set_swap_interval( const int interval ) { glfwSwapInterval( interval ); }
  void hide_cursor( const bool hidden );
  bool key_pressed( const int key ) const;
  std::pair<unsigned int, unsigned int> framebuffer_size() const;
  std::pair<unsigned int, unsigned int> window_size() const;
};

struct VertexObject
{
  float x[4];
  // float chroma_texture_x, chroma_texture_y;
};

template<GLenum id_>
class Buffer
{
public:
  Buffer() = delete;

  template<class T>
  static void bind( const T& obj )
  {
    glBindBuffer( id_, obj.num_ );
  }

  static void load( const std::vector<VertexObject>& vertices, const GLenum usage )
  {
    glBufferData( id, vertices.size() * sizeof( VertexObject ), &vertices.front(), usage );
  }

  constexpr static GLenum id = id_;
};

using ArrayBuffer = Buffer<GL_ARRAY_BUFFER>;

class VertexBufferObject
{
  friend ArrayBuffer;

  GLuint num_;

public:
  VertexBufferObject()
    : num_()
  {
    glGenBuffers( 1, &num_ );
  }
  ~VertexBufferObject() { glDeleteBuffers( 1, &num_ ); }

  /* forbid copy */
  VertexBufferObject( const VertexBufferObject& other ) = delete;
  VertexBufferObject& operator=( const VertexBufferObject& other ) = delete;
};

class VertexArrayObject
{
  GLuint num_;

public:
  VertexArrayObject()
    : num_()
  {
    glGenVertexArrays( 1, &num_ );
  }
  ~VertexArrayObject() { glDeleteVertexArrays( 1, &num_ ); }

  void bind() { glBindVertexArray( num_ ); }

  /* forbid copy */
  VertexArrayObject( const VertexArrayObject& other ) = delete;
  VertexArrayObject& operator=( const VertexArrayObject& other ) = delete;
};

class Texture
{
  GLuint num_;
  unsigned int width_, height_;

public:
  Texture( const unsigned int width, const unsigned int height )
    : num_()
    , width_( width )
    , height_( height )
  {
    glGenTextures( 1, &num_ );
  }

  ~Texture() { glDeleteTextures( 1, &num_ ); }

  void bind( const GLenum texture_unit ) const;
  void load( const std::vector<uint8_t>& raster, const GLenum texture_unit );
  unsigned int width() const { return width_; }
  unsigned int height() const { return height_; }

  /* disallow copy */
  Texture( const Texture& other ) = delete;
  Texture& operator=( const Texture& other ) = delete;
};

void compile_shader( const GLuint num, const std::string& source );

template<GLenum type_>
class Shader
{
  friend class Program;

protected:
  GLuint num_ = glCreateShader( type_ );

public:
  Shader( const std::string& source ) { compile_shader( num_, source ); }

  ~Shader() { glDeleteShader( num_ ); }

  /* forbid copy */
  Shader( const Shader& other ) = delete;
  Shader& operator=( const Shader& other ) = delete;
};

class Program
{
  GLuint num_ = glCreateProgram();

public:
  Program() {}
  ~Program() { glDeleteProgram( num_ ); }

  template<GLenum type_>
  void attach( const Shader<type_>& shader )
  {
    glAttachShader( num_, shader.num_ );
  }

  void link() { glLinkProgram( num_ ); }
  void use() { glUseProgram( num_ ); }

  GLint attribute_location( const std::string& name ) const;
  GLint uniform_location( const std::string& name ) const;

  /* forbid copy */
  Program( const Program& other ) = delete;
  Program& operator=( const Program& other ) = delete;
};

using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;
