// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// @Hardcode: pre-defined number of physical pages are assumed.
int ref_counts[PHYSTOP/PGSIZE];

uint64
pa2index(uint64 pa) {
  return pa / PGSIZE;
}

void
increment_ref(uint64 pa) {
  uint64 index = pa2index(pa);
  ref_counts[index]++;
}
void
decrement_ref(uint64 pa) {
  uint64 index = pa2index(pa);
  ref_counts[index]--;
}

int
get_ref_count(uint64 pa) {
  uint64 index = pa2index(pa);
  return ref_counts[index];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int count = 0;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
	int index = pa2index((uint64)p);
	ref_counts[index] = 1;
    kfree(p);
	count++;
  }
  printf("num of phyiscal pages: %d\n", count);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  decrement_ref((uint64)pa);
  int ref_count = get_ref_count((uint64)pa);
  /* printf("ref count for %p[%d]: %d\n", pa, pa2index((uint64)pa), ref_count); */

  if (ref_count == 0) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    // only when the reference count is 0 we free it.
	acquire(&kmem.lock);
	r->next = kmem.freelist;
	kmem.freelist = r;
	release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
	increment_ref((uint64)r);
  }

  return (void*)r;
}
