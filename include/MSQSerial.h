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
#include <cstdlib>
#include <iostream>
#include <string>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
/* http://www.easysw.com/~mike/serial/serial.html */
#include <sys/ioctl.h>

using namespace std;

int usleep(__useconds_t usec);  // to quiet the compiler

/**
 * Megasquirt serial device communications.
 *
 * These operations are meant to be generic to all
 * versions MegaSquirt.
 * Version specific functionality is supported through
 * command arguments (see cmd_A() for an example).
 *
 * See http://www.msextra.com/doc/ms2extra/RS232_MS2.html for a description
 * of the serial protocol.
 */
class MSQSerial {
	private:
		string dev_file;
		int devfd;

		/* {{{ sread() */
		/**
		 * sread() - serial read
		 *
		 * Reading from the serial port is not straightforward.
		 * Sometimes only part of the data is sent at a time.
		 *
		 * This function encapsulates the error detection and correction
		 * need for reading from a serial device.
		 */
		int sread(int fd, char* buf, int size, const int max_errs) {
			char* buf_start = buf;
			int errs = 0;
			int nr = 0;
			int n;

			/*
			fcntl(devfd, F_SETFL, FNDELAY);
			fcntl(devfd, F_SETFL, 0);
			*/
			fcntl(fd, F_SETFL, 0);

			nr = 0;

			while (errs < max_errs) {
				n = read(fd, buf, 20);
				buf += n;
				nr += n;

				//printf("read: %d\n", n);

				if (size == nr) {
					break;
				}

				if (n < 0) {
					errs++;
					perror("read failed");
					return -1; /* error */
				} else if (0 == n) {
					errs++;
				} else if (n > 0) {
					errs = 0; /* reset */
				}
			}
			buf = buf_start;  /* reset to begining */

			return nr; /* OK */
		}
		/* }}} */

		MSQSerial() {}

	public:

		// {{{ MSQSerial(dev_file)
		/**
		 * Create a new MSQSerial object.
		 *
		 * Connection to the device is not initiated here (see connect()).
		 *
		 * @arg serial device
		 */
		MSQSerial(const string _dev_file) {
			dev_file = _dev_file;
		}
		// }}}

		// {{{ ~MSQSerial
		~MSQSerial() {
			close(devfd);
		}
		// }}}

