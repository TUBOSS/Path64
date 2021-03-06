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


#pragma ident "@(#) libu/ffio/trcopen.c	92.1	06/29/99 13:16:47"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ffio.h>
#include "trcio.h"
#include "fxlist.h"

DECLARE(TRC_XLIST);
struct xtr_s TRC_XLIST_P = { TRC_XLIST };

#define SUFFIX	".trc"
/*
 * trace open requests
 */

int
_trc_open(
const char	*name,
int		flags,
int		mode,
struct fdinfo	*fio,
union spec_u	*spec,
struct ffsw	*stat,
long		cbits,
int		cblks,
struct gl_o_inf *oinf)
	{
	union spec_u *nspec;
	struct trace_f *trc_info;
	int *hp;
	char *ptr = NULL;
	int nextfio, namlen, trcfd;

	namlen = strlen(name);
	ptr = malloc(namlen + strlen(SUFFIX) + 1);
	if (ptr == NULL) goto badopen;

	trc_info = (struct trace_f *)calloc(sizeof(struct trace_f), 1);
	if (trc_info == NULL) goto badopen;

	fio->lyr_info = (char *)trc_info;
	strcpy(ptr, name);
	strcat(ptr, SUFFIX);
	trcfd = open(ptr, O_WRONLY | O_APPEND | O_CREAT, 0666);

	hp = (int *)malloc((HIST_INIT+1)*8);
	if (hp == NULL) goto badopen;
	trc_info->hist = hp;
	trc_info->histsiz = HIST_INIT;
	trc_info->histmax = HIST_MAX;
	trc_info->histincr = HIST_INCR;
	trc_info->histbkt = HIST_BKT;

	trc_info->lastseek = NO;
/*
 *	Yes, ignore errors on open.
 */
	trc_info->name = ptr;
	trc_info->trcfd = trcfd;
	ptr[namlen] = '\0';
	_trc_enter(fio, TRC_OPEN);
	_trc_info(fio, "(\"%s\", %o, %o, &statw[%d], %o);\n",
		name, flags, mode, stat, cbits);

/*
 *	Now, open the lower layers
 */
	nspec = spec;
	NEXT_SPEC(nspec);
	nextfio = _ffopen(name, flags, mode, nspec, stat, cbits, cblks, NULL,
			oinf);
	_trc_exit(fio, nextfio, stat);
	if (nextfio >= 0)
		{
		DUMP_IOB(fio); /* debugging only */
		return(nextfio);
		}
/*
 *	Error.  no memory.
 */
badopen:
	if (ptr != NULL) free(ptr);
	if (fio->lyr_info != NULL) free(fio->lyr_info);
	ERETURN(stat, FDC_ERR_NOMEM, 0);
	}
