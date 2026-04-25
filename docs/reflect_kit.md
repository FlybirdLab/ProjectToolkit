# ReflectKit 工程设计文档（v0.1）

## 1. 项目目标

ReflectKit 是一套面向 C++ 的反射系统，设计目标：

* 保持源码为 **合法 C++（.hpp/.h）**
* 不依赖重度宏展开或侵入式语法
* 支持高性能反射（无 Variant）
* 支持类型注册、字段访问、函数调用
* 支持动态调用（编辑器 / 控制台 / 脚本）
* 支持增量编译（基于 TkMoc）
* 可扩展（GC / 脚本桥 / 编辑器）

---

## 2. 整体架构

```txt
C++ 源码（rclass / rfield / rfunc / rattr）
        ↓
TkMoc 扫描
        ↓
生成 .reflect.cpp
        ↓
TypeInitializer 注册
        ↓
TypeRegistry 管理
        ↓
Runtime（Type / Field / Method / Invoke）
```

---

## 3. 语法规范

### 3.1 宏定义

```cpp
#define rattr(...)
#define rclass class
#define rstruct struct
#define renum enum
#define rfield
#define rfunc
```

类内部必须声明：

```cpp
#define R_OBJECT(Type)
```

---

### 3.2 示例

```cpp
rattr(abstract)
rclass Actor : public Object
{
    R_OBJECT(Actor)

public:
    rattr(edit, serialize, category = "Transform")
    rfield Vec3 position;

    rattr(edit, serialize, category = "Stats")
    rfield int hp = 100;

    rattr(callable)
    rfunc void takeDamage(int value);
};
```

---

### 3.3 Attribute

```cpp
rattr(name)
rattr(name1, name2)
rattr(name = value)
```

规则：

* 只作用于紧随其后的声明
* 原始数据保留
* 常用项编译为 flags

---

## 4. TkMoc 设计

### 4.1 职责

* 扫描以下标记：

  * `rattr`
  * `rclass / rstruct / renum`
  * `rfield`
  * `rfunc`
  * `R_OBJECT`
* 提取符号信息
* 生成 `.reflect.cpp`

---

### 4.2 解析策略

* 标记驱动（非完整 C++ parser）
* 支持递归类型解析（有限子集）
* 跳过无关语法（函数体 / 初始化 / 复杂表达式）

---

### 4.3 宏处理

* 允许简单宏映射：

```cpp
#define MY_FIELD rfield
```

* 不实现完整预处理器

---

### 4.4 构建系统

使用：

```txt
CMake + Ninja + depfile
```

TkMoc 输出：

```txt
xxx.reflect.cpp
xxx.reflect.d
```

---

## 5. 类型系统

### 5.1 TypeHandle

```cpp
struct TypeHandle
{
    uint32 module;
    uint32 index;
};
```

---

### 5.2 TypeKind

```cpp
enum class TypeKind
{
    Class,
    Struct,
    Enum,
    Interface
};
```

---

### 5.3 ReflectType

```cpp
struct ReflectType
{
    TypeKind kind;
    TypeHandle handle;
    const char* name;
    const char* fullName;
    size_t size;
    size_t align;
};
```

---

### 5.4 ReflectClass

```cpp
struct ReflectClass : ReflectType
{
    TypeHandle baseType;

    ObjectSpawnFn spawn;
    ObjectDestroyFn destroy;

    ReflectField* fields;
    uint32 fieldCount;

    ReflectMethod* methods;
    uint32 methodCount;
};
```

---

## 6. 字段系统

```cpp
struct ReflectField
{
    const char* name;
    TypeHandle type;

    void* (*getPtr)(void* obj);

    FieldFlags flags;
    AttributeList attrs;
};
```

字段访问通过生成函数：

```cpp
void* Get_Actor_hp(void* obj)
{
    return &(static_cast<Actor*>(obj)->hp);
}
```

不依赖 `offsetof`。

---

## 7. 方法系统

