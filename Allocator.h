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


// as it stands now this is just a class template and the functions return types
// and parameters are TBD

struct Block;    // ToDo, Define this
struct EndTag;   // ToDo, Define this

class Allocator
{
    void *const m_heapStart;                 // holds pointer to start of heap, return value of initial call to sbrk(0)
    void *m_headEnd;                         // holds a pointer to where the heap currently ends
    
    std::size_t m_heapSize;                  // current heap size in bytes
    std::size_t m_heapMemoryAllocated;       // total heap memory allocated in bytes will always be <= m_heapSize

    static bool m_allocatorActive;           // has the allocator been instantiated / activated
    
    public:

    Allocator();                   // Allocator contructor
    ~Allocator();                  // Allocator destructor

    void allocate();               // allocates a given amount of memory
    void free();                   // frees a section of memory
    
    void heapSize();               // returns size of the heap
    void memoryAllocated();        // return the number of bytes allocated

    void active();                 // returns wheather the Allocator is active or not

    private:

    void mergeBlocks();            // merges two blocks of memory together
    void splitBlock();             // splits a block of memory into 2 different pieces
    
    void expandHeap();             // expands the size of the heap
    void shortenHeap();            // shortens the size of the heap
};
