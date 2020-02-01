
#ifndef ENDIAN_H_


#define ENDIAN_H_

#include <endian.h>
#include "../c_vars.h"
	 
#if __BYTE_ORDER == __BIG_ENDIAN

#define ReadBE16(x) (x)
#define ReadBE32(x) (x)
#define ReadBE64(x) (x)

#define ReadLE16(x) ( ((((uint16)x) >> 8) & (uint16)0x00FF) | ((((uint16)x) << 8) & (uint16)0xFF00) )

#define ReadLE32((x)) \
 ( ((((uint32)x) >> 24) & (uint32)0x000000FF) | ((((uint32)x) >>  8) & (uint32)0x0000FF00) | \
   ((((uint32)x) <<  8) & (uint32)0x00FF0000) | ((((uint32)x) << 24) & (uint32)0xFF000000) )

#define ReadLE64((x)) \
 ( ((((uint64)x) >> 56) & (uint64)0x00000000000000FF) | ((((uint64)x) >> 40) & (uint64)0x000000000000FF00) | \
   ((((uint64)x) >> 24) & (uint64)0x0000000000FF0000) | ((((uint64)x) >>  8) & (uint64)0x00000000FF000000) | \
   ((((uint64)x) <<  8) & (uint64)0x000000FF00000000) | ((((uint64)x) << 24) & (uint64)0x0000FF0000000000) | \
   ((((uint64)x) << 40) & (uint64)0x00FF000000000000) | ((((uint64)x) << 56) & (uint64)0xFF00000000000000) )

#else

#define ReadBE16(x) ( ((((uint16)x) >> 8) & (uint16)0x00FF) | ((((uint16)x) << 8) & (uint16)0xFF00) )

#define ReadBE32(x)  \
 ( ((((uint32)x) >> 24) & (uint32)0x000000FF) | ((((uint32)x) >>  8) & (uint32)0x0000FF00) | \
   ((((uint32)x) <<  8) & (uint32)0x00FF0000) | ((((uint32)x) << 24) & (uint32)0xFF000000) )

#define ReadBE64(x)  \
 ( ((((uint64)x) >> 56) & (uint64)0x00000000000000FF) | ((((uint64)x) >> 40) & (uint64)0x000000000000FF00) | \
   ((((uint64)x) >> 24) & (uint64)0x0000000000FF0000) | ((((uint64)x) >>  8) & (uint64)0x00000000FF000000) | \
   ((((uint64)x) <<  8) & (uint64)0x000000FF00000000) | ((((uint64)x) << 24) & (uint64)0x0000FF0000000000) | \
   ((((uint64)x) << 40) & (uint64)0x00FF000000000000) | ((((uint64)x) << 56) & (uint64)0xFF00000000000000) )

#define ReadLE16(x) (x)
#define ReadLE32(x) (x)
#define ReadLE64(x) (x)

#endif

#endif