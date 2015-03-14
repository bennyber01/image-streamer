/*
 * ImageGenerator.cpp
 *
 *  Created on: 30 בספט 2014
 *      Author: benny
 */

#include "ImageGenerator.h"

ImageGenerator::ImageGenerator() : image(320, 240)
{
	regionX = 0;
	regionY = 110;
	regionSize = 40;
	isMoveRight = true;
}

ImageGenerator::~ImageGenerator()
{
	// TODO Auto-generated destructor stub
}

bool ImageGenerator::GetImageParams(int & imageWidth, int & imageHeight, int & imageBPP)
{
	imageWidth  = image.width();
	imageHeight = image.height();
	imageBPP    = image.bytes_per_pixel() * 8;
	return true;
}

unsigned char* ImageGenerator::GenerateImage()
{
	image.clear(0);

	if (isMoveRight)
		regionX += 5;
	else
		regionX -= 5;

	image.set_region(regionX, regionY, regionSize, regionSize, 0, 0, 255);

	if (regionX < 0)
		isMoveRight = true;
	else if (regionX > image.width() - regionSize)
		isMoveRight = false;

	return image.data();
}

