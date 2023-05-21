#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define stdin 0
#define MAX_ARG_LEN 100

int read_line(char *arg) {
	int index = 0;
	char buf;
	// Read bytes one by one.
	while(read(stdin, &buf, 1) == 1) {
		// The return character means that a command line is complete.
		if (buf == '\n') {
			arg[index] = '\0';
			return 1;
		} else {
			arg[index++] = buf;
		}
	}
	// if end of file, return -1.
	return -1;
}

int main(int argc, char *argv[]) {
	char *args[MAXARG];
	char arg[MAX_ARG_LEN];
	// Copy the prefix args.
	int index = 0;
	for (; index < argc - 1; index++) {
		args[index] = argv[index + 1];
	}
	while (read_line(arg) != -1) {
		args[index] = arg;
		args[index + 1] = 0;

		int pid = fork();
		// parent process
		if (pid != 0) {
			wait(0);
			// child process
		} else {
			int res = exec(args[0], args);
			printf("exe fail res: %d\n", res);
		}

	}
	exit(0);
}
