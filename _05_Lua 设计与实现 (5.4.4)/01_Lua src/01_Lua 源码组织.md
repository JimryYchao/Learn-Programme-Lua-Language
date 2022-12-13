### Lua 源码组织

---
- [1. 源码组织](#1-源码组织)
	- [1.1 虚拟机核心相关文件](#11-虚拟机核心相关文件)
	- [1.2 内嵌库相关文件](#12-内嵌库相关文件)
	- [1.3 解析器、字节码相关文件](#13-解析器字节码相关文件)
- [2. Lua 虚拟机工作流程](#2-lua-虚拟机工作流程)

---
## 1. 源码组织

- Lua 源码大体分为三个部分：虚拟机核心、内嵌库以及解释器、编译器

```makefile
LUA_A=	liblua.a
CORE_O=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o \
 	lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o \
 	lundump.o lvm.o lzio.o
LIB_O=	lauxlib.o lbaselib.o ldblib.o liolib.o lmathlib.o loadlib.o linit.o \
	loslib.o lstrlib.o ltablib.o lutf8lib.o lcorolib.o
BASE_O= $(CORE_O) $(LIB_O) $(MYOBJS)

LUA_T=	lua
LUA_O=	lua.o

LUAC_T=	luac
LUAC_O=	luac.o
```

---
### 1.1 虚拟机核心相关文件

```txt
文件名				作用							对外接口前缀
————————————————————————————————————————————————————————————————
lapi.c            C 语言接口                            lua_
lcode.c           源码生成器                            luaK_
ldebug.c		  调试库                                luaG_
ldo.c             函数调用及栈管理                      luaD_
ldump.o           序列化预编译的 Lua 字节码
lfunc.c			  提供操作函数原型及闭包的辅助函数       luaF_
lgc.c			  GC									luaC_
llex.c            词法分析                              luaX_
lmem.c            内存管理                              luaM_
lobject.c         对象管理                              luaO_
lopcodes.c        字节码操作                            luaP_
lparser.c		  分析器                                luaY_
lstate.c 		  全局状态机  						    luaE_
lstring.c		  字符串操作							luaS_
ltable.c		  表操作								luaH_
lundump.c	      加载预编译字节码						luaU_
ltm.c			  tag 方法 								luaT_
lzio.c		      缓存流接口   							luaZ_
————————————————————————————————————————————————————————————————
```

---
### 1.2 内嵌库相关文件

```txt
文件名							作用
—————————————————————————————————————————————————————
lauxlib.c			库编写时需要用到的辅助函数库
lbaselib.c			基础库
ldblib.c			调试库
liolib.c			IO 库
lmathlib.c			数学库
loslib.c			OS 库
ltablib.c			表操作库
lstrlib.c			字符串操作库
loadlib.c			动态扩展库加载器
linit.c				负责内嵌库的初始化
lutf8lib.c			用于 UTF-8 操作的标准库
lcorolib.c			协程库
—————————————————————————————————————————————————————
```

---
### 1.3 解析器、字节码相关文件

```txt
文件名							作用
—————————————————————————————————————————————————————
lua.c				解释器
luac.c				字节码编译器
—————————————————————————————————————————————————————
```

---
## 2. Lua 虚拟机工作流程

- Lua 代码是通过翻译成 Lua 虚拟机能识别的字节码运行的，主要分为

> 翻译代码以及编译为字节码的部分

- 这部分负责将 Lua 代码进行词法分析、语法分析等，最终生成字节码
- 相关功能文件包括 ```llex.c```（词法分析）和 ```lparser.c```（语法分析），最终生成的代码使用了 ```lcode.c``` 文件中的功能
- ```lopcodes.h```、```lopcodes.c``` 中定义了 Lua 虚拟机相关的字节码指令的格式以及相关的 API

> Lua 虚拟机相关部分

- 在第一步中，经过分析阶段之后，生成了对应的字节码；第二步就是将这些字节码装载到虚拟机中执行
- Lua 虚拟机相关的代码在 ```lvm.c```，虚拟机执行的主函数是 ```luaV_execute```

---