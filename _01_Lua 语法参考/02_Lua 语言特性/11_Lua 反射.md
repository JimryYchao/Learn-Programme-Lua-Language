### Lua 反射

---
- [1. 自省机制（introspective facility）](#1-自省机制introspective-facility)
	- [1.1 自省函数](#11-自省函数)
	- [1.2 访问局部变量](#12-访问局部变量)
	- [1.3 访问非局部变量](#13-访问非局部变量)
- [2. 钩子（hook）](#2-钩子hook)
- [3. 调优（profiler）](#3-调优profiler)
- [4. 沙盒（sandbox）](#4-沙盒sandbox)

---
## 1. 自省机制（introspective facility）

- 反射是程序用来检查和修改其自身某些部分的能力。Lua 支持的几种反射机制有：环境允许运行时观察全局变量；运行时检查和遍历未知数据结构（例如 ```type``` 和 ```pairs```）；允许程序在自身中追加代码或更新代码（```load``` 和 ```require```）。但是 Lua 不能检查局部变量，开发人员不能跟踪代码的执行，函数也不知道调用方，```debug.library``` 调试库填补了这些空缺

- 调试库由；两类函数组成：自省函数（introspective function）和钩子（hook）
  - 自省函数允许检查一个正在运行的活动函数的栈、当前正在执行的代码行、局部变量的名称和值等
  - 钩子允许跟踪一个程序的执行

---
### 1.1 自省函数

> 函数相关

- 函数 ```debug.getinfo(function|num [, what?])``` 返回与函数或栈层次的有关的一些数据的表，这个表包含：

<table>
    <tr>
        <th>表字段</th>
        <th>Lua 函数</th>
        <th>Lua 代码段</th>
        <th>C 函数</th>
    </tr>
    <tr>
        <td>source</td>
        <td>函数定义的位置</td>
        <td>load 返回定义字符串</td>
        <td>=[c]</td>
	</tr>
	<tr>
		<td>short_src</td>
		<td>source 的精简版本</td>
		<td>[string "code"]</td>
		<td>[c]</td>
	</tr>
	<tr>
		<td>linedefined</td>
		<td>函数定义的首行位置</td>
		<td>0</td>
		<td>-1</td>
	</tr>
	<tr>
		<td>lastlinedefined</td>
		<td>函数定义的末行位置</td>
		<td>0</td>
		<td>-1</td>
	</tr>
	<tr>
		<td>what</td>
		<td>函数类型 Lua</td>
		<td>main</td>
		<td>C</td>
	</tr>
	<tr>
		<td>name</td>
		<td colspan =3>返回函数名称的字段</td>
	</tr>
	<tr>
		<td>namewhat</td>
		<td colspan = 3>字段含义，可能是 global、local、method、field、""（空字符串）</td>
	</tr>
	<tr>
		<td>nups</td>
		<td colspan = 3>该函数上值的个数</td>
	</tr>
	<tr>
		<td>nparams</td>
		<td colspan = 3>函数参数的个数</td>
	</tr>
		<tr>
		<td>isvararg</td>
		<td colspan = 3>参数列表是否包含可变长参数</td>
	</tr>
	<tr>
		<td>activelines</td>
		<td colspan = 3>该函数所有活跃行的集合</td>
	</tr>
	<tr>
		<td>func</td>
		<td colspan = 3>该函数本身</td>
	</tr>
</table>

- 对于一个 C 函数，只有字段 what、name、namewhat、nups、func 是有意义的

> 栈层次相关

- 当使用一个数字 num 作为参数调用 ```getinfo``` 时，可以得到有关相应栈层次上活跃函数的数据。0 表示 ```getinfo``` 自己，1 表示调用 ```getinfo``` 的函数 A，2 表示调用函数 A 的函数，等。num 大于栈中活跃函数的数量时，将返回 ```nil```
- 与栈层次相关的两个字段：currentline 表示当前该函数正在执行的代码所在的行；istailcall 表示函数是否是尾调用
- name 只有在以一个数字为参数调用 ```getinfo``` 时才会起作用，即只能获取关于某一具体调用的信息

```lua
print(debug.getinfo(0).name)	-- getinfo
```

> 参数 what？

```lua
n          选择返回 name 和 namewhat
f          选择返回 func
S          选择返回 source、short_src、what、linedefined、lastlinedefined
l          选择返回 currentline
L          选择返回 activelines
u          选择返回 nup、nparams、isvararg
```

---
### 1.2 访问局部变量

- ```debug.getlocal(fun|integer, index)``` 检查任意活跃函数的局部变量，查询指定栈层次或函数的指定索引局部变量，返回变量名和值或 ```nil```
- Lua 按局部变量在函数中出现顺序对它们进行编号，编号只限于在函数当前作用域中活跃的变量（索引从 1 开始）

```lua
local t = {}
print(debug.getlocal(1,1))
-- t	table: 000001E7622C4610

function foo(a,b)
    local i = 1
    print(debug.getlocal(1,3))   -- i	1
end
print(debug.getlocal(foo,2))     -- b
print(debug.getlocal(foo,3))     -- nil

```

- ```debug.getlocal(thread, fun, index)``` 返回指定协程的栈层次局部变量信息。调试库中所有的自省函数都能够接受一个可选的协程作为第一个参数

```lua
function foo(a,b)
    local i = 1
    print(debug.getlocal(1,3))   -- i	1
end
local co = coroutine.create(foo)
print(debug.getlocal(co,foo,1))  -- a
print(debug.getlocal(co,foo,3))  -- nil
```

> 变长参数信息

- 值为负的索引获取可变长参数函数的额外参数，-1 指向第一个额外参数，以此类推，但变量的名称始终为 ```(vararg)```

```lua
function foo(a, ...)
    local i = 1
    print(debug.getlocal(1, -2))
end

foo(1, 3, 5)    -- (vararg)		5
```

> setlocal

- ```debug.setlocal(f|n,index,value)``` 用于改变局部变量的值，函数返回被修改值的变量名

---
### 1.3 访问非局部变量

- ```debug.getupvalue(f,index)``` 用于访问一个被 Lua 函数所使用的非局部变量，Lua 按照函数引用非局部变量的顺序对它们编号
- ```debug.setupvalue(f,index,value)``` 用于更新非局部变量的值，该函数返回被修改上值的变量名

```lua
local up1 = 1
function foo()
    local a = up1
end
print(debug.getupvalue(foo, 1))     -- up1	1
print(debug.setupvalue(foo, 1, 2))  -- up1
print(up1)                          -- 2
```

---
## 2. 钩子（hook）

- 调试库中的钩子机制允许用户注册一个钩子函数，并且在 Lua 程序运行中某个特定事件发生时被调用：
  - 调用一个函数时产生的 call 事件
  - 函数返回时产生的 return 事件
  - 开始执行一行新代码时产生的 line 事件
  - 执行完指定数量的指令后产生的 count 事件

- 函数 ```debug.sethook([thread,] hookf, mask [, count])``` 中，thread 可选，hookf 表示钩子函数，mask 描述要监视事件的掩码，count （可选）描述以何种频度获取 count 事件
- 关闭钩子，只需不带任何参数的调用函数 ```sethook```

```lua
debug.sethook(print,"l")
-- Lua 发生 line 事件时会调用它，并输出解释器执行的每一行代码
```

---
## 3. 调优（profiler）

- 反射的一个常见用法是用于调优，即程序使用资源的行为分析。对于时间相关的调优最好使用 C 接口
- 开发一个性能调优工具来列出程序执行的每个函数的调用次数

```lua
local Counters = {}
local Names    = {}

-- 可以在函数活动时获取其名称

local function hook()
    local f = debug.getinfo(2, "f").func
    local count = Counters[f]
    if (count) == nil then
        Counters[f] = 1
        Names[f] = debug.getinfo(2, "Sn")
    else
        Counters[f] = count + 1
    end
end

function getName(f)
    local n = Names[f]
    if (n.what == "C") then
        return n.name
    end
    local lc = string.format("[%s]:%d", n.short_src, n.linedefined)
    if n.what ~= "main" and n.namewhat ~= "" then
        return string.format("%s (%s)", lc, n.name)
    else
        return lc
    end
end

function Main()             -- 主函数
    print("This is Main function...")
end

debug.sethook(hook, "c")    -- 设置 call 事件的钩子
Main()                      -- 运行主程序
debug.sethook()             -- 关闭钩子

for func, count in pairs(Counters) do
    print(getName(func), count)
end

--[[
    This is Main function...
    print   1
    [d:\_Lua_\profiler.lua]:30 (Main)       1
    sethook 1
]]
```

---
## 4. 沙盒（sandbox）

> 一个使用钩子的简单沙盒

- 该程序把钩子设置为监听 count 事件，使得 Lua 每执行 100 条指令就调用一次钩子函数；钩子函数只是一个递增计数器，并检查其是否超过了设定的阙值

```lua
local debug = require "debug"
local steplimit = 1000     -- 最大能执行的 steps
local count = 0

local function step()
    count = count + 1
    if count > steplimit then
        error("script uses too much CPU")
    end
end
debug.sethook(step, "clr", 100) -- 设置钩子
```

> 控制内存使用

```lua
local function checkmem()
    if collectgarbage("count") > memlimit then
        error("script uses too much memory")
    end
end

local count = 0
local function step()
    checkmem()
    count = count + 1
    if count > steplimit then
        error("script uses to much CPU")
    end
end

debug.sethook(step, "clr", 100)

--[[
local s = "123456789012345"
for i = 1, 36 do
    s = s .. s
end
]]
```

> 使用钩子阻止对未授权函数的访问

```lua
local debug = require "debug"
local steplimit = 1000
local count = 0

-- 设置授权的函数
local validfunc = {
    [print] = true,
    [string.upper] = true,
    [string.lower] = true,
    [string.format] = false,
    --...    -- 其他授权函数
}

local function hook(event)
    print(tostring(event))
    if event == "call" then
        local info = debug.getinfo(2, "fn")
        if not validfunc[info.func] then
            error("calling bad function: " .. (info.name or "?"))
        end
    end
    count = count + 1
    if (count > steplimit) then
        error("script uses too much CPU")
    end
end

debug.sethook(hook, "clr", 100)
print(string.upper("hello"))
-- HELLO
print(string.format("%s %d", "foo", 1))
-- calling bad function: format
```

---