//------------------------------------------------------------------------------
// This file is part of the OpenStructure project <www.openstructure.org>
//
// Copyright (C) 2008-2010 by the OpenStructure authors
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 3.0 of the License, or (at your option)
// any later version.
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//------------------------------------------------------------------------------
/*
  Author: Ansgar Philippsen
*/

#include <vector>
#define ZLIB_WINAPI
#include <png.h>

#include <ost/log.hh>

#include "bitmap_io.hh"

namespace ost { namespace gfx {

void export_png(const String& filename, unsigned int width, unsigned int height, unsigned char* data)
{
  FILE *fp;
  
  if((fp=fopen(filename.c_str(),"wb"))==NULL) {
    LOGN_ERROR("error opening" << filename << " for exporting");
    return;
  }
  
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    LOGN_ERROR("error creating png write struct");
    return;
  }
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    LOGN_ERROR("error creating png info struct");
    return;
  }
  
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    LOGN_ERROR("error in png setjmp");
    return;
  }
  
  png_init_io(png_ptr, fp);

  int png_color_type = PNG_COLOR_TYPE_RGBA;
  unsigned int bpp = 8;

  png_set_IHDR(png_ptr, info_ptr, width, height, bpp, 
               png_color_type,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  
  png_write_info(png_ptr, info_ptr);
  
  png_bytep* row_pointers = new png_bytep[height];

  png_byte* png_data = data;

  for(uint i=0;i<height;++i){
    row_pointers[height-i-1]=&png_data[i*4*width];
  }

  png_write_image(png_ptr, row_pointers);

  delete []row_pointers;

  png_write_end(png_ptr, info_ptr);
  
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  
  fclose(fp);
}

Bitmap import_png(const String& filename)
{
  Bitmap bm;

  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 channels;
  FILE *fp;
  int png_transforms;
  png_bytep *row_pointers;
  png_uint_32 row,column,chn;
  png_uint_32 width,height,bpp;
  
  if((fp=fopen(filename.c_str(),"rb"))==NULL) {
    LOGN_ERROR("error opening " << filename);
    return bm;
  }
  
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				   NULL,NULL,NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    LOGN_ERROR("unexpected error #1 in png lib");
    return bm;
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    LOGN_ERROR("unexpected error #2 in png lib");
    return bm;
  }
  
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    LOGN_ERROR("unexpected error #3 in png lib");
    return bm;
  }
  
  png_init_io(png_ptr, fp);
  
  png_transforms=PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING;
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);
  
  channels=png_get_channels(png_ptr,info_ptr);

  if(channels<1 || channels>4) {
    LOGN_ERROR("error importing bitmap: " << filename << " has " << channels << " channels, excpected 1-4");
    return bm;
  }
  
  width=(unsigned int)png_get_image_width(png_ptr, info_ptr);
  height=(unsigned int)png_get_image_height(png_ptr, info_ptr);
  bpp=(unsigned int)png_get_bit_depth(png_ptr,info_ptr);

  boost::shared_array<unsigned char> data(new unsigned char[width*height*channels]);
  
  row_pointers = png_get_rows(png_ptr, info_ptr);
  
  for(row=0;row<height;++row) {
    for(column=0;column<width;++column) {
      for(chn=0;chn<channels;++chn) {
        data[(row*width+column)*channels+chn]=row_pointers[height-row-1][column*channels+chn];
      }
    }
  }

  LOGN_VERBOSE("loaded " << width << "x" << height << ":" << channels << " bitmap");

  bm.channels=channels;
  bm.width=width;
  bm.height=height;
  bm.data=data;

  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
  
  fclose(fp);
  return bm;
}

void BitmapExport(const String& fname, const String& ext, unsigned int width, unsigned int height,unsigned char* data)
{
  if(ext==".png") export_png(fname,width,height,data);
}

Bitmap BitmapImport(const String& fname, const String& ext)
{
  if(ext==".png") {
    return import_png(fname);
  }
  return Bitmap();
}

}} // ns
