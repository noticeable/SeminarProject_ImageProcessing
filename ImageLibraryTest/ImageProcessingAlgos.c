/********************************************************************************
*																				*
*	CV Library - ImageProcessingAlgos.c											*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Mirror image																*
*	- Crop																		*
*	- Morphology - opening/closing/dilatation/erotion							*
*	- Sharpening																*
*	- Atificial color															*
*	- BLur - over point or using mask											* 
*	- Brightness																*
*	- Contrast																	*
*	- WhiteBalance																*
*	- Noise																		*
*	- Gamma correction															*
*	- Affine transforms - scale / rotation / transaltion						*
*	- Edge extraction - Magnitude / Hysteresis / non-Max supp / follow edges	*
*	- Find Derivative															*
*	- Saturation																*
*	- .. and more..																*
*																				*
*																				*
*********************************************************************************/

#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <stdlib.h>


/*
	S H A R P   image - using EdgeExtraction function
*/
struct Image SharpImageContours(struct Image *Img_src, struct Image *Img_dst, float Percentage)
{
	int i, j, l;

	Image Img_dst_Grayscale = CreateNewImage(&Img_dst_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	Image Img_src_Grayscale = CreateNewImage(&Img_src_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	if (abs(Percentage) > 1) Percentage /= 100;
	Percentage *= -1;
	if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

	ConvertToGrayscale_1Channel(Img_src, &Img_src_Grayscale);

	EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= EDGE)//POSSIBLE_EDGE)
				{
					if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
					{	
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
					}
					else if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
					{
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
					}
					else
					{
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
					}
				}
				else
				{
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
					break;
				}
			}
		}
	}

	DestroyImage(&Img_dst_Grayscale);
	DestroyImage(&Img_src_Grayscale);

	return *Img_dst;
}

/*
	S H A R P   image - using Binary mask
*/
struct Image SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary, float Percentage)
{
	int i, j, l;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				if (Img_Binary->rgbpix[(i*Img_src->Width + j)] == 1)
				{
					if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
					else if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
					else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
				}
				else
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
			}
		}
	}

	return *Img_dst;
}

/*
	C O L O R  -  artificial
*/
struct Image ColorFromGray(struct Image *Img_src, struct Image *Img_dst, struct ColorPoint_RGB ColorPoint)
{
	int i, j, l;
	float R_to_G_Ratio = 0;
	float B_to_G_Ratio = 0;

	// The input should be 1 channel image. The output is 3 channel
	if (Img_src->Num_channels != 1 || Img_dst->Num_channels != 3)
	{
		return *Img_dst;
	}

	R_to_G_Ratio = ColorPoint.R / (float)ColorPoint.G;
	B_to_G_Ratio = ColorPoint.B / (float)ColorPoint.G;

	//step 1: copy the gray information to RGB channels
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			if (R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255) 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
			else 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[(i*Img_src->Width + j)];
			if (B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255) 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
			else 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
		}
	}
	//step 2: Gamma correction for B and R channels
	//GammaCorrection(Img_src, Img_dst, 0.6, 0.95, 0.7);

	return *Img_dst;
}


/*
	B L U R  image function - around point
*/
struct Image BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
{
	float MaxRatio = 0;
	float Distance = 0;
	float DistanceRatio = 0;
	float *matrix = (float *)calloc(Img_src->Width * Img_src->Height, sizeof(float));
	float Chislo = 0, Chislo2 = 0;
	int Sybiraemo = 0;
	int i, j, z, t, l;

	/* Only odd nubers are allowed (bigger than 5*/
	if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
	if (BlurPixelRadius < 5) BlurPixelRadius = 5;

	MaxRatio = (float)MAX(CentralPoint.X - ((float)SizeOfBlur * Img_dst->Width / 100), Img_dst->Width - CentralPoint.X + ((float)SizeOfBlur * Img_dst->Width / 100)) / ((float)SizeOfBlur * Img_dst->Width / 100);
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			//luma = ui->imageData[row * width + col];
			Distance = sqrt(pow(abs((float)CentralPoint.X - j), 2) + pow(abs((float)CentralPoint.Y - i), 2));
			if (Distance < ((float)SizeOfBlur * Img_dst->Width / 100))
			{
				matrix[i * Img_dst->Width + j] = 1;
			}
			else
			{
				DistanceRatio = Distance / ((float)SizeOfBlur * Img_dst->Width / 100);
				matrix[i * Img_dst->Width + j] = 1 - ((float)BlurAgression / 100 * (DistanceRatio / MaxRatio));
				if (matrix[i * Img_dst->Width + j] < 0) matrix[i * Img_dst->Width + j] = 0;
			}
		}
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{

			if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
			{
				for (l = 0; l < Img_dst->Num_channels; l++)
				{
					Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
				}
				continue;
			}
			for (l = 0; l < 3; l++)
			{
				if (Img_src->rgbpix[3 * (i*Img_dst->Width + j) + l] > 255)
					Chislo = 0;

				Sybiraemo = 0;
				if (BlurOrSharp == 0)
					Chislo2 = ((float)(matrix[i * Img_dst->Width + j]) / (pow((float)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				else
					Chislo2 = ((float)(1 - matrix[i * Img_dst->Width + j]) / (pow((float)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				for (z = 0; z < BlurPixelRadius / 2; z++)
				{
					for (t = 0; t < BlurPixelRadius / 2; t++)
					{
						if (z == 0 && t == 0) continue;
						Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j + t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j + t) + l];
					}
				}

				Chislo2 *= Sybiraemo;
				Chislo = 0;
				if (BlurOrSharp == 0)
					Chislo = (1 - matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
				else
					Chislo = (matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
				if (Chislo > 255)
					Chislo = 255;
				if (Chislo < 0)
					Chislo = 0;
				Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = Chislo;
			}
		}
	}

	return *Img_dst;
}


/*
	A F F I N E  - Translation
*/
struct Image TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint)
{
	int i, j, z;
	int NewX = 0, NewY = 0;
	
	int ShiftX = ToPoint.X - Img_dst->Width / 2;
	int ShiftY = ToPoint.Y - Img_dst->Height / 2;
	

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			NewX = j + ShiftX;
			NewY = i + ShiftY;
			if (ShiftX < 0 ) if(NewX < 0) continue;
			if (ShiftX > 0)if (NewX >= Img_dst->Width) continue;
			if (ShiftY < 0) if(NewY < 0) continue;
			if (ShiftY > 0)if (NewY >= Img_dst->Height) continue;
			
			for(z = 0; z < Img_dst->Num_channels; z++)
			{
				Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z];
			}
		}
	}
	return *Img_dst;
}

