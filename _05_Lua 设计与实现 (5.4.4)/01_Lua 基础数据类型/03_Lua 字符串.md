### Lua 字符串

---
- [1. Lua 中的字符串](#1-lua-中的字符串)
- [2. 字符串的实现](#2-字符串的实现)

---
## 1. Lua 中的字符串

- 一般来说，要表示一个字符串，其核心就是字符串的长度，和指向存放字符串内存数据的指针
- 在 Lua 中，字符串实际上是被内化的一种数据（每个 Lua 字符串变量仅保存这份字符串数据的引用）。每当新创建一个字符串时，先是检查当前系统中是否存在一份相同的字符串数据。存在时直接复用，否则创建出一份新的字符串数据。因此 Lua 中字符串是不可变数据

```lua
a = "1"
a = a .. "2"
-- 此时系统中存在两个字符串："1", "12"
```

- 在 Lua 虚拟机中存在一个全局的数据区（散列桶），用来存放当前系统中所有的字符串，Lua 变量仅保存字符串的引用，而不是实际内容
- 字符串内化方案的优点在于，进行字符串数据的比较和查找操作时，其核心是比较或查找字符串的散列值（整数比较）

---
## 2. 字符串的实现

> 字符串的数据结构定义

```c
// lobject.h
// Header for a string value.
typedef struct TString {
  CommonHeader;
  lu_byte extra;  		// 标记 Lua 虚拟机的中保留字，对于长字符串则是用哈希值
  lu_byte shrlen;  		// 短字符串的长度

  unsigned int hash;	// 该字符串的散列值
  union {
    size_t lnglen;  	// 长字符串的长度
    struct TString *hnext;	// 关联散列表下一个 TString
  } u;
  char contents[1];		// 关联实际的字符串
} TString;
```

> 保存字符串的全局散列桶

```c
// lstate.h
typedef struct stringtable {
  TString **hash;
  int nuse;  		/* number of elements */
  int size;
} stringtable;
```

