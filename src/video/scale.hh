#pragma once

#include <memory>

#include "raster.hh"

extern "C"
{
#include "libswscale/swscale.h"
}

class Scaler
{
  uint16_t input_width, input_height;
  static constexpr uint16_t output_width = 1280, output_height = 720;

  SwsContext* context_ { nullptr };

  uint16_t source_x_ { 0 }, source_y_ { 0 }, source_width_ { input_width }, source_height_ { input_height };

  void saturate_params();

  bool need_new_context_ { false };
  void create_context();

public:
  Scaler( const uint16_t s_input_width = 1280, const uint16_t s_input_height = 720 )
    : input_width( s_input_width )
    , input_height( s_input_height )
  {
    create_context();
  }

  ~Scaler()
  {
    if ( context_ ) {
      sws_freeContext( context_ );
    }
  }

  void setup( const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height );

  void scale( const RasterYUV422& source, RasterYUV420& dest );

  Scaler( const Scaler& other ) = delete;
  Scaler& operator=( const Scaler& other ) = delete;
};

class ColorspaceConverter
{
  SwsContext* yuv2rgba_ { nullptr };
  SwsContext* rgba2yuv_ { nullptr };

public:
  ColorspaceConverter( const uint16_t width, const uint16_t height );
  ~ColorspaceConverter();

  void convert( const RasterYUV420& yuv, RasterRGBA& output ) const;
  void convert( const RasterRGBA& rgba, RasterYUV420& output ) const;

  ColorspaceConverter( ColorspaceConverter&& other )
    : yuv2rgba_( other.yuv2rgba_ )
    , rgba2yuv_( other.rgba2yuv_ )
  {
    other.yuv2rgba_ = other.rgba2yuv_ = nullptr;
  }

  ColorspaceConverter& operator=( ColorspaceConverter&& other ) = default;

  ColorspaceConverter( const ColorspaceConverter& other ) = delete;
  ColorspaceConverter& operator=( const ColorspaceConverter& other ) = delete;
};
