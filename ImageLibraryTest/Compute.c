/********************************************************************************
*																				*
*	CV Library - Compute.c														*
*																				*
*	Author:  Petar Nikolov														*
*																				*
*																				*
*	The algorithms included in this file are:									*
*																				*
*	- Index to Position															*
*	- Position to Index															*
*	- Round																	    *
*	- Convolution / Binary														*
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
	convert  I N D E X t o P O S
*/
void getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col)
{
	*red = pixIdx / (Img_src->Num_channels * Img_src->Width);
	*col = pixIdx - ((*red)*Img_src->Width * Img_src->Num_channels);
}

/*
	convert   P O S t o I N D E X
*/
int getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col)
{
	int pixelIndex = 0;

	*pixIdx = (red * Img_src->Width + col)*Img_src->Num_channels;
	pixelIndex = *pixIdx;

	return pixelIndex;
}

/*
	R O U N D   -  RoundValues to X significant bit
*/
float RoundValue_toX_SignificantBits(float Value, int X)
{
	int ValueTimesX = 0;
	float Number = Value;

	ValueTimesX = Value * pow(10, X);
	Number *= pow(10, X);

	if (Number - ValueTimesX > 0.5) 
		ValueTimesX += 1;

	Number = ValueTimesX / pow(10, X);

	if (Number * pow(10, X) < ValueTimesX) 
		Number += (1 / pow(10, X));
	return Number;
}


/*
	C O N V O L U T I O N
*/
void Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize)
{
	int i, j, n, m;
	int FinalNum;
	int DevideNumber = pow((float)KernelSize, 2);

	for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
	{
		for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
		{
			//OutputArray[i * cols + j] = OutputArray[i * cols + j];
			size_t c = 0;
			FinalNum = 0;
			for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
			{
				DevideNumber = 9;
				for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
				{
					FinalNum += InputArray[(j - n) * cols + i - m] * Kernel[c];
					//if (FinalNum != 0)printf("%d\n", FinalNum);
					//if (Kernel[c] == 0) DevideNumber--;
					c++;
				}
			}
			if (DevideNumber <= 0) 
				FinalNum = 0;
			else 
				FinalNum = (float)FinalNum / DevideNumber;

			if (FinalNum < 0) FinalNum = 0;
			else if (FinalNum > 255) FinalNum = 255;
			//if (FinalNum > 128) FinalNum = 255;
			//else FinalNum = 0;

			OutputArray[j * cols + i] = FinalNum;
		}
	}
}

/*
	C O N V O L U T I O N - Binary Image
*/
void ConvolutionBinary(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize, int DilateOrErode)
{
	int i, j, n, m;
	int FinalNum;
	int DevideNumber = pow((float)KernelSize, 2);

	for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
	{
		for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
		{
			size_t c = 0;
			FinalNum = 0;

			if (InputArray[(j) * cols + i] > 0)
			{
				for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
				{
					for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
					{
						if (Kernel[c] == 1)
						{
							if (DilateOrErode == 0)
								OutputArray[(j - n) * cols + i - m] = 255;
							else
								OutputArray[(j - n) * cols + i - m] = 0;
						}
						else
						{
							OutputArray[(j - n) * cols + i - m] = InputArray[(j - n) * cols + i - m];
						}
						c++;
					}
				}
			}
			else
			{
				OutputArray[(j) * cols + i ] = InputArray[(j) * cols + i];
			}
		}
	}
}