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

#include <ctime>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include "MSQData.h"
#include "MSQData/Table.h"
#include "MSQSerial.h"
#include "MSQRealTime.h"

using namespace std;

#define LOG true

static void sig_update(int sig, siginfo_t *siginfo, void *context);

enum { none, file, ecu };
int update = none;

// {{{ log()
/*
 * Write a log message.
 */
void log(string msg) {

	if (! LOG)
		return;

	time_t rawtime;
	time(&rawtime);

	string tm(ctime(&rawtime));

	// remove the newline
	int last = tm.length();
	last--;
	if (tm[last] == '\n')
		tm.erase(last, 1);

	// format for log entry output
	tm += ": ";

	// currently outputs to standard out instead of a file
	cout << tm << msg << endl;

	return;
}
// }}}

int main(int argc, char** argv)
{
	struct sigaction sa_update;
	sa_update.sa_sigaction = &sig_update;
	sigemptyset(&sa_update.sa_mask);
	sa_update.sa_flags = 0;

	sigaction(SIGHUP, &sa_update, NULL);

	// {{{ command line arguments
    if (argc < 2) {
		cout << " USAGE:\n"
         	 << "   msqdev <device> [<options>]\n"
        	 << "   msqdev /dev/ttyUSB0 ./\n"
			 << " OPTIONS:\n"
			 << "   -d           directory of files, default './'\n"
			 << "   -ue          update ecu from files on startup\n"
			 << "   -uf          update files from ecu on startup\n"
			 << " SIGNALS:\n"
			 << "   SIGHUP       triggers update of ecu from files\n";

        return 1; // error
    }
	string serial_dev = argv[1];

	string dir = "";

	for (int i = 2; i < argc; i++) {
		string arg = argv[i];

		if (arg == "-ue") {
			update = ecu;
		} else if (arg == "-uf") {
			update = file;
		} else if (arg == "-d") {
			if ((i + 1) >= argc) {
				cerr << "the -d option requires a director" << endl;
				return 1;  // error
			}
		} else {
			cout << "unkown argument: '" << arg << "'\n";
			return 1;  // error
		}
	}
	// }}}

	// {{{ write the pid (process id) to a file
	{
	pid_t pid = getpid();

	string pidfile = dir;
	if (! pidfile.empty())
		pidfile += "/";
	pidfile += "pid";

	ofstream pidfd(pidfile.c_str(), ios_base::trunc);
	if (pidfd.fail()) {
		perror("unable to open pid file");
		return 1; // error
	}
	pidfd << pid << endl;

	pidfd.close();
	}
	// }}}

	log("start");

	MSQSerial serial(serial_dev);

	if (serial.connect()) {
		//log("unable to open serial device\n");
		cerr << "unable to open serial device\n";
		return 1; // error
	}

	MSQData* tables[3];
	int num_tables = 3;

	// {{{ read advanceTable1
	/*
	 * doc/ini/megasquirt-ii.ms2extra.alpha_3.0.3u_20100522.ini
	 *
	 * page = 3, table_idx = 10
	 *       advanceTable1   = array ,  S16,    000,    [12x12], "deg",      0.10000,   0.00000,-10.00,   90.00,      1 ; * (288 bytes)
	 *       srpm_table1     = array ,  U16,    576,    [   12], "RPM",      1.00000,   0.00000,  0.00,15000.00,      0 ; * ( 24 bytes)
	 *       smap_table1     = array ,  S16,    624,    [   12], "%",      0.10000,   0.00000,  0.00,  400.00,      1 ; * ( 24 bytes)
	 */
	MSQDataTable<float, int> advanceTable1("advanceTable1", "advanceTable1",
			12, 12,  			// x size, y size
			&serial,
			10, "U16", 576, 	// x: tbl_idx, type, offset,
			1, 0,				// mult, add,
			10, "S16", 624, 	// y: tbl_idx, type, offset,
			0.1, 0,
			10, "S16", 0, 		// values: tbl_idx, type, offset,
			0.1, 0,
			"RPM", "map(%)"  	// title (without spaces!)
			);

	advanceTable1.readEcu();
	advanceTable1.readFile();
	// }}}

	// {{{ read veTable1
	/*
	 * doc/ini/megasquirt-ii.ms2extra.alpha_3.0.3u_20100522.ini
	 *
	 * page = 5, table_idx = 9
	 *       veTable1        = array ,  U08,      0,      [16x16], "%",        1.00000,   0.00000,  0.00,  255.00,      0 ; * (144 bytes)
	 * 
	 *       frpm_table1     = array ,  U16,    768,    [   16], "RPM",      1.00000,   0.00000,  0.00,15000.00,      0 ; * ( 24 bytes)
	 *       fmap_table1     = array ,  S16,    864,    [   16], "%",      0.10000,   0.00000,  0.00,  400.00,      1 ; * ( 24 bytes)
	 * 
	 */
	MSQDataTable<float, int> veTable1("veTable1", "veTable1",
			16, 16,  			// x size, y size
			&serial,
			// frpm_table1
			9, "U16", 768, 	// tbl_idx, type, offset
			1, 0,				// mult, add
			// fmap_table1
			9, "S16", 864,
			0.1, 0,
			// veTable1
			9, "U08", 0, 
			1, 0,
			"RPM", "FuelLoad(%)"  	// title (without spaces!)
			);

	veTable1.readEcu();
	veTable1.readFile();
	// }}}

	// {{{ read afrTable1
	/*
	 * doc/ini/megasquirt-ii.ms2extra.alpha_3.0.3u_20100522.ini
	 * 
	 * page = 1, table_idx = 4
	 * #if LAMBDA
	 *       afrTable1       = array ,  U08,     48,    [12x12], "Lambda",  0.006803,   0.00000,  0.00,    2.00,      3 ; * (144 bytes)
	 * #else
	 *       afrTable1       = array ,  U08,     48,    [12x12], "AFR",      0.10000,   0.00000,  9.00,   20.00,      1
	 * #endif
	 *       arpm_table1     = array ,  U16,    374,    [   12], "RPM",      1.00000,   0.00000,  0.00,15000.00,      0 ; * ( 24 bytes)
	 *       amap_table1     = array ,  S16,    422,    [   12], "%",      0.10000,   0.00000,  0.00,  400.00,      1 ; * ( 24 bytes)
	 * 
	 */
	MSQDataTable<float, int> afrTable1("afrTable1", "afrTable1",
			12, 12,  			// x size, y size
			&serial,
			// arpm_table1
			4, "U16", 374, 		// tbl_idx, type, offset
			1, 0,				// mult, add
			// amap_table1
			4, "S16", 422,
			0.1, 0,
			// afrTable1
			4, "U08", 48, 
			0.1, 0,
			//0.006803, 0,  // lambda
			"RPM", "map(Kpa)"  	// title (without spaces!)
			);

	afrTable1.readEcu();
	afrTable1.readFile();
	// }}}

	tables[0] = &advanceTable1;
	tables[1] = &veTable1;
	tables[2] = &afrTable1;

	// {{{ configure real time data
	vector<RTConfig*> rtconfig;

	RTConfigScalar seconds("seconds", "U16", 0, 1, 0);
	rtconfig.push_back(&seconds);
	RTConfigScalar rpm("rpm", "U16", 6, 1, 0);
	rtconfig.push_back(&rpm);
	RTConfigScalar advance("advance", "S16", 8, 0.1, 0);
	rtconfig.push_back(&advance);

	MSQRealTime rtData(&serial, "rtdata", 169, rtconfig);
	// }}}

	while (1) {
		if (update == ecu) {
			update = none;

			log("updating ecu from files");

			for (int i = 0; i < num_tables; i++) {
				MSQData *table = tables[i];

				if (table->hasChanges()) {
					table->writeEcu();
				}
			}
		} else if (update == file) {
			update = none;
			log("updating files from ecu");

			for (int i = 0; i < num_tables; i++) {
				MSQData *table = tables[i];

				if (table->hasChanges()) {
					table->cpEcuToFile();
					table->writeFile();
				}
			}
		} else {
			rtData.readAppend();
			usleep(5e5);
		}
	}

    return 0;
}

static void sig_update(int sig, siginfo_t *siginfo, void *context) {
	update = file;
}
