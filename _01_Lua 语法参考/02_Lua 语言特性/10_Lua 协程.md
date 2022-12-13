### Lua 协程

---
- [1. 协程：thread 类型](#1-协程thread-类型)
	- [1.1 协程状态](#11-协程状态)
	- [1.2 挂起协程与恢复运行](#12-挂起协程与恢复运行)
	- [1.3 将协程用作迭代器](#13-将协程用作迭代器)
- [2. 协程：function 类型](#2-协程function-类型)
- [3. running 和 close](#3-running-和-close)
- [4. isyieldable](#4-isyieldable)

---
## 1. 协程：thread 类型

- 从多线程（multithreading）的角度看，协程（coroutine）与线程（thread）类似：协程是一系列的可执行语句，拥有自己的栈、局部变量和指令指针，同时协程又与其他协程共享了全局变量和其他几乎一切资源
- 线程与协程的区别在于，多线程程序可以并行运行多个线程，协程需要彼此协作，任意指定的时刻只能有一个协程运行，正在运行的协程只能显式的被挂起时其执行才会暂停

> 构造一个协程

- Lua 中协程相关的函数在表 ```coroutine``` 中，可以使用 ```coroutine.create``` 创建新协程，参数是一个函数类型或匿名函数，函数返回一个 ```thread``` 类型

```lua
co = coroutine.create(
	function ()
		print("hello world")
	end)
```

---
### 1.1 协程状态

- 一个协程有四种状态：挂起（suspended）、运行（running）、正常（normal）、死亡（dead）。利用 ```coroutine.status(co)``` 查看协程对象状态
- 当一个协程创建时，它不会自动运行而处于挂起（suspended）状态，利用 ```coroutine.resume(co)``` 用于启动或再次启动一个协程，并改状态为运行（running）；若协程体运行之后就终止了，它的状态转变为死亡（dead）
- 当协程 A 唤醒协程 B 时（执行权移交给 B），A 会变成正常状态（normal），而协程 B 会变成运行状态

```lua
co = coroutine.create(<function(params)>)
print(coroutine.status(co))		-- suspended

coroutine.resume(co [,params])	-- running
print(coroutine.status(co))		-- dead
```

---
### 1.2 挂起协程与恢复运行

- 函数 ```coroutine.yield()``` 可以让一个运行中的协程挂起，之后在 ```resume``` 后恢复运行，协程会继续执行直到遇到下一个 ```yield``` 或执行结束。```resume``` 已经结束的协程（dead 状态）会返回 ```false``` 和一条信息（cannot resume dead coroutine）
- 协程中通过一对 ```resume-yield``` 来交换数据，第一个 ```resume``` 会把额外的参数传递给协程的主函数；在 ```resume``` 的返回值中，第一个返回值为 ```true``` 时表示没有错误并在之后的返回值对应函数 ```yield``` 的参数
- 而在协程主函数体内，再次调用 ```resume``` 时，控制返回协程，并在函数 ```yield``` 位置返回 ```resume``` 传递的额外参数
- 当一个协程结束时，主函数返回的值会变成函数 ```resume``` 的返回值
- 函数 ```resume``` 运行在保护模式中，如果协程在执行中出错，Lua 不会显示错误，而是把错误信息返回给函数 ```resume```

```lua
function _print(t)
	local count = 0
	while true do
		count = count + 1
		print("yield:: ", coroutine.yield(count))
	end
end

local co = coroutine.create(_print)
print("resume:: " , coroutine.resume(co, "hi"))
print("resume:: " , coroutine.resume(co, 1, 1, 1))
print("resume:: " , coroutine.resume(co, "hello"))

--[[
	resume::        true    1
	yield::         1       1       1
	resume::        true    2
	yield::         hello
	resume::        true    3
]]
```

---
### 1.3 将协程用作迭代器

```lua
function permgen(a, n)
	n = n or #a
	if n <= 1 then
		coroutine.yield(a)
	else
		for i = 1, n do
			a[n], a[i] = a[i], a[n]
 			permgen(a, n - 1)
			a[n], a[i] = a[i], a[n]
		end
	end
 end

function printResult(a)
	for i = 1, #a do
		io.write(a[i], " ")
	end
	io.write('\n')
end

function permutations(a)
	local co = coroutine.create(function()
		permgen(a)
	end)
	return function()
		local code, res = coroutine.resume(co)
		return res
	end
end

for a in permutations { 1, 2, 3 } do
	printResult(a)
end
```

---
## 2. 协程：function 类型

- ```coroutine.wrap(f)``` 返回一个 ```function``` 类型的协程，和 ```create(co)``` 构造的 ```thread``` 区别在于，```coroutine.yield``` 或协程主函数结束返回时，不会返回函数是否正常运行或恢复运行的状态，也无法获得 ```function``` 协程的状态
- ```thread``` 协程的主函数发生错误时不会终止程序，会将错误发送到 ```resume``` 的返回中；而 ```function``` 协程直接将导致程序错误

```lua
local co = coroutine.create(f)
local cf = coroutine.wrap(f)
-- 调用 thread 协程
coroutine.resume(co)
-- 调用 function 协程
cf()
```

---
## 3. running 和 close

> *coroutine.running*

- 函数 ```coroutine.running(co)``` 返回正在运行的协程和一个 boolean 值，当正在运行的协程是主函数 ```main``` 时返回 ```true```

```lua
print(coroutine.running())
local co
co = coroutine.create(function()
	print("join in co")
	local c, ismain = coroutine.running()
	print(c, ismain, "\nco : " .. tostring(co))
end)
coroutine.resume(co)
--[[
	thread: 0000017C2872F458        true
	join in co
	thread: 0000017C287A43E8        false
	co : thread: 0000017C287A43E8
]]
```

> *coroutine.close*

- 函数 ```coroutine.close(co)``` 用于关闭待关闭的 suspended 或 dead 状态的协程并返回 ```true```；关闭正在运行的协程会发生错误并返回 ```false``` 和错误信息

```lua
local co
co = coroutine.create(function()
	print(coroutine.close(co)) -- cannot close a running coroutine
	print("join in co")
end)
coroutine.resume(co)
```

---
## 4. isyieldable

- 函数 ```coroutine.isyieldable(co?)``` 用于判断协程是否是可让步（yield）的，如果协程不是主线程，也不在不可让步 C 函数中，则该协程是可让步的

```lua
-- main
print(coroutine.isyieldable())	-- false, 主线程

local co = coroutine.create(function()
	print("join in co")
end)
print(coroutine.isyieldable(co)) -- true
```

---