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

#include "raster_handle.hh"

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "util/exception.hh"

using namespace std;

/* helper to dequeue front element from a queue */
template<typename T>
static T dequeue( queue<T>& q )
{
  T ret { move( q.front() ) };
  q.pop();
  return ret;
}

template<class RasterType>
class RasterPool
{
public:
  typedef std::unique_ptr<RasterType, RasterDeleter<RasterType>> RasterHolder;

private:
  queue<RasterHolder> unused_rasters_ {};
  mutex mutex_ {};

public:
  RasterHolder make_raster( const unsigned int display_width,
                            const unsigned int display_height )
  {
    unique_lock<mutex> lock { mutex_ };
    RasterHolder ret;

    if ( unused_rasters_.empty() ) {
      ret.reset( new RasterType(
        display_width, display_height, display_width, display_height ) );
    } else {
      if ( ( unused_rasters_.front()->display_width() != display_width )
           or ( unused_rasters_.front()->display_height()
                != display_height ) ) {
        throw Unsupported( "raster size has changed" );
      } else {
        ret = dequeue( unused_rasters_ );
      }
    }

    ret.get_deleter().set_raster_pool( this );

    return ret;
  }

  void free_raster( RasterType* raster )
  {
    unique_lock<mutex> lock { mutex_ };

    assert( raster );
    unused_rasters_.emplace( raster );
  }
};

template<class RasterType>
void RasterDeleter<RasterType>::operator()( RasterType* raster ) const
{
  if ( raster_pool_ ) {
    raster_pool_->free_raster( raster );
  } else {
    delete raster;
  }
}

template<class RasterType>
RasterPool<RasterType>* RasterDeleter<RasterType>::get_raster_pool( void ) const
{
  return raster_pool_;
}

template<class RasterType>
void RasterDeleter<RasterType>::set_raster_pool( RasterPool<RasterType>* pool )
{
  assert( not raster_pool_ );
  raster_pool_ = pool;
}

template<class RasterType>
static RasterPool<RasterType>& global_raster_pool( void )
{
  static RasterPool<RasterType> pool;
  return pool;
}

template<class RasterType>
BaseRasterHandle<RasterType>::BaseRasterHandle(
  const unsigned int display_width,
  const unsigned int display_height )
  : BaseRasterHandle( display_width,
                      display_height,
                      global_raster_pool<RasterType>() )
{}

template<class RasterType>
BaseRasterHandle<RasterType>::BaseRasterHandle(
  const unsigned int display_width,
  const unsigned int display_height,
  RasterPool<RasterType>& raster_pool )
  : raster_( raster_pool.make_raster( display_width, display_height ) )
{}

template class RasterDeleter<BaseRaster>;
template class BaseRasterHandle<BaseRaster>;