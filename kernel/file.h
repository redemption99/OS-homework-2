struct file {
	enum { FD_NONE, FD_PIPE, FD_INODE } type;
	int ref; // reference count
	char readable;
	char writable;
	struct pipe *pipe;
	struct inode *ip;
	uint off;
};


// in-memory copy of an inode
struct inode {
	uint dev;           // Device number
	uint inum;          // Inode number
	int ref;            // Reference count
	struct sleeplock lock; // protects everything below here
	int valid;          // inode has been read from disk?

	short type;         // copy of disk inode
	short major;
	short minor;
	short nlink;
	uint size;
	uint addrs[NDIRECT+1];
};

// table mapping major device number to
// device functions
struct devsw {
	int (*read)(struct inode*, char*, int);
	int (*write)(struct inode*, char*, int);
};

#define KMESG_BUF 512

// kmesg buf
struct kmesgbuf{
	char buf[KMESG_BUF];
	uint off;  // offset
	uint end;  // end of file
};

// disk 
struct diskbuf{
	uint off;  // offset 
	uint end;  // end of disk
};

extern struct devsw devsw[];
extern struct kmesgbuf kmesg_buf;
extern struct diskbuf disk;

#define CONSOLE 1

// novi major brojevi

#define DEVNULL 2
#define DEVZERO 2
#define DEVKMESG 3
#define DEVRANDOM 4
#define DEVDISK 5

// za whence
#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3