#include "MemoryPool.h"

/**
 * @brief 创建内存池（PAGE_SIZE = sizeof(pool_t) + used-for-user）
 */
void MemoryPool::createPool()
{
    int ret = posix_memalign((void **)&pool_, MP_ALIGN, PAGE_SIZE);
    if (ret)
    {
        return;
    }

    pool_->small_block_info.latest = (uchar *)pool_ + sizeof(pool_t);
    pool_->small_block_info.end = (uchar *)pool_ + PAGE_SIZE;
    pool_->small_block_info.next = nullptr;
    pool_->small_block_info.quote = 0;
    pool_->small_block_info.failed = 0;

    auto size = PAGE_SIZE - sizeof(pool_t);
    pool_->available = size < MAX_ALLOC_FROM_POOL ? size : MAX_ALLOC_FROM_POOL;
    pool_->current = &(pool_->small_block_info);
    pool_->large_list = nullptr;
}

void *MemoryPool::mallocMemory(ulong _size)
{
    if (_size <= 0)
        return nullptr;
    if (_size > pool_->available)
    {
        return mallocLarge(_size);
    }
    else
    {
        return mallocSmall(_size);
    }
}

void *MemoryPool::mallocLarge(ulong _size)
{
    uchar *block;
    int ret = posix_memalign((void **)&block, MP_ALIGN, _size);
    if (ret)
        return nullptr;

    int cnt = 0;
    auto *large_block = pool_->large_list;
    while (large_block) // 寻找空的大内存块指针
    {
        if (!large_block->addr) //
        {
            large_block->addr = block;
            return block;
        }

        // 连续4次未找到，停止寻找 & 申请新的链表节点
        if (cnt++ > 3)
        {
            break;
        }

        large_block = large_block->next;
    }

    // 向内存池申请管理大内存块的节点，mallocSmall()
    large_block = (large_block_t *)this->mallocMemory(sizeof(large_block_t));
    if (!large_block) // 申请节点失败，释放posix_memalign()申请的内存
    {
        free(block);
        return nullptr;
    }
    large_block->addr = block;
    large_block->next = pool_->large_list;
    pool_->large_list = large_block;
    return block;
}

void *MemoryPool::mallocSmall(ulong _size)
{
    uchar *small_block;           // 待使用的小块内存的起始位置
    auto *block = pool_->current; // 当前使用的内存块

    while (block)
    {
        small_block = (uchar *)mp_align_ptr(block->latest, MP_ALIGN);
        // 当前内存块的空间足够
        if ((ulong)(block->end - small_block) >= _size)
        {
            block->latest = small_block + _size;
            return small_block;
        }

        // 空间不足：寻找下一个内存块
        block = block->next;
    }

    // 所有内存块的空间不足：重新申请内存块，用于小块内存分配
    return mallocBlock(_size);
}

void *MemoryPool::mallocBlock(ulong _size)
{
    uchar *block;
    int ret = posix_memalign((void **)&block, MP_ALIGN, PAGE_SIZE);
    if (ret)
        return nullptr;

    auto *small_block = (small_block_t *)block;
    small_block->end = block + _size;
    small_block->next = nullptr;

    // 新内存块内存对齐后的起始可用位置
    uchar *addr = (uchar *)mp_align_ptr(block + sizeof(small_block_t), MP_ALIGN);
    small_block->latest = addr;
    small_block->quote++;

    small_block_t *curr;
    for (curr = pool_->current; curr->next; curr = curr->next)
    {
        if (curr->quote++ >= 4)
        {
            pool_->current = curr->next;
        }
    }

    curr->next = small_block;
    return addr;
}

void *MemoryPool::callocMemory(ulong _size)
{
    void *addr = malloc(_size);
    if (!addr)
    {
        memset(addr, 0, _size);
    }
    return addr;
}

/**
 * @brief 仅提供大块内存的释放接口；小块内存在内存池销毁时释放
 * @param _ptr 指定释放的大块内存
 */
void MemoryPool::freeMemory(void *_ptr)
{
    large_block_t *large;
    for (large = pool_->large_list; large; large = large->next)
    {
        if (_ptr == large->addr)
        {
            free(large->addr);
            large->addr = nullptr;
            return;
        }
    }
}

void MemoryPool::destroyPool()
{
    // clearn-up function

    // 释放大块内存
    large_block_t *large;
    for (large = pool_->large_list; large; large = large->next)
    {
        if (large->addr)
        {
            free(large->addr);
            large->addr = nullptr;
        }
    }

    // 释放内存块（包含小块内存）
    small_block_t *curr, *forw;
    for (curr = &(pool_->small_block_info), forw = pool_->small_block_info.next;; curr = forw, forw = forw->next)
    {
        free(curr);
        if (forw == nullptr)
            break;
    }
}