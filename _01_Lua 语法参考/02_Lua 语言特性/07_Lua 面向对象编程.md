### Lua 面向对象编程

---
- [1. Self 机制](#1-self-机制)
- [2. Class 类](#2-class-类)
	- [2.1 原型](#21-原型)
	- [2.2 继承](#22-继承)
- [3 私有性](#3-私有性)
- [4. 单方法对象](#4-单方法对象)
- [5. 对偶表示](#5-对偶表示)

---
## 1. Self 机制

- 使用参数 ```self``` 是所有面向对象语言的核心点。避免使用全局变量进行操作，把操作限定给特定对象工作。可以利用表的与值无关的 ```self``` 表示作为操作的接受者，来避免将操作仅限定在特定的全局变量中才能工作
- 冒号的作用是在一个方法调用中增加一个额外的实参，或在方法的定义中增加一个额外的隐藏形参

```lua
function t.foo(self, arg)    -- self 声明

t.foo(t, arg)	-- 调用 --> 等价于 <--
t:foo(arg)		-- : 表示隐藏 self 参数

-- 即使 t 被置换成别名，该操作和表对象本身无关
```


---
## 2. Class 类
### 2.1 原型

- Lua 可以利用原型的概念去实现面向对象编程，利用元表 ```__index``` 继承的方式（```setmetatable(A,{__index = B})```），让 B 成为 A 的一个原型。在此之后，A 就会在 B 中查找它没有的操作或字段

```lua
local prototype = {}
prototype.__index = prototype
function prototype.new(obj)
    obj = obj or {}
    setmetatable(obj, prototype)
    return obj
end
```

---
### 2.2 继承

- 利用 ```__index``` 和 ```self``` 机制可以用来实现继承

```lua
local metaclass = {}

---Create an object from metaclass
function metaclass:new(o)
    o = o or {}
    self.__index = self
    setmetatable(o, self)
    o.base = self			-- 关联超类
    return o
end

local o1 = metaclass:new()
o1.key1 = "hello"
local o2 = o1:new()	-- 单一继承
print(o2.key1)	-- hello
metaclass.key2 = 1
print(o2.key2)	-- 1
```

> 多重继承

- 多重继承意味着一个类可以具有多个超类，因此需要一个独立的方法（createClass）从一个类中创建子类，其参数为新类的所有超类

```lua
--- 在表 plist 的列表中查找 k
local function search(k, plist)
    for i = 1, #plist, 1 do
        local v = plist[i][k]
        if v then return v end
    end
end
--- 多重继承
function createClass(...)
    local c = {}
    local parent = { ... }

    setmetatable(c,
        { __index = function(t, k)
            return search(k, parent)
        end })

    function c:new()	-- 继承模式
        o = o or {}
        self.__index = self
        setmetatable(o, self)
        return o
    end
    return c
end
```

---
## 3 私有性

- 创造私有性的基本思想是通过两个表来表示一个对象，一个用来保存对象的状态，另一个保存对象的操作。通过第二个表来访问对象本身（通过接口进行访问），而表示对象状态的表只保存在方法的闭包中

```lua
function CtorClass()
    local privateStatus = {
        private = 1
    }
    local function extrefoo()
        print("extrefoo")
    end

    P1 = function() print(privateStatus.private) end
    P2 = function() extrefoo() end

    return {
        P1 = P1,
        P2 = P2
    }
end

local c = CtorClass()
c.P1()   -- 1
c.P2()   -- extrefoo
```

---
## 4. 单方法对象

- 单方法实际上是一个根据不同的参数完成不同任务的分发方法，例如一个在内部保存了状态的迭代器就是一个单方法对象（```io.lines```、```string.gmatch```）

```lua
function oneObj(key)
    return function(...)
        if key == "key1" then
            print("Invoke key1", ...)
        elseif key == "key2" then
            print("Invoke key2", ...)
        end
    end
end

local o = oneObj("key1")
o(1, 2, 3)  --  Invoke key1   1   2   3
```

---
## 5. 对偶表示

- 实现私有性的另一种方式是使用对偶表示：通常使用键把属性关联到表，使用对偶表示，把表当成键，同时把对象本身当作这个表的键

```lua
table[key] = value
----> 对偶
key[table] = value
```

---