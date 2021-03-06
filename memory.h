#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "paging.h"
#include "multiboot.h"
#include "memory_zones.h"
#include "vector.h"

#define MM_HEAP_ALLOC_CANARY_SIZE 5
#define MM_HEAP_ALLOC_CANARY_VALUE "QUACK"


enum mm_alloc_types // TODO DOCUMENT
{
    MEM_UNALLOC,
    MEM_ALLOC,
    MEM_STUB
};

enum mm_alloc_flags
{
    MEM_NOFLAGS = 0,
    MEM_CHECKED = 1, // Alloc has a canary on the last 5 bytes
    MEM_ZEROMEM = 2,
    MEM_PLACEHOLDER = 4
};

enum mm_free_flags
{
    MEM_FREENONE = 0,
    
};

struct m_allocation
{
    uint32_t size;
    void* p;
    BOOL allocated;

    enum mm_alloc_types type;
    uint32_t flags;

    struct m_allocation* previous;
    struct m_allocation* next;
};

enum heap_flags
{
    FHEAP_NONE = 0
};

struct m_heap
{
    enum heap_flags hflags;
    
    uint32_t startAddress;
    uint32_t endAddress;
    
    size_t allocs_count;
    struct m_allocation* firstAlloc;
    struct m_allocation* lastAlloc;
};

struct mm_zonedata
{
    uint32_t bios_start;
    uint32_t bios_end;
    
    uint32_t sections_start;
    uint32_t sections_end;
    
    uint32_t pfm_start;
    uint32_t pfm_end;
    
    uint32_t kpt_start;
    uint32_t kpt_end;
    
    uint32_t modules_start;
    uint32_t modules_end;
    
    uint32_t kheap_start;
    uint32_t kheap_end;
};

#define HEAP kernel_info->m_memory_manager->kernel_heap
//#define HEAP ks_get_current()->task_heap
#define HEAP_LENGTH (HEAP->endAddress - HEAP->startAddress)
#define DEFAULT_HEAP_SPACE 1024*1024*20

// size_t allocs_count;
// struct m_allocation* firstAlloc;
// struct m_allocation* lastAlloc;

struct memory_manager_module
{
    uint32_t memory_start;
    uint32_t memory_length;
    
    struct mm_zonedata zones;
    
    struct m_heap* kernel_heap; // Initial Kernel Heap
    struct vector heaps;
    
    size_t allocs_count;
    struct m_allocation* allocs;

    struct m_allocation* firstAlloc;
    struct m_allocation* lastAlloc;
};
struct memory_manager_module* mm_module;

// Internal methods

void init_memory_manager(struct kernel_info_block* kinfo, multiboot_info_t* mbi);

// Public API

// Standard C methods
#define malloc kmalloc
#define free kfree
#define memcpy kmemcpy

// Alloc functions targeting the Kernel Heap
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void* krealloc( void *ptr, uint32_t new_size );

// Alloc functions targeting the current process heap.
void* vmalloc(uint32_t size);
void vmfree(void* p);

// Extended methods
void* kmemcpy( void *dest, const void *src, uint32_t count );

void* kmallocf(uint32_t size, enum mm_alloc_flags f, struct m_heap* target);
void kfreef(void* ptr, struct m_heap* target);

void kmemplace(void* dest, uint32_t offset, const char* data, size_t count);
void kmemget(void* src, char* dest, uint32_t offset, size_t count, size_t* readSize);

// Heap methods
struct m_heap* mm_create_heap(enum heap_flags flags);

// Memory zones
void mm_zone_find_largest(multiboot_info_t* mbi, uint32_t* start, uint32_t* length);
void mm_calculate_zones(uint32_t start, uint32_t length, struct mm_zonedata* data);

// Private Methods

// Mem Allocations methods
uint32_t mm_get_space(struct m_allocation* first, struct m_allocation* second);

uint32_t mm_data_head(struct m_allocation* target);
uint32_t mm_data_tail(struct m_allocation* target);
uint32_t mm_space_to_end(struct m_allocation* target);

void mm_link_allocs(struct m_allocation* first, struct m_allocation* second);

struct m_allocation* mm_find_free_allocation();
struct m_allocation* mm_find_free_space(size_t bytes);

void  mm_set_alloc_canary(struct m_allocation* alloc);
BOOL mm_verify_alloc_canary(struct m_allocation* alloc);
BOOL mm_verify_all_allocs_canary();

// Heaps methods
uint32_t mm_find_heap_space(size_t heapSize);
uint32_t mm_heaps_spacing(struct m_heap* first, struct m_heap* second);
uint32_t mm_heaps_space_to_end(struct m_heap* lastHeap);

#endif
