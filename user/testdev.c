#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/file.h"

int null_fd, zero_fd, kmesg_fd, random_fd, disk_fd;

void
openDevs()
{
	null_fd = open("/dev/null", O_RDWR);
	zero_fd = open("/dev/zero", O_RDWR);
	
	// nije dozvoljeno otvaranje kmesg osim kao readonly (sysfile.c/sysopen())
	kmesg_fd = open("/dev/kmesg", O_RDONLY);
	/*if (open("/dev/kmesg", O_RDWR) < 0)
		printf("nesto nije okej\n");
	else
		kmesg_fd = open("/dev/kmesg", O_RDWR);*/
	
	random_fd = open("/dev/random", O_RDWR);

	disk_fd = open("/dev/disk", O_RDWR);
}

// /dev/null i /dev/zero
void
test2()
{
	char buf1[] = "test null";
	write(null_fd, buf1, sizeof(buf1));
	char buf2[5] = {0};
	read(null_fd, buf2, sizeof(buf2));
	printf("null:\n%s\n", buf2);

	char buf3[] = "test zero";
	write(zero_fd, buf3, 7);
	char buf4[5];
	read(null_fd, buf4, sizeof(buf4));
	printf("zero:\n%s\n", buf4);
}

// /dev/kmesg
void
test3()
{
	char buf1[20] = {0}, buf2[30] = {0}, buf3[50] = {0};

	read(kmesg_fd, buf1, sizeof(buf1)-1);
	printf("kmesg:\n%s\n", buf1);

	printf("------------------------\n");

	read(kmesg_fd, buf2, sizeof(buf2)-1);
	printf("kmesg:\n%s\n", buf2);

	printf("------------------------\n");

	read(kmesg_fd, buf3, 14);
	printf("kmesg:\n%s\n", buf3);

	printf("------------------------\n");

	char buf[KMESG_BUF];

	read(kmesg_fd, buf, sizeof(buf)-1);
	printf("kmesg:\n%s\n", buf);

	write(kmesg_fd, buf, sizeof(buf));
}

#define TESTRSZ 10

void
test4()
{
	
	char rbuf[TESTRSZ];

	// citanje iz /dev/random u bafer
	read(random_fd, rbuf, sizeof(rbuf));
	//ispis celobrojnih vrednosti dobijenih random brojeva
	printf("random int vrednosti : ");
	for (int i = 0; i<TESTRSZ; i++)
		printf("%d ", rbuf[i]);
	printf("\n");
	//ispis dobijenih random brojeva kao char
	printf("random char vrednosti : %s\n", rbuf);

	// ponovno citanje kako bi se prikazala nasumicnost iako se ne pritiska tastatura u medjuvremenu

	read(random_fd, rbuf, sizeof(rbuf));
	printf("random int vrednosti : ");
	for (int i = 0; i<TESTRSZ; i++)
		printf("%d ", rbuf[i]);
	printf("\n");
	printf("random char vrednosti : %s\n", rbuf);

	// kada se pozove ispis u /dev/random desava se skoro ista stvar kao i kod citanja,
	// ali vrednosti koje su ispisane uticu na dalju nasumicnost
	write(random_fd, rbuf, sizeof(rbuf));
	printf("random int vrednosti : ");
	for (int i = 0; i<TESTRSZ; i++)
		printf("%d ", rbuf[i]);
	printf("\n");
	printf("random char vrednosti : %s\n", rbuf);
}

void test61();

void
test5()
{
	test61();

	//int fd = open("/home/README", O_RDONLY);
	//int fd = open("/home/README2", O_RDWR);

	char buf1[20] = {0};


	lseek(disk_fd, 65, SEEK_SET);
	read(disk_fd, buf1, 19);
	printf("PRVI READ: %s\n", buf1);
	for (int i = 0; i<19; i++)
		printf("%d ", buf1[i]);
	printf("\n");


	lseek(disk_fd, 647, SEEK_SET);
	read(disk_fd, buf1, 19);
	printf("DRUGI READ: %s\n", buf1);


	read(random_fd, buf1, 19);
	printf("upisacu %s\n", buf1);
	lseek(disk_fd, 647, SEEK_SET);
	write(disk_fd, buf1, 8);

	lseek(disk_fd, 647, SEEK_SET);
	read(disk_fd, buf1, 19);
	printf("TRECI READ: %s\n", buf1);;

}

