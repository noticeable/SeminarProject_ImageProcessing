/********************************************************************************
*																				*
*	CV Library - Main file														*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	This library provide image processing algorithms.							*
*	It can be used in any projects for free.									*
*   If you find some errors, or if you want to contact me for any reason - 		*
*	please use my e-mail:														*
*						p.bijev@gmail.com										*
*																				*
*																				*
*																				*
*																				*
*********************************************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include "jpeglib.h"
#include <malloc.h>
#include "CV_Library.h"

int main()
{
	int i,j, Algotype, Threshold;
	float hui;
	FILE *LUT = NULL;

	/*OPEN*/
	Image Img_src = ReadImage("..\\InputImages\\noise.jpg");
	
	/*CREATE*/
	Image Img_srDst = CreateNewImage(&Img_srDst, Img_src.Width, Img_src.Height, Img_src.Num_channels, Img_src.ColorSpace);
	Image Img_srDst2 = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst2);
	Image Img_srDst3 = CreateNewImage_BasedOnPrototype(&Img_src, &Img_srDst3);
	Image Img_dst = CreateNewImage(&Img_dst, Img_src.Width, Img_src.Height, 1, 1);
	Image Img_dst2 = CreateNewImage(&Img_dst2, Img_src.Width, Img_src.Height, 1, 1);

	struct point_xy CentralPoint;
	
	struct Histogram hist;

	struct WhitePoint WhitePoint_lab1;
	struct WhitePoint WhitePoint_lab2;

	CentralPoint.X = Img_dst.Width / 2;
	CentralPoint.Y = Img_dst.Height / 2;

	// The color temperature of the imput image
	SetWhiteBalanceValues(&WhitePoint_lab1, 10);
	// The color temperature of the output image
	SetWhiteBalanceValues(&WhitePoint_lab2, 7);
	


	/*Exercise 1 - INVERT colors*/
	//InverseImage0to255(&Img_src, &Img_srDst);
	
	/*Exercise 2 - BRIGHTNESS & CONTRAST*/
	//BrightnessCorrection(&Img_src, &Img_srDst2, 15, 1);
	//ContrastCorrection(&Img_src, &Img_srDst2, 15);
	
	/*Exercise 3 - remove NOISE*/
	//NoiseCorrection(&Img_src, &Img_srDst);

	/*Exercise 4 - MIRROR*/
	//MirrorImageHorizontal(&Img_src, &Img_srDst);
	//MirrorImageVertical(&Img_src, &Img_srDst);

	/*Exercise 5 - AFFINE transforms - ROTATE & SCALE*/
	//RotateImage(&Img_src, &Img_srDst, 7, CentralPoint);
	//ScaleImage(&Img_src, &Img_srDst, -90);

	/*Exercise 6 - CROP*/
	//CropImage(&Img_src, &Img_srDst, CentralPoint,500,500 );

	/*Exercise 7 -BLUR - gaussian*/
	//BlurImageGussian(&Img_srDst, &Img_srDst2, 15, 0.7);

	/*Exercise 8 - MORPH operators*/
	//MorphDilate(&Img_dst, &Img_dst2, 3 , 7);
	//MorphErode(&Img_dst, &Img_dst2, 3, 1);

	/*Exercise 9 - WHITE BALANCE*/
	//WhiteBalanceCorrectionRGB(&Img_src, &Img_srDst2, 4);
	//WhiteBalanceGREENY(&Img_src, &Img_srDst2, WhitePoint_lab2);
	//WhitebalanceCorrectionBLUEorRED(&Img_src, &Img_srDst2, WhitePoint_lab2);

	/*
	O T H E R  functions
	*/

	/*GrayScale - result in 3 channels*/
	//ConvertToGrayscale_3Channels(&Img_src, &Img_dst);
	//ConvertToGrayscale_1Channel(&Img_src, &Img_dst);
	
	/*EDGE - Contour*/
	//EdgeExtraction(&Img_dst, &Img_dst2, EDGES_PREWITT , 1, 0.9);

	/*SHARP*/
	//SharpImageContours(&Img_src, &Img_srDst2, 60);

	/*BINARY  image*/
	//ConvertToBinary(&Img_src, &Img_dst, 0);
	
	/*RGB to HSL convert*/
	//ConvertImage_RGB_to_HSL(&Img_src, &Img_srDst);

	/*HSL to RGB convert*/
	//ConvertImage_HSL_to_RGB(&Img_srDst, &Img_srDst2);

	/*SATURATION*/
	//Saturation(&Img_src, &Img_srDst2, 50);

	/*BLEND image with another image */
	//BlendImage(&Img_src, &Img_src2, &Img_srDst, 50, BLEND_DONT_EXTRACT_EDGES, BLEND_REMOVE_WHITE, 20);
	
	/* RGB, XYZ, LAB convert */
	//ConvertImage_RGB_to_LAB(&Img_src, &Img_srDst, WhitePoint_lab1);

	//ConvertImage_LAB_to_RGB(&Img_srDst, &Img_srDst2, WhitePoint_lab2);

	/*Histogram */
	/* Create new Histpgram for a given Image */
	//HistogramForImage(&hist, &Img_srDst2, 3);
	//ConvertHistToImage(&hist, &Img_dst);

	/*WRITE*/
	WriteImage("..\\ResultImages\\Exercise5.jpg", Img_srDst, QUALITY_MAX);

	/* DESTROY images*/

	DestroyImage(&Img_src);
	//DestroyImage(&Img_dst);
	//DestroyImage(&Img_dst2);
	//DestroyImage(&Img_srDst);
	//DestroyImage(&Img_srDst2);
	//DestroyImage(&Img_srDst3);

	return 0;
}