/*
	E D G E   Extraction
*/
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2)
{
	int i, j, z, l;
	int NewX = 0, NewY = 0;
	struct ArrPoints ArrPts;
	struct Image DerrivativeX = CreateNewImage(&DerrivativeX, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image DerrivativeY = CreateNewImage(&DerrivativeY, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude = CreateNewImage(&Magnitude, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude2 = CreateNewImage(&Magnitude2, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude3 = CreateNewImage(&Magnitude3, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image NMS = CreateNewImage(&NMS, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Hysteresis = CreateNewImage(&Hysteresis, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	float Gx[] = 
	  { -1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 };
	float Gy[] = 
	  { 1, 2, 1,
		0, 0, 0,
	   -1, -2, -1 };

	float HighPass[] = 
	{ 1 / 9 * (
	    -1, -1, -1,
		-1,  8, -1,
		-1, -1, -1) };

	float Laplace[] = 
	 {  0,  1,  0,
		1, -4,  1,
		0,  1,  0 };

	float Prewitt_X_1[] = 
	 { -5, -5, -5,
		0,  0,  0,
		5,  5,  5 };
	float Prewitt_Y_1[] =
	 { -5,  0,  5,
	   -5,  0,  5,
	   -5,  0,  5 };

	float Prewitt_X_2[] = 
	 { 5,  5,  5,
	   0,  0,  0,
	  -5, -5, -5  };
	float Prewitt_Y_2[] = 
	 { 5,  0, -5,
	   5,  0, -5,
	   5,  0, -5 };

	float Sobel_X_1[] =
	{ -1, -2, -1,
	   0,  0,  0,
	   1,  2,  1 };
	float Sobel_Y_1[] =
	{ -1,  0,  1,
	  -2,  0,  2,
	  -1,  0,  1 };

	float Sobel_X_2[] =
	{  1,  2,  1,
	   0,  0,  0,
	  -1, -2, -1 };
	float Sobel_Y_2[] =
	{  1,  0, -1,
	   2,  0, -2,
	   1,  0, -1 };

	Img_dst->Width = Img_src->Width;
	Img_dst->Height = Img_src->Height;
	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Num_channels * Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
	ArrPts.ArrayOfPoints = (struct point_xy *)calloc(50,sizeof(struct point_xy));

	

	if (Algotype < 1 || Algotype > 3)
	{
		printf("Non existing Algo for Edge extraction\n");
		return ArrPts;
	}
	/* Canny */
	if (Algotype == 1)
	{
		// Step 1: Perfrom Gaussian Blur
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		FindDerrivative_XY(Img_dst, &DerrivativeX, &DerrivativeY);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);
		FindNonMaximumSupp(&Magnitude, &DerrivativeX, &DerrivativeY, &NMS);
		
		FindHysteresis(&Magnitude, &NMS, &Hysteresis, Algo_param1, Algo_param2);
		memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
	}
	/* Sobel */
	else if (Algotype == 2)
	{
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_1, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_1, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_2, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_2, 3);
		if (Algo_param1 == 0)
		{
			FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
			memcpy(Img_dst->rgbpix, Magnitude2.rgbpix, Magnitude2.Width* Magnitude2.Height * sizeof(unsigned char));
		}
		else
		{
			memcpy(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Width* DerrivativeX.Height * sizeof(unsigned char));
		}
	}
	/* Prewitt */
	else if (Algotype == 3)
	{
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_1, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_1, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_2, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_2, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
		FindMagnitudeOfGradient(&Magnitude, &Magnitude2, &Magnitude3);
		if (Algo_param1 == 0)
		{
			FindNonMaximumSupp(&Magnitude3, &Magnitude, &Magnitude2, &NMS);
			FindHysteresis(&Magnitude3, &NMS, &Hysteresis, Algo_param1, Algo_param2);

			memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
		}
		else
			memcpy(Img_dst->rgbpix, Magnitude3.rgbpix, Magnitude3.Width* Magnitude3.Height * sizeof(unsigned char));
	}

	//DestroyImage(&Hysteresis);
	DestroyImage(&DerrivativeX);
	DestroyImage(&DerrivativeY);
	DestroyImage(&Magnitude);
	DestroyImage(&Magnitude2);
	DestroyImage(&Magnitude3);
	DestroyImage(&NMS);
	
	return ArrPts;
}

/* 
	D E R R I V A T I V E    calculation
*/
void FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image)
{
	int r, c, pos;
	int rows = Img_src->Height;
	int cols = Img_src->Width;
	
	/* Calculate X - derrivative image */
	for (r = 0; r < rows-1; r++)
	{
		pos = r * cols;
		//DerrivativeX_image->rgbpix[pos] = Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos];
		//pos++;
		for (c = 0; c < (cols - 1); c++, pos++)
		{
			DerrivativeX_image->rgbpix[pos] = abs((Img_src->rgbpix[pos] - Img_src->rgbpix[pos + cols + 1]));
		}
		DerrivativeX_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - 1]);
	}
	

	/* Calculate Y - derrivative image */
	for (c = 0; c < cols-1; c++)
	{
		pos = c;
		//DerrivativeY_image->rgbpix[pos] = Img_src->rgbpix[pos + cols] - Img_src->rgbpix[pos];
		//pos += cols;
		for (r = 0; r < (rows - 1); r++, pos += cols)
		{
			DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos + cols]);
		}
		DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - cols]);
	}
}

