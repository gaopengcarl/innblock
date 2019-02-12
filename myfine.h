/*************************************************************************
  > File Name: myfine.h
  > Author: gaopeng QQ:22389860 all right reserved
  > Mail: gaopp_200217@163.com
  > Created Time: Thu 28 Sep 2017 03:14:20 PM CST
 ************************************************************************/
#ifndef __HEADER__
#define __HEADER__
#include<iostream>
#include<stdio.h>
#include<stdint.h>
#include <string.h>
#define KB 10
#define byte unsigned char
//#define DEBUG //debug only

#define ERRPRINT(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#define NOWN     0X000000000000000F
#define DELF     0X0000000000000020
#define MINF     0X0000000000000010
#define HEAP     0X000000000000FFF8
#define RECT     0X0000000000000007
#define LSN_HIGH 0X00000000FFFFFFFF
#define HEAP_N   0X0000000000007FFF

typedef uint64_t	ulint;
typedef uint64_t ib_uint64_t;

#endif
