/**
 * @File Name: memleak_hw.c
 * @Brief : ÄÚ´æÐ¹Â©¼ì²â¹¤¾ß
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-31
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#define MEMLEAK_1_0 1
#define MEMLEAK_1_1 1
#define MEMLEAK_2_0 1
#define MEMLEAK_3_0 1

#if MEMLEAK_3_0
#include <mcheck.h>
#endif

#if MEMLEAK_1_0

#endif

#if MEMLEAK_1_1

#endif

#if MEMLEAK_2_0

#endif

#if MEMLEAK_3_0

#endif

int main()
{
#if MEMLEAK_1_0 || MEMLEAK_1_1

#endif

#if MEMLEAK_2_0

#endif

#if MEMLEAK_3_0

#endif
}