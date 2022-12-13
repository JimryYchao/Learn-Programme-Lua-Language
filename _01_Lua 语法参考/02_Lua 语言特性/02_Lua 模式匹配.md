### Lua 模式匹配

---
- [1. 模式匹配的相关函数](#1-模式匹配的相关函数)
	- [1.1 string.find](#11-stringfind)
	- [1.2 string.match](#12-stringmatch)
	- [1.3 string.gsub](#13-stringgsub)
	- [1.4 string.gmatch](#14-stringgmatch)
- [2. 模式](#2-模式)
- [3. 捕获](#3-捕获)
- [4. 替换](#4-替换)
- [5. 出现频率最高的单词](#5-出现频率最高的单词)


---
## 1. 模式匹配的相关函数

### 1.1 string.find

- 函数 ```string.find``` 用于在指定的目标字符串中搜索指定的模式，并在找到时返回第一个匹配到的模式的开始和结束位置的索引。可以指定开始搜索的位置

```lua
s = "hello world"
local i,j = string.find(s, "hello")	-- 1, 5
print(string.sub(s,i,j))			-- hello
```

> 简单搜索模式

- ```string.find(s, pattern, start?, searchMode?)``` 第四个参数用来说明是否进行简单搜索，即忽略模式而在目标字符串中进行纯粹的查找字符串的动作

```lua
string.find("a [word]","[")			-- error, [ 在模式中有特殊意义
string.find("a [word]","[",1,true)	--> 3	3
```

---
### 1.2 string.match

- 函数 ```string.match``` 用于返回与模式匹配的子串，不同于 ```find``` 返回索引位置

```lua
print(string.match("hello world", "hello"))		--> hello

date = "Today is 1/1/1970"
print(string.match(date, "%d+/%d+/%d+"))		--> 1/1/1970
```

---
### 1.3 string.gsub

- 函数 ```string.gsub(s, pattern, repl, n?)``` 用于将目标字符串中所有出现模式的地方换成替换字符串。n 用于限制替换的最大次数

```lua
s = string.gsub("Lua is cute", "cute", "great")
-- Lua is great		1 --> 返回第二个结果表示发生替换的次数
```

---
### 1.4 string.gmatch

- 函数 ```string.gmatch``` 返回一个函数用于遍历每一个字符串中所有出现的指定模式

```lua
s = "some string"
words = {}
for w in string.gmatch(s, "%a+") do
	words[#words + 1] = w	-- words = {"some", "string"}
end
```

---
## 2. 模式

> 预置的字符分类及其对应的含义

```Lua
.		-- 任意字符
%a		-- 字母
%c		-- 控制字符
%d		-- 数字
%g		-- 除空格外的可打印字符
%l		-- 小写字母
%p		-- 标点符号
%s		-- 空白字符
%u		-- 大写字母
%w		-- 字母和数字
%x		-- 十六进制数字
-- 以上字符的大写形式表示该字符分类的补集

%M		-- M 表示对魔法字符的转义，可以是 ().%+-*?[]^$，例如 %% 表示 %
```

> 魔法字符

```Lua
[]		-- 字符集	[0123456] 表示 0-6
- 		-- 连接字符集范围 [0-6]
^		-- 字符集的补集 [^\n] 表示换行符外的字符，%S 等价于 [^%s]
+		-- 重复至少一次，%d+ 表示匹配一个或多个数字
*		-- 重复最少零次
-		-- 重复最少零次（最小匹配）
?		-- 出现零次或一次
^,$		-- 锚定目标字符串开头(^)或结尾($)，^ 表示从字符串开头开始查找
%b xy	-- 匹配成对的字符串，x 为起始，y 为结束。例如 %b() 返回包含 () 内中间的字符串
```

> ```%f[char-set]```

- ```%f[char-set]``` 前置模式，该模式只有在后一个字符位于 char-set 内而前一个字符不在范围内时匹配一个空字符串。前置模式把目标字符串中第一个字符前和最后一个字符后的位置当成空字符串

```lua
s = "the anthem is the theme"
print(string.gsub(s, "%f[%w]the%f[%W]", "one"))
	--> one anthem is one theme
```

---
## 3. 捕获

- 捕获（capture）机制允许根据一个模式从目标字符中抽出与该模式匹配的内容用于后续用途，可以通过把模式中需要捕获的部分放到 ```()``` 中来指定捕获。函数 ```string.match``` 会将所有捕获到的值最为单独的结果返回

```lua
pair = "name = Anna"
key, value = string.match(pair, "(%a+)%s*=%s*(%a+)")
print(key, value)	--> name	Anna
```

- 空白捕捉 ```()``` 用于捕获模式在目标字符串中的位置

```lua
print(string.match("hello", "()ll()"))	-->  3	5 (和 string.find 有所区别                      )
```

- ```% num``` 表示匹配第 num 个捕获的副本，```%0``` 表示整个匹配

```lua
s = [[then he said: "it's all right"!]]
q, quotePart = string.match(s, "([\"'])(.-)%1")
q	 		--> "
quotePart 	--> it's all right
```

> 剔除字符串两端空格

```lua
function string.trim(s)
	s = string.gsub(s, "^%s*(.-)%s*$", "%1")
	return s
end

print(string.trim("  some string  "))
	--> some string
```

---
## 4. 替换

- ```string.gsub(s,pattern,any)``` 第三个参数可以是替换字符串，可以是一个函数或表。当每次找到匹配时，函数会将匹配作为参数进行调用，并把返回内容作为替换；表会将匹配作为键进行搜索并将返回作为替换。```nil``` 作为替换时，函数不改变这个匹配

```lua
function expand(s)
	return (string.gsub(s, "$(%w+)", _G))
end
name = "Lua"; status = "great"
print(expand("$name is $status"))	--> Lua is great
```

---
## 5. 出现频率最高的单词

- 设计一个读取并输出一段文本中出现频率最高的单词的程序：
  - 读取文本并计算每一个单词的出现次数
  - 按照出现次数的降序对单词列表进行排序
  - 输出有序列表中的前 n 个元素

```lua
-- 读取文本，并记录每个单词出现的次数
function F(file, n)
	assert(tonumber(n) > 0)
	local f = io.input(file)
	local counter = {}
	for line in io.lines() do
		for word in string.gmatch(line, "%w+") do
			counter[word] = (counter[word] or 0) + 1
		end
	end
	io.close(f)
	local words = {}
	for w in pairs(counter) do
		words[#words + 1] = w
	end
	table.sort(words, function(w1, w2)
		return counter[w1] > counter[w2] or counter[w1] == counter[w2] and w1 < w2
	end)
	for i = 1, n > #words and #words or n do
		io.output(io.stdout):write(words[i], "\t", counter[words[i]], "\n")
	end
	io.close(io.stdout)
end

F("a.lua", 99)
```

---