		// {{{ connect()
		/**
		 * Connect (or reconnect) to the serial device.
		 * 
		 * @returns true on error, false otherwise
		 *
		 * Currently it only supports version 3.* series with a
		 * baud rate of 115200.
		 */
		bool connect() {
			struct termios options;

			devfd = open(dev_file.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
			if (devfd == -1) {
				char buf[1000];

				sprintf(buf, "Unable to open device '%s'", dev_file.c_str());
				perror(buf);

				return true;  // error
			} else {
				fcntl(devfd, F_SETFL, 0);
			}

			tcgetattr(devfd, &options);

			cfsetispeed(&options, B115200);
			options.c_cflag |= (CLOCAL | CREAD);

			// No parity (8N1)
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;

			tcsetattr(devfd, TCSANOW, &options);

			return false;  // OK
		}
		// }}}

		// {{{ cmd_Q
		/**
		 * Get the MS2 code version.
		 *
		 * @returns version as a string on success, empty string on error
		 */
		string cmd_Q()
		{
			int n;  // read/write counts

			n = write(devfd, "Q", 1);
			if (n != 1) {
				cerr << "write of Q command failed\n";
				return "";
			}

			char buf[21];
			n = sread(devfd, buf, 20, 3);
			if (n < 0) {
				cerr << "read of Q command failed\n";
				return "";  // error
			}

			string ver(buf);
			return ver;  // OK
		}
		// }}}

		// {{{ cmd_S
		/**
		 * Get the ecu signature.
		 *
		 * @returns signature as a string on success, empty string on error
		 */
		string cmd_S()
		{
			int n;  // read/write counts
			
			n = write(devfd, "S", 1);
			if (n < 0)  {
				cerr << "write of S command failed!\n";
				return "";  // error
			}

			char buf[61];
			n = sread(devfd, buf, 60, 5);
			if (n < 0) {
				cerr << "error reading result of S command\n";
				return "";  // error
			}
			string sig(buf);

			return sig;  // OK
		}
		// }}}

		// {{{ cmd_r
		/**
		 * Read data such as tables and settings from the ecu.
		 *
		 * cmd_r(<tbl_idx>, <offset>, <bytes>)
		 */
		int cmd_r(const int tbl_idx,
					const int offset, const int num_bytes, char* msg)
		{
			int n;
			char _buf[7];
			char* buf;
			buf = &_buf[0];

			*buf = 114;  /* 'r' */
			buf++;

			*buf = 0;
			buf++;

			*buf = (unsigned char) tbl_idx;
			buf++;

			buf = &_buf[0]; /* reset to begining */

			n = write(devfd, buf, 3);
			if (n < 3)
				fputs("write of r command failed!\n", stderr);

			/* 200 ms delay required when switching pages (or anytime) */
			usleep(200000);   /* 200 ms -> 200000 us (micro seconds) */


			*buf = (unsigned char)((offset >> 8) & 255);
			buf++;
			*buf = (unsigned char)(offset & 255);
			buf++;

			*buf = (unsigned char)((num_bytes >> 8) & 255);
			buf++;
			*buf = (unsigned char)(num_bytes & 255);
			/*buf++;*/

			buf = &_buf[0]; /* reset to begining */

			n = write(devfd, buf, 4);
			if (n < 4)
				fputs("write of r command failed!\n", stderr);

			n = sread(devfd, msg, num_bytes, 10);
			if (n < num_bytes) {
				printf("num read: %d\n", n);
				return -1;  /* error */
			}

			return n;  /* OK */
		}
		// }}}

		// {{{ cmd_w
		/**
		 * Write data such as tables and settings to the ecu.
		 *
		 * This command is the opposite of the "r" (read) command.
		 *
		 * cmd_w(<tbl_idx>, <offset>, <num_bytes>, <bytes>)
		 */
		int cmd_w(const int tbl_idx,
					const int offset, const int num_bytes, char* bytes)
		{
			int n;
			char _buf[7];
			char* buf;
			buf = &_buf[0];

			*buf = 119;  // 'w'
			buf++;

			// can-id?
			*buf = 0;
			buf++;

			*buf = (unsigned char) tbl_idx;
			buf++;

			buf = &_buf[0]; /* reset to begining */

			n = write(devfd, buf, 3);
			if (n < 3)
				fputs("write of w command failed!\n", stderr);

			/* 200 ms delay required when switching pages (or anytime) */
			usleep(200000);   /* 200 ms -> 200000 us (micro seconds) */

			*buf = (unsigned char)((offset >> 8) & 255);
			buf++;
			*buf = (unsigned char)(offset & 255);
			buf++;

			*buf = (unsigned char)((num_bytes >> 8) & 255);
			buf++;
			*buf = (unsigned char)(num_bytes & 255);
			//buf++;

			buf = &_buf[0]; // reset to begining

			n = write(devfd, buf, 4);
			if (n < 4) {
				cerr << "write of r command failed!\n";
				return false;
			}

			for (int i = 0; i < num_bytes; i++) {
				n = write(devfd, bytes, 1);
				bytes++;
				if (n != 1) {
					cerr << "cmd_w(), write error\n";
					return false;
				}
			}

			return true;  // OK
		}
		// }}}

		// {{{ cmd_A
		/**
		 * Read real time data.
		 *
		 * @arg number of bytes (see .ini)
		 *
		 * @arg buffer to store bytes
		 *
		 * @returns false on success, true on error
		 *
		 * The number of bytes is dependent upon the MegaSquirt version
		 * being used.  The ini file included with MegaSquirt should
		 * specify what this size is.
		 * The buffer must be large enough to store this many bytes.
		 * And the actual values described by the bytes should also be
		 * described in the ini file.
		 */
		bool cmd_A(const int num_bytes, char* buf)
		{
			int n;  // read/write counts
			
			n = write(devfd, "A", 1);
			if (n < 0)  {
				cerr << "write of A command failed\n";
				return true;  // error
			}

			n = sread(devfd, buf, num_bytes, 5);
			if (n < 0) {
				cerr << "error reading result of A command\n";
				return true;  // error
			}

			return false;  // OK
		}
		// }}}

};

