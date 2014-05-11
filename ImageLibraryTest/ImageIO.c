/********************************************************************************
*																				*
*	CV Library - ImageIO.c											*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Open / read image															*
*	- Write image																*
*	- Create new Image / with prototype											*
*	- Destroy Image																*
*	- White Balance - fill structure											*
*																				*
*																				*
*********************************************************************************/

#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix);
static int debug = 0;                /* = 1; prints every pixel */

/*
	O P E N   I M A G E
*/
struct Image ReadImage(char *FileName)
{
	FILE *f_ptr = NULL;
	struct Image Img_src;

#ifdef VS_LIBRARIES
	int isJpeg = 0;
	int Counter = 0;
	Img_src.isLoaded = 0;
	/* Open the file only with extension JPEG or JPG*/
#pragma warning (disable : 4996)
	f_ptr = fopen(FileName, "rb");
	if (f_ptr == NULL)
	{
		printf("Error opening file. Program will now close\n");
		_getch();
		return Img_src;
	}
	/* Check filename extension */
	for (Counter = 0; Counter < 255; Counter++)
	{
		if (FileName[Counter] == '.' && Counter < 253)
		{
			if (FileName[Counter + 1] == 'J' || FileName[Counter + 1] == 'j')
			{
				if (FileName[Counter + 2] == 'P' || FileName[Counter + 2] == 'p')
				{
					isJpeg = 1;
				}
			}
		}
	}
	if (isJpeg == 0)
	{
		printf("Only Jpeg file extensions are allowed\n");
		_getch();
		return Img_src;
	}

	Img_src = read_Image_file(f_ptr);
	Img_src.Image_FileName = FileName;
#endif
#ifdef QT_LIBRARIES

	QImage InputImage;
	InputImage.load(FileName);
	Img_src.rgbpix = InputImage.scanLine(0);
	Img_src.Width = InputImage.width();
	Img_src.Height = InoutImage.height();
	Img_src.NumChannels = InputImage.depth();

#endif // QT_LIBRARIES

	Img_src.isLoaded = 1;
	return Img_src;
}


/*
	W R I T E   I M A G E
*/
void WriteImage(char * filename, struct Image Img_src, int quality)//uint16 *image_buffer, int image_width, int image_height)
{
#ifdef VS_LIBRARIES
  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  //filename = "C:\\Users\\Petar\\Downloads\\pic.jpg";
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
	return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = Img_src.Width; 	/* image width and height, in pixels */
  cinfo.image_height = Img_src.Height;
  if (Img_src.Num_channels == 3)
  {
	  cinfo.input_components = 3;		/* # of color components per pixel */
	  /*Currently HSL and Lab are not supported so we will save it as RGB.*/
	  if (Img_src.ColorSpace == 2 || Img_src.ColorSpace == 5 || Img_src.ColorSpace == 4)
		  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	  else if (Img_src.ColorSpace == 3)
		  cinfo.in_color_space = JCS_YCbCr;
	  
	  row_stride = Img_src.Width * 3;
  }
  else if (Img_src.Num_channels == 1)
  {
	  cinfo.input_components = 1;		/* # of color components per pixel */
	  cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	  row_stride = Img_src.Width;
  }
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) 
  {
    row_pointer[0] = & Img_src.rgbpix[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);
#endif
#ifdef QT_LIBRARIES

  QImage Image_Output(Img_src->rgbpix,Img_src->Width, Img_src->Height);

  Image_Output.save(filename);

#endif
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
struct Image read_Image_file(FILE * infile)
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
  
  
  /* TO  DELETE */
  Img_src.ColorSpace = 2;
  Img_src.imageDepth = 8;
  /***************/
  
  
  Img_src.rgbpix = (unsigned char *)calloc(Img_src.Num_channels * Img_src.Width*Img_src.Height, sizeof(unsigned char));
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
  int i, k;
  
    //k = (height-line)*3*width;
	k = (line-1) * 3 * width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */

/*
	C R E A T E -  New Image - Based On Prototype
*/
struct Image CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst)
{
	Img_dst->ColorSpace = Prototype->ColorSpace;
	Img_dst->Height = Prototype->Height;
	Img_dst->Width = Prototype->Width;
	Img_dst->Num_channels = Prototype->Num_channels;
	Img_dst->ColorSpace = Prototype->ColorSpace;
	Img_dst->imageDepth = Prototype->imageDepth;

	Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL) 
	{
		printf("cannot allocate memory for the new image\n"); 
		Img_dst->isLoaded = 0;
	}
	else 
		Img_dst->isLoaded = 1;

	memcpy(Img_dst->rgbpix, Prototype->rgbpix, Prototype->Num_channels * Prototype->Width * Prototype->Height * sizeof(unsigned char));

	return *Img_dst;
}

