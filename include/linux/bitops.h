#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H

#ifdef __BAREBOX__
#ifndef __ASSEMBLY__
#define BIT(nr)			(1UL << (nr))
#else
#define BIT(nr)			(1 << (nr))
#endif
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#endif

#include <asm/bitops.h>


#endif
