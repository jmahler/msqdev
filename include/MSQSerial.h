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

#ifndef DEBUG
#define DEBUG false
#endif

using namespace std;

int usleep(__useconds_t usec);  // to appease the compiler

/**
 * Megasquirt serial device communications.
 *
 * These operations are meant to be generic to all
 * versions of MegaSquirt.
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

		// {{{ sread()
		/**
		 * sread() - serial read
		 *
		 * Reading from the serial port is not straightforward.
		 * Sometimes only part of the data is sent at a time.
		 *
		 * This function encapsulates the error detection and correction
		 * need for reading from a serial device.
		 */
		int sread(int fd, char* buf, const int size, const int max_errs) {
			char* buf_start = buf;
			int errs = 0;  // number of errors
			int nr = 0;    // total num read
			int n; 		   // sing num read or possibly error

			if (DEBUG) { cout << "sread()\n"; }
			if (DEBUG) { cout << "  size req: " << size << "\n"; }

			//fcntl(devfd, F_SETFL, FNDELAY);
			fcntl(fd, F_SETFL, 0);

			while (errs < max_errs) {

				if (DEBUG) { cout << "read()..."; }
				n = read(fd, buf, size - nr);
				if (DEBUG) { cout << "done."; }

				if (n < 0) {  // an error
					errs++;

					if (errno == EINTR) {
						if (DEBUG) { cout << "  EINTR\n"; }
						//perror("EINTR error");
						// see read(2) for a description of EINTR
					} else {
						// some error we are not prepared to handle
						perror("read failed");

						buf = buf_start;  // reset to begining
						return -1; // error
					}
				} else if (0 == n) {
					if (DEBUG) { cout << "  0 read\n"; }

					errs++;
				} else if (n > 0) {
					// got some good data
					if (DEBUG) { cout << "  got: " << n << "\n"; }

					errs = 0; // reset

					buf += n;
					nr += n;
				}

				if (nr == size) {
					break;
				}
			}
			if (errs >= max_errs) {
				cerr << "sread() reached max_errs of " << errs << " got " << nr << " bytes\n";
				buf = buf_start;  // reset to begining
				return -1; // error
			}

			buf = buf_start;  // reset to begining
			return nr; 		  // OK
		}
		// }}}

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
			devfd = -1;
		}
		// }}}

		// {{{ ~MSQSerial
		~MSQSerial() {
			if (devfd > -1)
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

			// get current options
			tcgetattr(devfd, &options);

			// flush both
			tcflush(devfd, TCIOFLUSH);

			//cfsetispeed(&options, 0);  // 0 -> same as ospeed
			cfsetispeed(&options, B115200);
			cfsetospeed(&options, B115200);

			// Control options (c_cflag)
			options.c_cflag &= ~( PARENB | CSTOPB | CSIZE );
			options.c_cflag |= CLOCAL | CREAD;
			// No parity (8N1)
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;

			// Line options (clflag)
			options.c_lflag = ~(ICANON | ECHO | ECHOE | ECHONL | IEXTEN | ISIG);

			// Input options (c_iflag)
			// disable software flow control
			options.c_iflag &= ~(IXON | IXOFF | IXANY);

			// Output options (c_oflag)
			options.c_oflag &= ~OPOST;

			// Control characters
			options.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
			options.c_cc[VERASE]   = 0;     /* del */
			options.c_cc[VKILL]    = 0;     /* @ */
			options.c_cc[VEOF]     = 0;     /* Ctrl-d */
			options.c_cc[VEOL]     = 0;     /* '\0' */
			options.c_cc[VMIN]     = 0;     
			options.c_cc[VTIME]    = 1;     /* 100ms timeout */

			// write, verify, retry if needed
			int i = 0;
			struct termios voptions;
			for (i = 0; i < 5; i++) {
				tcsetattr(devfd, TCSANOW, &options);

				tcgetattr(devfd, &voptions);

				if (B115200 != cfgetispeed(&voptions)) {
					cerr << "cfsetispeed failed\n";
					continue;  // retry
				}

				if (B115200 != cfgetospeed(&voptions)) {
					cout << "cfsetospeed failed\n";
					continue;  // retry
				}

				break;  // success
			}
			if (i == 5) {
				cerr << "failed to configure serial port\n";
				return true;  // error
			}

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
			if (DEBUG) { cout << "cmd_r()\n"; }

			int max_tries = 2;
			int tries = 0;

			int n;
			while (tries++ < max_tries) {
				char _buf[7];
				char* buf;
				buf = &_buf[0];

				*buf = 114;  // 'r'
				buf++;

				*buf = 0;
				buf++;

				*buf = (unsigned char) tbl_idx;
				buf++;

				buf = &_buf[0]; /* reset to begining */

				n = write(devfd, buf, 3);
				if (n < 3) {
					cerr << "write of r command failed!\n";
					tcflush(devfd, TCIOFLUSH);
					continue;
				}

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
				if (n < 4) {
					cerr << "write of r command failed!\n";
					tcflush(devfd, TCIOFLUSH);
					continue;
				}

				n = sread(devfd, msg, num_bytes, 10);
				if (n < num_bytes) {
					cerr << "cmd_r sread() returned too few bytes, got " << n << ", expecting " << num_bytes << endl;
					tcflush(devfd, TCIOFLUSH);
					continue;
				}

				break;
			}
			if (tries >= max_tries) {
				cerr << "cmd_r max_tries of " << tries << " reached\n";
				return -1;  // error
			}

			return n;  // OK
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
			if (n < 3) {
				fputs("write of w command failed!\n", stderr);
				return -1;  // error
			}

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
				return -1;  // error
			}

			for (int i = 0; i < num_bytes; i++) {
				n = write(devfd, bytes, 1);
				bytes++;
				if (n != 1) {
					cerr << "cmd_w(), write error\n";
					return -1;  // error
				}
			}

			return 0;  // OK
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
			int tries = 0;
			int max_tries = 2;
			
			while (tries++ < max_tries) {
				n = write(devfd, "A", 1);
				if (n != 1)  {
					cerr << "write of A command failed\n";
					tcflush(devfd, TCIOFLUSH);	
					continue;
				}

				n = sread(devfd, buf, num_bytes, 5);
				if (n != num_bytes) {
					cerr << "error reading result of A command\n";
					tcflush(devfd, TCIOFLUSH);	
				}

				break;
			}
			if (tries >= max_tries) {
				cerr << "cmd_A() max_tries of " << max_tries << "exceeded\n";
				return true;  // error
			}

			return false;  // OK
		}
		// }}}

		// {{{ cmd_b
		/**
		 * Burn the ram copy of a table in to flash memory.
		 *
		 * @arg table index
		 *
		 * @returns false on success, true on error
		 */
		bool cmd_b(const int tbl_idx)
		{
			int n;  // read/write counts
			char _buf[2];
			char *buf;
			buf = &_buf[0];
			
			*buf = 'b';
			buf++;

			// can-id
			*buf = 0;
			buf++;

			*buf = (unsigned char) tbl_idx;

			buf = &_buf[0]; // reset to begining

			n = write(devfd, buf, 3);
			if (n < 3)  {
				cerr << "write of b command failed\n";
				return true;  // error
			}

			return false;  // OK
		}
		// }}}

};

