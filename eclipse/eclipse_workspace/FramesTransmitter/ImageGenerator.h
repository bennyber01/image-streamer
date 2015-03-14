/*
 * ImageGenerator.h
 *
 *  Created on: 30 בספט 2014
 *      Author: benny
 */

#ifndef IMAGEGENERATOR_H_
#define IMAGEGENERATOR_H_

#include "bitmap_image.hpp"

class ImageGenerator {
public:
	ImageGenerator();
	virtual ~ImageGenerator();

	bool GetImageParams(int & imageWidth, int & imageHeight, int & imageBPP);
	unsigned char * GenerateImage();

private:
	bitmap_image image;

	int regionX, regionY, regionSize;
	bool isMoveRight;
};

#endif /* IMAGEGENERATOR_H_ */
