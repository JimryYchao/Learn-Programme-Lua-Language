### Lua 语句和表达式

---
- [1. 运算符](#1-运算符)
	- [1.1 算术运算符](#11-算术运算符)
	- [1.2 关系运算符](#12-关系运算符)
	- [1.3 逻辑运算符](#13-逻辑运算符)
	- [1.4 位运算符](#14-位运算符)
	- [1.5 其他运算符](#15-其他运算符)
	- [1.6 运算符优先级](#16-运算符优先级)
- [2. 语句](#2-语句)
	- [2.1 条件控制语句 if](#21-条件控制语句-if)
	- [2.2 循环控制语句 while](#22-循环控制语句-while)
	- [2.3 循环控制语句 repeat](#23-循环控制语句-repeat)
	- [2.4 循环语句 for](#24-循环语句-for)
		- [2.4.1 泛型 For 迭代器](#241-泛型-for-迭代器)
		- [2.4.2 无状态迭代器](#242-无状态迭代器)
		- [2.4.3 多状态迭代器](#243-多状态迭代器)
	- [2.5 跳转语句 break、return 和 goto](#25-跳转语句-breakreturn-和-goto)
	- [2.6 代码块 do ... end](#26-代码块-do--end)

---
## 1. 运算符

### 1.1 算术运算符

> 传统算术运算

```lua
  -- 加法
    13 + 15      --> 28
    13.0 + 15.0  --> 28.0
  -- 减法
    13 - 15      --> -2
    13.0 - 15    --> -2.0
  -- 乘法
    13 * 15      --> 195
    13 * 15.0    --> 195.0
  -- 除法
    13 / 2       --> 6.5
    6 / 2        --> 3.0
```

> 幂运算

```lua
2^5	         --> 32
```

> 取模与 floor 整除法

- floor 除法对得到的商向负无穷取整

```Lua
3 // 2         --> 1
3.0 // 2       --> 1.0
-9 // 2        --> -5
1.5 // 0.5     --> 3.0
```

- 取模运算的定义 ```a%b == a-((a//b)*b)```，其结果的符号与第二操作数的符号保持一致

```lua
-9 % 2      --> 1:  -9-(-5)*2 = 1
-9 % 2.0    --> 1.0
```

> 利用取模运算保留浮点运算有效位

- ```X - X%(1E-n)```，n 表示要保留的小数有效位数

```lua
math.pi - math.pi % 0.0001	--> 3.1415
```

---
### 1.2 关系运算符

```Lua
<     小于
<=    小于等于
>     大于
>=    大于等于
==    相等
~=    不等
```

---
### 1.3 逻辑运算符

```lua
and  --> 与
or   --> 或
not  --> 非
```

- 逻辑运算符 ```and``` 的运算结果为：第一个操作数为 ```false``` 时返回第一个操作数，否则返回第二个操作数
- 逻辑运算符 ```or``` 的运算结果为：第一个操作数为 ```true``` 时返回第一个操作数，否则返回第二个操作数
- ```and``` 和 ```or``` 遵循短路求值原则
- ```not``` 返回取反 boolean

```lua
print(true and false)   --> false
print(false or true)    --> true
print(not true)         --> false
-- Lua 的逻辑运算支持短路, 左操作数不满足, 则右操作数不执行
print(false and print("123"))   --> false；print("123被短路")，并不执行
print(true and print("123"))    --> 123  nil；(print() 方法的返回值是 nil）
```

> 检查一个变量是否初始化

- 某个变量未被初始化时，为其赋值默认值

```lua
x = x or v
```

> 仿三目运算

- 利用 ```and``` 和 ```or``` 的短路特征构造三目运算

```lua
X and Y or Z
--> X == true --> Y
--> X == false --> Z
```

---
### 1.4 位运算符

- 按位运算只能用于整型数的所有位。
- Lua 中的按位移位是逻辑移位法，无论左移还是右移均以 0 补齐空位；Lua 中没有提供算术右移
- 移位数是负数表示向相反的方向移位，```a >> n``` 与 ```a << -n``` 等价
- Lua 移位数超过数的宽度时，结果均为 0

```lua
-- 按位与
    1&1 = 1
    0&1 = 0
    0&0	= 0
-- 按位或
    1|1 = 1
    1|0 = 1
    0|0 = 0
-- 按位取反
    ~1 = 0
    ~0 = 1
-- 按位异或
    1~1 = 0
    0~0 = 0
    1~0 = 1
-- 按位移位
    12(10) = 00001100(2)
    00001100 >> 1 = 00000110  --> 12>>1 = 6
    00001100 << 2 = 00110000  --> 12<<2 = 48

    -18(10) = 11101110(2)     -- 补码表示
    11101110 << 1 = 11011100  --> -18<<1 = -36
    11101110 >> 2 = 01111011  --> -18>>2 = 123 (逻辑移位)

    100 >> 66  --> 宽度为 64，结果为 0
```

> 实现算术移位

- 利用 floor 向下整除法模拟实现算术移位，公式为 ```num // (2^n)|0```，当 $n>0$ 表示算术右移；当 $n<0$ 表示算术左移

```lua
-- 负数的算术右移
    -10 >> 2 等价于 -10//2^2|0 --> -3
-- 算术左移
    -10 >> -2 == -10 << 2 == -10//(2^-2|0) --> -40
```

---
### 1.5 其他运算符

- ```#``` 用于获取数组或字符串的长度
- ```..``` 用于字符串拼接
- ```type()``` 用于获取变量的类型

---
### 1.6 运算符优先级

```lua
^
-、#、not、~(按位取反)
*、/、//、%
+、-
..             字符串拼接
<<、>>         按位移位
&              按位与
~              按位异或
|              按位或
<、>、<=、>=、==、~=
and
or
```

---
## 2. 语句

### 2.1 条件控制语句 if

- 条件控制表达式结果可以是任何值，其中 ```false``` 和 ```nil``` 值为假，```true``` 和非 ```nil``` 值为真

```lua
if <condition> then
    <code>
elseif <condition> then
    <code>
else
    <code>
end
```

---
### 2.2 循环控制语句 while

- ```while``` 循环语句的跳出条件是不满足控制表达式的限制

```lua
while <condition> do
    <code body>
end
------------------------
-- 控制表达式括号是可选的
while a > b do	-- 不满足时跳出
    a = a - 1
end
```

---
### 2.3 循环控制语句 repeat

- ```repeat``` 循环语句的跳出条件是直至满足控制表达式

```lua
repeat
    <code body>
until <condition>
------------------------
-- 控制表达式括号是可选的
repeat
    a = a - 1
until a < b	-- 满足时跳出
```

---
### 2.4 循环语句 for

```lua
for i=exp1, exp2 [,exp3] do
    <code body>
end
--[[
    exp1 表示光标的起始值
    exp2 表示光标的最终值
    exp3 表示光标的迭代步长，默认值为 1，可选
]]--

for i=1,10 do
    print(i)
end
```

> 遍历列表时

- 若列表的索引是显式定义时，列表中存在连续空洞 ```nil``` 时，列表的长度比实际长度变小。当 ```for``` 遍历元素时，会出现中断现象

```lua
arr = { [1.0] = 1, [2] = 2, [4] = 4, [5] = 5, [7] = 7, [9] = 9 }
print(#arr) -- 5
for i = 1, #arr do
    print(arr[i]) ----- 1,2,nil,4,5  (中断)
end
```

---
#### 2.4.1 泛型 For 迭代器

- 泛型 For 在循环过程中在其内部保存了迭代函数。实际上的泛型 For 保存了三个值：一个迭代函数、一个不可变状态、一个控制变量

```lua
for	var-list in exp-list do
    body
end
-- var-list 一个或多个变量名组成的列表
-- exp-list 一个或多个表达式组成的列表
```

> 泛型 For 执行顺序

- ```for``` 先对 ```in``` 后面的表达式求值，并返回三个值供 ```for``` 保存：迭代函数、不可变状态和控制变量的初始值（只有最后一个表达式能够产生多个值）。表达式列表的结果只会保存三个（多余的被丢弃，不足时 ```nil``` 补全）
- 初始化完成后，```for``` 使用不可变状态和控制变量为参数来调用迭代函数。然后，```for``` 将迭代函数的返回值赋值给变量列表中声明的变量，若第一个返回值为 ```nil``` 时循环终止

```lua
for var_1, ..., var_n in explist do block end
---- 等价于
do
    local _f, _s, _var = explist
    while true do
        local var_1, ... var_n = _f(_s, _var)
        _var = _var_1
        if _var == nil then break end
        block
    end
end
```

> 内置泛型迭代器

- ```for...pairs``` 可用于迭代表中的所有键值对，包括列表部分，但迭代顺序不是固定

```lua
arr = {1,3,4,5,a="A",b="B"}
for k,v in pairs(arr) do	-- 键值对遍历
    print(k,v)
end
```

---
#### 2.4.2 无状态迭代器

- 无状态的迭代器是指不保留任何状态的迭代器，因此在循环中可以利用无状态迭代器避免创建闭包而花费额外的代价
- 每一次迭代，迭代函数都是用两个变量（状态常量和控制变量）的值作为参数被调用，一个无状态的迭代器只利用这两个值可以获取下一个元素

```lua
function func(maxCount,value) 
    if value < maxCount then
    value = value+1
    return value,value*2
    end
end
-----------------------
for i,v in func,5,0 do
    print(i,v)
end
--[[
    当5，0传入函数时，会将返回的值依次赋给i，v，然后 i，v 进入
循环体执行语句块, 后再次进入函数func，函数返回值赋值给 i,
v，直到函数控制变量不满足条件, 跳出函数作用域，迭代或循环体
结束。
    当函数一直处于无法 end 状态时, for 循环进入死循环
--]]
```

> 内置无状态迭代器

- 这类迭代器的典型例子是 ```ipairs```, 常用于迭代表的列表部分

```lua
arr = {1,3,4,5,a="A",b="B"}
for i,v in ipairs(arr) do	-- 列表遍历
    print(i,v)	-- 1,3,4,5
end
```

> 列表中存在 nil 的情况

- For 迭代器会跳过表中值为 nil 的元素；对于存在 ```nil``` 值的列表时，```ipairs``` 迭代器在首次遇到 ```nil``` 值时停止迭代，```pair``` 会跳过 ```nil```

```lua
a = { 1, 2, 3, 4, 5, 6, nil, 7, nil, nil, nil, 7}

print(#a)	-- 12
for i = 1, #a do
    print(a[i])		-- 1,2,3,4,5,6,nil,7,nil,nil,nil,7
end
print("-------")
for index, value in ipairs(a) do
    print(value)	-- 1,2,3,4,5,6
end
print("-------")
for index, value in pairs(a) do
    print(value)	-- 1,2,3,4,5,6,7,7
end
```

> ```ipairs```

```lua
local function iter(t, i)
    i = i + 1
    local v = t[i]
    if v then
        return i, v
    end
end

function ipairs(t)
    return iter, t, 0
end
```

> ```pairs```

- 泛型迭代器 ```pairs``` 的迭代函数实质上是 Lua 中的一个基本函数 ```next```

```lua
function pairs(t)
    return next, t, nil
end

for k,v in pairs(t) do block end
---- 等价于
for k,v in next,t do block end
```

---
#### 2.4.3 多状态迭代器

- 迭代器需要保存多个状态信息而不是简单的状态常量和控制变量，最简单的方法是使用闭包
- 还有一种方法就是将所有的状态信息封装到 table 内，将 table 作为迭代器的状态常量，因为这种情况下可以将所有的信息存放在 table 内，所以迭代函数通常不需要第二个参数

```lua
array = {1,2,3,4,5,6,7}
function elementIterator (collection)
    local index = 0
    local count = #collection
    -- 闭包函数
    return function ()
        index = index + 1
        if index <= count then
        --  返回迭代器的当前元素
            return collection[index]
        end
    end
end
for element in elementIterator(array) do
    print(element)
end
```

- 在 elementIterator 函数内定义了另外一个匿名函数。此匿名函数中使用了一个外部变量 index（译注：此变量在匿名函数之外，elementIterator 函数内）。每次内部的匿名函数被调用时，都会将 index 的值增加 1，并统计数返回的每个元素
- 利用函数的闭包原则内部嵌套函数存储临时变量，在 ```for``` 循环体持续更新，直至返回空 ```nil``` 时跳出循环
- 上述的迭代器有一个缺点，即需要为每个新的循环创建一个新的闭包

---
### 2.5 跳转语句 break、return 和 goto

- ```break``` 和 ```return``` 语句用于从当前的循环结构中跳出，```goto``` 则允许跳转到函数中的任何地方
- ```break``` 语句会中断包含它的内层循环，不影响外部循环。```break``` 中断后，程序会紧接着被中断的循环继续执行
- ```return``` 语句用于返回函数的执行结果或简单地结束函数的运行，所有的函数都有一个隐含的 ```return```，但是 ```return``` 只能是代码块中的最后一句。函数域内的 ```return``` 会跳出函数作用域返回调用方；文件作用域 ```return``` 之后的语句不再执行
- ```goto``` 语句用于将当前程序跳转到相应的标签处继续执行。Lua 中的 ```goto``` 设置了一些限制条件，不能直接跳转到一个代码块中的标签（该标签仅对当前块可见）；```goto``` 也不能跳转到函数外，不能跳转到局部变量的作用域

```lua
function Factorial(n)
    local rt = 1;
    ::start::
    if n == 0 then
        return rt
    else
        rt = rt * n
        n = n - 1
        goto start
    end
end

local i = 1
while (true) do
    if (i > 10) then
        break
    end
    print(i .. "  " .. Factorial(i))
    i = i + 1
end
```

> 模拟 redo、continue

```lua
while some_condition do
    ::redo::
    if some_other_condition then
        goto continue
    else if yet_another_condition then
        goto redo
    end
    <some_code>
    ::continue::
end
```

---
### 2.6 代码块 do ... end

- ```do end``` 块可以在文件或函数域出现

```lua
do
    <some-code>
end
```

---