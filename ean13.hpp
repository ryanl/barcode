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

#include <set>
#include <vector>
#include <string>

/*
 * row_bw:         row of bits, TRUE for black, FALSE for white
 * barcodes: (OUT) barcodes seen
 */
void addBarcodesFromBWRow(const std::vector<bool>& row_bw,
                          std::set<std::string>& barcodes);
