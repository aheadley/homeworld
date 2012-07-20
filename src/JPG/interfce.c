//-----------------------------------------------------------------------------
// ---------------------
// File ....: interfce.c
// ---------------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: Interface to JPEG library
//
// History .: Oct, 27 1995 - Started
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interfce.h"
#include "jpeglib.h"
#include "jerror.h"
#include <setjmp.h>

//-----------------------------------------------------------------------------
// Error handling
//

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF void my_error_exit (j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  longjmp(myerr->setjmp_buffer, 1);
}

//-----------------------------------------------------------------------------
// *> Info()
//

void JpegInfo( JPEGDATA *data ) {

  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr           jerr;

  cinfo.err           = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    data->status = 1;
    return;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, data->input_file);

  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  data->width      = cinfo.output_width;
  data->height     = cinfo.output_height;
  data->components = cinfo.num_components;

  //(void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  data->status = 0;

}

//-----------------------------------------------------------------------------
// *> Write()
//

void JpegWrite( JPEGDATA *data ) {

     JSAMPROW row_pointer[1];
     int      row_stride;
     struct   jpeg_compress_struct cinfo;
     struct   jpeg_error_mgr       jerr;

     //-- Error Handling ------------------------------------------------------

     jmp_buf env;

     if (setjmp(env)) {

        //-- Define Error


        data->status = 1;
        jpeg_destroy_compress(&cinfo);
        return;

     }

     //-- Allocate and Initialize Jpeg Structures -----------------------------

     memset(&cinfo,0,sizeof(struct jpeg_compress_struct));
     memset(&jerr, 0,sizeof(struct jpeg_error_mgr));

     //-- Initialize the JPEG compression object with default error handling --

     cinfo.err       = jpeg_std_error(&jerr);
     jerr.error_exit = my_error_exit;

     jpeg_create_compress(&cinfo);

     //-- Specify data destination for compression ----------------------------

     jpeg_stdio_dest(&cinfo, data->output_file);

     //-- Initialize JPEG parameters ------------------------------------------

     cinfo.in_color_space   = JCS_RGB;
     cinfo.image_width      = data->width;
     cinfo.image_height     = data->height;
     cinfo.input_components = 3;

     jpeg_set_defaults(&cinfo);

     cinfo.data_precision   = 8;
     cinfo.arith_code       = data->aritcoding;
     cinfo.optimize_coding  = TRUE;
     cinfo.CCIR601_sampling = data->CCIR601sampling;
     cinfo.smoothing_factor = data->smoothingfactor;

     jpeg_set_quality(&cinfo, data->quality, TRUE);
     jpeg_default_colorspace(&cinfo);

     //-- Start compressor

     jpeg_start_compress(&cinfo, TRUE);

     //-- Process data

     row_stride = data->width * 3;

     while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &data->ptr[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        /*
        if (data->hWnd)
           SendMessage((HWND)data->hWnd,data->ProgressMsg,
                              cinfo.next_scanline,cinfo.image_height);
        */
     }

     //-- Finish compression and release memory

     jpeg_finish_compress(&cinfo);
     jpeg_destroy_compress(&cinfo);

     //-- Status

     data->status = 0;

}

//-----------------------------------------------------------------------------
// *> Read()
//

void JpegRead( JPEGDATA *data ) {

     struct jpeg_decompress_struct cinfo;
     struct my_error_mgr           jerr;

     ubyte     *bf;
     JSAMPARRAY buffer;
     int        row_stride,y;

     cinfo.err           = jpeg_std_error(&jerr.pub);
     jerr.pub.error_exit = my_error_exit;

     if (setjmp(jerr.setjmp_buffer)) {


        data->status = 1;
        jpeg_destroy_decompress(&cinfo);
        return;
     }

     jpeg_create_decompress(&cinfo);
     jpeg_stdio_src(&cinfo, data->input_file);

     (void) jpeg_read_header(&cinfo, TRUE);
     (void) jpeg_start_decompress(&cinfo);

     data->components = cinfo.num_components;

     row_stride = cinfo.output_width * cinfo.output_components;

     buffer = (*cinfo.mem->alloc_sarray)
     		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

     bf = data->ptr;
     y  = 0;

     while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(bf,buffer[0],row_stride);
        bf += row_stride;
        /*
        if (data->hWnd)
           SendMessage((HWND)data->hWnd,
                data->ProgressMsg,++y,cinfo.image_height);
        */
     }

     (void) jpeg_finish_decompress(&cinfo);
     jpeg_destroy_decompress(&cinfo);

     //-- Status

     data->status = 0;

}

//-- interfce.c ---------------------------------------------------------------
