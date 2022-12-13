### Lua 编译、执行与错误

---
- [1. 编译](#1-编译)
- [2. 预编译的代码](#2-预编译的代码)
- [3. 错误](#3-错误)
	- [3.1 错误处理和异常](#31-错误处理和异常)
	- [3.2 错误信息和栈回溯](#32-错误信息和栈回溯)

---
## 1. 编译

- Lua 语言称为解释型语言，但总是在运行代码前先预编译源码为中间代码。

> dofile 与 loadfile

- 函数 ```dofile(filename)``` 是运行 Lua 代码段的主要方式之一，其核心工作是由函数 ```loadfile``` 完成。发生错误时，```dofile``` 会将错误反馈给调用方
- ```loadfile``` 与 ```dofile``` 类似，都是从文件中加载 Lua 代码段，但 ```loadfile``` 不会运行代码，只是编译代码并作为一个函数返回，编译错误时也会返回错误码而不抛出异常
- 简单的需求而言，调用 ```loadfile``` 编译一次的开销比多次调用函数 ```dofile``` 小得多

```lua
-- 可以认为 dofile 就是
function dofile(filename)
    local f = assert(loadfile(filename))
    return f()
end
```

> load

- ```load``` 和 ```loadfile``` 类似，用于从一串字符串中读取一段代码段。如果是简单的用后即弃的代码段，可以尝试使用 ```assert(load("code"))()```
- 函数 ```load``` 在编译时不涉及词法定界，总是在全局环境中编译代码段。Lua 语言将所有独立的代码段当作匿名可变长参数函数的函数体，即 ```load("a=1")``` 等价于 ```function(...) a=1 end```
- 这些函数编译时并不会创建或改变变量，也不会文件写入，只是将程序段编译为一种中间形式，并作为匿名函数返回。只有在调用它们的返回函数时，才会执行这些中间代码

```lua
i = 32
local i = 0
f = load("i = i + 1; print(i)")
g = function() i = i + 1; print(i) end
f()    -- 33
g()    -- 1
```

- ```load``` 最典型的用法是执行外部代码或动态生成的代码

```lua
print "enter your expression:"
local line = io.read()
local func = assert(load("return " .. line))
print("the value of your expression is " .. func())
```

- ```load(io.lines(filename,"*L"))``` 与函数 ```loadfile``` 等价

---
## 2. 预编译的代码

- Lua 允许我们以预编译的形式分发代码。利用标准发行的 luac 程序生成预编译文件（binary chunk）

```powershell
# 预编译
$ luac -o <name.lc> <source.lua>
# 执行
$ lua <name.lc>
```

- 可以在所有能够使用源码的地方都可以使用预编译代码，函数 ```load``` 和 ```loadfile``` 也可以接受预编译代码

> 一个简单的 luac 程序

```lua
p = loadfile(arg[1])
f = io.open(arg[2],"wb")
f:write(string.dump(p))
f:close()
```

> 预编译形式

- 预编译形式的代码不一定比源代码小，但加载得更快；可以避免由于意外而修改源码。与源代码不同，蓄意损坏或构造的二进制代码可能会让 Lua 解析器崩溃或执行用户提供的机器码。应避免运行以预编译形式的非受信代码，在函数 ```load``` 沙盒环境中运行会更加安全

---
## 3. 错误

- Lua 作为嵌入式语言，当发生错误时并不能简单地崩溃或退出。只要错误发生，Lua 就必须提供处理错误的方式
- 可以显式地通过调用函数 ```error(any)``` 来引发一个错误并中止程序继续运行，发生错误时会把 any 抛出。```error``` 永远不会返回

```lua
print "enter a number"
n = io.read("n")	-- 指示转化为一个浮点数
if not n then error("invalid input") end
```

- 函数 ```assert(exp [,strError])``` 检查表达式返回是否为真，真则返回该参数的值，否则引发一个错误

```lua
n = io.read("n")
assert(tonumber(n),"invalid input: " .. n .. " is not a number")
```

- 当一个函数发现某种意外的错误发生时，在进行异常处理时可以选择返回错误代码（nil 或 false）或调用 ```error``` 引发一个错误

---
### 3.1 错误处理和异常

- 要在 Lua 代码中处理错误，可以使用函数 ```pcall(function [,arg, ...])``` 来封装代码。```pcall``` 以安全模式调用 ```function``` 且不会抛出任何错误。如果没有错误发生，返回 ```true``` 和保护函数的返回；发生错误时返回 ```false``` 和错误信息

```lua
local status,err  = pcall(function()
    <somecode-noreturn>
end)
if status then
    <Regular-code>
else
    <Error-handling-code>
end
```

- 函数 ```pcall``` 能够返回传递给 ```error``` 的任意 Lua 语言类型的值

```lua
local status,err = pcall(function() error({errCode = 121}) end)
print(status, err and err.errCode or nil)	-- false  121
```

---
### 3.2 错误信息和栈回溯

- 对于函数 ```error(message|any [,level])```，如果消息是字符串，```error``` 会在消息的开头添加一些关于错误位置的信息。```level``` 参数指定如何获取错误位置
  - 对于第 1 级(默认值)，错误位置是调用错误函数的位置
  - 第 2 级将错误指向调用错误的函数所在的位置，以此类推
  - 传递 0 可以避免向消息中添加错误位置信息

- 使用 ```pcall``` 返回错误信息时，部分的调用栈已经被破坏，因此必须在 ```pcall``` 返回前将调用栈构造好。函数 ```xpcall``` 的第二个参数是一个消息处理函数，当发生错误时，Lua 会调用栈展开前调用这个消息处理函数，以便消息处理函数能够使用调试库来获取有关错误的更多信息
- 常用 ```debug.debug``` 和 ```debug.traceback``` 作为 ```xpcall``` 的消息处理函数。```debug``` 为用户提供一个 Lua 提示符来检查错误发生的原因；```traceback``` 使用调用栈来构造详细的错误信息（Lua 解释器默认使用这个函数来构造错误信息）

```lua
print(xpcall(function() error("Error", 0) end, debug.traceback))
--[[
false Error
stack traceback:
        [C]: in function 'error'
        d:\_Lua_\test.lua:1: in function <d:\_Lua_\test.lua:1>
        [C]: in function 'xpcall'
        d:\_Lua_\config.lua:1: in main chunk
        [C]: in ?
]]
```

---