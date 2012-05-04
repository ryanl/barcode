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

#include "gd.h"
#include <vector>


class Image {
private:
    gdImagePtr im;

    /*
     * No copy constructor (in order to prevent double frees).
     */
    Image(const Image&);
    Image& operator = (const Image&);
    
public:
    Image(const char* path);
        
    ~Image() {
        gdFree(im);
    }
    
    unsigned int getWidth() const {
        return gdImageSX(im);
    }
    
    unsigned int getHeight() const {
        return gdImageSY(im);
    }
    
    std::vector<unsigned int> getRowBrightness(unsigned int y) const;
};
