/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

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

#ifndef RASTER_POOL_HH
#define RASTER_POOL_HH

#include <mutex>

#include "raster.hh"

template<class RasterType>
class RasterPool;

template<class RasterType>
class RasterDeleter
{
private:
  RasterPool<RasterType>* raster_pool_ = nullptr;

public:
  void operator()( RasterType* raster ) const;

  RasterPool<RasterType>* get_raster_pool( void ) const;
  void set_raster_pool( RasterPool<RasterType>* pool );
};

template<class RasterType>
class BaseRasterHandle
{
private:
  std::unique_ptr<RasterType, RasterDeleter<RasterType>> raster_;

public:
  BaseRasterHandle( const unsigned int display_width,
                    const unsigned int display_height );

  BaseRasterHandle( const unsigned int display_width,
                    const unsigned int display_height,
                    RasterPool<RasterType>& raster_pool );

  operator const RasterType&() const { return *raster_; }
  operator RasterType&() { return *raster_; }

  const RasterType& get( void ) const { return *raster_; }
  RasterType& get( void ) { return *raster_; }
};

using RasterHandle = BaseRasterHandle<BaseRaster>;

#endif /* RASTER_POOL_HH */