/*
	C R E A T E -  New Image
*/
struct Image CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace, int Depth)
{
	FILE *fdebug = NULL;
	Img_dst->ColorSpace = ColorSpace;
	Img_dst->Height = Height;
	Img_dst->Width = Width;
	Img_dst->Num_channels = NumChannels;
	Img_dst->imageDepth = 8;

	/* If we have Binary or Grayscale image, NumChannels should be == 1*/
	if (ColorSpace < 2 && NumChannels != 1)
	{
		Img_dst->isLoaded = 0;
		return *Img_dst;
	}
	Img_dst->ColorSpace = ColorSpace;

	Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL)
	{

		#ifdef DEBUG_FILE
			fdebug = fopen(DEBUG_FILE, "wt");
			fprintf(fdebug,"cannot allocate memory for the new image\n");
			fclose(fdebug);
		#endif // DEBUG_FILE
		
		Img_dst->isLoaded = 0;
	}
	else
		Img_dst->isLoaded = 1;

	return *Img_dst;
}

/* 
	S E T   D E S T I N A T I O N  - to match the size of the prototype
*/
struct Image SetDestination(struct Image *Prototype, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	
	Img_dst->ColorSpace = Prototype->ColorSpace;
	Img_dst->Height = Prototype->Height;
	Img_dst->Width = Prototype->Width;
	Img_dst->imageDepth = Prototype->imageDepth;
	Img_dst->Num_channels = Prototype->Num_channels;

	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "cannot allocate memory for the new image\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		Img_dst->isLoaded = 0;
	}
	else
		Img_dst->isLoaded = 1;
}

/*
	D E S T R O Y  image
*/
void DestroyImage(struct Image *Img)
{
	free(Img->rgbpix);
}

/*
	L A Y E R S - Create number of layers for prototype image
*/
struct Image * CreateImageLayersBasedOnPrototype(struct Image *Prototype, int NumberofLayers)
{
	int i = 0;
	struct Image *ArrOfLayers = NULL;
	struct Image Layer = CreateNewImage_BasedOnPrototype(Prototype, &Layer);

	ArrOfLayers = (struct Image *)calloc(NumberofLayers, sizeof(Layer));

	for (i = 0; i < NumberofLayers; i++)
	{
		ArrOfLayers[i] = CreateNewImage_BasedOnPrototype(Prototype, &ArrOfLayers[i]);
	}
	return ArrOfLayers;
}

/*
	L A Y E R S - Merge
*/
struct Image CombineLayers(struct Image *Layers, struct Image *Img_dst, struct Image Mask)
{
	
	int i, j, k;
	if (Layers[0].Width != Img_dst->Width)
	{
		SetDestination(&Layers[0], Img_dst);
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (k = 0; k < Img_dst->Num_channels; k++)
			{
				/* we do not verify binary mask */
				Img_dst->rgbpix[Img_dst->Num_channels *(i * Img_dst->Width + j) + k] = Layers[Mask.rgbpix[i * Img_dst->Width + j]].rgbpix[Img_dst->Num_channels *(i * Img_dst->Width + j) + k];
			}
		}
	}

	return *Img_dst;
}

/*
	M A S K -  for Merging Layers
*/
struct Image  CreateMaskForLayers(struct Image *LayerPrototype, int MaskType, int NumberOfLayers, int algoParam1, int algoparam2)
{
	int i, j, k;
	struct Image Mask;
	int CurWidth, CurHeight = 0;
	if (MaskType < 1) MaskType = 1;

	Mask = CreateNewImage(&Mask, LayerPrototype->Width, LayerPrototype->Height, 1, 1, 8);

