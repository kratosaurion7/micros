#include "memory.h"

#include "kernel_log.h"
#include "common.h"
#include "string.h"

void kmInitManager()
{
    basePoolsAddress = 1024 * 1024 * 5; // 1MB
    
    uint32_t smallPoolStartAddress = basePoolsAddress;
    uint32_t pagePoolStartAddress = smallPoolStartAddress + small_pool_size * small_pool_unit;
    uint32_t largePoolStartAddress = pagePoolStartAddress + page_pool_size + page_pool_unit;
    
    for(int i = 0; i < small_pool_size; i++)
    {
        smallPool[i].size = 256;
        smallPool[i].isFree = TRUE;
        smallPool[i].p = (void*)(basePoolsAddress + 256 * i);
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        pagePool[i].size = 4096;
        pagePool[i].isFree = TRUE;
        pagePool[i].p = (void*)(pagePoolStartAddress + 4096*i);
    }
    
    for(int i = 0; i < large_pool_size; i++)
    {
        largePool[i].size = 32 * 1024;
        largePool[i].isFree = TRUE;
        largePool[i].p = (void*)(largePoolStartAddress + 32 * 1024 * i);
    }
    
    // Start the allocations at the 45MB mark
    kmNextAvailableMemoryAddress = 45 * (1024 * 1024);
}

void* kmKernelAlloc(size_t size)
{
    // kmNextAvailableMemoryAddress += size;
    // return (void*)kmNextAvailableMemoryAddress;
    //
    if(size <= small_pool_unit)
    {
        for(int i = 0; i < small_pool_size; i++)
        {
            if(smallPool[i].isFree)
            {
                smallPool[i].isFree = FALSE;
                
                return smallPool[i].p;
            }
        }
        
        Debugger();
        kWriteLog("Small pool is full !");
    }
    else if(size <= page_pool_unit)
    {
        for(int i = 0; i < page_pool_size; i++)
        {
            if(pagePool[i].isFree)
            {
                pagePool[i].isFree = FALSE;
                
                return pagePool[i].p;
            }
        }
        
        Debugger();        
        kWriteLog("Page pool is full !");
    }
    else if(size <= large_pool_unit)
    {
        for(int i = 0; i < large_pool_size; i++)
        {
            if(largePool[i].isFree)
            {
                largePool[i].isFree = FALSE;
                
                return largePool[i].p;
            }
        }
        
        Debugger();
        kWriteLog("Large pool is full !");
    }
    else
    {
        // Allocation is too big, not supported for now.
        kWriteLog("Trying to allocate too much.");
        
        Debugger();
    }
    
    
    return (void*)0;
    // kmNextAvailableMemoryAddress += size;
    // return (void*)kmNextAvailableMemoryAddress;
}

void kmKernelFree(void* ptr)
{
    // TODO : For now, no other way than checking each allocation unit
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].p == ptr)
        {
            largePool[i].isFree = TRUE;
            
            return;
            // Maybe zero out the memory or something.
        }
    }
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].p == ptr)
        {
            pagePool[i].isFree = TRUE;
            
            return;
        }
    }

    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].p == ptr)
        {
            smallPool[i].isFree = TRUE;
            
            return;
        }
    }
    
    kWriteLog_format1d("Could not find allocation for %d", (uint32_t)ptr);
}

void kmKernelCopy(void* ptrFrom, void* ptrTo)
{
    char* ptrFromChar = (char*)ptrFrom;
    char* ptrToChar = (char*)ptrTo;
    
    size_t fromSize = sizeof(*ptrFrom);
    
    for(size_t i = 0; i < fromSize; i++)
    {
        ptrToChar[i] = ptrFromChar[i];
    }
}

void kmKernelZero(void* ptrFrom)
{
    char* ptrFromChar = (char*)ptrFrom;
    
    size_t fromSize = sizeof(*ptrFrom);
    
    for(size_t i = 0; i < fromSize; i++)
    {
        ptrFromChar[i] = 0;
    }
}


size_t kmCountFreeSmallPoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}

size_t kmCountFreePagePoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}

size_t kmCountFreeLargePoolUnits()
{
    size_t total = 0;
    
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].isFree)
        {
            total++;
        }
    }
    
    return total;
}

struct memstats* kmGetMemoryStats()
{
    struct memstats* stats = kmKernelAlloc(sizeof(struct memstats));
    
