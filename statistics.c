/*
   Copyright (C) 2010, 2011, 2013, 2014 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2011-01-12, mercoledì 17:37 (CET) Massimo Lauria"
   Time-stamp: "2014-06-02, 09:23 (EDT) Massimo Lauria"

   Description::

   Utilities for printing statistics about the running time.

*/

/* Preamble */
#include <stdlib.h>
#include "statistics.h"


/* Code */

void statistics_make_report(FILE *stream,Statistic *const s) {

  Counter tmp_clock;

  /* Add to the total amount */
  s->processed_T            += s->processed;
  s->queued_T               += s->queued;
  s->delayed_T              += s->delayed;
  s->queued_and_discarded_T += s->queued_and_discarded;
  s->first_queuing_T        += s->first_queuing;

  s->requeuing_T            += s->requeuing;
  s->above_upper_bound_T    += s->above_upper_bound;
  s->suboptimal_T           += s->suboptimal;
  s->offspring_T            += s->offspring;
  s->final_T                += s->final;

  s->dict_hops_T            +=s->dict_hops;
  s->dict_queries_T         +=s->dict_queries;
  s->dict_writes_T          +=s->dict_writes;
  s->dict_misses_T          +=s->dict_misses;

  tmp_clock = s->clock;
  if (tmp_clock==0) tmp_clock=1;

  /*             "[   Queued]  = 000000000000000  |  000000000000000"  */
  fprintf(stream,"                    PER SECOND  |            TOTAL\n"
                 "              ------------------------------------\n");

  fprintf(stream,"[   Queued]  = %15llu  |  %15llu\n",
          s->queued_T/tmp_clock,s->queued_T);
  fprintf(stream,"[   Unique]  = %15llu  |  %15llu\n",
          s->first_queuing_T/tmp_clock,s->first_queuing_T);
  fprintf(stream,"[ Requeued]  = %15llu  |  %15llu\n",
          s->requeuing_T/tmp_clock,s->requeuing_T);
  fprintf(stream,"[  Delayed]  = %15llu  |  %15llu\n",
          s->delayed_T/tmp_clock,s->delayed_T);

  fprintf(stderr,"\n");

  fprintf(stream,"[Processed]  = %15llu  |  %15llu\n",
          s->processed_T/tmp_clock,s->processed_T);
  fprintf(stream,"[Discarded]  = %15llu  |  %15llu\n",
          s->queued_and_discarded_T/tmp_clock,s->queued_and_discarded_T);
  fprintf(stream,"[Sub Optim]  = %15llu  |  %15llu\n",
          s->suboptimal_T/tmp_clock,s->suboptimal_T);
  fprintf(stream,"[Expensive]  = %15llu  |  %15llu\n",
          s->above_upper_bound_T/tmp_clock,s->above_upper_bound_T);
  fprintf(stream,"[    Final]  = %15llu  |  %15llu\n",
          s->final_T/tmp_clock,s->final_T);

  fprintf(stderr,"\n");

  fprintf(stream,"[Offspring]  = %15llu  |  %15llu\n",
          s->offspring_T/tmp_clock,s->offspring_T);
  if (s->processed_T) {
    fprintf(stream,"[Avg.Neig.]  = %15llu\n\n"  ,s->offspring_T/s->processed_T);
  }

  fprintf(stderr,"\n");

  fprintf(stream,"[D.Queries]  = %15llu  |  %15llu\n",
          s->dict_queries_T/tmp_clock,s->dict_queries_T);
  fprintf(stream,"[D. Writes]  = %15llu  |  %15llu\n",
          s->dict_writes_T/tmp_clock,s->dict_writes_T);
  fprintf(stream,"[D. Misses]  = %15llu  |  %15llu\n",
          s->dict_misses_T/tmp_clock,s->dict_misses_T);

  if (s->dict_queries_T && s->dict_size) {
    fprintf(stream,"[Avg. Hops]  = %15llu vs %llu\n\n",s->dict_hops_T/s->dict_queries_T,s->first_queuing_T/s->dict_size);
  }

  /* Reset partial counters */
  s->processed            = 0;
  s->queued               = 0;
  s->delayed              = 0;
  s->queued_and_discarded = 0;
  s->first_queuing        = 0;
  s->requeuing            = 0;
  s->above_upper_bound    = 0;
  s->suboptimal           = 0;
  s->offspring            = 0;
  s->final                = 0;
  s->dict_hops            = 0;
  s->dict_queries         = 0;
  s->dict_writes          = 0;
  s->dict_misses          = 0;

}


