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


#pragma ident "@(#) libf/pxf/pxftimes.c	92.1	06/29/99 11:36:06"


#include <fortran.h>
#include <errno.h>
#include <liberrno.h>
#include <sys/times.h>
#include <stdlib.h>
#include "pxfstruct.h"
#include "table.h"

#ifndef _UNICOS
#include <stddef.h>
#endif

/*
 *  PXFTIMES  -- get process times (section 4.5.2 of Posix 1003.9-1992)
 *
 *  Call from Fortran:
 *
 *       SUBROUTINE PXFTIMES(JTMS,ITIME, IERROR)
 *       INTEGER JTMS, ITIME, IERROR
 *
 *  Where:
 *
 *  JTMS    is a handle of type 'tms' created with PXFSTRUCTCREATE.
 *
 *  ITIME   is an output integer variable for the number of
 *          system hardware clock ticks since some arbitrary
 *          point in the past, e.g., system start up time.
 *          This point does not change from one invocation to
 *          another during the execution of the process.
 *
 *  IERROR  is an output integer variable that will contain
 *	    the status:
 *
 *	     zero    - environment varible was changed.
 *
 *	     nonzero - PXFTIME was not successful.
 *
 *          PXFTIMES may return any of the following error
 *          values:
 *
 *            EBADHANDLE  If JTMS is invalid.
 *
 */

#ifdef _UNICOS
void
PXFTIMES(
#else
void
_PXFTIMES(
#endif
	  _f_int *JTMS,
	  _f_int *ITIME,
	  _f_int *IERROR
)
{
  struct tms *timebuf;
  struct pxfhandle pxfhand;

  *IERROR = 0;

  pxfhand = _pxfhandle_table_lookup(&_pxfhandle_table, *JTMS);
  if (pxfhand.pxfstructptr == NULL || pxfhand.pxftype != PXF_TMSBUF) {
    *IERROR = EBADHANDLE;
    return;
  }


  timebuf = pxfhand.pxfstructptr;

  if ((*ITIME = times(timebuf)) == -1)
    *IERROR = errno;

  return;
}


#ifndef _UNICOS

void
pxftimes_(
	  _f_int *JTMS,
	  _f_int *ITIME,
	  _f_int *IERROR)
{
	_PXFTIMES(JTMS, ITIME, IERROR);
}

#endif





