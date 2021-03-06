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
#include <sys/time.h>
#include <string>
#include <sstream>
#include <ctime>
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
			sep = ",";  // separator

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

				out << "localtime" << sep;  // add special "time" column
				for (it = config.begin(); (it + 1) != config.end(); it++) {
					out << ((*it)->name) << sep;
				}
				out << ((*it)->name) ;  // without sep
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

			static struct timeval tv;
			static struct timezone tz;
			static time_t first_time = 0;
			// first_time is used to change seconds since epoch to
			// seconds since first readAppend call.
			// This is to make the time number smaller so it works
			// better with applications such as R.
			//static float max_suseconds = pow(2, (sizeof(suseconds_t) * 8)) - 1;
			double t;


			if (serial->cmd_A(num_bytes, buf)) {
				cerr << "readAppend(), cmd_A() failed\n";
				return;
			}

			vector<RTConfig*>::iterator it;

			stringstream line;  // build a line of the data

			gettimeofday(&tv, &tz);
			if (0 == first_time) {
				first_time = tv.tv_sec;
			}

			t = tv.tv_sec - first_time;
			t += (tv.tv_usec / 1.0e6);  // add microseconds converted to seconds

			line << t << sep;

			for (it = config.begin(); it != config.end(); it++) {

				unsigned int offset = (*it)->offset;
				if (RTConfigScalar* rtc = dynamic_cast<RTConfigScalar*>(*it)) {
					float add = rtc->add;
					float mult = rtc->mult;

					string type = rtc->type;
					float val = bufToValue(type, add, mult, &buf[offset]);

					line << val;
					if ((it + 1) != config.end()) {
						line << sep;
					}

				// TODO
				//} else if (RTConfigBits* rtc
				//				= dynamic_cast<RTConfigBits*>(*it)) {
				}
				// else unkown type, ignore
			}

			out << line.str() << endl;
		}
		// }}}

};
