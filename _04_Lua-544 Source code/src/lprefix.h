/*
** $Id: lprefix.h $
** 有关 Lua 代码的定义必须在任何其他头文件之前
** See Copyright Notice in lua.h
*/

#ifndef lprefix_h
#define lprefix_h

/*
** Allows POSIX/XSI stuff
*/
#if !defined(LUA_USE_C89) /* { */

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#elif _XOPEN_SOURCE == 0
#undef _XOPEN_SOURCE /* use -D_XOPEN_SOURCE=0 to undefine it */
#endif

/*
** Allows manipulation of large files in gcc and some other compilers
** 允许在 gcc 和其他编译器中操作大文件
*/
#if !defined(LUA_32BITS) && !defined(_FILE_OFFSET_BITS)
#define _LARGEFILE_SOURCE 1

/*
	大文件偏移位数
*/
#define _FILE_OFFSET_BITS 64
#endif

#endif /* } */

/*
** Windows stuff
*/
#if defined(_WIN32) /* { */

#if !defined(_CRT_SECURE_NO_WARNINGS)

/*
 	avoid warnings about ISO C functions
	避免关于 ISO C 函数的警告
*/
#define _CRT_SECURE_NO_WARNINGS
#endif

#endif /* } */

#endif
