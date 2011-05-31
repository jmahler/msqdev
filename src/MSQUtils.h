/*
 * Copyright (C) 2011 Jeremiah Mahler <jmmahler@gmail.com>
 *
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

#pragma once

#include <string>

using std::string;

float bufToValue(string type, float add, float mult, char* buf) {

	double v;

	if ("U16" == type) {
		unsigned short s;
		s = *buf & 0xFF;
		s = (s << 8);
		s = (*(buf + 1) & 0xFF) | s;
		v = s;
	} else if ("S16" == type) {
		short s;
		s = *buf & 0xFF;
		s = (s << 8);
		s = (*(buf + 1) & 0xFF) | s;
		v = s;
	} else if ("U08") {
		unsigned char c;
		c = *buf & 0xFF;
		v = c;
	} else if ("S08") {
		char c;
		c = *buf & 0xFF;
		v = c;
	}

	v += add;
	v *= mult;

	return v;
}

