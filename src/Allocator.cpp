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

// splits the given block into a block with the segmentSegment size of newSize
// and creates a new block wich holds the remaining amount
Tag *Allocator::splitBlock(Tag* blockHeader, std::size_t newSize)
{
    // safety conditional
    if(blockHeader->segmentSize < newSize || !blockHeader->isHeader)
        return nullptr;

    // make a footer for the new block
    Tag *newBlockFooter = blockHeader + (sizeof(Tag) + newSize) / sizeof(Tag);

    // set the first block's footers variables
    newBlockFooter->segmentSize = newSize;
    newBlockFooter->free = blockHeader->free;
    newBlockFooter->isHeader = false;


    // make the second block's header
    Tag *secondBlockHeader = newBlockFooter + sizeof(Tag) / sizeof(Tag);

    // set the second block's header variables
    // every time a block spilts is loses sizeof(Tag) * 2 bytes for the new footer and new header needed
    secondBlockHeader->segmentSize = blockHeader->segmentSize - (newSize + sizeof(Tag)*2);
    secondBlockHeader->free = true;
    secondBlockHeader->isHeader = true;

    // set the second block's footer
    Tag *secondBlockFooter = secondBlockHeader + (sizeof(Tag) + secondBlockHeader->segmentSize) / sizeof(Tag);
    
    // set the second block's footer's variables
    secondBlockFooter->segmentSize = secondBlockHeader->segmentSize;
    secondBlockFooter->free = true;
    secondBlockFooter->isHeader = false;
    
    // set the original block headers segmentSize to the newSize
    blockHeader->segmentSize = newSize;

    return blockHeader;
}
