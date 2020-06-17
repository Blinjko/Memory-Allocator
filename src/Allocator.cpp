#include "../include/Allocator.h"
#include <cstddef>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <errno.h>

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

    // set the heapMemoryAllocated to the space of 2 Tag structs
    // becuase 2 tag structs were made, the header and the footer,
    // and they must be factored into m_heapMemoryAllocated
    m_heapMemoryAllocated = sizeof(Tag)*2;
}

// Allocator splitBlock function
// splits the given block into a block with the segmentSegment size of newSize
// and creates a new block wich holds the remaining amount
Tag* Allocator::splitBlock(Tag *blockHeader, std::size_t newSize)
{
    // safety conditional
    if(blockHeader->segmentSize < newSize || !blockHeader->isHeader)
        return nullptr;

    else if(blockHeader->segmentSize == newSize)
        return blockHeader;

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

    // 2 new Tags were made, so make sure to
    // factor that into m_heapMemoryAllocated
    m_heapMemoryAllocated += sizeof(Tag)*2;

    return blockHeader;
}

// Allocator mergeBlocks function
// given a Tag pointer to a header of a block it will merge the blocks before and after the pointer
// with the block the pointer is pointing to, as long as they are free
Tag* Allocator::mergeBlocks(Tag *blockHeader)
{
    // safety conditional
    if(!blockHeader->isHeader || !blockHeader->free)
        return nullptr;

    Tag *blockFooter = blockHeader + (blockHeader->segmentSize + sizeof(Tag)) / sizeof(Tag);

    // if the header is not the first block and the previous block is free
    if(blockHeader != m_heapStart && (blockHeader - sizeof(Tag) / sizeof(Tag))->free)
    {
        // get the previous blocks footer
        Tag *previousBlockFooter = blockHeader - sizeof(Tag) / sizeof(Tag);

        // get the previous blocks header, which is the footer - segmentSize + sizeof(Tag)
        Tag *previousBlockHeader = previousBlockFooter - (previousBlockFooter->segmentSize + sizeof(Tag)) / sizeof(Tag);

        // calcuate the new segment size
        std::size_t segmentSize = previousBlockHeader->segmentSize + sizeof(Tag) + sizeof(Tag) + blockHeader->segmentSize;

        // change the blockHeader to the previous block header, since they have merged
        blockHeader = previousBlockHeader;

        // set the segmentSize of the header
        blockHeader->segmentSize = segmentSize; 

        // set the footer's segmentSize
        blockFooter->segmentSize = segmentSize;

        // 2 tags were removed when merging the 2 blocks
        // so subtract that amount from memory allocated
        // remember tags are inlcluded in m_heapMemoryAllocated
        m_heapMemoryAllocated -= sizeof(Tag)*2;
    }

    // if the block footer plus its size, is not equal to the heapEnd and the next block is free, execute this
    // + 1 is equal to sizeof(Tag) / sizeof(Tag), but had to put 1 here because compiler was complaining
    if((blockFooter + 1) != m_heapEnd && (blockFooter + sizeof(Tag) / sizeof(Tag))->free)
    {
        // get the next blocks header
        Tag *nextBlockHeader = blockFooter + sizeof(Tag) / sizeof(Tag);

        // get the next blocks footer
        Tag *nextBlockFooter = nextBlockHeader + (nextBlockHeader->segmentSize + sizeof(Tag)) / sizeof(Tag);

        // calculate segment size
        std::size_t segmentSize = blockHeader->segmentSize + sizeof(Tag) + sizeof(Tag) + nextBlockFooter->segmentSize;

        // set the new segmentSize for blockHeader
        blockHeader->segmentSize = segmentSize;

        // set the new segmentSize for the nextBlockFooter
        nextBlockFooter->segmentSize = segmentSize;

        // set the blockFooter to nextBlockFooter
        blockFooter = nextBlockFooter;

        // 2 Tags were removed so make sure to remove that
        // space from m_heapMemoryAlocated
        m_heapMemoryAllocated -= sizeof(Tag)*2;
    }
    return blockHeader;
}


// Allocator expandHeap
// returns nullptr if failed to expand heap
// returns m_heapEnd if successful
void* Allocator::expandHeap(std::size_t amount)
{
    // extend the heap
    void *result = sbrk(amount);
    
    // will execute if sbrk failed to expand the heap
    if(errno == ENOMEM)
    {
        errno = 0;
        return nullptr;
    }

    // create a new block
    Tag *newBlockHeader = static_cast<Tag*>(m_heapEnd);

    // set the segmentSize, and other variables
    newBlockHeader->segmentSize = amount - sizeof(Tag)*2;
    newBlockHeader->free = true;
    newBlockHeader->isHeader = true;

    // create the footer
    Tag *newBlockFooter = newBlockHeader + (newBlockHeader->segmentSize + sizeof(Tag)) / sizeof(Tag);

    // set the variables
    newBlockFooter->segmentSize = newBlockHeader->segmentSize;
    newBlockFooter->free = true;
    newBlockFooter->isHeader = false;

    // set m_heapEnd to the new heap end
    m_heapEnd = sbrk(0);

    // make sure to count in the new tags
    m_heapMemoryAllocated += sizeof(Tag)*2;

    // update m_heapSize;
    m_heapSize += amount;

    return m_heapEnd;
}

// Allocator allocate function
void* Allocator::allocate(std::size_t bytes)
{
    // check for negative
    if(bytes <= 0)
        return nullptr;


    // make sure the # of bytes is a multiple of bitAlignment
    // to ensure that the memory segment retured will
    // be aligned to a boundry of bitAlignment's value
    if(bytes % bitAlignment != 0)
    {
        bytes += bitAlignment - (bytes % bitAlignment);
    }

    // Tag pointer pointing to the first block's header
    Tag *blockHeader = static_cast<Tag*>(m_heapStart);

    // while not at the end of the heap or the block isn't free or the segmentSize is < bytes keep looping
    while((blockHeader != m_heapEnd) && (!blockHeader->free || (blockHeader->segmentSize < bytes )))
    {
        // move onto the next blocks header
        blockHeader = blockHeader + (blockHeader->segmentSize + sizeof(Tag)*2) / sizeof(Tag);
    }

    // there was no adequet block of memory availble
    if(blockHeader == m_heapEnd)
    {
        // attempt to expand the heap
        void *result = expandHeap(bytes + sizeof(Tag)*2);

        assert(result != nullptr && "Failed to allocate memory");
        
        // potentially helps with fragmentation
        blockHeader = mergeBlocks(blockHeader);
    }

    // split the block if needed
    blockHeader = splitBlock(blockHeader, bytes);

    // set the header free variable to false
    blockHeader->free = false;
    
    // get the footer
    Tag *blockFooter = blockHeader + (blockHeader->segmentSize + sizeof(Tag)) / sizeof(Tag);

    // set the free variable to false
    blockFooter->free = false;

    // add the amount allocated to total memory allocated
    m_heapMemoryAllocated += bytes;

    // return a pointer to the start of the data segment
    return blockHeader + sizeof(Tag) / sizeof(Tag);
}
