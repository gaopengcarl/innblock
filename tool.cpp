/*************************************************************************
  > File Name: tool.cpp
  > Author: gaopeng QQ:22389860 all right reserved
  > Mail: gaopp_200217@163.com
  > Created Time: Thu 28 Sep 2017 02:01:47 PM CST
 ************************************************************************/

#include<iostream>
#include <stdlib.h>
#include"myfine.h"
using namespace std;


void* aligned_malloc(size_t size, size_t alignment)  
{

	if(alignment & (alignment - 1))  
	{

		return NULL;  
	}  
	else  
	{

		void *praw = malloc(sizeof(void*) + size + alignment);  
		if(praw)  
		{

			void *pbuf = reinterpret_cast<void*>(reinterpret_cast<size_t>(praw) + sizeof(void*));  
			void *palignedbuf = reinterpret_cast<void*>((reinterpret_cast<size_t>(pbuf) | (alignment - 1)) + 1);  
			static_cast<void**>(palignedbuf)[-1] = praw;  
			return palignedbuf;  
		}  
		else  
		{

			return NULL;  
		}  
	}  
}  

void aligned_free(void *palignedmem)  
{

	free(reinterpret_cast<void*>((static_cast<void**>(palignedmem))[-1]));  
}  


ulint
mach_read_from_1(
		/*=============*/
		const byte*     b)      /*!< in: pointer to byte */
{


	return((ulint)(b[0]));
}

ulint
mach_read_from_2(
		/*=============*/
		const byte*     b)      /*!< in: pointer to 2 bytes */
{


	return(((ulint)(b[0]) << 8) | (ulint)(b[1]));
}

ulint
mach_read_from_3(
		/*=============*/
		const byte*     b)      /*!< in: pointer to 3 bytes */
{


	return( ((ulint)(b[0]) << 16)
			| ((ulint)(b[1]) << 8)
			| (ulint)(b[2])
		  );
}

ulint
mach_read_from_4(
		/*=============*/
		const byte*     b)      /*!< in: pointer to four bytes */
{


	return( ((ulint)(b[0]) << 24)
			| ((ulint)(b[1]) << 16)
			| ((ulint)(b[2]) << 8)
			| (ulint)(b[3])
		  );
}

ib_uint64_t
mach_read_from_8(
		/*=============*/
		const byte*     b)      /*!< in: pointer to 8 bytes */
{


	ib_uint64_t     u64;

	u64 = mach_read_from_4(b);
	u64 <<= 32;
	u64 |= mach_read_from_4(b + 4);

	return(u64);
}

int16_t
mach_read_from_2_16(const byte*     b)
{
	return(((int16_t)(b[0]) << 8) | (ulint)(b[1]));
}

void*
ut_align_down(
		const void*     ptr,            /*!< in: pointer */
		ulint           align_no)       /*!< in: align by this number 16384*/ 
{
	return((void*)((((ulint) ptr)) & ~(align_no - 1)));
}


