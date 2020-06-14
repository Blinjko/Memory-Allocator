#include "../include/Allocator.h"
#include <cstddef>
#include <unistd.h>
#include <iostream>

using namespace Alloc;

// define the m_allocatorActive static variable
const bool Allocator::m_allocatorActive = false;


// Allocator class constructor
Allocator::Allocator(std::size_t initialSize) :
    m_heapStart{sbrk(0)}
{
    if(initialSize % bitAlignment != 0)
    {
        // ensures the initial size is a multiple of bitAlignments value, by default it's 16
        initialSize += bitAlignment - (initialSize % bitAlignment);
    }

    // set the heap size to initial size
    m_heapSize = initialSize;

    // move the system break initialSize amount of bytes
    sbrk(initialSize);

    // set m_heapEnd to the end of the heap
    m_heapEnd = sbrk(0);

    // make a header for the new block of memory requested
    Tag *initialBlockHeader = static_cast<Tag*>(m_heapStart);

    // the segmentSize is the memory to be delivered by an allocate request
    // segmentSize = the initial heaps size - the size of the tags
    initialBlockHeader->segmentSize = m_heapSize - (sizeof(Tag)*2);

    // set the free variable to true
    initialBlockHeader->free = true;

    // set the isHeader variable to true
    initialBlockHeader->isHeader = true;

    // make the footer tag
    Tag *initialBlockFooter = initialBlockHeader + (sizeof(Tag) + initialBlockHeader->segmentSize) / sizeof(Tag);

    // set the footers segmentSize
    initialBlockFooter->segmentSize = initialBlockHeader->segmentSize;
    
    initialBlockFooter->free = true;
    initialBlockFooter->isHeader = false;
}
