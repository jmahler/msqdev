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

#include "LookUpTable.h"
#include "MSQSerial.h"
#include "MSQData.h"

#include <cmath>  // fabs()
#include <iostream>
#include <set>
#include <unistd.h> // sleep

using namespace std;

#define EPSILON 0.001	// for floating point comparisons

template <class T, class U>
class MSQDataTable : public MSQData
{
	private:

		LookUpTable<T, U>* file_data;
		LookUpTable<T, U>* ecu_data;

		int x_size;
		int y_size;

		MSQSerial* serial;

		// cmd_r options
		int x_idx;
		int x_offset;
		string x_type;
		float x_add;
		float x_mult;

		int y_idx;
		int y_offset;
		string y_type;
		float y_add;
		float y_mult;

		int v_idx;
		int v_offset;
		string v_type;
		float v_add;
		float v_mult;

		MSQDataTable() {};  // prevent use of the default constructor

		// table_idx'es that have changed and need to be burned
		set<unsigned char> need_burn;

	public:

		// {{{ MSQDataTable(...
		MSQDataTable(const string _name, const string _file_name,
					const unsigned int _x_size, const unsigned int _y_size,
					MSQSerial* _serial,
					const int _x_idx, const string _x_type, const int _x_offset,
					const float _x_mult, const float _x_add,
					const int _y_idx, const string _y_type, const int _y_offset,
					const float _y_mult, const float _y_add,
					const int _v_idx, const string _v_type, const int _v_offset,
					const float _v_mult, const float _v_add,
					const string _x_title, const string _y_title)
			: MSQData(_name, _file_name)
		{
			file_data = new LookUpTable<T, U>(_x_size, _y_size, _x_title, _y_title);
			ecu_data = new LookUpTable<T, U>(_x_size, _y_size, _x_title, _y_title);	
			file_data->set_file(_file_name);
			ecu_data->set_file(_file_name);
			// It is necessary to set a file name for the ecu
			// for when the ecu data is copied to the file data.

			serial = _serial;

			x_size = _x_size;
			y_size = _y_size;

			v_idx = _v_idx;
			v_offset = _v_offset;
			v_type = _v_type; // TODO - check types
			v_add = _v_add;
			v_mult = _v_mult;

			x_idx = _x_idx;
			x_offset = _x_offset;
			x_type = _x_type; // TODO - check types
			x_add = _x_add;
			x_mult = _x_mult;

			y_idx = _y_idx;
			y_offset = _y_offset;
			y_type = _y_type; // TODO - check types
			y_add = _y_add;
			y_mult = _y_mult;
		};
		// }}}

		// {{{ ~MSQDataTable
		~MSQDataTable() {
			if (file_data != NULL) {
				delete file_data;
				file_data = NULL;
			}

			if (ecu_data != NULL) {
				delete ecu_data;
				ecu_data = NULL;
			}
		}
		// }}}

