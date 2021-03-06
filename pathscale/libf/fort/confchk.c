/*

  Copyright (C) 2000, 2001, Silicon Graphics, Inc.  All Rights Reserved.

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



static char USMID[] = "@(#) libf/fort/confchk.c	92.0	10/08/98 14:30:10";
#include <liberrno.h>

#define	MAX_ENT_LEN	32	/* Maximum entry point name length	*/

extern	int	_who_called_me();

void
$CONFCHK(int *dimension, int *extent1, int *extent2, int *linenum)
{
	int	len;
	char	name[MAX_ENT_LEN];

	len	= _who_called_me(&len, name, MAX_ENT_LEN, 1);

	if (len < 0) {		/* If there was an error */
		name[0]	= '?';
		name[1]	= '?';
		name[2]	= '?';
		len	= 3;
	}

	name[len]	= '\0';		/* Terminate name */

	(void) _fwarn(FWCONFCK, *linenum, name, *dimension, *extent1,
			*extent2);

	return;
}
