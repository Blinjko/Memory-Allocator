/*
Copyright 2020 Blinjko

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include <cstddef>

// as it stands now this is just a class template and the functions return types
// and parameters are TBD

namespace Alloc
{
const std::size_t bitAlignment = 16;// bit alignment, every piece of memory handled will be a multiple of this

struct Tag
{
    std::size_t segmentSize;        // size of the memory segment, not including the header tag or footer tag
    bool free;                      // is this block of memory free
    bool isHeader;                  // is this tag a header true = yes , false = no
};

class Allocator
{
    void *const m_heapStart;                 // holds pointer to start of heap, return value of initial call to sbrk(0)
    void *m_heapEnd;                         // holds a pointer to where the heap currently ends
    
    std::size_t m_heapSize;                  // current heap size in bytes
    std::size_t m_heapMemoryAllocated;       // total heap memory allocated in bytes will always be <= m_heapSize, includes Tags

    public:
    static const bool m_allocatorActive;     // has the allocator been instantiated / activated

    Allocator(std::size_t);                  // Allocator contructor
    ~Allocator();                            // Allocator destructor

    void* allocate(std::size_t);             // allocates a given amount of memory
    void deallocate(void*);                  // frees the memory block in posesion of the pointer given
    
    void heapSize();                         // returns size of the heap
    void memoryAllocated();                  // return the number of bytes allocated

    void active();                           // returns wheather the Allocator is active or not

    private:

    Tag* mergeBlocks(Tag*);                  // given a pointer it merges the bock before with the pointer and the block after it with the pointer as long as they are free
    Tag* splitBlock(Tag*, std::size_t);      // splits a block of memory into 2 different pieces given a header pointer and the blocks new size
    
    void* expandHeap(std::size_t);           // expands the size of the heap, returns nullptr if it fails, and the new heapEnd if successful
    void* shortenHeap(std::size_t);          // shortens the heap by the number of bytes passed, returns nullptr if fail, and new m_heapEnd if successful
};

}
