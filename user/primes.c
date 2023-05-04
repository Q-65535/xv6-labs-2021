
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define RD 0
#define WR 1
#define INT_SIZE sizeof(int)

void primes(int read_pipe[]);

int main(int argc, char *argv[]) {
	int p[2];
	pipe(p);
	int pid = fork();
	// child
	if (pid == 0) {
		//@Note: Remember to close the write descriptor in child process!!
		close(p[WR]);
		primes(p);
	// parent
	} else {
		for (int i = 2; i <= 35; i++) {
			write(p[WR], &i, INT_SIZE);
		}
		close(p[WR]);
		wait(0);
	}
	exit(0);
	return 0;
}

void primes(int read_pipe[]) {
	int first_num;
	if (read(read_pipe[RD], &first_num, INT_SIZE) == 0) {
		close(read_pipe[RD]);
		return;
	}
	printf("prime %d\n", first_num);
	int write_pipe[2];
	pipe(write_pipe);
	int pid = fork();
	// child
	if (pid == 0) {
		//@Note: Remember to close the write descriptor in child process!!
		close(write_pipe[WR]);
		primes(write_pipe);
	// parent
	} else {
		int num;
		while (read(read_pipe[RD], &num, INT_SIZE)) {
			if (num % first_num != 0) {
				write(write_pipe[WR], &num, INT_SIZE);
			}
		}
		close(read_pipe[RD]);
		close(write_pipe[WR]);
		wait(0);
	}
	return;
}