		// {{{ readEcu()
		bool readEcu() {

			// A complete table consists of three parts:
			//  the x coordinates, y coordinates, and the values.
			// Each one must be obtained seperately and aggregated together.

			enum { X, Y, V, END };

			for (int i = 0; i < END; i++) {
				string type;
				if (X == i)
					type = x_type;
				else if (Y == i)
					type = y_type;
				else if (V == i)
					type = v_type;
				else
					return true; // error

				int byte_mult = 1;
				if (type == "U16")
					byte_mult = 2;
				else if (type == "U08")
					byte_mult = 1;
				else if (type == "S16")
					byte_mult = 2;
				else if (type == "S08")
					byte_mult = 1;
				else
					return true; // error

				float mult;
				float add;
				int num_bytes;
				int size;
				int offset;
				int idx;
				if (X == i) {
					size = x_size;
					idx = x_idx;
					offset = x_offset;
					add = x_add;
					mult = x_mult;
				} else if (Y == i) {
					size = y_size;
					idx = y_idx;
					offset = y_offset;
					add = y_add;
					mult = y_mult;
				} else if (V == i) {
					size = x_size * y_size;
					idx = v_idx;
					offset = v_offset;
					add = v_add;
					mult = v_mult;
				} else {
					return true;  // error
				}
				num_bytes = byte_mult * size;

				char* buf = new char[num_bytes];
				int* vals = new int[size];
				// The values read are always integers (signed, unsigned, 8, 16),
				// they will be converted to floats later if needed.

				int n = serial->cmd_r(idx, offset, num_bytes, buf);
				if (num_bytes != n) {
					// read error
					delete[] buf; 
					delete[] vals; 
					return true;  // error
				}

				// TODO - use bufToValue()

				// convert the values by their integer type
				for (int j = 0, k = 0; j < num_bytes; j += byte_mult, k++) {
					if ("U16" == type) {
						unsigned short s;
						s = buf[j] & 0xFF;
						s = (s << 8);
						s = (buf[j+1] & 0xFF) | s;
						vals[k] = s;
					} else if ("S16" == type) {
						short s;
						s = buf[j] & 0xFF;
						s = (s << 8);
						s = (buf[j+1] & 0xFF) | s;
						vals[k] = s;
					} else if ("U08") {
						unsigned char c;
						c = buf[j] & 0xFF;
						vals[k] = c;
					} else if ("S08") {
						char c;
						c = buf[j] & 0xFF;
						vals[k] = c;
					}
				}

				// Assign the values/coordinates to the look up table.
				// This is where values are converted to floats and
				// adjusted.
				if (X == i) {
					for (int i = 0; i < size; i++) {
						U x = vals[i];
						x += add;
						x *= mult;

						ecu_data->set_x_coord(i, x);
					}
				} else if (Y == i) {
					for (int i = 0; i < size; i++) {
						U y = vals[i];
						y += add;
						y *= mult;

						if (! ecu_data->set_y_coord((size - 1) - i, y)) {
							cout << "error set_y(" << i << ", " << y << ", ...)\n";
						}
					}
				} else if (V == i) {
					int p = 0;
					for (int y = 0; y < y_size; y++) {
						for (int x = 0; x < x_size; x++) {
							T v = vals[p];
							v += add;
							v *= mult;

							if (! ecu_data->set(x, (y_size - 1) - y, v))
								cout << "error set(" << x << ", " << y << ", ...)\n";
								p++;
						}
					}
				}

				delete[] buf; 
				delete[] vals; 
			}

			return false;  // OK
		}
		// }}}

		// {{{ readFile()
		bool readFile() {
			int n = 0;  // number of errors
			int max_err = 5;

			while (1) {
				if (! file_data->load_file()) {
					n++;

					if (n >= max_err) {
						return true; // error
					}

					int s_left = 1;
					while ((s_left = sleep(s_left)) > 0) {}
					// loop needed in case it is interrupted
					// if there is an excessive number of interrupts
					// it will get stuck here.
				} else {
					break;  // OK
				}
			}

			return false; // OK
		}
		// }}}

		// {{{ writeEcu()
	private:

		// {{{ _type_val_buf()
		void _type_val_buf(const string type, const float v, int &num_bytes,
							char* bytes)
		{
			if ("U16" == type) {
				unsigned short s = v;
				bytes[1] = s & 0xFF;
				s = s & (0xFF << 8);
				s = s >> 8;
				bytes[0] = s;
				num_bytes = 2;
			} else if ("S16" == type) {
				signed short s = v;
				bytes[1] = s & 0xFF;
				s = s & (0xFF << 8);
				s = s >> 8;
				bytes[0] = s;
				num_bytes = 2;
			} else if ("U08") {
				unsigned char c = v;
				bytes[0] = c & 0xFF;
				num_bytes = 1;
			} else if ("S08") {
				unsigned char c = v;
				bytes[0] = c & 0xFF;
				num_bytes = 1;
			}
		}
		// }}}

