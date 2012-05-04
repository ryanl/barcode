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

#include "ean13.hpp"
#include <set>
#include <vector>
#include <string>

using namespace std;

/*
 * Map a string of 7 bits to a pair a,b such that 0 <= a < 10, 0 <= b < 3.
 */
bool decodeEAN13Digit(string digit, int *a, int *b) {
	string encodings[10][3] = {
		{ "0001101", "0100111", "1110010" },
		{ "0011001", "0110011", "1100110" },
		{ "0010011", "0011011", "1101100" },
		{ "0111101", "0100001", "1000010" },
		{ "0100011", "0011101", "1011100" },
		{ "0110001", "0111001", "1001110" },
		{ "0101111", "0000101", "1010000" },
		{ "0111011", "0010001", "1000100" },
		{ "0110111", "0001001", "1001000" },
		{ "0001011", "0010111", "1110100" }
	};
	
	for (*a = 0; *a < 10; (*a)++) {
		for (*b = 0; *b < 3; (*b)++) {
			if (encodings[*a][*b] == digit) return true;
		} 
	}
	return false;
}

/*
 * Calculate the EAN checksum of 13 decimal digits.
 */
int getEAN13CChecksum(const int a[13]) {
	int weight = 3, sum = 0;
	
	for (int i = 11; i >= 0; i--) {
		sum += a[i] * weight;
		weight = 4 - weight; // 1 <-> 3	
	}
	sum = 10 - (sum % 10);
	if (sum == 10) sum = 0;
	return sum;
}

int getEAN13FirstDigit(const int b[12]) {
	char letter_for[3] = {'L', 'G', 'R'};
	
	const char* left_lookup[10] = {
		"LLLLLL",
		"LLGLGG",
		"LLGGLG",
		"LLGGGL",
		"LGLLGG",
		"LGGLLG",
		"LGGGLL",
		"LGLGLG",
		"LGLGGL",
		"LGGLGL"	
	};
	for (int i = 6; i < 12; i++) {
		if (letter_for[b[i]] != 'R') return -1;
	}
	for (int v = 0; v < 10; v++) {
		bool okay = true;
		for (int i = 0; i < 6; i++) {
			if (letter_for[b[i]] != left_lookup[v][i]) {
				okay = false;
				break;
			}
		}
		if (okay) {
			return v;
		}
	}
	return -1;
}

/*
 * Convert a bit vector to a barcode string, if a barcode exists.
 * Returns "" if no valid barcode was found.
 */
string getEAN13FromBits(vector<bool> bit) {
    bool      valid = true;
    
    /*
     * EAN format: 101, then 7 sets of 6-bits, each of which gives a digit,
                   then 10101, then 7 sets of 6-bits, each of which gives a 
                   digit, then 101.
     */
	const int bits = 3 + (7 * 6) + 5 + (7 * 6) + 3;
	string digit[12];
	int a[12], b[12];
	
	/*
	 * Valid barcodes start 101 and end 101.
	 */
	if (bit[0] != true         ||
	        bit[1] != false        ||
	        bit[2] != true         ||
	        bit[bits - 3] != true  || 
	        bit[bits - 2] != false ||
            bit[bits - 1] != true) {
	    
	    valid = false;
	}
	
	int offset = 3;
	
	/*
	 * For each of the first 6 digits.
	 */
	for (int i = 0; valid && i < 6; i++) {
		for (int j = 0; valid && j < 7; j++) {
			digit[i] = digit[i] + (bit[offset] ? "1" : "0");
			offset++;
		}
		if (!decodeEAN13Digit(digit[i], &a[i], &b[i])) {
            valid = false;
		}
	}	
	
	if (bit[offset] != false || bit[offset + 1] != true || bit[offset + 2] != false
	    || bit[offset + 3] != true || bit[offset + 4] != false) return "";

	offset += 5;
	for (int i = 6; i < 12; i++) {
		for (int j = 0; j < 7; j++) {
			digit[i] = digit[i] + (bit[offset] ? "1" : "0");
			offset++;
		}
		if (!decodeEAN13Digit(digit[i], &a[i], &b[i])) {
			return "";
		}
	}
	
	
	if (bit[offset] != true || bit[offset + 1] != false || bit[offset + 2] != true) return "";
	
	int first_digit = getEAN13FirstDigit(b);
	
	if (first_digit == -1) {
		return "";
	}
	

	char fd[2] = { first_digit + '0', '\0' };
	string ret = fd;
	
	int c[13];
	c[0] = first_digit;
	 
	for (int i = 0; i < 12; i++) {
		fd[0] = a[i] + '0';
		c[i + 1] = a[i];
		ret += string(fd);
	}
	
	int checksum = getEAN13CChecksum(c);
	if (c[12] != checksum) {
        // failed checksum
		return "";
	}
	
	return ret;
}


