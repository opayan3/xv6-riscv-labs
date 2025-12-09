#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

struct semtab semtable;

void
seminit(void)
{
  initlock(&semtable.lock, "semtable");
  for (int i = 0; i < NSEM; i++){
    initlock(&semtable.sem[i].lock, "sem");
    semtable.sem[i].count = 0;
    semtable.sem[i].valid = 0;
  }
}

int
semalloc(void)
{
  acquire(&semtable.lock);
  for (int i = 0; i < NSEM; i++){
    if(!semtable.sem[i].valid){
      semtable.sem[i].valid = 1;
      release(&semtable.lock);
      return i;
    }
  }
  release(&semtable.lock);
  return -1;
}

void
semdealloc(int i)
{
  if (i < 0 || i>= NSEM)
    return;

  acquire(&semtable.lock);
  semtable.sem[i].valid = 0;
  semtable.sem[i].count = 0;
  release(&semtable.lock);
}

struct semaphore*
get_semaphore(int id)
{
  if (id < 0 || id >= NSEM)
    return 0;
  struct semaphore *s = &semtable.sem[id];
  if (!s->valid)
    return 0;
  return s;
}
