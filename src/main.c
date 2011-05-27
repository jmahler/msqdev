
#include "msqserial.h"  /* Megasquirt serial library */

int main(int argc, char** argv)
{
    int logfd;
	int err, n, i;
	unsigned short xs[200];
	char* buf = malloc(1000*sizeof(char));
	char* buf_start;
	char* msg = malloc(1000*sizeof(char));
	char* msg_start;
	MsqDev msq;

	buf_start = buf;
	msg_start = msg;

    if (3 != argc) {

        printf("USAGE:\n");
        printf("  a.out <device>  <dir>\n");
        printf("  a.out /dev/ttyUSB0 ~/msqdev/car1-20110406\n");

        return 1;
    }

    /* setup log fd */
	sprintf(buf, "%s/log", argv[2]);
	logfd = open(buf, O_WRONLY | O_APPEND, 0);
	if (-1 == logfd) {
		sprintf(msg, "Unable to open log file '%s'", buf);
		perror(msg);
		return 1;
	}

	/* write first log message */
	buf = "start!\n";
	write(logfd, buf, strlen(buf));

	/* open Megasquirt device */
	err = msq_connect(argv[1], &msq);
	if (err < 0) {
		printf("error opening megasquirt device\n");
	}

	/*
	buf = buf_start;
	n = cmd_Q(&msq, buf);
	if (n >= 0) {
		printf("cmd_Q: [%d] '%s'\n", n, buf);
	}

	buf = buf_start;
	n = cmd_S(&msq, buf);
	if (n >= 0) {
		printf("cmd_S: [%d] '%s'\n", n, buf);
	}
	*/

	buf = buf_start;  /* reset */

	/*
	n = read_advanceTable1(&msq, buf);
	if (n >= 0) {
		printf("read_advanceTable1: successful\n");
	} else {
		printf("read_advanceTable1: failed\n");
	}
	*/

	n = read_veTable1(&msq, buf);
	if (n >= 0) {
		printf("read_veTable1: successful\n");
	} else {
		printf("read_veTable1: failed\n");
	}

	n = read_srpm_table1(&msq, xs);
	for (i = 0; i < n; i++) {
		printf("%u ", xs[i]);
	}
	printf("\n");

	/* don't forget to flush the toilet! */
	msq_close(&msq);
	close(logfd);
	free(buf);
	free(msg);

    return 0;
}

