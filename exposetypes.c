/*
 *  Massimo Lauria, 2010
 *
 *  This is a little program which exposes the size of some common
 *  standard numerical/character data types of C, also showing their
 *  minimal and maximal values.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <stdbool.h>
#include <complex.h>
#include <float.h>


/* The record of the table containing
   - NAME: name of the data type
   - SIZE: size in byte of the data type
*/
struct size_record {
  char* name;
  size_t size;
};
struct size_record BIT_SIZE_TABLE[]={
  {"char",sizeof(char)},
  {"signed char",sizeof(signed char)},
  {"unsigned char",sizeof(unsigned char)},
  {"short int",sizeof(short int)},
  {"short unsigned int",sizeof(short unsigned int)},
  {"int",sizeof(int)},
  {"unsigned int",sizeof(unsigned int)},
  {"long int",sizeof(long int)},
  {"long unsigned int",sizeof(long unsigned int)},
  {"long long int",sizeof(long long int)},
  {"long long unsigned int",sizeof(long long unsigned int)},
  {"float",sizeof(float)},
  {"double",sizeof(double)},
  {"long double",sizeof(long double)},
  {"float complex",sizeof(float complex)},
  {"double complex",sizeof(double complex)},
  {"long double complex",sizeof(long double complex)},
  {"wchar_t",sizeof(wchar_t)},
  {"wint_t",sizeof(wint_t)},
  {"void *",sizeof(void*)},
  {"ptrdiff_t",sizeof(ptrdiff_t)},
  {"site_t",sizeof(size_t)},
  {"bool",sizeof(bool)},
  {"0x1"  ,sizeof(-0x1)},
  {"0x1L" ,sizeof(-0x1L)},
  {"0x1LL",sizeof(-0x1LL)},
  {"0x1U"  ,sizeof(0x1U)},
  {"0x1UL" ,sizeof(0x1UL)},
  {"0x1ULL",sizeof(0x1ULL)},
  {"1.0F"  ,sizeof(1.0F)},
  {"1.0"  ,sizeof(1.0)},
  {"1.0L" ,sizeof(1.0L)},
  {NULL,0}
};


char greetings_string[]=
  "|-----------------------------------------------------------------------------------\n"
  "|                                                                                  |\n"
  "| Often C compilers do not strictly respect ANSI standards regarding size of data  |\n"
  "| types. Also the very same standards allow some degree of flexibility, maybe      |\n"
  "| requiring only minimum features instead of a precise implementation.  In many    |\n"
  "| cases the standard requirements depends on machine dependant characteristics     |\n"
  "| like wordsize.  This program audits the characteristics of the system in hand.   |\n"
  "|                                                                                  |\n"
  "| (C) Massimo Lauria, 2010                                                         |\n"
  "|                                                                                  |\n"
  "|----------------------------------------------------------------------------------|\n";

/* The record of the table specific to decimal data types. It contains
   - NAME
   - NUMBER OF DIGITS OF PRECISION
   - MINIMAL VALUE
   - MAXIMUM VALUE
*/
struct float_record {
  char* name;
  unsigned int precision;
  long double  min;
  long double  max;
};
/* The given values are defined in float.h */
struct float_record FLOAT_PRECISION_TABLE[]={
  {"float",       FLT_DIG , FLT_MIN,  FLT_MAX },
  {"double",      DBL_DIG , DBL_MIN,  DBL_MAX  },
  {"long double", LDBL_DIG, LDBL_MIN, LDBL_MAX },
  {NULL,0}
};


/* The record of the table specific to integral data types. It contains
   - NAME
   - MINIMAL VALUE
   - MAXIMUM VALUE
*/
struct int_record {
  char* name;
  long long int min;
  unsigned long long int max;
};
/* Many of this values are defined in stdint.h */
struct int_record INTEGRAL_TABLE[]={
  {"char",CHAR_MIN,CHAR_MAX},
  {"signed char",SCHAR_MIN,SCHAR_MAX},
  {"unsigned char",0,UCHAR_MAX},
  {"short int",SHRT_MIN,SHRT_MAX},
  {"short unsigned int",0,USHRT_MAX},
  {"int",INT_MIN,INT_MAX},
  {"unsigned int",0,UINT_MAX},
  {"long int",LONG_MIN,LONG_MAX},
  {"long unsigned int",0,ULONG_MAX},
  {"long long int",LLONG_MIN,LLONG_MAX},
  {"long long unsigned int",0,ULLONG_MAX},
  {"wchar_t",WCHAR_MIN,WCHAR_MAX},
  {"wint_t",WINT_MIN,WINT_MAX},
  {"site_t",0,SIZE_MAX},
  {"ptrdiff_t",PTRDIFF_MIN,PTRDIFF_MAX},
  {"bool",false,true},
  {NULL,0}
};


/* Main entry point of the program. */
int main() {

  int i=0;

  printf("%s\n\n",greetings_string);

  /* Print data type dimensions. */
  printf("SIZE: these are the sizes of canonical types of C.\n\n");
  for(i=0;BIT_SIZE_TABLE[i].name;i++)
    printf("* %3d bits for a %s\n",CHAR_BIT*(int)BIT_SIZE_TABLE[i].size,BIT_SIZE_TABLE[i].name);
  printf("\n\n");


  /* Print float data type precisions. */
  printf("PRECISION: this is the digit precision of float data types of C.\n\n");

  for(i=0;FLOAT_PRECISION_TABLE[i].name;i++)
    printf("* Decimal precision of %s is %d digits. <%Lg,%Lg>.\n",
           FLOAT_PRECISION_TABLE[i].name,
           FLOAT_PRECISION_TABLE[i].precision,
           FLOAT_PRECISION_TABLE[i].min,
           FLOAT_PRECISION_TABLE[i].max);
  printf("There is a \"complex\" type for every float type, which contains two"
         "floats to represent real and imaginary part\n");
  printf("\n\n");


  printf("RANGE: these are the intervals of the integer data types.\n\n");
  for(i=0;INTEGRAL_TABLE[i].name;i++)
    printf("* %s gets values between %lld and %llu\n",INTEGRAL_TABLE[i].name,INTEGRAL_TABLE[i].min,INTEGRAL_TABLE[i].max);
  exit(0);
}