/*
	Find   M A G N I T U D E  of Gradient - working with images
*/
void FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude)
{
	int r, c, pos, sq1, sq2;
	int rows = DerrivativeX_image->Height;
	int cols = DerrivativeX_image->Width;

	for (r = 0, pos = 0; r < rows; r++)
	{
		for (c = 0; c < cols; c++, pos++)
		{
			sq1 = DerrivativeX_image->rgbpix[pos] * DerrivativeX_image->rgbpix[pos];
			sq2 = DerrivativeY_image->rgbpix[pos] * DerrivativeY_image->rgbpix[pos];
			Magnitude->rgbpix[pos] = (int)(0.5 + sqrt((float)sq1 + (float)sq2));
		}
	}
}

/*
	Find   P H A S E
*/
void FindPhase_Arrays(long double *real_arr, long double *imag_arr, long double *Phase, int dimensionX, int dimensionY)
{
	int r, c;
	long int pos;
	long double sq1, sq2;
	int rows = dimensionY;
	int cols = dimensionX;

	for (r = 0, pos = 0; r < rows; r++)
	{
		for (c = 0; c < cols; c++, pos++)
		{
			Phase[pos] = atan2(imag_arr[pos], real_arr[pos]);
		}
	}
}

/*
	Find   M A G N I T U D E  of Gradient - working With arrays
*/
void FindMagnitudeOfGradient_Arrays(long double *array_1, long double *array_2, long double *Magnitude, int dimensionX, int dimensionY, int *Min, int *Max)
{
	int r, c;
	long int pos;
	long double sq1, sq2;
	int rows = dimensionY;
	int cols = dimensionX;

	for (r = 0, pos = 0; r < rows; r++)
	{
		for (c = 0; c < cols; c++, pos++)
		{
			sq1 = array_1[pos] * array_1[pos];
			sq2 = array_2[pos] * array_2[pos];
			Magnitude[pos] = (int)(0.5 + sqrt((float)sq1 + (float)sq2));
			if (Magnitude[pos] < *Min) *Min = Magnitude[pos];
			else if (Magnitude[pos] > *Max) *Max = Magnitude[pos];
		}
	}
}
/*
	Find  N O N - M A X - S U P P
*/
void FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS)
{
	int rowcount, colcount, count;
	unsigned char *magrowptr, *magptr;
	unsigned char *gxrowptr, *gxptr;
	unsigned char *gyrowptr, *gyptr, z1, z2;
	unsigned char m00, gx = 0, gy = 0;
	float mag1 = 0, mag2 = 0, xperp = 0, yperp = 0;
	unsigned char *resultrowptr, *resultptr;
	int nrows = DerrivativeX->Height;
	int ncols = DerrivativeX->Width;

	unsigned char *result = NMS->rgbpix;
	unsigned char *mag = Magnitude->rgbpix;
	unsigned char *gradx = DerrivativeX->rgbpix;
	unsigned char *grady = DerrivativeY->rgbpix;


	/****************************************************************************
	* Zero the edges of the result image.
	****************************************************************************/
	for (count = 0, resultrowptr = NMS->rgbpix, resultptr = NMS->rgbpix + ncols*(nrows - 1);
		count<ncols; resultptr++, resultrowptr++, count++){
		*resultrowptr = *resultptr = (unsigned char)0;
	}

	for (count = 0, resultptr = NMS->rgbpix, resultrowptr = NMS->rgbpix + ncols - 1;
		count<nrows; count++, resultptr += ncols, resultrowptr += ncols){
		*resultptr = *resultrowptr = (unsigned char)0;
	}

	/****************************************************************************
	* Suppress non-maximum points.
	****************************************************************************/
	for (rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr = gradx + ncols + 1,
		gyrowptr = grady + ncols + 1, resultrowptr = result + ncols + 1;
		rowcount<nrows - 2; rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr += ncols,
		resultrowptr += ncols)
	{
		for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr,
			resultptr = resultrowptr; colcount<ncols - 2;
			colcount++, magptr++, gxptr++, gyptr++, resultptr++)
		{
			m00 = *magptr;
			if (m00 == 0){
				*resultptr = (unsigned char)NOEDGE;
			}
			else{
				xperp = -(gx = *gxptr) / ((float)m00);
				yperp = (gy = *gyptr) / ((float)m00);
			}

			if (gx >= 0){
				if (gy >= 0){
					if (gx >= gy)
					{
						/* 111 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr - ncols - 1);

						mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr + ncols + 1);

						mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 110 */
						/* Left point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols - 1);

						mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols + 1);

						mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (gx >= -gy)
					{
						/* 101 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr + ncols - 1);

						mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr - ncols + 1);

						mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 100 */
						/* Left point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols - 1);

						mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols + 1);

						mag2 = (z1 - z2)*xperp + (m00 - z1)*yperp;
					}
				}
			}
			else
			{
				if ((gy = *gyptr) >= 0)
				{
					if (-gx >= gy)
					{
						/* 011 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr - ncols + 1);

						mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr + ncols - 1);

						mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 010 */
						/* Left point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols + 1);

						mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols - 1);

						mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (-gx > -gy)
					{
						/* 001 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr + ncols + 1);

						mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr - ncols - 1);

						mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 000 */
						/* Left point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols + 1);

						mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols - 1);

						mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
					}
				}
			}

			/* Now determine if the current point is a maximum point */

			if ((mag1 > 0.0) || (mag2 > 0.0))
			{
				*resultptr = (unsigned char)NOEDGE;
			}
			else
			{
				if (mag2 == 0.0)
					*resultptr = (unsigned char)NOEDGE;
				else
					*resultptr = (unsigned char)POSSIBLE_EDGE;
			}
		}
	}
}

/*
	Find     H Y S T E R E S I S
*/
void FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2)
{
	int r, c, pos, edges, highcount, lowthreshold, highthreshold,
		i, hist[32768], rr, cc;
	unsigned char maximum_mag, sumpix;

	int rows = Img_dst->Height;
	int cols = Img_dst->Width;

	/****************************************************************************
	* Initialize the Img_dst->rgbpix map to possible Img_dst->rgbpixs everywhere the non-maximal
	* suppression suggested there could be an Img_dst->rgbpix except for the border. At
	* the border we say there can not be an Img_dst->rgbpix because it makes the
	* follow_Img_dst->rgbpixs algorithm more efficient to not worry about tracking an
	* Img_dst->rgbpix off the side of the image.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++){
		for (c = 0; c<cols; c++, pos++){
			if (NMS->rgbpix[pos] == POSSIBLE_EDGE) Img_dst->rgbpix[pos] = POSSIBLE_EDGE;
			else Img_dst->rgbpix[pos] = NOEDGE;
		}
	}

	for (r = 0, pos = 0; r<rows; r++, pos += cols){
		Img_dst->rgbpix[pos] = NOEDGE;
		Img_dst->rgbpix[pos + cols - 1] = NOEDGE;
	}
	pos = (rows - 1) * cols;
	for (c = 0; c<cols; c++, pos++){
		Img_dst->rgbpix[c] = NOEDGE;
		Img_dst->rgbpix[pos] = NOEDGE;
	}

	/****************************************************************************
	* Compute the histogram of the magnitude image. Then use the histogram to
	* compute hysteresis thresholds.
	****************************************************************************/
	for (r = 0; r<32768; r++) hist[r] = 0;
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++)
		{
			if (Img_dst->rgbpix[pos] == POSSIBLE_EDGE) hist[Magnitude->rgbpix[pos]]++;
		}
	}

	/****************************************************************************
	* Compute the number of pixels that passed the nonmaximal suppression.
	****************************************************************************/
	for (r = 1, edges = 0; r<32768; r++)
	{
		if (hist[r] != 0) maximum_mag = r;
		edges += hist[r];
	}

	highcount = (int)(edges * Algo_param2 + 0.5);

	/****************************************************************************
	* Compute the high threshold value as the (100 * Algo_param2) percentage point
	* in the magnitude of the gradient histogram of all the pixels that passes
	* non-maximal suppression. Then calculate the low threshold as a fraction
	* of the computed high threshold value. John Canny said in his paper
	* "A Computational Approach to Img_dst->rgbpix Detection" that "The ratio of the
	* high to low threshold in the implementation is in the range two or three
	* to one." That means that in terms of this implementation, we should
	* choose Algo_param1 ~= 0.5 or 0.33333.
	****************************************************************************/
	r = 1;
	edges = hist[1];
	while ((r<(maximum_mag - 1)) && (edges < highcount))
	{
		r++;
		edges += hist[r];
	}
	highthreshold = r;
	lowthreshold = (int)(highthreshold * Algo_param1 + 0.5);


	/****************************************************************************
	* This loop looks for pixels above the highthreshold to locate Img_dst->rgbpixs and
	* then calls follow_Img_dst->rgbpixs to continue the Img_dst->rgbpix.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++)
		{
			if ((Img_dst->rgbpix[pos] == POSSIBLE_EDGE) && (Magnitude->rgbpix[pos] >= highthreshold)){
				Img_dst->rgbpix[pos] = EDGE;
				Follow_edges((Img_dst->rgbpix + pos), (Magnitude->rgbpix + pos), lowthreshold, cols);
			}
		}
	}

	/****************************************************************************
	* Set all the remaining possible Img_dst->rgbpixs to non-Img_dst->rgbpixs.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++) if (Img_dst->rgbpix[pos] != EDGE) Img_dst->rgbpix[pos] = NOEDGE;
	}
}

/*
	Follow  E D G E S
*/
void Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols)
{
	unsigned char *tempmagptr;
	unsigned char *tempmapptr;
	int i;
	float thethresh;
	int x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 },
		y[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	for (i = 0; i<8; i++){
		tempmapptr = edgemapptr - y[i] * cols + x[i];
		tempmagptr = edgemagptr - y[i] * cols + x[i];

		if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)){
			*tempmapptr = (unsigned char)EDGE;
			Follow_edges(tempmapptr, tempmagptr, lowval, cols);
		}
	}
}

