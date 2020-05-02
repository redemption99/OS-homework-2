#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "buf.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

// funkcija read za major broj 2
int
dev2read(struct inode *ip, char *dst, int n)
{	
	// ako je /dev/null
	if (ip -> minor == 1)
	{
		dst[0] = 0;
		return 0;
	}
	// ako je /dev/zero
	
	for (int i = 0; i<n; i++)
		dst[i] = 0;

	return n;
	
}

// funkcija write za major broj 2
int
dev2write(struct inode *ip, char *buf, int n)
{
	return 0;
}

/*
	Pisanje u kmesg_buf se izvrsava u console.c u funkciji kmesgputc()
*/

// funkcija read za major broj 3
int
dev3read(struct inode *ip, char *dst, int n)
{
	uint target;
	int c;

	if (kmesg_buf.off > kmesg_buf.end)
		return -1;

	target = n;

	while(n > 0){
		if (kmesg_buf.off == kmesg_buf.end){
			return target - n;
		}

		c = kmesg_buf.buf[kmesg_buf.off++ % KMESG_BUF];

		*dst++ = c;
		--n;
	}

	//*dst++ = 0;

	return target - n;
}

// funkcija write za major broj 3
int
dev3write(struct inode *ip, char *buf, int n)
{
	return 0;
}

/* 
	Generisanje random brojeva izvodi se preko linearnog kongruentnog generatora (LKG)

	X[n] = (a * X[n-1] + c) % rmod
	rmod = 256 (trebaju nam 8bitni brojevi)
	X[0] je jednako vremenu prvog poziva alldevsinit() (ticks % 256)

	A je 74729, random prost broj

	Kako bi spoljasnji faktori (tastatura) uticali na nasumicnost  c ima vrednost 
	prvog veceg ili jednakog neparnog broja (kako bi bio uzajamno prost sa 256) od 
	proizvoda koda pritisnutog tastera i vremenskog trenutka (ticks) (kada se pritisne tastatura)
	ili zbira ticks i indeksa broja koji se generise u trenutnom pozivu (kada se pozove citanje iz /dev/random)
	ili proizvoda ticks i vrednosti u baferu (kada se pozove pisanje u /dev/random)
*/

int X;

#define rmod 256 

void
updateRandom(int c)
{
	X = (74729 * X + ticks*c + 1 - (ticks*c) % 2) % rmod;
}

// funkcija read za major broj 4
int
dev4read(struct inode *ip, char *dst, int n)
{

	for (int i = 0; i<n; i++) 
	{
		X = (74729 * X + ticks+i + 1 - (ticks+i) % 2) % rmod;
		dst[i] = X;
	}

	//dst[n] = '\0';

	return n;
}

// funkcija write za major broj 4
int
dev4write(struct inode *ip, char *buf, int n)
{
	
	for (int i = 0; i<n; i++) 
	{
		X = (74729 * X + ticks*buf[i] + 1 - (ticks*buf[i])%2) % rmod;
		buf[i] = X;
	}

	//buf[n] = '\0';

	return n;
}

// funkcija read za major broj 5
int
dev5read(struct inode *ip, char *dst, int n)
{
	uint tot, m;
	struct buf *blok;

	if (disk.off >= sb.size*BSIZE || disk.off + n < disk.off)
		return -1;

	if (disk.off + n > sb.size*BSIZE)
		n = sb.size*BSIZE - disk.off;

	for(tot=0; tot<n; tot+=m, disk.off+=m, dst+=m){
		blok = bfind(disk.off/BSIZE);
		m = min(n - tot, BSIZE - disk.off%BSIZE);
		memmove(dst, blok->data + disk.off%BSIZE, m);
		brelse(blok);
	}

	return n;
}

// funkcija write za major broj 5
int
dev5write(struct inode *ip, char *buff, int n)
{
	uint tot, m;
	struct buf *blok;

	if (disk.off >= sb.size*BSIZE || disk.off + n < disk.off)
		return -1;

	if (disk.off + n > sb.size*BSIZE)
		n = sb.size*BSIZE - disk.off;

	for(tot=0; tot<n; tot+=m, disk.off+=m, buff+=m){
		blok = bfind(disk.off/BSIZE);
		m = min(n - tot, BSIZE - disk.off%BSIZE);
		memmove(blok->data + disk.off%BSIZE, buff, m);
		bwrite(blok);
		log_write(blok);
		brelse(blok);
	}

	return n;
}

// consoleinit
void
alldevsinit(void)
{

	devsw[DEVNULL].write = dev2write;
	devsw[DEVNULL].read = dev2read;

	devsw[DEVKMESG].write = dev3write;
	devsw[DEVKMESG].read = dev3read;

	devsw[DEVRANDOM].write = dev4write;
	devsw[DEVRANDOM].read = dev4read;

	devsw[DEVDISK].write = dev5write;
	devsw[DEVDISK].read = dev5read;

	X = ticks % rmod;
}