	public:
		bool writeEcu() {
			enum { X, Y, V, END };

			for (int i = 0; i < END; i++) {
				string type;
				if (X == i)
					type = x_type;
				else if (Y == i)
					type = y_type;
				else if (V == i)
					type = v_type;
				else
					return true; // error

				int byte_mult = 1;
				if (type == "U16")
					byte_mult = 2;
				else if (type == "U08")
					byte_mult = 1;
				else if (type == "S16")
					byte_mult = 2;
				else if (type == "S08")
					byte_mult = 1;
				else
					return true; // error

				float mult;
				float add;
				int offset;
				int idx;
				if (X == i) {
					idx = x_idx;
					offset = x_offset;
					add = x_add;
					mult = x_mult;
				} else if (Y == i) {
					idx = y_idx;
					offset = y_offset;
					add = y_add;
					mult = y_mult;
				} else if (V == i) {
					idx = v_idx;
					offset = v_offset;
					add = v_add;
					mult = v_mult;
				} else {
					return true;  // error
				}

				// find differences and write changes

				if (X == i) {
					for (int j = 0; j < x_size; j++) {
						T eval = ecu_data->get_y_coord(j);
						T fval = file_data->get_y_coord(j);
						if (fabs(eval - fval) < EPSILON) {
							continue;  // no differences
						}
						// de-translate and write a single value

						fval /= mult;
						fval -= add;

						char buf[2];
						int num_bytes = 0;

						_type_val_buf(type, fval, num_bytes, buf);

						if (-1 == serial->cmd_w(idx, offset + j * byte_mult, num_bytes, buf)) {
							cerr << "serial->cmd_w X error\n";
							return true;  // error
						}
						need_burn.insert(idx);
					}
				} else if (Y == i) {
					for (int j = 0; j < y_size; j++) {

						T eval = ecu_data->get_y_coord(j);
						T fval = file_data->get_y_coord(j);
						if (fabs(eval - fval) < EPSILON) {
							continue;
						}

						fval /= mult;
						fval -= add;
						// division should be performed in floating point
						// to minimize roundoff errors.

						char buf[2];
						int num_bytes;

						_type_val_buf(type, fval, num_bytes, buf);

						if (-1 == serial->cmd_w(idx, offset + ((y_size - 1) - j) * byte_mult, num_bytes, buf)) {
							cerr << "serial->cmd_w Y error\n";
							return true;  // error
						}
						need_burn.insert(idx);
					}
				} else if (V == i) {
					int p = 0;
					//int size = x_size * y_size;
					for (int y = 0; y < y_size; y++) {
						for (int x = 0; x < x_size; x++, p++) {
							int _y = (y_size - 1) - y;  // reverse

							T eval = ecu_data->get(x, _y);
							T fval = file_data->get(x, _y);
							if (fabs(eval - fval) < EPSILON) {
								continue;
							}
							
							fval /= mult;
							fval -= add;

							char buf[2];
							int num_bytes;

							_type_val_buf(type, fval, num_bytes, buf);

							if (-1 == serial->cmd_w(idx, offset + (p * byte_mult), num_bytes, buf)) {
								cerr << "serial->cmd_w error\n";
								return true;  // error
							}
							need_burn.insert(idx);
						}
					}
				}
			}
			*ecu_data = *file_data;

			return false;  // OK
		}
		// }}}

		// {{{ burnEcu()
		// NOTE: tables are added to the need_burn set in writeEcu()
		bool burnEcu() {

			set<unsigned char>::iterator it;

			for (it = need_burn.begin(); it != need_burn.end(); it++) {
				//printf("burnEcu: '%i'\n", (*it));

				if (serial->cmd_b(*it)) {
					cerr << "serial->cmd_b error\n";
					return true;  // error
				}
			}
			need_burn.clear();

			return false;  // OK
		}
		// }}}

		// {{{ needBurn
		bool needBurn() {
			return (! need_burn.empty());
		}
		// }}}

		// {{{ writeFile()
		void writeFile() {
			if (! file_data->save()) {
				cerr << "writeFile(): error saving file\n";
			}
		}
		// }}}

		// {{{ cpEcuToFile()
		void cpEcuToFile() {
			*file_data = *ecu_data;
		}
		// }}}

		// {{{ hasChanges
		bool hasChanges() {
			return (*ecu_data != *file_data);
		}
		// }}}

};

