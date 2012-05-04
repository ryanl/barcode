/*
 * Copyright (C) Ryan Lothian 2008-2011
 *
 * Program to take a list of jpeg files and output all the EAN13 barcodes
 * visible in those images.
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "image_handling.hpp"
#include <vector>
#include <cstdio>
#include <iostream>
#include "assert.h"

using namespace std;

/*
 * Load an image from a file.
 */ 
Image::Image(const char *path) {
	FILE *jpeg_in = fopen(path, "r");
	if (jpeg_in == NULL) {
	    cerr << "Could not open " << path << endl;
		assert(false);
	}
	im = gdImageCreateFromJpeg(jpeg_in);
	fclose(jpeg_in);    
}

/*
 * Get monochrome brightness values for a row of an image as a vector.
 */
vector<unsigned int> Image::getRowBrightness(unsigned int y) const {
    
    assert(y < getHeight());

    unsigned int width = gdImageSX(im);
    
	vector<unsigned int> ret;
	ret.reserve(width);
	
	
	for (unsigned int x = 0; x < width; x++) {	
		int colour = gdImageGetPixel(im, x, y);
				
		unsigned int channel_sum = gdImageRed(im,   colour) + 
		                           gdImageGreen(im, colour) + 
		                           gdImageBlue(im,  colour);
        
		ret.push_back(channel_sum);
	}
	
	return ret;
}



