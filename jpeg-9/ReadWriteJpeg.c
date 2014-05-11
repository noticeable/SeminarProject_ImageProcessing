#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include "C:\\Users\\petar.nikolov\\Documents\\Visual Studio 2013\\Projects\\ImageLibraryTest\\ImageLibraryTest\\CV_Library.h"

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix);
static int debug = 0;                /* = 1; prints every pixel */

/*
	W R I T E   I M A G E
*/
GLOBAL(void) write_JPEG_file (char * filename, int quality, struct Image Img_src)//uint16 *image_buffer, int image_width, int image_height)
{
  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);

  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = Img_src.Width; 	/* image width and height, in pixels */
  cinfo.image_height = Img_src.Height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = Img_src.Width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & Img_src.rgbpix[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);

}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  (*cinfo->err->output_message) (cinfo);

  longjmp(myerr->setjmp_buffer, 1);
}

/*
	R E A D   I M A G E
*/
struct Image read_JPEG_file ( FILE * infile)
{
  struct jpeg_decompress_struct cinfo;

  struct Image Img_src;
  struct my_error_mgr jerr;
 
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
	return Img_src;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);


  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;

  /* Fill the structure */
  Img_src.Width = cinfo.image_width;
  Img_src.Height = cinfo.image_height;
  Img_src.Num_channels = cinfo.num_components;
  Img_src.ColorSpace = cinfo.jpeg_color_space;

  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  while (cinfo.output_scanline < cinfo.output_height) 
  {

    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    if(debug)printf("cinfo.output_scanline=%d\n", cinfo.output_scanline);
    if(debug)printf("cinfo.output_height=%d\n", cinfo.output_height);
    if(debug)printf("row_stride=%d\n", row_stride);
    put_scanline(buffer[0], cinfo.output_scanline, cinfo.output_width,
                 cinfo.output_height, Img_src.rgbpix);
  }

  (void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  fclose(infile);

  return Img_src;
}

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix)
{
  int i, j, k;
  
    k = (height-line)*3*width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */
