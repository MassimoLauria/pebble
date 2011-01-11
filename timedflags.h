/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2011-01-11, martedì 23:53 (CET) Massimo Lauria"
   Time-stamp: "2011-01-12, mercoledì 00:10 (CET) Massimo Lauria"

   Description::




*/


/* Preamble */
#ifndef  TIMEDFLAGS_H
#define  TIMEDFLAGS_H


/* Flags */
extern volatile sig_atomic_t print_running_stats_flag;

/* Flags installer */
extern void install_timed_flags(unsigned int offset,unsigned int freq);

#endif /* TIMEDFLAGS_H */