/*
	S A T U R A T I O N 
*/
struct Image Saturation(struct Image *Img_src, struct Image *Img_dst, float percentage)
{
	FILE * fdebug = NULL;
	int i, j;
	struct Image WorkCopy = CreateNewImage(&WorkCopy, Img_src->Width, Img_src->Height, 3, Img_src->ColorSpace);

	/* If the input is RGB -> the output is also RGB. if the input is HSL -> the output is also HSL */
	if (Img_src->ColorSpace != 2 && Img_src->ColorSpace != 5)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in HSL format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if ((Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height) || (Img_src->ColorSpace != Img_dst->ColorSpace))
	{
		SetDestination(Img_src, Img_dst);
	}
	
	/* We have to work in HSL color space */
	if (Img_src->ColorSpace == 5)  // if the input image is HSL
	{
		memcpy(WorkCopy.rgbpix, Img_src->rgbpix, 3 * Img_src->Width * Img_src->Height * sizeof(unsigned char));
	}
	else // if the input image is RGB
	{
		ConvertImage_RGB_to_HSL(Img_src, &WorkCopy);
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			// We work over WorkCopy
			if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] > 100)
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 100;
			else if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] < 0)
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 0;
			else
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = RoundValue_toX_SignificantBits((WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100) + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1], 2);
		}
	}

	/* We have to return Image with the same color space as the input */
	if (Img_src->ColorSpace == 5)  // if the input image is HSL
	{			
		memcpy(Img_dst->rgbpix, WorkCopy.rgbpix, 3 * WorkCopy.Width * WorkCopy.Height * sizeof(unsigned char));
		Img_dst->ColorSpace = WorkCopy.ColorSpace;
		//DestroyImage(&WorkCopy);
		return WorkCopy;
	}
	else // if the input image is RGB
	{
		ConvertImage_HSL_to_RGB(&WorkCopy, Img_dst);
		DestroyImage(&WorkCopy);
		return *Img_dst;
	}
}

