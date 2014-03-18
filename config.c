/*
  Copyright (C) 2014 by Massimo Lauria <lauria.massimo@gmail.com>
  
  Created   : "2014-03-18, Tuesday 17:04 (CET) Massimo Lauria"
  Time-stamp: "2014-03-18, 17:15 (CET) Massimo Lauria"
  
  Description::
  
  Get the information about the configuration of the pebbling variant.
  
*/

/* Preamble */
#include <stdlib.h>
#include "config.h"

/* Code */

#if WHITE_PEBBLES==0 && BLACK_PEBBLES==1 && REVERSIBLE==1

const char* pebbling_type() {
  static const char* name="reversible pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Reversible pebbling";
  return Name;
}

#endif


#if WHITE_PEBBLES==0 && BLACK_PEBBLES==1 && REVERSIBLE==0

const char* pebbling_type() {
  static const char* name="pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Pebbling";
  return Name;
}

#endif


#if WHITE_PEBBLES==1 && BLACK_PEBBLES==1 && REVERSIBLE==0

const char* pebbling_type() {
  static const char* name="black-white pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Black-white pebbling";
  return Name;
}

#endif











