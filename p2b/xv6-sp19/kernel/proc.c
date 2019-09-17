#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int time_slice[4] = {0, 32, 16, 8};

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
	
  p->priority = 3;
  p->inuse = 1;
  for(int i =0;i < 4;i++)
  	p->ticks[i] = 0;
  for(int i =0;i < 4;i++)
  	p->wait_ticks[i] = 0;
  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  cprintf("proc.c userinit - pid: %d\n", p->pid);


  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  cprintf("proc.c fork(): pid:%d\n", proc->pid);
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

	// loop to to get the highest priority, then execute from there 
	int max = -1;
	struct proc *p5;
	for(p5 = ptable.proc; p5 < &ptable.proc[NPROC]; p5++){
		if(p5->state != RUNNABLE) 
			continue;
		if(p5->priority > max)
			max = p5->priority;
	}

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

	  int bigger = 0;
	  if(p->priority != max) {
		struct proc *p3;
		for(p3 = ptable.proc; p3 < &ptable.proc[NPROC]; p3++){
			if(p3->state == RUNNABLE && p3->priority > max) {
				bigger = 1;
			}
		}
	  }
	  if(bigger == 1)
	  	break;
	  else
	  	goto execute;

execute:
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
	  int currPrior = p->priority;
	  if(currPrior == 0) {
		  int isPrem = 0;
		  int max_in_0 = -1;
		  struct proc *p8;
		  for(p8 = ptable.proc; p8 < &ptable.proc[NPROC]; p8++){
		  	if(p8->state != RUNNABLE || p8->priority != 0)
				continue;
			if(p8->time_in_0 > max_in_0) {
				max_in_0 = p8->time_in_0;
				p = p8;
			}
		  }

		  for(;;){
		  	if(p->state != RUNNABLE || isPrem == 1)
				break;
			switchuvm(p);
			p->state = RUNNING;
			swtch(&cpu->scheduler, proc->context);
			switchkvm();
			p->wait_ticks[currPrior] = 0;
			p->ticks[currPrior]++;
			p->time_in_0++;
			struct proc *p2;
			for(p2 = ptable.proc; p2 < &ptable.proc[NPROC]; p2++){
				if(p2->state != RUNNABLE || p2->pid == p->pid)
					continue;
				int otherPrior = p2->priority;
				p2->wait_ticks[otherPrior]++;
				int otherWait = p2->wait_ticks[otherPrior];
				if(otherPrior == 3)
					continue;
				if(otherPrior == 0 && otherWait >= 500){
					p2->wait_ticks[otherPrior] = 0;
					p2->priority++;
				}	
				else if(otherWait >= time_slice[otherPrior] * 10) {
					p2->wait_ticks[otherPrior] = 0;
					p2->priority++;
				}
				if(p2->priority > currPrior) {
					isPrem = 1;
				}
			}
		  }
	  }
	  else {
		  int isPrem = 0;
		  int remain = time_slice[currPrior] - p->ticks[currPrior];
		  if(remain == 0 && p->priority != 0)
		  	p->priority--;
		  for(int j = 0;j < remain;j++) {
			if(p->ticks[currPrior] - time_slice[currPrior] == 0) {
				p->priority--;
				break;
			}
		  	if(p->state != RUNNABLE || isPrem ==1)
				break;
      		switchuvm(p);
      		p->state = RUNNING;
			swtch(&cpu->scheduler, proc->context);
      		switchkvm();
			p->wait_ticks[currPrior] = 0;
			p->ticks[currPrior]++;
			if(p->ticks[currPrior] - time_slice[currPrior] == 0) {
				p->priority--;
				break;
			}
  			struct proc *p2;
    		for(p2 = ptable.proc; p2 < &ptable.proc[NPROC]; p2++){
				if(p2->state != RUNNABLE || p2->pid == p->pid)
					continue;
				int otherPrior = p2->priority;
				p2->wait_ticks[otherPrior]++;
				int otherWait = p2->wait_ticks[otherPrior];
				if(otherPrior == 3)
					continue;
				if(otherPrior == 0 && otherWait >= 500) {
					p2->wait_ticks[otherPrior] = 0;
					p2->priority++;
				}
				else if(otherWait >= time_slice[otherPrior] * 10) {
					p2->wait_ticks[otherPrior] = 0;
					p2->priority++;
				}
				if(p2->priority > currPrior) {
					isPrem = 1;
				}
	  		//	cprintf("name = %s, pid = %d, currPrior = %d, ticks = %d, wait_ticks = %d \n",
	  		//	p2->name, p2->pid, otherPrior, p2->ticks[otherPrior], p2->wait_ticks[otherPrior]);
			}
		  }
	  }
      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
//	  cprintf("name = %s, pid = %d, currPrior = %d, ticks = %d, wait_ticks = %d \n",
//	  			p->name, p->pid, currPrior, p->ticks[currPrior], p->wait_ticks[currPrior]);
    }
    release(&ptable.lock);
  }
  
}

int getpinfo(struct pstat* info) {
  	struct proc *p;
	int i = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		if(p->state == UNUSED)
			p->inuse = 0;
		else
			p->inuse = 1;
		int pid = p->pid;
		info->inuse[i] = p->inuse;
		info->pid[i] = pid;
		info->priority[i] = p->priority;
		info->state[i] = p->state;
		for(int j = 0;j < 4;j++)
			info->ticks[i][j] = p->ticks[j];
		for(int j = 0;j < 4;j++)
			info->wait_ticks[i][j] = p->wait_ticks[j];
		i++;
	}			
	return 0;
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}


