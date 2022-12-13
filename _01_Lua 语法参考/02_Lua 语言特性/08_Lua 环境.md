### Lua 环境

---
- [1. 具有动态名称的全局变量](#1-具有动态名称的全局变量)
- [2. 全局变量的声明](#2-全局变量的声明)
- [3. 非全局环境](#3-非全局环境)
- [4. _ENV](#4-_env)
- [5. 环境与模块](#5-环境与模块)
- [6. _ENV 和 load](#6-_env-和-load)

---
## 1. 具有动态名称的全局变量

- Lua 将所有的全局变量保存在一个称为全局环境（global environment，```_G```）的普通表中，其中 ```_G._G``` 与 ```_G``` 等价
- 全局变量都可以在 ```_G``` 表中通过变量名的方式直接进行索引，或对全局变量进行赋值

```lua
value = _G[varname]
_G[varname] = newVal
```

- 不能直接使用 ```_G["io.read"]``` 去获取 ```io``` 表中的元素，可以设计一个函数，从 ```_G``` 开始逐个字段地进行求值或赋值

> 从 _G 中查找元素

```lua
function getField(f)
	local v = _G
	for w in string.gmatch(f, "[%a_][%w_]*") do
		v = v[w]
	end
	return v
end

Rt = getField("io.read")	--> function
```

> 修改 _G 中某个元素

```lua
function setField(f,v)
	local t = _G
	for w,d in string.gmatch(f,"([%a_][%w_]*)(%.?)") do
		if d == "." then
			t[w] = t[w] or {}
			t = t[w]
		else
			t[w] = v
		end
	end
end

setField("a.b.c.d",newVal);
```

---
## 2. 全局变量的声明

- Lua 语言中的全局变量可以不声明就可以直接使用，这样也容易造成难以发现的错误。Lua 将全局变量存放在 ```_G``` 中，可以通过元表发现访问不存在的全局变量

> 设计一个方法简单检测所有对全局表中不存在键的访问（读写均受限制）

```lua
setmetatable(_G, {
	__newindex = function(_, n)
		error("Attempt to write to undeclared variable " .. n, 2)
	end,
	__index = function(_, n)
		error("Attempt to read undeclared variable " .. n, 2)
	end
})

print(var) -- Attempt to read undeclared variable var
```

- 如何绕过元方法创建新的变量？

```lua
-- 需要声明在 setmetatable 重写方法之前
function declare(name, initval)
	rawset(_G, name, initval or false)
end

declare("a", 1)
print(a)
```

- 若要测试一个变量是否存在，不能简单的和 ```nil``` 比较，访问一个 ```nil``` 值会引发一个错误。可以利用 ```rawget``` 绕过元方法 ```setmetatable```

```lua
function isExist(varName)
	if rawget(_G, varName) == nil then
		return false
	else return true
	end
end

isExist("varName")
```

> 改良版 setmetatable，访问未声明的全局变量会导致错误

```lua
local declareNames = {}
setmetatable(_G,{
	__newindex = function (t,n,v)
		if not declareNames[n] then
			local w = debug.getinfo(2,"S").what
			if w~= "main" and w~="C" then
				error("Attempt to write to undeclared variable " .. n,2)
			end
			declareNames[n] = true
		end
		rawset(t,n,v)
	end,

	__index = function (_,n)
		if not declareNames[n] then
			error("Attempt to read undeclared variable ".. n,2)
		else
			return nil
		end
	end
})
```

---
## 3. 非全局环境

- 在 Lua 中，本质上是没有全局变量的。首先，Lua 编译器将代码中的所有自由名称 ```x``` 转换成 ```_ENV.x```

```lua
local z = 10
x = y + z
------- 转换
local z = 10
_ENV.x = _ENV.y + z
```

- ```_ENV``` 不可能是全局变量，Lua 将所有的代码段都当做是匿名函数，编译器实际上将原来的代码段编译成：

```lua
local _ENV = <some-value>
return function(...)
	local z = 10
	_ENV.x = _ENV.y + z
end
```

- 实际上，Lua 是在一个名为 ```_ENV``` 的预定义上值（外部的局部变量）存在的情况下编译所有的代码段。因此在当前环境下，所有的变量要么是被绑定到了一个局部变量，要么是 ```_ENV``` 中的一个字段，而 ```_ENV``` 本身是一个局部变量
- ```_ENV``` 初始值可以是任意的表，任何一个这样的表都被称为一个环境，用来维持全局变量存在的幻觉。Lua 中在内部维护了一个表用作全局环境，通常，当加载一个代码段时，函数 ```load``` 会使用预定义的上值来初始化全局环境，即 ```local _ENV = <global ENV>```

> Lua 中处理全局变量的方式

- 编译器在编译所有代码段前，在外层创建局部变量 ```_ENV```
- 编译器将所有的自由名称 ```var``` 转换成 ```_ENV.var```
- 函数 ```load``` 使用全局环境初始化代码段的第一个上值

---
## 4. _ENV

- 通常，```_G``` 和 ```_ENV``` 指向同一个表，实质上，```_ENV``` 是一个局部变量，所有对 “全局变量” 的访问实质上访问的都是 ```_ENV```
- 按照定义，```_ENV``` 永远指的是当前的环境，```_G``` 指的是全局环境。```_ENV``` 的主要用途是来改变代码段使用的环境

```lua
_ENV = {} 	-- 将当前的环境改为一个新的空表
a = 1
print(a) 	-- attempt to call global 'print'(a nil value)
```

> 改变 _ENV 环境

- 起初，```_G``` 和 ```_ENV``` 指向同一个表，创建的全局变量均可通过两者进行访问；若 ```_ENV``` 指向一个新的环境，```_ENV``` 将丢失起初的初始化状态

```lua
a = 15				-- 创建 _ENV.a
_ENV = {_G = _G}	-- 改变当前环境
a = 1				-- 在新环境中创建 _ENV.a
_G.print(_ENV.a, _G.a) 	--> 1	15
```

> 创建新环境继承旧环境

```lua
a = 1
local newgt = {}
setmetatable(newgt, { __index = _G })
_ENV = newgt

print(a, _G.a)	-- 10	1
a = 10
_G.a = 20
print(a, _G.a)	-- 10	20
```

> 定界规则

- ```_ENV``` 遵循通常的定界规则。如果定义一个名为 ```_ENV``` 的局部变量，那么对自由名称的引用将会绑定到新变量上

```lua
a = 2
do
	local _ENV = {print = print, a = 14}
	print(a)	--> 14
end
print(a)		--> 2，返回原始的 _ENV
```

---
## 5. 环境与模块

- 模块的缺点之一在于很容易污染全局空间，例如私有声明中忘记 ```local```，一个模块的主程序块有自己独占的环境，当被其他模块加载时，其被加载环境的全局变量会自动进入新环境中。可以利用 ```_ENV``` 遵循定界的特性，将模块进行分离，将当前模块的全局变量进行隔离

```lua
local M = {}
_ENV = M

-- func 会自动变成 M.func，其他模块加载时，不会把当前环境的全局变量共享过去
function func()
	<code>
end
return M;
```

- 也可以将其他模块加载到私有变量上，达到隔离其他模块的全局变量效果

```lua
local M = require "modName"
```

> 自定义 _ENV

```lua
-- mENV.ua
<some-config>

return _ENV

-- other.lua
local _ENV = require "mENV"
-- 本地 _ENV 继承 mENV._ENV
```

---
## 6. _ENV 和 load

- 函数 ```load``` 通常把被加载代码段的上值 ```_ENV``` 初始化为全局环境。函数 ```load``` 提供一个可选的第四个参数为 ```_ENV``` 指定一个不同的初始值，```loadfile``` 原理相同

> load

```lua
local ENV_1 = _ENV
local ENV_2 = {}

load('print("Hello")', nil, "bt", ENV_1)()
--> Hello
load('print("Hello")', nil, "bt", ENV_2)()
--> attempt to call a nil value (global 'print')
```

> loadfile

```lua
-- config.lua
print("config test")

-- other.lua
local ENV_1 = _ENV
local ENV_2 = {}

loadfile("config.lua", "t", ENV_1)()
--> config test
loadfile("config.lua", "t", ENV_2)()
--> attempt to call a nil value (global 'print')
```

---