void
test61()
{
	int fd = open("/home/README", O_RDONLY);
	int fd2 = open("/home/README2", O_CREATE | O_RDWR);

	char buf[2171];

	read(fd, buf, sizeof(buf));
	write(fd2, buf, sizeof(buf));

}

void
test6()
{
	test61();

	//int fd = open("/dev/kmesg", O_RDONLY);
	int fd = open("/home/README2", O_RDWR);

	char buf1[20] = {0};

	lseek(fd, 23, SEEK_SET);

	read(fd, buf1, 19);

	printf("PRVI READ: %s\n", buf1);


	char buf[] = "okej, ovo onda neka zapravo ima smisla";

	//read(random_fd, buf, sizeof(buf));

	lseek(fd, 50, SEEK_END);

	write(fd, buf, sizeof(buf));

	//fprintf(fd, "ODAVDE KRECE ISPIS %s", buf);
	//printf("ovo se ispisuje %s\n", buf);


	lseek(fd, -27, SEEK_END);

	char buf2[20] = {0};

	read(fd, buf2, 19);
	printf("DRUGI READ: %s\n", buf2);

	for (int i = 0; i<19; i++)
		printf("%c ", buf2[i]);
	printf("\n");

	char buf3[20] = {0};

	lseek(fd, 70, SEEK_END);

	int p1 = read(fd, buf3, 19);

	printf("p1 %d\n", p1);;
	
	printf("TRECI READ: %s\n", buf3);
	for (int i = 0; i<19; i++)
		printf("%d ", buf3[i]);
	printf("\n");




	char buf12[20] = {0};

	lseek(kmesg_fd, 23, SEEK_SET);

	read(kmesg_fd, buf12, 19);

	printf("PRVI READ: %s\n", buf12);

	lseek(kmesg_fd, 15, SEEK_CUR);

	//lseek(kmesg_fd, -5, SEEK_END);

	char buf22[20] = {0};

	read(kmesg_fd, buf22, 19);
	printf("DRUGI READ: %s\n", buf22);

	for (int i = 0; i<19; i++)
		printf("%d ", buf22[i]);
	printf("\n");

	lseek(kmesg_fd, -5, SEEK_END);

	char buf221[20] = {0};

	read(kmesg_fd, buf221, 19);
	printf("TRECI READ: %s\n", buf221);

	for (int i = 0; i<19; i++)
		printf("%d ", buf221[i]);
	printf("\n");

	char buf32[20] = {0};

	lseek(kmesg_fd, -3, SEEK_END);

	int p2 = read(kmesg_fd, buf32, 19);

	printf("p2 %d\n", p2);
	
	printf("CETVRTI READ: %s\n", buf32);
	for (int i = 0; i<19; i++)
		printf("%d ", buf32[i]);
	printf("\n");
}

void
test7()
{
	clrscr();
}

void
test8()
{
	int x, y;

	getcp(&x, &y);

	printf("U trenutku poziva kursor je bio na poziciji %d %d\n", y, x);
}

void
test9()
{
	int x = 40, y = 7;

	setcp(x, y);
}

int
main(int argc, char *argv[])
{
	openDevs();

	printf("U testdev.c se nalaze test funkcije za svaku stavku domaceg.\nPozovi u mainu su zakomentarisani.\n");
	
	// major broj 2 - /dev/null i /dev/zero
	//test2();

	// major broj 3 - /dev/kmesg
	//test3();

	// major broj 4 - /dev/random
	//test4();

	// major broj 5 - /dev/disk
	//test5();

	// test za lseek
	//test6();

	// test za clrscr
	//test7();

	// test za getcp
	//test8();

	// test za setcp
	//test9();


	exit();
}
