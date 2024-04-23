#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "base/Mutex.h"

// 定义一个内存分配器类
class Allocator
{
public:
    // 定义一些常量，用于内存对齐和自由链表的大小
    // 1. 作用域：enum定义的常量在类的作用域内，而直接定义的常量（例如const int ALIGN = 8或#define ALIGN 8）在全局作用域。
    // 在类内部使用enum可以避免全局命名空间的污染。
    // 2. 整型常量表达式：enum定义的是整型常量表达式，可以用在需要编译时常量的地方，例如数组的大小、模板参数等。
    // 而const int定义的常量在某些编译器中可能不被视为编译时常量。
    enum {ALIGN = 8};
    enum {MAX_BYTES = 128};
    enum {NFREELISTS = MAX_BYTES / ALIGN};

    // 定义一个联合体，用于自由链表的节点
    union Obj {
        union Obj* next;
        char data[1];
    };

    // 提供公共的内存分配和释放接口
    static void* allocate(uint32_t size);

    static void deallocate(void* p, uint32_t size);

private:
    // 构造函数和析构函数
    Allocator() : mStartFree(nullptr), mEndFree(nullptr), mHeapSize(0)
    {
        mMutex = new Mutex;
        memset(mFreeList, 0, sizeof(mFreeList));
    };

    ~Allocator() {

    };

    // 获取单例实例
    static Allocator* getInstance();

    // 私有的内存分配和释放接口
    void* alloc(uint32_t size);
    void dealloc(void* p, uint32_t size);

    // 获取指定字节数在自由链表的下标
    uint32_t freelistIndex(uint32_t bytes) {
        return (((bytes) + ALIGN-1) / ALIGN - 1);
    }

    // 字节对齐
    uint32_t roundup(uint32_t bytes) {
        return (((bytes) + ALIGN-1) & ~(ALIGN - 1));
    }

    // 从内存池中获取内存
    void *refill(uint32_t bytes);
    // 从系统中获取内存
    char* chunkAlloc(uint32_t size, int& nobjs);

private:
    // 单例实例
    static Allocator* mAllocator;

    // 互斥锁
    Mutex* mMutex;

    // 维护缓存块
    char* mStartFree;
    char* mEndFree;
    uint32_t mHeapSize;

    // 自由链表
    Obj* mFreeList[NFREELISTS];

};

#endif //_ALLOCATOR_H_