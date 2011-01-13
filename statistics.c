/*
   Copyright (C) 2010, 2011 by Massimo Lauria <lauria.massimo@gmail.com>

   Created   : "2011-01-12, mercoledì 17:37 (CET) Massimo Lauria"
   Time-stamp: "2011-01-13, giovedì 10:57 (CET) Massimo Lauria"

   Description::

   Utilities for printing statistics about the running time.

*/

/* Preamble */
#include <stdlib.h>
#include "statistics.h"


/* Code */

void statistics_make_report(FILE *stream,Statistic *const s) {

      /* Add to the total amount */
      s->processed_T            += s->processed;
      s->queued_T               += s->queued;
      s->queued_and_discarded_T += s->queued_and_discarded;
      s->first_queuing_T        += s->first_queuing;

      s->requeuing_T            += s->requeuing;
      s->above_upper_bound_T    += s->above_upper_bound;
      s->suboptimal_T           += s->suboptimal;
      s->offspring_T            += s->offspring;

      s->dict_hops_T            +=s->dict_hops;
      s->dict_queries_T         +=s->dict_queries;
      s->dict_writes_T          +=s->dict_writes;
      s->dict_misses_T          +=s->dict_misses;

      fprintf(stream,"TOTAL\n");
      fprintf(stream,"[   Queued] = %llu\n",s->queued_T);
      fprintf(stream,"[   Unique] = %llu of %llu\n",s->first_queuing_T,
              s->search_space);
      fprintf(stream,"[ Requeued] = %llu\n\n",s->requeuing_T);
      fprintf(stream,"[Processed] = %llu\n"  ,s->processed_T);
      fprintf(stream,"[Discarded] = %llu\n\n",s->queued_and_discarded_T);
      fprintf(stream,"[Offspring] = %llu\n"  ,s->offspring_T);
      fprintf(stream,"[Sub Optim] = %llu\n"  ,s->suboptimal_T);
      fprintf(stream,"[Expensive] = %llu\n"  ,s->above_upper_bound_T);
      fprintf(stream,"[Avg.Neig.] = %llu\n\n"  ,(s->offspring_T+1)/(s->processed_T+1));

      fprintf(stream,"[D.Queries] = %llu\n"  ,s->dict_queries_T);
      fprintf(stream,"[D. Writes] = %llu\n"  ,s->dict_writes_T);
      fprintf(stream,"[D. Misses] = %llu\n"  ,s->dict_misses_T);
      if (s->dict_queries_T) {
        fprintf(stream,"[Avg. Hops] = %llu\n\n",s->dict_hops_T/s->dict_queries_T);
      }

      fprintf(stream,"\nLAST INTERVAL\n");
      fprintf(stream,"[   Queued] = %llu\n"  ,s->queued);
      fprintf(stream,"[   Unique] = %llu\n"  ,s->first_queuing);
      fprintf(stream,"[ Requeued] = %llu\n\n",s->requeuing);
      fprintf(stream,"[Processed] = %llu\n"  ,s->processed);
      fprintf(stream,"[Discarded] = %llu\n\n",s->queued_and_discarded);
      fprintf(stream,"[Offspring] = %llu\n"  ,s->offspring);
      fprintf(stream,"[Sub Optim] = %llu\n"  ,s->suboptimal);
      fprintf(stream,"[Expensive] = %llu\n\n",s->above_upper_bound);

      fprintf(stream,"[D.Queries] = %llu\n"  ,s->dict_queries);
      fprintf(stream,"[D. Writes] = %llu\n"  ,s->dict_writes);
      fprintf(stream,"[D. Misses] = %llu\n"  ,s->dict_misses);
      if (s->dict_queries) {
        fprintf(stream,"[Avg. Hops] = %llu\n\n",s->dict_hops/s->dict_queries);
      }

      if (s->clock!=0) {

        fprintf(stream,"\nPER SECOND\n");
        fprintf(stream,"[   Queued] = %llu\n"  ,s->queued_T/s->clock);
        fprintf(stream,"[   Unique] = %llu\n"  ,s->first_queuing_T/s->clock);
        fprintf(stream,"[ Requeued] = %llu\n\n",s->requeuing_T/s->clock);
        fprintf(stream,"[Processed] = %llu\n"  ,s->processed_T/s->clock);
        fprintf(stream,"[Discarded] = %llu\n\n",s->queued_and_discarded_T/s->clock);
        fprintf(stream,"[Offspring] = %llu\n"  ,s->offspring_T/s->clock);
        fprintf(stream,"[Sub Optim] = %llu\n"  ,s->suboptimal_T/s->clock);
        fprintf(stream,"[Expensive] = %llu\n\n",s->above_upper_bound_T/s->clock);

        fprintf(stream,"[D.Queries] = %llu\n"  ,s->dict_queries_T/s->clock);
        fprintf(stream,"[D. Writes] = %llu\n"  ,s->dict_writes_T/s->clock);
        fprintf(stream,"[D. Misses] = %llu\n"  ,s->dict_misses_T/s->clock);
      }

      /* Reset partial counters */
      s->processed            = 0;
      s->queued               = 0;
      s->queued_and_discarded = 0;
      s->first_queuing        = 0;
      s->requeuing            = 0;
      s->above_upper_bound    = 0;
      s->suboptimal           = 0;
      s->offspring            = 0;
      s->dict_hops            = 0;
      s->dict_queries         = 0;
      s->dict_writes          = 0;
      s->dict_misses          = 0;


}


