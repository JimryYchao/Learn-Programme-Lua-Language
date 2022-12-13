### Lua 输入与输出

---
- [1. 简单 I/O 模型](#1-简单-io-模型)
- [2. 完整 I/O 模型](#2-完整-io-模型)
- [3. 其他文件操作](#3-其他文件操作)
- [4. 其他系统调用](#4-其他系统调用)

---
## 1. 简单 I/O 模型

- 对于文件操作，I/O 库只提供了当前输入流和一个当前输出流，其 I/O 操作是通过这些流实现的。当前输入流初始化为进程的标准输入（C stdin），输出流为进程的标准输出（C stdout）。函数 ```io.input()``` 和 ```io.output``` 用于改变当前的输入输出流
- ```io.input(filename)``` 以只读模式打开文件，并将文件设置当前输入流，之后所有的输入都来自于该文件，除非再次调用 ```io.input```；对于输出，```in.output``` 逻辑类似
- 函数 ```io.write(...)``` 可以读取任意数量的字符串并将其逐个写入当前输出流，对数值转换为字符串遵循一般的转换规则。一般在用后即弃的代码或调试输出会使用 ```print```，当需要完全控制输出时，应使用 ```io.write```。可以对 ```io.write``` 的输出进行重定向
- 函数 ```io.read(...)``` 可以从当前输出流中读取字符串，其参数决定了要读取的数据：
  - ```"a"``` 从当前位置开始读取当前输入文件的全部内容，到末尾或文件为空返回 ```nil```
  - ```"l"``` 读取下一行并丢弃换行符，```"L"``` 读取下一行且保留换行符。```"l"``` 是 ```io.read``` 的默认参数
  - ```"n"``` 用于读取一个数值
  - ```num``` 表示最多从输入流读取到 n 个字符，可以利用 ```io.read(0)``` 的返回是否为 ```nil``` 来判断读取到达文件末尾

```lua
-- 复制文件
function CopyFile(file, newfile)
	local line = ""
	if io.input(file) and io.output(newfile) then
		repeat
			io.write(line)
			line = io.read("L")
		until not line
	end
	io.close()
end

CopyFile("file.lua", "newfile.lua")
```

- 逐行迭代一个文件可以利用函数 ```io.lines```

```lua
-- 读取 a.lua 写入到 b.txt
io.input("a.lua")
io.output("b.txt")
local count = 0
for line in io.lines() do
	count = count + 1
	io.write(string.format("%6d  ", count), line, "\n")
end
io.close()
```

---
## 2. 完整 I/O 模型

- 函数 ```io.open(filename, mode?)``` 来打开一个文件并返回对于文件的流，相当于 C 的 ```fopen```。参数 ```mode``` 表示打开模式（```w``` 只写、```r``` 只读，```a``` 追加、```b``` 可选的二进制文件。常见使用 ```local f = assert(io.open(filename,mode))```
- I/O 库提供了 C 语言流句柄 ```io.stdin```、```io.stdout```、```io.stderr``` 用于设定 Lua 语言流。例如 ```io.stderr:write(message)```

> 混用完整和简单 IO 模型

```lua
local temp = io.input() 	-- 保存当前输入流
io.input("newinput")
-- 对新的输入流进行操作
io.input():close()			-- 关闭当前流
io.input(temp)				-- 恢复此前流
```

- ```io.read(arg)``` 实际是 ```io.input():read(args)``` 简写，```io.write(args)``` 是 ```io.output():write(args)``` 的简写

---
## 3. 其他文件操作

- 函数 ```io.tmpfile()``` 返回一个操作临时文件的句柄，该句柄是以读/写模式打开的，当程序运行结束后，该临时文件会自动删除
- 函数 ```io.flush()``` 将所有缓冲数据写入文件，也可以利用 ```io.flush()``` 或 ```f:flush()``` 刷新当前输出流
- 函数 ```file:setvbuf(mode, size?)``` 用于设置流的缓冲模式。Mode：```"no"``` 表示无缓冲，```"full"``` 表示在缓冲区满时或显式地刷新文件时才写入数据，```"line"``` 表示行缓冲写入。Size 用于指定缓冲区的大小。大多数情况，```io.stderr``` 是不被缓冲的，```io.stdout``` 是按行缓冲
- 函数 ```file:seek(whence?, offset?)``` 用来获取和设置文件的当前位置。Whence 指定如何使用偏移的字符串：```"set"``` 表示文件开头，```"cur"``` 表示当前文件位置（默认），```"end"``` 表示文件末尾。偏移量 Offset 默认为 0。```file:seek("set")``` 位置重置为文件开头并返回 0；```file:seek("end")``` 位置置于末尾并返回文件字节大小
- 函数 ```os.rename``` 文件重命名；函数 ```os.remove``` 删除文件

---
## 4. 其他系统调用

- 函数 ```os.exit(integer|boolean?, boolean?)``` 用于终止程序的执行，第一个可选参数可以用于表示程序的返回状态，第二个可选参数 ```true``` 表示调用所有析构器释放所有 Lua 占用的资源
- 函数 ```os.getenv("ENV")``` 用于获取某个环境变量
- 函数 ```os.execute``` 用于运行系统命令。可以利用函数创建新目录 ```os.execute("mkdir " .. dirname)```
- 函数 ```io.popen``` 用于重定向命令的输入/输出，使得程序可以向命令中写入或从命令的输入中读取

> 模拟发送邮件

```lua
-- 仅限在 POSIX 中运行
local subject= "send mail test"
local ad = "someone@somewhere.org"

local cmd = string.format("mail -s '%s' '%s'",subject,ad)
local f = io.popen(cmd, "w")	-- w 表示写入
f:write([[
sir:
	Nothing important to say.
	-- me
]])
f:close()
```

---