    // SMALL POOL
    stats->small_pool_count = small_pool_size;
    stats->small_pool_used = 0;
    stats->small_pool_free = 0;
    stats->small_pool_mem_unit = small_pool_unit;
    stats->small_pool_mem_used = 0;
    stats->small_pool_mem_free = 0;
    for(int i = 0; i < small_pool_size; i++)
    {
        if(smallPool[i].isFree)
        {
            stats->small_pool_free++;
        }
        else
        {
            stats->small_pool_used++;
            stats->small_pool_mem_used += small_pool_unit;
        }
    }
    stats->small_pool_mem_free = (small_pool_size * small_pool_unit) - stats->small_pool_mem_used;
    
    // PAGE POOL
    stats->page_pool_count = page_pool_size;
    stats->page_pool_used = 0;
    stats->page_pool_free = 0;
    stats->page_pool_mem_unit = page_pool_unit;
    stats->page_pool_mem_used = 0;
    stats->page_pool_mem_free = 0;
    for(int i = 0; i < page_pool_size; i++)
    {
        if(pagePool[i].isFree)
        {
            stats->page_pool_free++;
        }
        else
        {
            stats->page_pool_used++;
            stats->page_pool_mem_used += page_pool_unit;
        }
    }
    stats->page_pool_mem_free = (page_pool_size * page_pool_unit) - stats->page_pool_mem_used;

    // LARGE POOL
    stats->large_pool_count = large_pool_size;
    stats->large_pool_used = 0;
    stats->large_pool_free = 0;
    stats->large_pool_mem_unit = large_pool_unit;
    stats->large_pool_mem_used = 0;
    stats->large_pool_mem_free = 0;
    for(int i = 0; i < large_pool_size; i++)
    {
        if(largePool[i].isFree)
        {
            stats->large_pool_free++;
        }
        else
        {
            stats->large_pool_used++;
            stats->large_pool_mem_used += large_pool_unit;
        }
    }
    stats->large_pool_mem_free = (large_pool_size * large_pool_unit) - stats->large_pool_mem_used;
    
    stats->total_alloc_amount = small_pool_size + page_pool_size + large_pool_size;
    stats->total_alloc_used = 0;
    stats->total_alloc_used += stats->small_pool_used;
    stats->total_alloc_used += stats->page_pool_used;
    stats->total_alloc_used += stats->large_pool_used;
    stats->total_alloc_free = stats->total_alloc_amount - stats->total_alloc_used;

    stats->total_memory_amount = (small_pool_unit * small_pool_size) + (page_pool_unit * page_pool_size) + (large_pool_unit * large_pool_size);
    stats->total_memory_used = 0;
    stats->total_memory_used += stats->small_pool_mem_used;
    stats->total_memory_used += stats->page_pool_mem_used;
    stats->total_memory_used += stats->large_pool_mem_used;
    stats->total_memory_free = stats->total_memory_amount - stats->total_memory_used;
    
    return stats;
}

char** kmGetMemoryStatsText(int* linesCount)
{
    int currentLine = 0;
    int nbLinesTotal = 24;
    char** statsLines = kmKernelAlloc(sizeof(char*) * nbLinesTotal); // 24 fields in memstats
    struct memstats* stats = kmGetMemoryStats();
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL COUNT = %d", stats->small_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL USED = %d", stats->small_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL FREE = %d", stats->small_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM UNIT = %d", stats->small_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM USED = %d", stats->small_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "SMALL POOL MEM FREE = %d", stats->small_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL COUNT = %d", stats->page_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL USED = %d", stats->page_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL FREE = %d", stats->page_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM UNIT = %d", stats->page_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM USED = %d", stats->page_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "PAGE POOL MEM FREE = %d", stats->page_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL COUNT = %d", stats->large_pool_count, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL USED = %d", stats->large_pool_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL FREE = %d", stats->large_pool_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM UNIT = %d", stats->large_pool_mem_unit, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM USED = %d", stats->large_pool_mem_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "LARGE POOL MEM FREE = %d", stats->large_pool_mem_free, NULL);currentLine++;
    
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS SLOTS = %d", stats->total_alloc_amount, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS USED = %d", stats->total_alloc_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL ALLOCS FREE = %d", stats->total_alloc_free, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY AMOUNT = %d", stats->total_memory_amount, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY USED = %d", stats->total_memory_used, NULL);currentLine++;
    statsLines[currentLine] = alloc_sprintf_1d(statsLines[currentLine], "TOTAL MEMORY FREE = %d", stats->total_memory_free, NULL);currentLine++;
    
    ASSERT(currentLine == nbLinesTotal, "WRONG AMOUNT OF LINES WRITTEN");
    
    *linesCount = nbLinesTotal;
    return statsLines;
}
