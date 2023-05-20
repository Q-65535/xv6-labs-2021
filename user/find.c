#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *name) {
	char buf[512];
	char *p;
	int fd;
	struct dirent de;
	struct stat st;

	if ((fd = open(path, 0)) < 0) {
		fprintf(2, ": cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}

	if (st.type != T_DIR) {
		fprintf(2, "this is not a directory: %s\n", path);
		close(fd);
		return;
	}
	strcpy(buf, path);
	p = buf + strlen(buf);
	*p++ = '/';
	// Iterate through the directory.
	while (read(fd, &de, sizeof(de)) == sizeof(de)) {
		if (de.inum == 0) {
			continue;
		}

		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		int stat_num = stat(buf, &st);
		if(stat_num < 0) {
			printf("ls: cannot stat %s\n", buf);
			continue;
		}
		// open the current file
		// @Problem: the following code will cause stat function return -1? wtf?
		/* int sfd; */
		/* if ((sfd = open(buf, 0)) < 0) { */
		/* 	fprintf(2, ": cannot open %s\n", buf); */
		/* 	return; */
		/* } */

		char *cur_file_name = p;
		printf("cur file name: %s\n", buf);

		if (st.type == T_DIR) {
			if (strcmp(cur_file_name, "..") == 0) {
				continue;
			}
			if (strcmp(cur_file_name, ".") == 0) {
				continue;
			}
			printf("hit recursive!\n");
			find(buf, name);
		} else {
			if (strcmp(cur_file_name, name) == 0) {
				printf("%s\n", buf);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("please specify the directory and the file name you want to find\n");
		exit(0);
	}
	find(argv[1], argv[2]);
	exit(0);
}
