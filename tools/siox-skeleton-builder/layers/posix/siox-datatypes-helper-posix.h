#ifndef SIOX_DATATYPES_HELPER_LOW_LEVEL_IO_H_JFTT3HM5
#define SIOX_DATATYPES_HELPER_LOW_LEVEL_IO_H_JFTT3HM5

// Generic flags for file open modes
// apply to: open, creat, dup3,
// TODO: mapping to bits.. 1,2,4,8,16..
enum POSIXOpenFileFlags {
	/* as defined by fcntl.h */
	SIOX_LOW_LEVEL_O_RDONLY=0,
	SIOX_LOW_LEVEL_O_WRONLY=1,
	SIOX_LOW_LEVEL_O_RDWR=2,
	SIOX_LOW_LEVEL_O_APPEND=1024,
	SIOX_LOW_LEVEL_O_ASYNC=8192,
	SIOX_LOW_LEVEL_O_CLOEXEC=524288,
	SIOX_LOW_LEVEL_O_CREAT=64,
	SIOX_LOW_LEVEL_O_DIRECTORY=65536,
	SIOX_LOW_LEVEL_O_EXCL=128,
	SIOX_LOW_LEVEL_O_NOCTTY=256,
	SIOX_LOW_LEVEL_O_NOFOLLOW=131072,
	SIOX_LOW_LEVEL_O_NONBLOCK=2048,
	SIOX_LOW_LEVEL_O_TRUNC=512,

	/* as from manpage open(2), bits are arbitrary
	SIOX_POSIX_MODE_O_RDONLY=0,
	SIOX_POSIX_MODE_O_WRONLY=1,
	SIOX_POSIX_MODE_O_RDWR=2,
	SIOX_POSIX_MODE_O_APPEND=4,
	SIOX_POSIX_MODE_O_ASYNC=8,
	SIOX_POSIX_MODE_O_CLOEXEC=16,
	SIOX_POSIX_MODE_O_CREAT=32,
	SIOX_POSIX_MODE_O_DIRECT=64,
	SIOX_POSIX_MODE_O_DIRECTORY=128,
	SIOX_POSIX_MODE_O_EXCL=256,
	SIOX_POSIX_MODE_O_LARGEFILE=512,
	SIOX_POSIX_MODE_O_NOATIME=,
	SIOX_POSIX_MODE_O_NOCTTY,
	SIOX_POSIX_MODE_O_NOFOLLOW,
	SIOX_POSIX_MODE_O_NONBLOCK,  // or O_NDELAY
	SIOX_POSIX_MODE_O_PATH,
	SIOX_POSIX_MODE_O_TRUNC
	*/
};




#endif /* end of include guard: SIOX_DATATYPES_HELPER_LOW_LEVEL_IO_H_JFTT3HM5 */
