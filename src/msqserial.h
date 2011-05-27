
/*#define NDEBUG*/
#include <assert.h>

#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
/* http://www.easysw.com/~mike/serial/serial.html */
#include <sys/ioctl.h>

int usleep(__useconds_t usec);  /* to quiet compiler */

/**
 * msqserial.h
 * Megasquirt serial device communications.
 *
 */

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

		printf("read: %d\n", n);

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

/* versions */
enum {
	MS2EXTRAREL303S = 1
};

typedef struct {
	int devfd;
	int version;
} MsqDev;

/* {{{ msq_connect */
/**
 * msq_connect
 */
int msq_connect(char* dev, MsqDev* msq) {
	int devfd;
	char buf[1000];
	struct termios options;

	/* {{{ open serial port */
	devfd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (devfd == -1) {
		sprintf(buf, "Unable to open device '%s'", dev);
		perror(buf);
		return -1;  /* error */
	} else {
		fcntl(devfd, F_SETFL, 0);
	}

	tcgetattr(devfd, &options);

	cfsetispeed(&options, B115200);
	options.c_cflag |= (CLOCAL | CREAD);

	/* No parity (8N1) */
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	tcsetattr(devfd, TCSANOW, &options);
	/* }}} */

	msq->devfd = devfd;

	msq->version = MS2EXTRAREL303S;
	/* FIXME - expand this with more versions */

	return 0;  /* OK */
}

/* }}} */

/* {{{ close */
void msq_close(MsqDev* msq)
{
	close(msq->devfd);

	return;
}
/* }}} */

/* {{{ cmd_Q */
/**
 * Run the "Q" command and get the result.
 */
int cmd_Q(MsqDev* msq, char* buf)
{
	int n;
	int devfd = msq->devfd;

	n = write(devfd, "Q", 1);
	if (n != 1)
		fputs("write of Q command failed!\n", stderr);

	n = sread(devfd, buf, 20, 3);
	if (n < 0) {
		return -1;  /* error */
	}

	return n;  /* OK */
}
/* }}} */

/* {{{ cmd_S */
/**
 * Run the "S" command and get the result.
 */
int cmd_S(MsqDev* msq, char* buf)
{
	int n;
	int devfd = msq->devfd;
	
	n = write(devfd, "S", 1);
	if (n < 0)
		fputs("write of S command failed!\n", stderr);

	n = sread(devfd, buf, 60, 5);
	if (n < 0) {
		return -1;  /* error */
	}

	return n;  /* OK */
}
/* }}} */

/* {{{ cmd_r */

/**
 * Run the "r" command and get the result.
 *
 * cmd_r(msq, <tbl_idx>, <offset>, <bytes>)
 */

int cmd_r(MsqDev* msq, const int tbl_idx,
			const int offset, const int num_bytes, char* msg)
{
	int n;
	int devfd = msq->devfd;
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
/* }}} */

/* {{{ read_srpm_table1 */
/**
  * Reads the values of the rpm coordiants for advanceTable1.
  */
int read_srpm_table1(MsqDev* msq, unsigned short* xs)
{
	int i, j, n;
	char buf[24];
	unsigned short s;
	
	if (MS2EXTRAREL303S == msq->version) {
		/*
		 * doc/ini/megasquirt-ii.ms2extra.alpha_3.0.3u_20100522.ini
		 * page = 3, tble_idx = 10
		 *
		 *   srpm_table1     = array ,  U16,    576,    [   12], "RPM",      1.00000,   0.00000,  0.00,15000.00, 
		 */
		n = cmd_r(msq, 10, 576, 24, buf);
		if (24 != n) {
			fputs("read_srpm_table1: read error", stderr);
			return -1;
		}

		for (i = 0, j = 0; i < 24; i += 2, j++) {
			/* unsigned short */
			s = buf[i] & 0xFF;
			s = (s << 8);
			s = (buf[i+1] & 0xFF) | s;
			xs[j] = s;

			/* xs[j] = xs[j] * 1.0000; */
		}
	} else {
		fputs("unknown version", stderr);
	}

	return 12;
}
/* }}} */

/* {{{ read_smap_table1 */
/**
  * Reads the values of the rpm coordiants for advanceTable1.
  */
int read_smap_table1(MsqDev* msq, char* buf)
{
	int n;
	
	if (MS2EXTRAREL303S == msq->version) {
		/*
		 * doc/ini/megasquirt-ii.ms2extra.alpha_3.0.3u_20100522.ini
		 * page = 3, tble_idx = 10
		 *
		 *  smap_table1 = array ,  S16,    624,    [   12], "%",      0.10000,   0.00000,  0.00,  400.00,      1 ; * ( 24 bytes)
		 */
		n = cmd_r(msq, 10, 624, 24, buf);
	} else {
		fputs("unknown version", stderr);
	}

	return n;
}
/* }}} */

/* {{{ read_advanceTable1 */
/**
 * Read data from advanceTable1
 */
int read_advanceTable1(MsqDev* msq, char* buf)
{
	int n;
	
	if (MS2EXTRAREL303S == msq->version) {
		n = cmd_r(msq, 10, 0, 288, buf);
	} else {
		fputs("unknown version", stderr);
	}

	return n;
}
/* }}} */

/* {{{ read_veTable1 */
/**
 * Read data from advanceTable1
 */
int read_veTable1(MsqDev* msq, char* buf)
{
	int n;
	
	if (MS2EXTRAREL303S == msq->version) {
		n = cmd_r(msq, 9, 0, 256, buf);
	} else {
		fputs("unknown version", stderr);
	}

	return n;
}
/* }}} */

