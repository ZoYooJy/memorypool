#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

#define DEFAULT_POOL_SIZE (16 * 1024)
#define PAGE_SIZE 4096
#define MAX_ALLOC_FROM_POOL (PAGE_SIZE - 1)
#define MP_ALIGN 16
#define mp_align(n, a) (((n) + (a - 1)) & ~(a - 1))
#define mp_align_ptr(p, a) (void *)((((size_t)p) + ((a)-1)) & ~((a)-1))

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

// 当前内存块的信息
struct small_block_t
{
    small_block_t *next; // 指向下一个内存块
    uchar *latest;       // 指向该内存块已分配内存的末尾地址，下一个待分配内存的起始地址
    uchar *end;          // 指向该内存块的末尾地址
    uint failed;         // 当前内存块分配空间失败的次数
    uint quote;          // 当前内存块分配空间的次数
};

struct large_block_t
{
    large_block_t *next; // 指向下一个大块内存节点
    void *addr;          // 指向实际分配的大块的内存
};

struct pool_t
{
    small_block_t small_block_info; // 内存块管理信息
    size_t available;               // PAGE_SIZE - sizeof(pool_t, 不会改变，用于大/小内存块的选择
    small_block_t *current;         // 指向起始可分配的内存块
    large_block_t *large_list;      // 大块内存链表管理指针，指向的节点存储在内存池的小块内存
    // TODO log
    // TODO callback for clean-up
};

class MemoryPool
{
  public:
    MemoryPool() = default;
    ~MemoryPool() = default;

    void createPool();
    void destroyPool();
    void resetPool();

    void *mallocMemory(ulong _size);
    void *callocMemory(ulong _size);

    void freeMemory(void *_ptr);

    pool_t *getPool()
    {
        return pool_;
    }

  private:
    void *mallocLarge(ulong _size);
    void *mallocSmall(ulong _size);
    void *mallocBlock(ulong _size);

    // Pool *pool_ = nullptr;
    pool_t *pool_ = nullptr;
};