/*
	B L E N D I N G  - similar to image sharpening - but the contours are from another image
*/
struct Image BlendImage(struct Image *Img_src, struct Image *Img_BlendedSrc, struct Image *Img_dst, float Percentage, int AlgoParam1, int Algoparam2, int BlacOrWhiteThreshold)
{
	int i, j, l;

	Image Img_dst_Grayscale = CreateNewImage(&Img_dst_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	Image Img_src_Grayscale = CreateNewImage(&Img_src_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	if (abs(Percentage) > 1) Percentage /= 100;
	//Percentage *= -1;
	if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

	if (AlgoParam1 == BLEND_EXTRACT_EDGES)
	{
		ConvertToGrayscale_1Channel(Img_BlendedSrc, &Img_src_Grayscale);
		EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);
	}
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				if (AlgoParam1 == BLEND_EXTRACT_EDGES)
				{
					if ((Algoparam2 == BLEND_REMOVE_BLACK && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] <= BlacOrWhiteThreshold))
						||
						(Algoparam2 == BLEND_REMOVE_WHITE && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] >= 255 - BlacOrWhiteThreshold)))
						goto Same;

					if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= 15)
					{
						if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
						}
						else if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage)* Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
						}
						else
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = (Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
						}
					}
					else
					{
Same:					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
					}
				}
				else
				{
					if (!((Algoparam2 == BLEND_REMOVE_BLACK && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] <= BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] <= BlacOrWhiteThreshold))
						||
						(Algoparam2 == BLEND_REMOVE_WHITE && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 0] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 1] >= 255 - BlacOrWhiteThreshold) && (Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + 2] >= 255 - BlacOrWhiteThreshold))))
					{
						if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
						}
						else if ((Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage)* Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
						}
						else
						{
							Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = (Percentage)* Img_BlendedSrc->rgbpix[3 * (i*Img_src->Width + j) + l] + (1 - Percentage) * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
						}
					}
					else
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
				}
			}
		}
	}

	DestroyImage(&Img_dst_Grayscale);
	DestroyImage(&Img_src_Grayscale);

	return Img_dst_Grayscale;
}



