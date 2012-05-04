/*
 * Copyright (C) Ryan Lothian 2008-2011
 *
 * Program to take a list of jpeg files and output all the EAN13 barcodes
 * visible in those images.  It is assumed that any barcodes will be
 * horizontal.
 *
 * Compilation requirements: gd image library
 *
 * Algorithm: for each image, we first need to convert the image to black and 
 * white.  We try a range of different thresholds on the sum of red, green, 
 * blue and look for barcodes at each threshold value.
 *
 * This algorithm is quite CPU intensive - I recommend compiling with
 * optimisations turned on.
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

#include <string>
#include <iostream>
#include <vector>
#include <set>

#include "image_handling.hpp"
#include "ean13.hpp"

using namespace std;


/*
 * Classify a brightness value as black or white.
 */
inline bool isBlack(unsigned int v, unsigned int black) {
	return (v <= black);
}

/*
 * Convert a row of pixel brightness values to black/white (true, false)
 * All values <= black_level are considered black, all values above are
 * considered white.
 */ 
vector<bool> convertToBlackWhite(const vector<unsigned int>& a,
                                 unsigned int black_level) {	
	vector<bool> ret;
	ret.reserve(a.size());
	
	for (unsigned int i = 0; i < a.size(); i++) {
		ret.push_back(isBlack(a[i], black_level));
	}
	
	return ret;
}


/*
 * Output any barcodes found in a given image.
 */
void outputBarcodesFound(const Image &img) {
	set<string> seen;  // set of barcodes seen
	
	// Try a range of black/white threshold values.
	for (float black_level = 250; black_level <= 550; black_level += 70) {
	
	    // Test each row in the image. If performance is an issue, you could
	    //  modify this to test every other row, say, but you might miss some
	    // barcodes.
	    for (unsigned int y = 0; y < img.getHeight(); y++) {	
	    
    	    // get pixel brightnesses 		
		    vector<unsigned int> row_brightnesses = img.getRowBrightness(y);
		    
		    // convert to black/white (true/false)
		    vector<bool>   row_bw = convertToBlackWhite(row_brightnesses, 
		                                                black_level); 
            
            addBarcodesFromBWRow(row_bw, seen);
	    }
	}
	
	/*
	 * Output every barcode found in the image.
	 */
	for (set<string>::iterator  it = seen.begin(); it != seen.end(); ++it) {
	    cout << *it << endl;
	}
}


/*
 * Takes a list of jpeg files from the command-line and scans each file 
 * for barcodes, outputting any found to stdout.
 */
int main(int argc, char* argv[]) {
    /*
     * If no filename was given, print a usage string.
     */
	if (argc < 2) {
		cout << "Usage: " << argv[0] <<" <file> [<file>] [<file>] ..." << endl
		     << endl
		     << "  file - the path to a JPEG file" << endl;
	} else {	
	    /*
	     * For each filename given...
	     */
	    for (unsigned int i = 1; i < (unsigned int)argc; i++) {
        	/*
        	 * Load the file (which is assumed to be a JPEG).
             */
		    Image img(argv[i]);

	        /*
	         * Scan the image for EAN13 barcodes.
	         */
		    outputBarcodesFound(img);		
	    }
    }
}