/*
 * Convert a line of black/white (-1/1) values to a barcode, if one exists.
 *
 * line:  black/white (true/false) values
 * start: index in line to begin at
 * end:   index in line to end at
 */
string getEAN13FromLine(const vector<bool>&  line,
                        unsigned int         start,
                        unsigned int         end) {
                        
	vector<int> block_lengths;
	
	unsigned int v = line[start], len = 1;

    // look for where the colour changes, record the widths in block_lengths	
	for (unsigned int i = start + 1; i <= end; i++) {
		if (line[i] == v) {
			len++;
			
		    // give up if a solid block was too wide - barcodes always contain
		    // frequent alternation		    			
			if (len > (end - start) / 10) return "";			
		} else {
			block_lengths.push_back(len);
			len = 1;
			v = line[i];
		}
	}
	
	// add the final block
	block_lengths.push_back(len);

	const unsigned int bits = 3 + (7 * 6) + 5 + (7 * 6) + 3;
	float width = end - start;
	float expected_bit_width = width / bits;
	
	bool state = true;
	vector<bool> bit(bits);
	unsigned int write = 0, pos = start;
	
	for (unsigned int i = 0; i < block_lengths.size(); i++) {
		if (write == bits) break;
		
		expected_bit_width = float(width + start - pos) / (bits - write);
		
		int num_bits = int(0.5f + (block_lengths[i] / expected_bit_width));
		if (num_bits < 1) num_bits = 1;
		
		if (num_bits + write > bits) return ""; 
		
		pos += block_lengths[i];
		
		for (int j = 0; j < num_bits; j++) {
			bit.at(write++) = state;
		}
		
		state = !state;	
	}

	return getEAN13FromBits(bit);
}


/*
 * Find any barcodes present in a black/white row.
 */
void addBarcodesFromBWRow(const vector<bool>& row_bw,
                          set<string>& barcodes) {
                          
    // barcodes occur between blocks of white - look for large 
    // blocks of white
    vector<int> long_white_begin_positions;
    vector<int> long_white_end_positions;

    const bool WHITE = false;
    
    unsigned int white_count = 0;
    unsigned int begin = 0;

    for (unsigned int i = 0; i < row_bw.size(); i++) {
	    if (row_bw[i] == WHITE) {
		    if (white_count == 0) {
			    begin = i;	
		    }
		    white_count++;
	    } else {
	        // if there was a block of white
		    if (white_count > 35) {
		        // record its position
			    long_white_end_positions.push_back(i);
			    if (begin > 0) {
				    long_white_begin_positions.push_back(begin);
			    }
		    }
		    white_count = 0;
	    }
    }
    
    if (white_count > 35) {
	    long_white_begin_positions.push_back(begin);
    }

    for (unsigned int i = 0; i < long_white_end_positions.size(); i++) {
	    for (unsigned int j = 0; j < long_white_begin_positions.size(); j++) {
	
		    int barcode_start = long_white_end_positions.at(i);
		    int barcode_end   = long_white_begin_positions.at(j) - 1;
		
		    if (barcode_end > barcode_start) {
			    // cout << barcode_start << " to " << barcode_end << endl;
		
			    string EAN13 = getEAN13FromLine(row_bw, barcode_start,
			                                    barcode_end);
			    if (EAN13 != "" && barcodes.find(EAN13) == barcodes.end()) {
				    barcodes.insert(EAN13);
			    }
		    }
	    }
    }
}


