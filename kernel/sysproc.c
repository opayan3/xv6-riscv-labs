#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;

  addr = p->sz;
  if (n == 0)
    return addr;

  uint64 new_sz = addr + n;
  if(new_sz < p->sz){
    return (uint64)-1;
  }
  p->sz = new_sz;
  /*old eager allocatoin, we don't call growproc right away for lazy allocatoin*/
  /*if(growproc(n) < 0)
    return -1;*/
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

uint64
sys_freepmem(void)
{
  uint64 pages = kfreepages_count();
  return pages * PGSIZE;
}

uint64
sys_sem_init(void)
{
  uint64 useraddr;
  int shared;
  int value;

  if (argaddr(0, &useraddr) < 0 || argint(1, &shared) < 0 || argint(2, &value) < 0)
    return -1;

  int id = semalloc();
  if (id < 0)
    return -1;

  struct semaphore *s = &semtable.sem[id];
  acquire(&s->lock);
  s->count = value;
  release(&s->lock);

  sem_t semid = id;
  struct proc *p = myproc();
  if (copyout(p->pagetable, useraddr, (char*)&semid, sizeof(semid)) < 0) {
    semdealloc(id);
    return -1;
  }
  return 0;
}

uint64
sys_sem_destroy(void)
{
  uint64 useraddr;
  if (argaddr(0, &useraddr) < 0)
    return -1;
  struct proc *p = myproc();
  sem_t semid;
  if (copyin(p->pagetable, (char*)&semid, useraddr, sizeof(semid)) < 0)
    return -1;

  struct semaphore *s = get_semaphore(semid);
  if (s == 0)
    return -1;

  semdealloc(semid);
  return 0;

}

uint64
sys_sem_wait(void)
{
  uint64 useraddr;
  if (argaddr(0, &useraddr) < 0)
    return -1;

  struct proc* p = myproc();
  sem_t semid;
  if (copyin(p->pagetable, (char*)&semid, useraddr, sizeof(semid)) < 0)
    return -1;

  struct semaphore *s = get_semaphore(semid);
  if (s == 0)
    return -1;

  acquire(&s->lock);
  while (s->count == 0)
  {
    sleep(s, &s->lock);
  }
  s->count--;
  release(&s->lock);

  return 0;
}

uint64
sys_sem_post(void)
{
  uint64 useraddr;
  if (argaddr(0, &useraddr) < 0)
    return -1;

  struct proc *p = myproc();
  sem_t semid;
  if (copyin(p->pagetable, (char*)&semid, useraddr, sizeof(semid)) < 0)
    return -1;

  struct semaphore *s = get_semaphore(semid);
  if (s == 0)
    return -1;

  acquire(&s->lock);
  s->count++;
  wakeup(s);
  release(&s->lock);

  return 0;
}
