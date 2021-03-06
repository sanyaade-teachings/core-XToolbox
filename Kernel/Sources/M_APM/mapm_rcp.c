
/* 
 *  M_APM  -  mapm_rcp.c
 *
 *  Copyright (C) 2000   Michael C. Ring
 *
 *  Permission to use, copy, and distribute this software and its
 *  documentation for any purpose with or without fee is hereby granted, 
 *  provided that the above copyright notice appear in all copies and 
 *  that both that copyright notice and this permission notice appear 
 *  in supporting documentation.
 *
 *  Permission to modify the software is granted, but not the right to
 *  distribute the modified code.  Modifications are to be distributed 
 *  as patches to released version.
 *  
 *  This software is provided "as is" without express or implied warranty.
 */

/*
 *      $Id: mapm_rcp.c,v 1.2 2000/09/26 16:27:48 mike Exp $
 *
 *      This file contains the fast division and reciprocal functions
 *
 *      $Log: mapm_rcp.c,v $
 *      Revision 1.2  2000/09/26 16:27:48  mike
 *      add some comments
 *
 *      Revision 1.1  2000/09/26 16:16:00  mike
 *      Initial revision
 */

#include "m_apm_lc.h"
#include <math.h>

/****************************************************************************/
void	m_apm_divide(rr,places,aa,bb)
M_APM	rr, aa, bb;
int	places;
{
M_APM   tmp0, tmp1;
int     sn, nexp, dplaces;

sn = aa->m_apm_sign * bb->m_apm_sign;

if (sn == 0)                  /* one number is zero, result is zero */
  {
   if (bb->m_apm_sign == 0)
     {
      fprintf(stderr,"Warning! ... \'m_apm_divide\', Divide by 0\n");
     }

   rr->m_apm_datalength = 1;
   rr->m_apm_sign       = 0;
   rr->m_apm_exponent   = 0;
   rr->m_apm_data[0]    = 0;
   return;
  }

/*
 *    Use the original 'Knuth' method for smaller divides. On the
 *    author's system, this was the *approx* break even point before
 *    the reciprocal method used below became faster.
 */

if (places < 250)
  {
   M_apm_sdivide(rr, places, aa, bb);
   return;
  }

/* mimic the decimal place behavior of the original divide */

nexp = aa->m_apm_exponent - bb->m_apm_exponent;

if (nexp > 0)
  dplaces = nexp + places;
else
  dplaces = places;

tmp0 = M_get_stack_var();
tmp1 = M_get_stack_var();

m_apm_reciprocal(tmp0, (dplaces + 8), bb);
m_apm_multiply(tmp1, tmp0, aa);
m_apm_round(rr, dplaces, tmp1);

M_restore_stack(2);
}
/****************************************************************************/
void	m_apm_reciprocal(rr,places,aa)
M_APM	rr, aa;
int	places;
{
M_APM   last_x, guess, tmpN, tmp1, tmp2;
char    sbuf[32];
int	ii, bflag, dplaces, nexp, tolerance;

if (aa->m_apm_sign == 0)
  {
   fprintf(stderr,"Warning! ... \'m_apm_reciprocal\', Input = 0\n");

   rr->m_apm_datalength = 1;
   rr->m_apm_sign       = 0;
   rr->m_apm_exponent   = 0;
   rr->m_apm_data[0]    = 0;
   return;
  }

last_x = M_get_stack_var();
guess  = M_get_stack_var();
tmpN   = M_get_stack_var();
tmp1   = M_get_stack_var();
tmp2   = M_get_stack_var();

m_apm_absolute_value(tmpN, aa);

/* 
    normalize the input number (make the exponent 0) so
    the 'guess' below will not over/under flow on large
    magnitude exponents.
*/

nexp = aa->m_apm_exponent;
tmpN->m_apm_exponent -= nexp;

m_apm_to_string(sbuf, 15, tmpN);
m_apm_set_double(guess, (1.0 / atof(sbuf)));

tolerance = places + 4;
dplaces   = places + 16;
bflag     = FALSE;

m_apm_negate(last_x, MM_Ten);

/*   Use the following iteration to calculate the reciprocal :


         X     =  X  *  [ 2 - N * X ]
          n+1
*/

ii = 0;

while (TRUE)
  {
   m_apm_multiply(tmp1, tmpN, guess);
   m_apm_subtract(tmp2, MM_Two, tmp1);
   m_apm_multiply(tmp1, tmp2, guess);

   if (bflag)
     break;

   m_apm_round(guess, dplaces, tmp1);

   /* force at least 2 iterations so 'last_x' has valid data */

   if (ii != 0)
     {
      m_apm_subtract(tmp2, guess, last_x);

      if (tmp2->m_apm_sign == 0)
        break;

      /* 
       *   if we are within a factor of 4 on the error term,
       *   we will be accurate enough after the *next* iteration
       *   is complete.
       */

      if ((-4 * tmp2->m_apm_exponent) > tolerance)
        bflag = TRUE;
     }

   m_apm_copy(last_x, guess);
   ii++;
  }

m_apm_round(rr, places, tmp1);
rr->m_apm_exponent -= nexp;
rr->m_apm_sign = aa->m_apm_sign;
M_restore_stack(5);
}
/****************************************************************************/
