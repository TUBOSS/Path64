/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#pragma ident "@(#) libfi/mathlb/ieee_mips.c	92.2	07/09/99 11:00:36"

/* 
 * This implements several of the IEEE intrinsics on the IRIX platforms 
 */

#include <fortran.h>
#include <ieeefp.h>

#define BITMASK 0x1f
#define STICKY_SHIFT 12
#define STICKY_MASK (BITMASK << STICKY_SHIFT)
#define MASK_SHIFT  7
#define MASK_MASK   (BITMASK << MASK_SHIFT)

_f_int4
__get_ieee_interrupts(void)
{
	return (fpgetmask() << MASK_SHIFT);
}

_f_int4
__get_ieee_status(void)
{
	_f_int4	r;
   
	r	= fpgetround() |
		((fpgetsticky() & BITMASK) << STICKY_SHIFT) |
		((fpgetmask() & BITMASK) << MASK_SHIFT);
	return (r);
}

void
__set_ieee_status(_f_int4 status)
{
	_f_int4	mask;
	_f_int4	sticky;
	fp_rnd	round;
	round	= (fp_rnd) (status & 3);
	mask	= (status >> MASK_SHIFT) & BITMASK;
	sticky	= (status >> STICKY_SHIFT) & BITMASK;
	(void) fpsetmask(mask);
	(void) fpsetsticky(sticky);
	(void) fpsetround(round);
}

void
__set_ieee_exceptions(_f_int4 exceptions)
{
	(void) fpsetsticky( (exceptions >> STICKY_SHIFT) & BITMASK);
}

_f_int4
__get_ieee_exceptions(void)
{
	_f_int4	exceptions;
	exceptions	= (fpgetsticky() & BITMASK) << STICKY_SHIFT;
	return (exceptions);
}

void
__set_ieee_exception(_f_int4 exception,
			_f_int4 value)
{
	_f_int4	mask;

	mask	= fpgetsticky();
	exception	= (exception >> STICKY_SHIFT) & BITMASK;
	/* exception should have only 1 bit set, be we don't test that */
	if (value == 0) {
		mask &= ~exception;
	} else {
		mask |= exception;
	}
	fpsetsticky(mask);
}

void
__set_ieee_interrupts(_f_int4 interrupt)
{
	(void) fpsetmask( (interrupt >> MASK_SHIFT) & BITMASK);
}

void
__enable_ieee_interrupt(_f_int4 interrupt)
{
	_f_int4 mask;

	mask	= fpgetmask();
	interrupt	= (interrupt >> MASK_SHIFT) & BITMASK;
	(void) fpsetmask( mask | interrupt);
}

void
__disable_ieee_interrupt(_f_int4 interrupt)
{
	_f_int4	mask;

	mask	= fpgetmask();
	interrupt	= (interrupt >> MASK_SHIFT) & BITMASK;
	(void) fpsetmask( mask & ~interrupt);
}

_f_log4
__test_ieee_interrupt(_f_int4 interrupt)
{
	_f_int4	mask;

	mask	= fpgetmask();
	interrupt	= (interrupt >> MASK_SHIFT) & BITMASK;
	return ((mask & interrupt) != 0);
}

_f_log4
__test_ieee_exception(_f_int4 exception)
{
	_f_int4	mask;

	mask	= fpgetsticky();
	exception	= (exception >> STICKY_SHIFT) & BITMASK;
	return ((mask & exception) != 0);
}

_f_real16
_IEEE_REMAINDER_Q (_f_real16 x, _f_real16 y)
{
/* This is not strictly IEEE compiliant, but neither is REAL(16) in
 * general. This computes x - y * n, where n is the integer nearest x/y
 */
	typedef struct {
			double	h;
			double	l;
	} qparts;
	union {
		long double	ld;
		qparts		dd;
	} q;
	long double	iq;
	extern long double __qnint(double  uhi,double  ulo );

	q.ld	= x / y;
  
	/* Compute a long double integer */
	iq	= __qnint( q.dd.h, q.dd.l);
	return (x - y*iq);
}

_f_real16
_IEEE_NEXT_AFTER_H (_f_real16 x,
		_f_real16 y)
{
	extern _f_real16 _NEAREST_16(_f_real16 x, _f_real16 s);
   
	long double	direction;
	if (x==y) return (x);
   
	direction = (y>x) - (y<x);
	return (_NEAREST_16(x,direction));
}

