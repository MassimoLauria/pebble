/*
  Copyright (C) 2014 by Massimo Lauria <lauria.massimo@gmail.com>
  
  Created   : "2014-03-18, Tuesday 17:04 (CET) Massimo Lauria"
  Time-stamp: "2014-06-02, 09:33 (EDT) Massimo Lauria"
  
  Description::
  
  Get the information about the configuration of the pebbling variant.
  
*/

/* Preamble */
#include <stdlib.h>
#include "config.h"

/* Code */

#if REVERSIBLE_PEBBLING

const char* pebbling_type() {
  static const char* name="reversible pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Reversible pebbling";
  return Name;
}

#elif BLACK_WHITE_PEBBLING

const char* pebbling_type() {
  static const char* name="black-white pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Black-white pebbling";
  return Name;
}

#else 

const char* pebbling_type() {
  static const char* name="pebbling";
  return name;
}

const char* pebbling_Type() {
  static const char* Name="Pebbling";
  return Name;
}

#endif


