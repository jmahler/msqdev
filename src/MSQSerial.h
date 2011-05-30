#pragma once

#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
/* http://www.easysw.com/~mike/serial/serial.html */
#include <sys/ioctl.h>

int usleep(__useconds_t usec);  // to quiet the compiler

/**
 * Megasquirt serial device communications.
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
		// TODO - What is the Q command for?
		/**
		 * Run the "Q" command and get the result.
		 */
		int cmd_Q(char* buf)
		{
			int n;

			n = write(devfd, "Q", 1);
			if (n != 1)
				fputs("write of Q command failed!\n", stderr);

			n = sread(devfd, buf, 20, 3);
			if (n < 0) {
				return -1;  // error
			}

			return n;  // OK
		}
		// }}}

		// {{{ cmd_S
		/**
		 * Run the "S" command and get the result.
		 */
		int cmd_S(char* buf)
		{
			int n;
			
			n = write(devfd, "S", 1);
			if (n < 0)
				fputs("write of S command failed!\n", stderr);

			n = sread(devfd, buf, 60, 5);
			if (n < 0) {
				return -1;  // error
			}

			return n;  // OK
		}
		// }}}

		// {{{ cmd_r
		/**
		 * Run the "r" command to read data from the ecu.
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
		 * Run the "w" command and get the result.
		 * The w command is similar to the r command except that
		 * it writes instead of reading.
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

};

