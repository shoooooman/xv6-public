#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_getdate(void) {
  struct rtcdate *dp;
  if (argptr(0, (char **)&dp, sizeof(dp)) < 0) {
	  return -1;
  }
  cmostime(dp);
  return 0;
}

int
sys_sleep_sec(void) {
  int sec;
  uint hour0, minute0, sec0, score0;
  struct rtcdate *dp;

  if (argint(0, &sec) < 0) {
    return -1;
  }
  if (argptr(0, (char **)&dp, sizeof(dp)) < 0) {
    return -1;
  }
  // cmostime(dp0);
  // acquire(&tickslock);
  wakeup(&tickslock);
  // sleep(&ticks, &tickslock);
  cmostime(dp);
  hour0 = dp->hour;
  minute0 = dp->minute;
  sec0 = dp->second;
  score0 = hour0*60*60 + minute0*60 + sec0;
  while((dp->hour*60*60 + dp->minute*60 + dp->second) - score0 < sec) {
    if (myproc()->killed) {
      wakeup(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
    cmostime(dp);
  }
  release(&tickslock);
  return 0;
}