/*
	Create   H I S T O G R A M
*/
void HistogramForImage(struct Histogram *hist, struct Image *Img_src, short NumberOfLayers)
{
	//FILE * fp;
	int i, j, z;
	long int maxValue = 0;
	int curValue = 0;

	struct Image grayscaledImage = CreateNewImage(&grayscaledImage, Img_src->Width, Img_src->Height, 1, 1);
	hist->Bins = pow(2, Img_src->imageDepth);
	
	hist->NumberOfLayers = NumberOfLayers;

	if (NumberOfLayers == 1)
	{
		/*if the image depth is 8bit -> we will have a histogram for 256 values*/
		hist->values = (long int *)calloc(hist->Bins, sizeof(long int));

		/* check if the input image is RGB or grayscaled. If it is RGB, convert to grayscaled */
		if (Img_src->Num_channels == 1)
		{
			memcpy(grayscaledImage.rgbpix, Img_src->rgbpix, Img_src->Width* Img_src->Height * sizeof(unsigned char));
		}
		else
		{
			ConvertToGrayscale_1Channel(Img_src, &grayscaledImage);
		}

		/* Work with grayscaledImage */
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]]++;
				if (hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]] > maxValue)
					maxValue = hist->values[grayscaledImage.rgbpix[i * Img_src->Width + j]];
			}
		}
	}
	else
	{
		if (Img_src->Num_channels != 3) return;

		/*if the image depth is 8bit -> we will have a histogram for 256 * 3 values*/
		hist->values = (long int *)calloc(3 *hist->Bins, sizeof(long int));
		
		/* Work with colored imput image */
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				for (z = 0; z < 3; z++)
				{
					(hist->values[3 *(Img_src->rgbpix[3* (i * Img_src->Width + j) +z]) + z])++;
					curValue = hist->values[3 *(Img_src->rgbpix[3* (i * Img_src->Width + j) +z]) + z];
					
					if (curValue > maxValue)
						maxValue = curValue;
				}
			}
		}
	}
	/*
	fopen_s(&fp, "blqk.txt", "wt");
	for (i = 0; i < 256; i++)
	{
		fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 0]));
	}
	fprintf(fp, "\n\n");
	for (i = 0; i < 256; i++)
	{
		fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 1]));
	}

	fprintf(fp, "\n\n");
	for (i = 0; i < 256; i++)
	{
		fprintf(fp, "%ld: %ld\n", i, (hist->values[3 * i + 2]));
	}

	fclose(fp);
	*/
	//if(NumberOfLayers == 3) maxValue /= 3;
	hist->MaxValue = maxValue;
}

