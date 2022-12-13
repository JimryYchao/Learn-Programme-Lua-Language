### Lua 元表与元方法

---
- [1. 元表](#1-元表)
- [2. 算术运算相关的元方法](#2-算术运算相关的元方法)
- [3. 关系运算相关的元方法](#3-关系运算相关的元方法)
- [4. 库定义相关的元方法](#4-库定义相关的元方法)
- [5. 表相关的元方法](#5-表相关的元方法)
	- [5.1 具有默认值的表](#51-具有默认值的表)
	- [5.2 跟踪对表的访问](#52-跟踪对表的访问)
	- [5.3 只读的表](#53-只读的表)

---
## 1. 元表

- 元表可以修改一个值在面对一个未知操作时的行为。可以认为，元表是面向对象领域中的受限制类，Lua 中的每一个值都可以有元表
- 使用函数 ```setmetatable(t,metatable?)``` 来设置或修改任意表的元素，函数 ```getmetatable(obj)``` 来获取父级元表实例。当参数 ```metatable``` 为 ```nil``` 时，表示删除该表的元表

```lua
local subTable, t = {}, {}
print("Father:", setmetatable(subTable, metatable));
print("SubTable:", getmetatable(subTable), metatable)
--[[
	Father: 	table: 00000111D4579E30
	SubTable: 	table: 00000111D4579DB0 table: 00000111D4579DB0
]]
```

---
## 2. 算术运算相关的元方法

- 每种算术运算符都有一个对应的元方法。当试图将两个不同的表进行算术操作时，前提是它们需要有相同的元表，可以将元表对应的算术元方法进行重定义或重写，定义它们算术操作时的行为。例如将两个表相加时，会查找它们元表的 ```__add``` 元方法
- Lua 会按照两个值的顺序查找对应的元方法：若第一个值有元表及对应的元方法，那会采用这个元方法且与第二个值无关；或采用第二个值的元方法；否则抛出异常

```lua
local mt = {}
mt.__add = function(t1, t2)
	local set = {}
	local len = #t1 > #t2 and #t1 or #t2
	for i = 1, len do
		set[i] = t1[i]
	end
	for j = 1, #set do
		set[j] = t1[j] + t2[j]
	end
	return set
end

local t1 = { 1, 2, 3, 4, 5, 6 }
local t2 = { 6, 5, 4, 3, 2, 1 }
setmetatable(t1, mt)
setmetatable(t2, mt)

local newt = t1 + t2
for i = 1, #newt do
	print(newt[i])	-- 7 7 7 7 7 7
end
```

> 其他算术元方法

```lua
-- 算术
__add	-- 加法	+
__mul	-- 乘法	*
__sub	-- 减法	-
__div	-- 除法	\
__idiv	-- floor 除法 \\
__unm	-- 负数 -
__mod	-- 取模	%
__pow	-- 幂运算
-- 位运算
__band	-- 按位与 &
__bor	-- 按位或 |
__bxor	-- 按位异或 ~
__bnot	-- 按位取反 ~
__shl	-- 左移 <<
__shr	-- 右移 >>
-- 其他
__concat 	-- 连接运算符 ..
__len		-- 长度运算符 #
```

---
## 3. 关系运算相关的元方法

```lua
__eq	-- 等于 ==
__lt	-- 小于 <
__le	-- 小于等于

-- 对于其他三种关系运算符，Lua 会进行转换，例如
a ~= b	--> not (a == b)
a > b	--> b < a
a >= b	--> b <= a
```

> 通常会将 a <= b 认为 a 是 b 的一个子集，常定义

```lua
mt.__le = function(a, b)
	for k in pairs(a) do
		if not b[k] then return false end
	end
	return true
end
mt.__lt = function(a, b)
	return a <= b and not (b <= a)
end
mt.__eq = function(a, b)
	return a <= b and b <= a
end

local t1 = { 4, 10, 2 }
local t2 = { 4, 2 }
setmetatable(t1, mt)
setmetatable(t2, mt)
print(t1 > t2)	-- true
print(t1 >= t2)	-- true
print(t1 == t2)	-- false
print(t1 < t2)	-- false
print(t1 <= t2)	-- false
```

---
## 4. 库定义相关的元方法

> ```__tostring```

- 函数 ```print``` 总是调用 ```tostring``` 来进行格式化输出，Lua 首先会检测一个操作中涉及的值是否有存在对应元方法的元表

```lua
print({})	-- table: 00000111D4579E30

-- print 调用了表的元方法 __tostring
local t1 = { 4, 10, 2 }
mt.__tostring = function(self)
	if #self == 0 then
		return tostring(self)
	end
	local s = self[1]
	for i = 1, #self do
		if i ~= 1 then
			s = string.format(s .. ',' .. tostring(self[i]))
		end
	end
	return s
end
print(t1)	-- 4,10,2
t1 = {}
print(t1)	-- table: 000002A055677EE0
```

> 其他

- 函数 ```setmetatable``` 和 ```getmetatable``` 用到了元方法，用于保护元表。假设要保护集合，即使用户既看不到也不能修改集合的元表，可以在元表中设置 ```__metatable``` 字段，那么 ```getmetatable``` 会返回这个字段，```setmetatable``` 会引发一个错误
- 函数 ```pairs``` 对应元方法 ```__pairs```

---
## 5. 表相关的元方法

- Lua 提供了一种改变表在访问和修改表中不存在字段这两种行为的方式

> ```__index```

- 当访问表中一个不存在的字段时会得到 ```nil```，这些访问实质上会查找表中的 ```__index``` 的元方法，由这个元方法来提供最终结果
- 可以使用元方法 ```__index``` 来实现继承。如果希望在访问一个表时不调用元方法，可以使用函数 ```rawget(t,i)``` 对表 t 进行原始访问

```lua
mt.__index = function (...)
	error("Index was outside the bounds of the table")
end
print(t1[10])
```

> ```__newindex```

- 元方法 ```__newindex``` 用于表的更新。当对一个表中不存在的索引赋值时，解释器会查找 ```__newindex``` 元方法。可以调用 ```rawset(t,k,v)``` 绕过元方法在表中进行赋值
- 组合使用 ```__index``` 和 ```__newindex``` 可以实现 Lua 中的一些特殊的结构，只读的表，具有默认值的表和面向对象编程中的继承等

---
### 5.1 具有默认值的表

```lua
function setDefault(t, d)
	local dt = { __index = function() return d end }
	setmetatable(t, dt)
end

local nt = { x = 1, y = 2 }
setDefault(nt, 0)
print(nt.x, nt.z)	-- 1	0
```

---
### 5.2 跟踪对表的访问

```lua
function track(t)
	local proxy = {}
	local mt = {
		__index = function(_, k)
			print("*access to element " .. tostring(k))
			return t[k]
		end,
		__newindex = function(_, k, v)
			print("*update of element " .. tostring(k) .. " to " .. tostring(v))
			t[k] = v
		end,
		__pairs = function()
			return function(_, k)
				local nextKey, nextVal = next(t, k)
				if nextKey ~= nil then
					print("*traversing element " .. tostring(nextKey))
				end
				return nextKey, nextVal
			end
		end,
		__len = function() return #t end
	}
	setmetatable(proxy, mt)
	return proxy
end

t = { x = 1, y = 2, z = 3 }
t = track(t)
t[2] = "2"
-- *update of element 2 to 2
print(t[2])
-- *access to element 2
-- 2
```

---
### 5.3 只读的表

```lua
function readonly(t)
	local proxy = {}
	local mt = {
		__index = t,
		__newindex = function(t, k, v)
			error("*attempt to update a readonly table", 2)
		end
	}
	setmetatable(proxy, mt)
	return proxy
end

days = readonly { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" }
print(days[1])
days[2] = "Noday"		-- *attempt to update a readonly table
```

---