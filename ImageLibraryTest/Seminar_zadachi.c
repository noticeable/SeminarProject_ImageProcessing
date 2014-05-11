#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <stdlib.h>

/*
//Sample for all exercises:

int i, j, k;
for (i = 0; i < Img_dst->Height; i++)
{
	for (j = 0; j < Img_dst->Width; j++)
	{
		for (k = 0; k < Img_dst->Num_channels; k++)
		{
			// access pixel element 
			Img_dst->rgbpix[Num_channels * (i*Img_dst->Width + j) + k] =  ....
		}
	}
}

*/
// Functions that may help:
/*
CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst);
CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace);
SetDestination(struct Image *Prototype, struct Image *Img_dst);
DestroyImage(struct Image *Img);
ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst);
ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst);
ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, int Threshold);
HistogramForImage(struct Histogram *hist, struct Image *Img_src, short NumberOfLayers);
ConvertHistToImage(struct Histogram *hist, struct Image *Img_src);

// Check CV_FuncPrototypes for all available functions
*/



/*	Exercise 1.
	I N V E R S E  image
*/
struct Image InverseImage0to255(struct Image *Img_src, struct Image *Img_dst)
{
	// access image pixels : Img_dst->rgbpix[]
	// Access image struct info : Img_dst->Width ..

}


/* ............................................................................. */
/*	Exercise 2
	Correct   B R I G H T N E S S
*/
struct Image BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, float Algo_paramBrightnessOrEV, int Algotype)
{
	
	return *Img_dst;
}

/*	Exercise 2
	Correct    C O N T R A S T 
*/
struct Image ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, float percentage)
{
	/* The percentage value should be between -100 and 100*/
	

	return *Img_dst;
}


/* ................................................................................................................ */
/*  Exercise 3.
	Correct    N O I S E 
*/
struct Image NoiseCorrection(struct Image *Img_src, struct Image *Img_dst)
{
	
}

/* .................................................................................................................. */
/*	Exercise 4.
	M I R R O R  image - horizontal
*/
struct Image MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst)
{
	
	return *Img_dst;
}

/*	Exercise 4.
	M I R R O R  image - vertical
*/
struct Image MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst)
{
	

	return *Img_dst;
}

/* .................................................................................................................. */
/*	Exercise 5.
	A F F I N E  -  Transformation: Rotation
*/
struct Image RotateImage(struct Image *Img_src, struct Image *Img_dst, float RotationAngle, struct point_xy CentralPoint)
{
	/* use cos() and sin() functions */
	/* example: 
		float angle = RotationAngle * 3.14 / 180.0;

	*/
	
	return *Img_dst;
}

/*
	A F F I N E  - scale (zoom) image - in/out
*/
struct Image ScaleImage(struct Image *Img_src, struct Image *Img_dst, float ScalePercentage)
{


	return *Img_dst;
}

/* .................................................................................................................. */
/*	Exercise 6.
	C R O P  image - around given point and given new dimensions
*/
struct Image CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight)
{
	/* Check dimensions. If necessery -> move the central point. NewWidth/ Height should be smaller than Img_src Width/Height*/
	/* Change Width/Height of the destination image */


	return *Img_dst;
}


/* .................................................................................................................. */
/*	Exercise 7.
	B L U R  image function - Gaussian
*/
struct Image BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, float NeighborCoefficient)
{

	
	return *Img_dst;
}


/* .................................................................................................................... */
/*  Exercise 8.
	M O R P H O L O G Y  -  Dilation
*/
struct Image MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	//you can use any struct element.. but for now.. try this one 3x3 ( ElementSize == 3)
	//float StructureElement[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	//Only Grayscale images, or do morph on each layer 
	// use ConvolutionBinary(). Example: ConvolutionBinary(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, &StructureElement, ElementSize, 0);
	
	
}

/*
	M O R P H O L O G Y  -  Erosion
*/
struct Image MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	// almost the same as Dilate...


}


/*
	M O R P H O L O G Y  -  Opening
*/
struct Image MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	//First Erode, then Dilate

	return *Img_dst;

}

/*
	M O R P H O L O G Y  -  Closing
*/
struct Image MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	// First Dilate, then Erode

	return *Img_dst;
}


/* .................................................................................................................... */
/*  Exercise 9.
	Correct     W H I T E  B A L A N C E  - RGB
*/
// Use ColorTemperature()
struct Image WhiteBalanceCorrectionRGB(struct Image *Img_src, struct Image *Img_dst, int Algotype)
{

}
void WhiteBalanceGREENY(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_XYZ)
{
}
void WhitebalanceCorrectionBLUEorRED(struct Image *src, struct Image *dst, struct WhitePoint WhitePoint_lab)
{
}