/*
	Convert  H I S TO G R A M   to  I M A G E
*/
void ConvertHistToImage(struct Histogram *hist, struct Image *Img_src)
{
	int i, j, k, z;
	int write = 1;
	float ScaleNumber = 0;
	long int MaxValue = hist->MaxValue;
	int Char_array[81];
	int NumberofCalcs = 0;
	int average;

	/* Set size for the Hist Image */
	if (hist->Bins == 256)
	{
		hist->Size_x = 2 * hist->Bins + 30;
		if (MaxValue > 600) // we have to scale if it is too big
		{
			ScaleNumber = MaxValue / 600.0;
			hist->Size_y = ((MaxValue / ScaleNumber) + 40);
		}
		else {
			hist->Size_y = MaxValue + 40;
			ScaleNumber = ((MaxValue + 40) / 600.0);
		}
	}

	Img_src->Num_channels = hist->NumberOfLayers;
	if (hist->NumberOfLayers == 3) Img_src->ColorSpace = 2;
	else Img_src->ColorSpace = 1;

	Img_src->rgbpix = (unsigned char *)realloc(Img_src->rgbpix, hist->NumberOfLayers * hist->Size_y * hist->Size_x * sizeof(unsigned char));
	Img_src->Width = hist->Size_x;
	Img_src->Height = hist->Size_y;

	for (j = 0; j < hist->Size_x; j++)
	{
		for (i = hist->Size_y - 1;  i >= 0; i--)
		{
			for (k = 0; k < hist->NumberOfLayers; k++)
			{
				//if (Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] != 255) 
				Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;

				if (i == 0 || j == 0 || j == hist->Size_x - 1 || i == hist->Size_y - 1)
				{
					Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
					continue;
				}

				if (j % 2 == 0)
				{
					if ((j <= 2 * hist->Bins + 20) && j > 21) // ako e chetno
					{
						if (i > (hist->Size_y - 20 - (hist->values[hist->NumberOfLayers * ((j - 20) / 2) + k] / ScaleNumber)) && (i < hist->Size_y - 20))
						{
							Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
						}
					}
				}
				else
				{
					if ((j < 2 * hist->Bins + 20) && j > 21)
					{
						average = ((hist->values[hist->NumberOfLayers *((j - 21) / 2) + k] / ScaleNumber) + (hist->values[hist->NumberOfLayers * ((j - 19) / 2) + k] / ScaleNumber)) / 2;
						if (i >(hist->Size_y - 20 - (average)) && (i < hist->Size_y - 20))
						{
							Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 255;
						}
					}
					//Img_src->rgbpix[hist->NumberOfLayers * (i * hist->Size_x + j) + k] = 0;
				}
			}
		}
	}

	
	for (k = MaxValue; k > 0; k--)
	{
		for (i = 0; i < 81; i++)
		{
			Char_array[i] = 0;
		}
		NumberofCalcs++;
		write = k / 10;
		write = k - (write * 10);
		k =k /10;
		if (write == 0)
		{
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[12] = 255;
			Char_array[15] = 255;
			Char_array[20] = 255;
			Char_array[25] = 255;
			Char_array[28] = 255;
			Char_array[35] = 255;
			Char_array[37] = 255;
			Char_array[44] = 255;
			Char_array[46] = 255;
			Char_array[53] = 255;
			Char_array[56] = 255;
			Char_array[61] = 255;
			Char_array[66] = 255;
			Char_array[69] = 255;
			Char_array[76] = 255;
			Char_array[77] = 255;
		}
		else if(write == 1)
		{
			Char_array[20] = 255;
			Char_array[12] = 255;
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[13] = 255;
			Char_array[14] = 255;
			Char_array[22] = 255;
			Char_array[23] = 255;
			Char_array[31] = 255;
			Char_array[32] = 255;
			Char_array[40] = 255;
			Char_array[41] = 255;
			Char_array[49] = 255;
			Char_array[50] = 255;
			Char_array[58] = 255;
			Char_array[59] = 255;
			Char_array[67] = 255;
			Char_array[68] = 255;
			Char_array[69] = 255;
			Char_array[66] = 255;
		}
		else if (write == 2)
		{
			Char_array[10] = 255;
			Char_array[2] = 255;
			Char_array[3] = 255;
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[6] = 255;
			Char_array[15] = 255;
			Char_array[24] = 255;
			Char_array[32] = 255;
			Char_array[40] = 255;
			Char_array[48] = 255;
			Char_array[56] = 255;
			Char_array[64] = 255;
			Char_array[65] = 255;
			Char_array[66] = 255;
			Char_array[67] = 255;
			Char_array[68] = 255;
			Char_array[69] = 255;
		}
		else if (write == 3)
		{
			Char_array[11] = 255;
			Char_array[3]  = 255;
			Char_array[4]  = 255;
			Char_array[14] = 255;
			Char_array[24] = 255;
			Char_array[32] = 255;
			Char_array[40] = 255;
			Char_array[50] = 255;
			Char_array[60] = 255;
			Char_array[68] = 255;
			Char_array[76] = 255;
			Char_array[75] = 255;
			Char_array[65] = 255;
		}
		else if (write == 4)
		{
			Char_array[5] = 255;
			Char_array[13] = 255;
			Char_array[21] = 255;
			Char_array[29] = 255;
			Char_array[37] = 255;
			Char_array[38] = 255;
			Char_array[39] = 255;
			Char_array[40] = 255;
			Char_array[41] = 255;
			Char_array[42] = 255;
			Char_array[43] = 255;
			Char_array[34] = 255;
			Char_array[25] = 255;
			Char_array[16] = 255;
			Char_array[52] = 255;
			Char_array[61] = 255;
			Char_array[70] = 255;
			Char_array[79] = 255;
		}
		else if (write == 5)
		{
			Char_array[10] = 255;
			Char_array[2] = 255;
			Char_array[3] = 255;
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[6] = 255;
			Char_array[19] = 255;
			Char_array[28] = 255;
			Char_array[37] = 255;
			Char_array[38] = 255;
			Char_array[39] = 255;
			Char_array[40] = 255;
			Char_array[41] = 255;
			Char_array[42] = 255;
			Char_array[43] = 255;
			Char_array[52] = 255;
			Char_array[61] = 255;
			Char_array[70] = 255;
			Char_array[69] = 255;
			Char_array[68] = 255;
			Char_array[67] = 255;
			Char_array[66] = 255;
			Char_array[65] = 255;
			Char_array[64] = 255;
		}
		else if (write == 6)
		{
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[12] = 255;
			Char_array[15] = 255;
			Char_array[20] = 255;
			Char_array[28] = 255;
			Char_array[37] = 255;
			Char_array[46] = 255;
			Char_array[56] = 255;
			Char_array[66] = 255;
			Char_array[76] = 255;
			Char_array[77] = 255;
			Char_array[69] = 255;
			Char_array[61] = 255;
			Char_array[51] = 255;
			Char_array[41] = 255;
			Char_array[40] = 255;
			Char_array[48] = 255;
		}
		else if (write == 7)
		{
			Char_array[2] = 255;
			Char_array[3] = 255;
			Char_array[4] = 255;
			Char_array[5] = 255;
			Char_array[6] = 255;
			Char_array[7] = 255;
			Char_array[16] = 255;
			Char_array[24] = 255;
			Char_array[33] = 255;
			Char_array[41] = 255;
			Char_array[50] = 255;
			Char_array[58] = 255;
			Char_array[67] = 255;
			Char_array[75] = 255;
		}
		else if (write == 8)
		{
			Char_array[3] = 255;
			Char_array[4] = 255;
			Char_array[11] = 255;
			Char_array[14] = 255;
			Char_array[19] = 255;
			Char_array[24] = 255;
			Char_array[29] = 255;
			Char_array[32] = 255;
			Char_array[39] = 255;
			Char_array[40] = 255;
			Char_array[47] = 255;
			Char_array[50] = 255;
			Char_array[55] = 255;
			Char_array[60] = 255;
			Char_array[65] = 255;
			Char_array[68] = 255;
			Char_array[75] = 255;
			Char_array[76] = 255;
		}
		else if (write == 9)
		{
			Char_array[3] = 255;
			Char_array[4] = 255;
			Char_array[11] = 255;
			Char_array[14] = 255;
			Char_array[19] = 255;
			Char_array[24] = 255;
			Char_array[29] = 255;
			Char_array[32] = 255;
			Char_array[39] = 255;
			Char_array[40] = 255;
			Char_array[33] = 255;
			Char_array[42] = 255;
			Char_array[51] = 255;
			Char_array[60] = 255;
			Char_array[69] = 255;
			Char_array[77] = 255;
			Char_array[76] = 255;
			Char_array[75] = 255;
		}
		for (j = 0; j < 9; j++)
		{
			for (i = 0; i < 9; i++)
			{
				for (z = 0; z < hist->NumberOfLayers; z++)
				{
//					Img_src->rgbpix[hist->NumberOfLayers * ((15 + i) * hist->Size_x + (hist->Size_x / 2) - (10 * NumberofCalcs) + j) + z] = Char_array[i * 9 + j];
				}
			}
		}
	}

	/* write 0*/
	for (i = 0; i < 81; i++)
	{
		Char_array[i] = 0;
	}
	Char_array[4] = 255;
	Char_array[5] = 255;
	Char_array[12] = 255;
	Char_array[15] = 255;
	Char_array[20] = 255;
	Char_array[25] = 255;
	Char_array[28] = 255;
	Char_array[35] = 255;
	Char_array[37] = 255;
	Char_array[44] = 255;
	Char_array[46] = 255;
	Char_array[53] = 255;
	Char_array[56] = 255;
	Char_array[61] = 255;
	Char_array[66] = 255;
	Char_array[69] = 255;
	Char_array[76] = 255;
	Char_array[77] = 255;

	for (j = 0; j < 9; j++)
	{
		for (i = 0; i < 9; i++)
		{
			for (k = 0; k < hist->NumberOfLayers; k++)
			{
				Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + 10 + j) + k] = Char_array[i * 9 + j];
			}
		}
	}

	/* write 2*/
	for (i = 0; i < 81; i++)
	{
		Char_array[i] = 0;
	}
	Char_array[10] = 255;
	Char_array[2] = 255;
	Char_array[3] = 255;
	Char_array[4] = 255;
	Char_array[5] = 255;
	Char_array[6] = 255;
	Char_array[15] = 255;
	Char_array[24] = 255;
	Char_array[32] = 255;
	Char_array[40] = 255;
	Char_array[48] = 255;
	Char_array[56] = 255;
	Char_array[64] = 255;
	Char_array[65] = 255;
	Char_array[66] = 255;
	Char_array[67] = 255;
	Char_array[68] = 255;
	Char_array[69] = 255;

	for (j = 0; j < 9; j++)
	{
		for (i = 0; i < 9; i++)
		{
			for (k = 0; k < hist->NumberOfLayers; k++)
			{
				Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 40 + j) + k] = Char_array[i * 9 + j];
			}
		}
	}

	/* write 5*/
	for (i = 0; i < 81; i++)
	{
		Char_array[i] = 0;
	}
	Char_array[10] = 255;
	Char_array[2] = 255;
	Char_array[3] = 255;
	Char_array[4] = 255;
	Char_array[5] = 255;
	Char_array[6] = 255;
	Char_array[19] = 255;
	Char_array[28] = 255;
	Char_array[37] = 255;
	Char_array[38] = 255;
	Char_array[39] = 255;
	Char_array[40] = 255;
	Char_array[41] = 255;
	Char_array[42] = 255;
	Char_array[43] = 255;
	Char_array[52] = 255;
	Char_array[61] = 255;
	Char_array[70] = 255;
	Char_array[69] = 255;
	Char_array[68] = 255;
	Char_array[67] = 255;
	Char_array[66] = 255;
	Char_array[65] = 255;
	Char_array[64] = 255;

	for (j = 0; j < 9; j++)
	{
		for (i = 0; i < 9; i++)
		{
			for (k = 0; k < hist->NumberOfLayers; k++)
			{
				Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 30 + j) + k] = Char_array[i * 9 + j];
				Img_src->rgbpix[hist->NumberOfLayers * ((hist->Size_y - 15 + i) * hist->Size_x + hist->Size_x - 20 + j) + k] = Char_array[i * 9 + j];
			}
		}
	}

}