	if (MaskType == 1)
	{
		for (k = 0; k < NumberOfLayers; k++)
		{
			for (i = k * Mask.Height / NumberOfLayers; i < (k + 1) * Mask.Height / NumberOfLayers; i++)
			{
				for (j = 0; j < Mask.Width; j++)
				{
					Mask.rgbpix[i * Mask.Width + j] = k;
				}
			}
		}
	}
	if (MaskType == 2)
	{
		for (k = 0; k < NumberOfLayers; k++)
		{
			for (j = k * Mask.Width / NumberOfLayers; j < (k + 1) * Mask.Width / NumberOfLayers; j++)
			{
				for (i = 0; i < Mask.Height; i++)
				{
					Mask.rgbpix[i * Mask.Width + j] = k;
				}
			}
		}
	}

	return Mask;
}

/*
	 W H I T E   P O I N T - fill structure
*/
/* White Balance function and structure*/
void SetWhiteBalanceValues(struct WhitePoint *WhitePoint_lab, int TYPE)
{
	if (TYPE == 0)			// A         // 2856K  // Halogen
	{
		WhitePoint_lab->Temperature = 2856;
		WhitePoint_lab->X = 0.44757;
		WhitePoint_lab->Y = 0.40744;
		WhitePoint_lab->Z = 0.14499;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 1)		// F11       // 4000K  // Narrow-band Fluorescent
	{
		WhitePoint_lab->Temperature = 4000;
		WhitePoint_lab->X = 0.38054;
		WhitePoint_lab->Y = 0.37691;
		WhitePoint_lab->Z = 0.24254;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 2)		// F2        // 4200K   // Cool white Fluorescent
	{
		WhitePoint_lab->Temperature = 4200;
		WhitePoint_lab->X = 0.372;
		WhitePoint_lab->Y = 0.3751;
		WhitePoint_lab->Z = 0.2528;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 3)		// B         // 4874K  // Direct sunlight at noon - obsolete
	{
		WhitePoint_lab->Temperature = 4874;
		WhitePoint_lab->X = 0.44757;
		WhitePoint_lab->Y = 0.40744;
		WhitePoint_lab->Z = 0.14499;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 4)		// D50       // 5000K  // Daylight - for color rendering
	{
		WhitePoint_lab->Temperature = 5000;
		WhitePoint_lab->X = 0.34567;
		WhitePoint_lab->Y = 0.35850;
		WhitePoint_lab->Z = 0.29583;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 5)		// E		 // 5400K  // Uniform energy
	{
		WhitePoint_lab->Temperature = 5400;
		WhitePoint_lab->X = 0.333;
		WhitePoint_lab->Y = 0.333;
		WhitePoint_lab->Z = 0.333;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 6)		// D55       // 5500K  // Daylight - for photography
	{
		WhitePoint_lab->Temperature = 5500;
		WhitePoint_lab->X = 0.9642;//0.33242;
		WhitePoint_lab->Y = 1;// 0.34743;
		WhitePoint_lab->Z = 0.8252;// 0.32015;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 7)		// D65       //  6504K  // North Sky - Daylight(NewVersion)
	{						
		WhitePoint_lab->Temperature = 6504;
		WhitePoint_lab->X = 0.3127;		 
		WhitePoint_lab->Y = 0.329;
		WhitePoint_lab->Z = 0.3583;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}									 
	else if (TYPE == 8)		// C         //  6774K  // North Sky - Daylight
	{
		WhitePoint_lab->Temperature = 6774;
		WhitePoint_lab->X = 0.31006;
		WhitePoint_lab->Y = 0.31615;
		WhitePoint_lab->Z = 0.37379;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else if (TYPE == 9)		// D75	     //  7500K  // Daylight
	{					
		WhitePoint_lab->Temperature = 7500;
		WhitePoint_lab->X = 0.29902;     
		WhitePoint_lab->Y = 0.31485;     
		WhitePoint_lab->Z = 0.38613;  
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}								     
	else if (TYPE == 10)	// D93	     //  9300K  // High eff.Blue Phosphor monitors
	{
		WhitePoint_lab->Temperature = 9300;
		WhitePoint_lab->X = 0.2848;
		WhitePoint_lab->Y = 0.2932;
		WhitePoint_lab->Z = 0.422;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;
	}
	else 	// NonExisting - Like Uniform
	{
		WhitePoint_lab->Temperature = 5400;
		WhitePoint_lab->X = 0.333;
		WhitePoint_lab->Y = 0.333;
		WhitePoint_lab->Z = 0.333;
		WhitePoint_lab->u = 0;
		WhitePoint_lab->v = 0;

		//WhitePoint_lab->X = 1;
		//WhitePoint_lab->Y = 1;
		//WhitePoint_lab->Z = 1;
	}
}