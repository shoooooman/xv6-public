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

uint
ymdhms_to_posix(int year, int month, int day, int hour, int minute, int second) {
  const int YEAR_BASE = 2018, MONTH_BASE = 1, DAY_BASE = 1, HOUR_BASE = 0, MINUTE_BASE = 0, SECOND_BASE = 0; // base time
  int year_diff   = year   - YEAR_BASE;
  int month_diff  = month  - MONTH_BASE;
  int day_diff    = day    - DAY_BASE;
  int hour_diff   = hour   - HOUR_BASE;
  int minute_diff = minute - MINUTE_BASE;
  int second_diff = second - SECOND_BASE;
  uint amount_sec = 0;

  // 含まれている閏年の数を計算
  int num_leap = 0;
  num_leap += year_diff/4;
  num_leap -= year_diff/100;
  num_leap += year_diff/400;

  int is_leap; // 今年が閏年か
  if (year % 400 == 0) {
    is_leap = 1;
  } else if (year % 100 == 0) {
    is_leap = 0;
  } else if (year % 4 == 0) {
    is_leap = 1;
  } else {
    is_leap = 0;
  }

  amount_sec += (year_diff - num_leap)*365*24*60*60 + num_leap*366*24*60*60;
  amount_sec += month_diff*30*24*60*60; // 全て30日の場合
  switch (month_diff) { // 1ヶ月30日を基準として，それからの差分を加算
    case 12:
	    amount_sec += 1;
    case 11: case 10:
	    amount_sec += 1;
    case 9: case 8:
	    amount_sec += 1;
    case 7:
	    amount_sec += 1;
    case 6: case 5:
	    amount_sec += 1;
    case 4: case 3:
	    amount_sec += 1;
    case 2:
	    if (is_leap) amount_sec -= 2;
	    else 	 amount_sec -= 1;
    case 1:
	    amount_sec += 1;
  }
  amount_sec += day_diff*24*60*60;
  amount_sec +=   hour_diff*60*60;
  amount_sec +=    minute_diff*60;
  amount_sec +=       second_diff;
  return amount_sec;
}

int
sys_sleep_sec(void) {
  int sec;
  uint begin;
  struct rtcdate *dp;
  struct rtcdate _dp;

  if (argint(0, &sec) < 0) {
    return -1;
  }
  cmostime(&_dp);
  dp = &_dp;

  begin = ymdhms_to_posix(dp->year, dp->month, dp->day, dp->hour, dp->minute, dp->second);
  acquire(&tickslock);
  while(ymdhms_to_posix(dp->year, dp->month, dp->day, dp->hour, dp->minute, dp->second) - begin < sec) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
    cmostime(dp);
  }
  release(&tickslock);
  return 0;
}
