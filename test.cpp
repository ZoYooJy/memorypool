#include "MemoryPool.h"
#include <chrono>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

#define ALLOCATE_COUNT 1000000
#define ALLOCATE_SIZE 10

void testMalloc()
{
    void *memory[ALLOCATE_COUNT];
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < ALLOCATE_COUNT; i++)
    {
        memory[i] = malloc(ALLOCATE_SIZE);
    }
    auto end = std::chrono::steady_clock::now();
    cout << "malloc cost " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us"
         << endl;

    start = std::chrono::steady_clock::now();
    for (int i = 0; i < ALLOCATE_COUNT; i++)
    {
        free(memory[i]);
    }
    end = std::chrono::steady_clock::now();
    cout << "free cost " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us" << endl;
}

void testMemoryPool()
{
    void *memory[ALLOCATE_COUNT];
    MemoryPool pool;
    pool.createPool();

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < ALLOCATE_COUNT; i++)
    {
        memory[i] = pool.mallocMemory(ALLOCATE_SIZE);
    }
    auto end = std::chrono::steady_clock::now();
    cout << "memory pool malloc cost " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
         << "us" << endl;

    start = std::chrono::steady_clock::now();
    pool.destroyPool();
    end = std::chrono::steady_clock::now();
    cout << "memory pool free cost " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
         << "us" << endl;
}

int main(int argc, char **argv)
{
    testMalloc();
    cout << endl;
    testMemoryPool();

    return 0;
}