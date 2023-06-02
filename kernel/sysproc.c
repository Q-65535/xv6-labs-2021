#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
	uint64 va;
	uint64 npages;
	uint64 buf;
	pte_t *pte;

	uint64 record = 0;
	pagetable_t pagetable = myproc()->pagetable;

	if (argaddr(0, &va) < 0) {
		return -1;
	}
	if (argaddr(1, &npages) < 0) {
		return -1;
	}
	if (argaddr(2, &buf) < 0) {
		return -1;
	}

	for(int i = 0; i < npages && i < 64; i++) {
		uint64 a = va + i * PGSIZE;
		if((pte = walk(pagetable, a, 0)) == 0)
			panic("pgaccess: walk");
		// we don't deal with invalid pte.
		if((*pte & PTE_V) == 0)
			continue;
		if(PTE_FLAGS(*pte) == PTE_V)
			panic("pgaccess: not a leaf");

		if (*pte & PTE_A) {
			// record in the bitmap
			record |= (1L << i);
			*pte &= (~PTE_A);
		}
	}
	copyout(pagetable, buf, (char*)&record, sizeof(uint64));
	return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
