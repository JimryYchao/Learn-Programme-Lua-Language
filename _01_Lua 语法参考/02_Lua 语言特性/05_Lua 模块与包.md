### Lua 模块与包

---
- [1. 模块系统](#1-模块系统)
	- [1.1 require 函数](#11-require-函数)
	- [1.2 搜索路径](#12-搜索路径)
	- [1.3 搜索器](#13-搜索器)
	- [1.4 构建一个模块](#14-构建一个模块)
- [2. 子模块和包](#2-子模块和包)

---
## 1. 模块系统

- Lua 中模块系统的主要目标之一就是允许不同的人共享代码，缺乏公共规则就无法实现共享。从用户角度来看，一个模块（Module）是由 Lua 或 C 编写的一些代码，这些代码通过函数 ```require``` 加载并创建一个表，这个表包含有从模块中导出的变量。Lua 中所有的标准库都是模块

```lua
mod = require(moduleName)
---------------------------------
local m = require "mod"			--> 引入 mod 模块 >> m.foo()
local f = require "mod".foo		--> 引入 mod 模块中的 foo 函数
```

---
### 1.1 require 函数

- 首先，函数 ```require``` 在表 ```package.loaded``` 中检查模块是否已被加载。加载时返回相应的值。一个模块被加载过，后续的对于同一模块的所有 ```require``` 调用都将返回同一个值，且不会再运行任何代码

- 若模块尚未加载，函数 ```require``` 则搜索指定模块名的 Lua 文件，搜索路径由 ```package.path``` 指定；当函数找到相应的文件，就用函数 ```loadfile``` 将其进行加载，返回一个加载器函数 ```loader```，该加载器是一个用于加载模块的函数
- 若 ```require``` 找不到指定模块，那它就搜索相应名称的 C 标准库，搜索路径由 ```package.cpath``` 指定；当找到相应的 C 标准库时，使用函数 ```package.loadlib``` 进行加载，该函数会查找 ```luaopen_modname``` 的函数，加载函数就是 ```loadlib``` 的执行结果
- 如果加载函数有返回值，那么函数 ```require``` 会返回这个值，并将值保存到表 ```package.loaded``` 中，以便将来在加载同一模块时返回相同的值，避免重复加载模块（模块在任何情况下只加载一次）
- 若要强制函数 ```require``` 加载同一模块两次，可以先将模块从 ```package.loaded``` 中删除（```package.loaded.ModName = nil```）

```lua
local mod = require "Mod"	--> 首次加载
package.loaded["Mod"] = nil --> 卸载 Mod
local M = require "Mod"		--> 再次加载
```

---
### 1.2 搜索路径

- 函数 ```require``` 使用的路径与典型的路径略有不同。典型的路径是很多目录组成的列表，函数 ```require``` 使用的路径是一组模板，其中的每项都指定了将模块名转换为文件名的方式，分号用于将多个模板分隔开

```lua
package.path = [[?;?.lua;c:windows\?;/usr/local/lua/?/?.lua]]
local mod = require "sql"	-- 尝试搜索
	--[[
		spl
		sql.lua
		c:\windows\sql
		/usr/local/lua/sql/sql.lua
	]]

package.cpath = [[.\"?.dll;C:\Program Files\Lua504\dll\?.dll]]
```

- ```package``` 模块加载时，把变量 ```package.path``` 设置为环境变量 ```LUA_PATH_5_4``` 的值，或是 ```LUA_PATH``` 的值，若都未定义，则使用一个编译时定义的默认路径

> ```package.searchpath```

- 函数 ```package.searchpath``` 实现了搜索库的所有规则，该函数的参数包括模块名和路径。若找到文件，就返回第一个存在的文件的文件名，否则返回 ```nil``` 和所有文件都无法打开的错误信息

---
### 1.3 搜索器

- 函数列表 ```package.searchers``` 包含了函数 ```require``` 使用的所有搜索器。在寻找模块时，函数 ```require``` 传入模块名并调用列表中的每一个搜索器。可以向搜索器列表中扩展合适的搜索器函数
- 列表中第一位是预加载搜索器，二、三位的搜索器用于搜索 Lua 文件和 C 标准库，第四个函数只与子模块有关

> 预加载搜索器

- 预加载搜索器（preload）使得我们能够为要加载的模块定义任意的加载函数。预加载搜索器使用表 ```package.preload``` 来映射模块名称和加载函数
- 预加载搜索器只是简单地在表中搜索指定的名称，找到时返回对应的函数作为该模块的加载函数，否则返回 ```nil```
- 使用预加载的方式，程序不会为没有用到的模块浪费资源

---
### 1.4 构建一个模块

```lua
-- Mod.lua : 创建一个模块的一般方法
local M = {}
Mod.Add = function(c1, c2) end
Mod.Sub = function(c1, c2) end
Mod.Mul = function(c1, c2) end
Mod.Inv = function(c1, c2) end
Mod.Div = function(c1, c2) end
return M
-------------------------------
-- other.lua : 加载 Mod 模块
local mod = require "Mod"
```

> 也可以直接加载到包管理器中

```lua
local M = {}
package.loaded["Mod"] = M
```

---
## 2. 子模块和包

- Lua 支持具有层次结构的模块名，通过点 ```.``` 来分隔名称中的层次。一个名为 ```mod.sub``` 的模块是模块 ```mod``` 的一个子模块
- 一个包（package）是一颗由模块组成的完整的树，是 Lua 语言用于发行程序的单位
- 最先 ```require``` 搜索时，先将 ```mod.sub``` 作为键来查询表 ```package.loaded``` 和 ```package.preload```。当搜索一个定义子模块的文件时，函数 ```require``` 会将点转换成操作系统的目录分隔符

```lua
-- 假设 path = ./?.lua;/usr/local/lua/?.lua;/usr/local/lua/?/init.lua
-- 调用 require "a.b" 会尝试打开
	"./a/b.lua"
	"/usr/local/lua/a/b.lua"
	"/usr/local/lua/a/b/init.lua"
```

- Lua 语言使用的目录分隔符是编译时配置的，可以是任意的字符串。例如没有目录层次的系统可以使用下划线作为目录分隔符 ```require "a.b"``` 会搜索 ```a_b.lua```
- C 标准库的名称中不能包含点，因为不能导出 ```luaopen_a.b``` 的函数，函数 ```require``` 一般会将点转换成下划线。一个名为 a.b 的 C 标准库的加载函数命名为 ```luaopen_a_b```
- 加载一个模块不会加载它的任何子模块，加载子模块也不会自动加载其父模块

---