#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/file.h"

int
strncmp(char *s1, char *s2, int n)
{
	for (int i = 0; i < n; i++)
		if (s1[i] != s2[i])
			return s1[i] - s2[i];

	return 0;
}

void
mystrcpy(char *dest, char *src, int n)
{
	int i;

	for (i=n; src[i] != 0; i++)
		dest[i-n] = src[i];

	dest[i-n] = 0;
}

int
myatoi(char *s, int n)
{
	int ret = 0;

	for (int i=n; s[i] != 0; i++)
		ret = ret * 10 + s[i] - '0';

	return ret;
}

int
main(int argc, char *argv[])
{

	char input_file[50] = {0};
	char output_file[50] = {0};
	int in_fd = 0;
	int out_fd = 1;
	int bs = 512;
	int count = -1;
	int skip = 0;
	int seek = 0;

	for (int i=1; i<argc; i++) {

		if (strncmp(argv[i], "if=", strlen("if=")) == 0)
			mystrcpy(input_file, argv[i], strlen("if="));
		
		if (strncmp(argv[i], "of=", strlen("of=")) == 0)
			mystrcpy(output_file, argv[i], strlen("of="));

		if (strncmp(argv[i], "bs=", strlen("bs=")) == 0)
			bs = myatoi(argv[i], strlen("bs="));

		if (strncmp(argv[i], "count=", strlen("count=")) == 0)
			count = myatoi(argv[i], strlen("count="));

		if (strncmp(argv[i], "skip=", strlen("skip=")) == 0)
			skip = myatoi(argv[i], strlen("skip="));

		if (strncmp(argv[i], "seek=", strlen("seek=")) == 0)
			seek = myatoi(argv[i], strlen("seek="));
	}

	int f1, f2;

	if ((f1 = open(input_file, O_RDONLY)) > -1)
		in_fd = f1; 

	if ((f2 = open(output_file, O_RDWR | O_CREATE)) > -1)
		out_fd = f2; 


	/*printf("input file %s\n", input_file);
	printf("output file %s\n", output_file);

	printf("in_fd %d\n", in_fd);
	printf("out_fd %d\n", out_fd);
	printf("bs %d\n", bs);
	printf("count %d\n", count);
	printf("skip %d\n", skip);
	printf("seek %d\n", seek);*/

	lseek(in_fd, skip*bs, SEEK_SET);
	lseek(out_fd, seek*bs, SEEK_SET);

	int blu = 0, bu = 0;

	while (count != 0)
	{
		char *bufi = (char *) malloc(bs); 
		
		int rr = read(in_fd, bufi, bs);

		int rw = write(out_fd, bufi, rr);

		blu++;
		bu+=rw;

		if (count > 0)
			count--;
		else
			if (rr < bs || rw < 0) //ispisan je poslednji blok
				break;
	}

	fprintf(2, "\nUkupno blokova: %d, ukupno bajtova: %d\n", blu, bu);

	exit();
}