```cpp
struct ReflectMethod
{
    const char* name;
    uint64 signatureHash;

    TypeHandle returnType;
    ReflectParam* params;
    uint32 paramCount;

    MethodFlags flags;

    MethodInvokeFn invoke;
};
```

---

## 8. 调用系统

### 8.1 核心结构

```txt
CallFrame
ArgValue
InvokeContext
ReturnValue
```

---

### 8.2 CallFrame

```cpp
class CallFrame
{
public:
    void* allocate(size_t size, size_t align);
    void addDestructor(void*, void(*)(void*));
    void reset();
};
```

* 栈内小 buffer
* fallback 到 arena
* 自动析构

---

### 8.3 ArgValue

```cpp
enum class ArgPassing
{
    Value,
    LValueRef,
    ConstLValueRef,
    RValueRef,
    Pointer
};

struct ArgValue
{
    TypeHandle type;
    void* data;
    ArgPassing passing;
    bool owns;
};
```

---

### 8.4 InvokeContext

```cpp
struct InvokeContext
{
    void* instance;
    ArgValue* args;
    uint32 argCount;
    void* result;
};
```

---

## 9. 调用 API

### 9.1 模板调用（推荐）

```cpp
method.invoke<void>(obj, 10);

int hp = method.invoke<int>(obj);

Vec3 pos = method.invoke<Vec3>(obj);

Vec3& ref = method.invokeRef<Vec3>(obj);
```

---

### 9.2 动态调用

```cpp
auto ret = method.call(obj)
    .arg(10)
    .cref(pos)
    .invoke();

int v = ret.as<int>();
```

---

### 9.3 底层调用

```cpp
CallFrame frame;

ArgValue args[...];

method.invokeRaw(obj, frame, args, count, result);
```

---

## 10. 返回值模型

```cpp
enum class ReturnKind
{
    Value,
    LValueRef,
    ConstLValueRef,
    Pointer
};
```

规则：

* Value：安全
* Ref：调用方保证生命周期
* Pointer：不拥有对象

---

## 11. Object 系统

```cpp
class Object
{
public:
    virtual ReflectClass* getClass() const = 0;
};
```

---

### 11.1 Handle 机制

对外使用：

```cpp
ObjectHandle
```

避免直接暴露裸指针。

---

### 11.2 ObjectFactory

```cpp
Object* create(ReflectClass*);
void destroy(Object*);
```

---

## 12. 类型注册

### 12.1 TypeInitializer

由 `.reflect.cpp` 生成：

```cpp
TypeInitializer init(...);
```

---

### 12.2 注册阶段

两阶段：

```txt
Phase 1：注册 TypeDesc
Phase 2：resolve 类型依赖
```

---

## 13. Method 重载

通过 signature hash：

```txt
name + 参数类型 + 返回类型
```

查找：

```cpp
findMethod(name, paramTypes)
```

---

## 14. Attribute 系统

```cpp
struct Attribute
{
    const char* name;
    const char* value;
};
```

策略：

```txt
字符串保留 + 常用项编译为 flags
```

---

## 15. 安全规则

### 15.1 参数绑定

| 类型       | 规则          |
| -------- | ----------- |
| T        | 拷贝/移动       |
| const T& | 可绑定临时       |
| T&       | 只能绑定 lvalue |
| T&&      | 必须 rvalue   |
| T*       | 传值          |

---

### 15.2 生命周期

* owned：CallFrame 管理
* borrowed：外部管理
* 返回引用不能指向临时

---

## 16. 约束（当前版本）

不支持：

* 模板函数反射
* 默认参数
* C 风格数组
* 复杂宏展开
* move-only 类型（默认）

---

## 17. 关键设计决策总结

```txt
1. 不使用 Variant
2. 使用 CallFrame 管理调用生命周期
3. 使用 ArgValue 表达参数语义
4. 使用 Handle 替代长期裸指针
5. 使用生成 accessor 替代 offsetof
6. 使用 signature hash 支持重载
7. 使用 depfile 支持增量编译
8. 使用两阶段注册解决依赖问题
```
