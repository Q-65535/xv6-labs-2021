#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(char *argv[]) {
	int p[2];
	char buf;
	pipe(p);
	int pid = fork();
	// Child process
	if (pid == 0) {
		while (read(p[0], &buf, 1) == 0) {  // wait for the parent to send a ping
			continue;
		}
		printf("%d: received ping\n", getpid());
		printf("child(%d): the received character is %c\n", getpid(), buf);
		buf = 'b';
		printf("child(%d): the character to be sent is %c\n", getpid(), buf);
		write(p[1], &buf, 1);
	// Parent process
	} else {
		buf = 'a';
		printf("parent(%d): the character to be sent is %c\n", getpid(), buf);
		write(p[1], &buf, 1);
		/* int child_pid = wait(0); */
		while (read(p[0], &buf, 1) == 0) {  // wait for the child to send a pong
			continue;
		}
		printf("%d: received pong\n", getpid());
		printf("parent(%d): the received character is %c\n", getpid(), buf);
	}
	exit(0);
	return 0;
}
