/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* Copyright 2013-2019 the Alfalfa authors
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

#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>
#include <optional>
#include <string_view>

#include "raster.hh"

class JPEGException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

class JPEGDecompresser
{
  jpeg_decompress_struct decompresser_ {};
  jpeg_error_mgr error_manager_ {};

  static void error( const j_common_ptr cinfo );
  static void info( const j_common_ptr cinfo, const int level );

  bool bad_ {};

public:
  JPEGDecompresser();
  ~JPEGDecompresser();

  void begin_decoding( const std::string_view chunk );

  void decode( RasterYUV422& r );

  unsigned int width() const;
  unsigned int height() const;

  bool bad() const { return bad_; }

  void reset();
};
