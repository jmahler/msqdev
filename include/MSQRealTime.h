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

#include <cerrno>
#include <cstdio>
#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include "MSQSerial.h"
#include "MSQUtils.h"

using namespace std;

// {{{ RTConfig*
/**
 * The classes RTConfigScalar and RTConfigBits correspond to
 * the types found in the .ini.
 * The RTConfig class is common to both types.
 */
class RTConfig {
private:
	virtual void fn() {};  	// for dynamic_cast to work
public:

	RTConfig(string _name, string _type, unsigned int _offset) {
		name = _name;
		type = _type;
		offset = _offset;
	}

	virtual ~RTConfig() {};
	string name;
	string type;			// U16, S16, ...
	string config_type;
	unsigned int offset;
};

class RTConfigScalar : public RTConfig {
public:
	RTConfigScalar(string _name, string _type, unsigned int _offset, float _mult, float _add)
		: RTConfig(_name, _type, _offset)
	{
		mult = _mult;
		add = _add;
	}
	string units;
	float add;
	float mult;
};

class RTConfigBits : public RTConfig {
public:
	unsigned int bit;  // [0:0] -> bit 0
	string one_value;  // values to assign when bits are 0 or 1
	string zero_value;
};
// }}}

/**
 * The MSQRealTimeData object is used to obtain real time
 * data (cmd_A()) and append it to a file.
 */
class MSQRealTime {
	private:
		MSQSerial* serial;

		string file;
		ofstream out;

		// Number of bytes that will be read by the "A" command (cmd_A()).
		// Found under config option 'ochBlockSize' in the ini.
		int num_bytes;
		char* buf;

		vector<RTConfig*> config;
		string sep;  // separator for output
	public:

		// {{{ MSQRealTime
		/**
		 * Construct a new object for reading real time
		 * data.
		 *
		 * Each configuration option is built by examing
		 * the .ini file for a particular version.
		 * Usually it is found in the [BurstMode] and [OutputChannels]
		 * section.
		 */
		MSQRealTime(MSQSerial* _serial,
						const string _file,
						const int _num_bytes,
						vector<RTConfig*> _config)
		{
			serial = _serial;
			file = _file;
			config = _config;
			num_bytes = _num_bytes;
			sep = " ";  // separator

			// If the file does not exists,
			// set a flag to add the columns after the
			// file is created.
			bool write_cols = false;
			out.open(file.c_str(), ios_base::in);
			if (out.fail()) {
				write_cols = true;
			}
			out.close();

			out.open(file.c_str(), ios_base::app);
			if (out.fail()) {
				perror("unable to open file for real time data");
			}
			// add the column names if this is a new file
			if (write_cols) {
				vector<RTConfig*>::iterator it;

				for (it = config.begin(); it != config.end(); it++) {
					out << ((*it)->name) << sep;
				}
				out << endl;
			}

			buf = new char[num_bytes];
		}
		// }}}

		// {{{ ~MSQRealTime
		~MSQRealTime() {
			delete[] buf;
		};
		// }}}

		// {{{ readAppend()
		/**
		 * Read one chunk of data and append it to the output file.
		 */
		void readAppend() {
			if (serial->cmd_A(num_bytes, buf)) {
				cerr << "readAppend(), cmd_A() failed\n";
				return;
			}

			vector<RTConfig*>::iterator it;

			stringstream line;  // build a line of the data
			for (it = config.begin(); it != config.end(); it++) {

				unsigned int offset = (*it)->offset;
				if (RTConfigScalar* rtc = dynamic_cast<RTConfigScalar*>(*it)) {
					float add = rtc->add;
					float mult = rtc->mult;

					string type = rtc->type;
					float val = bufToValue(type, add, mult, &buf[offset]);

					line << val << sep;

				// TODO
				//} else if (RTConfigBits* rtc
				//				= dynamic_cast<RTConfigBits*>(*it)) {
				}
				// else unkown type, ignore
			}
			// TODO remove trailing sep'arator

			out << line.str() << endl;
		}
		// }}}

};
