/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2011-01-11, martedì 22:41 (CET) Massimo Lauria"
   Time-stamp: "2011-01-13, giovedì 16:34 (CET) Massimo Lauria"

   Description::

   Some flags are activated a certain intervals, to tell the main code
   to perform some operations like printing running stats, and so on...

   TODO: Understand why if compiled according to ANSI or ISO the
   signals would not work on Linux.

*/

/* Preamble */
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "timedflags.h"

/* Flags */
volatile sig_atomic_t print_running_stats_flag=0;
unsigned int timedflags_clock_freq;


void timed_flags_handler(int sig) {
  print_running_stats_flag=1;
}


void install_timed_flags(unsigned int freq){
  struct itimerval t;
  signal(SIGALRM,timed_flags_handler);
  t.it_interval.tv_sec=freq;
  t.it_interval.tv_usec=0;
  t.it_value.tv_sec=freq;
  t.it_value.tv_usec=0;
  setitimer(ITIMER_REAL,&t,NULL);
  timedflags_clock_freq=freq;
}
