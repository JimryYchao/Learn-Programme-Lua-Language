### Lua 中的数据类型

---
- [1. C 中实现通用数据结构的一般做法](#1-c-中实现通用数据结构的一般做法)
- [2. Lua 通用数据结构的实现](#2-lua-通用数据结构的实现)
	- [2.1 GC 对象的表示](#21-gc-对象的表示)
	- [2.2 Lua 动态类型表示](#22-lua-动态类型表示)
	- [2.3 数据转换](#23-数据转换)

---
## 1. C 中实现通用数据结构的一般做法

- 如何使用一个通用的数据结构来表示不同的数据结构，有两种比较常见的做法

```c
// 使用公共的数据结构作为基础类型
struct base{
	int type;
};
struct string{
	struct base info;
	int len;
	char *data[0];
};
struct number{
	struct base info;
	double num;
};

// 使用联合将所有的数据包进来
struct string{
	int len;
	char *data[0];
};
struct number{
	double num;
};
struct value{
	int type;
	union{
		string str;
		number num;
	} value;
};
```

---
## 2. Lua 通用数据结构的实现

> Lua 中定义的数据类型

```c
// basic types in lua.h
#define LUA_TNONE		(-1)		// 无类型

#define LUA_TNIL		0			// 空类型
#define LUA_TBOOLEAN		1		// 布尔类型
#define LUA_TLIGHTUSERDATA	2		// 指针	void *
#define LUA_TNUMBER		3			// 数据	lua_Number
#define LUA_TSTRING		4			// 字符串 TString
#define LUA_TTABLE		5			// 表 Table
#define LUA_TFUNCTION		6		// 函数 CClosure、LClosure
#define LUA_TUSERDATA		7		// 指针 void *
#define LUA_TTHREAD		8			// Lua 虚拟机、协程	lua_state

#define LUA_NUMTYPES		9		// 数据子类型 integer、double
```

---
### 2.1 GC 对象的表示

> Lua 内部用一个宏表示哪些数据类型需要进行 GC 操作

```c
// lobject.h
#define iscollectable(o) (ttype(o) >= LUA_TSTRING)
```

- 这些需要进行 GC 操作的数据类型都会有一个 ```CommonHeader``` 宏定义的成员，例如表的数据类型定义

```c
// lobject.h
typedef struct Table {
  CommonHeader;
  lu_byte flags;  		/* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  	/* log2 of size of 'node' array */
  unsigned int alimit;  /* "limit" of 'array' array */
  TValue *array;  		/* array part */
  Node *node;
  Node *lastfree;  		/* any free position is before this position */
  struct Table *metatable;
  GCObject *gclist;
} Table;
```

> 其中 ```CommonHeader``` 定义为

- Lua 在具体类型中用 ```CommonHeader``` 来存放所有数据类型都通用的字段

```c
/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	struct GCObject *next; lu_byte tt; lu_byte marked
/*
	next	指向下一个 GC 链表的成员
	tt		表示数据的类型，即表示数据类型的宏
	marked	GC 相关的标记位
*/
```

> Lua 中使用了 ```GCUnion``` 联合体囊括了所有的垃圾回收对象

```c
// lobject.h
/* Common type for all collectable objects */
typedef struct GCObject {
  CommonHeader;
} GCObject;

// lstate.h
union GCUnion {
  GCObject gc;  		/* common header */
  struct TString ts;
  struct Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct lua_State th;  /* thread */
  struct UpVal upv;
};
```

> 定位 GC

```c
// lobject.h
#define val_(o)		((o)->value_)
#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)
```

---
### 2.2 Lua 动态类型表示

- Lua 内部用 ```Value``` 将所有的数据组合起来

```c
// Union of all Lua values
typedef union Value {
  struct GCObject *gc;    /* collectable objects */
  void *p;         		  /* light userdata */
  lua_CFunction f; 	 	  /* light C functions */
  lua_Integer i;   		  /* integer numbers */
  lua_Number n;    		  /* float numbers */
} Value;
```

> 通用数据结构

- ```TValue``` 用于统一表示所有数据的数据结构， Lua 中的任何数据都可以通过该结构体表示

```c
// 标记值，这是 Lua 中值的基本表示: 一个实际值加上一个标签及其类型
#define TValuefields	Value value_; lu_byte tt_
typedef struct TValue {
  TValuefields;
} TValue;
```

---
### 2.3 数据转换

- ```TValue``` 用于统一地表示数据，而在 C 中操作数据就需要使用具体的类型。涉及对 ```TValue``` 与具体类型之间转换的代码，其主要逻辑是将 TValue 中的 ```tt_``` 和 ```value_``` 与具体类型的数据进行转换

> number 转换

```c
// lobject
// set float:
#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_VNUMFLT); }
// set integer:
#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_VNUMINT); }
```

---