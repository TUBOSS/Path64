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


#pragma ident "@(#) libu/ffio/trclistio.c	92.1	06/29/99 13:16:47"

#include <ffio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/iosw.h>
#include <sys/listio.h>
#include "trcio.h"

extern void _trace_listio( struct fdinfo *fio, int which, int cmd, 
	struct fflistreq *lr, int nreq);

extern int _ff_err_listio(int cmd, struct fflistreq *list, int nreq,
	struct ffsw *stat);

/*
 * trace listio requests
 * The trace layer indicates that it can handle listio requests iff
 * the lower layer can.
 */
_trc_listio(
int			cmd,		/* LC_WAIT or LC_START */
struct fflistreq	*list,		/* list of requests (see fflistio) */
int			nreq,		/* number of requests */
struct ffsw		*stat)		/* status structure */
{

	int	ret;
	int	i;
	struct fdinfo	*llfio;
	struct fdinfo	*first_llfio;
	struct fdinfo	*fio;
	struct fflistreq *copy;		/* copy of the list of requests */

	if (nreq == 0)
		return(0);

	copy = malloc(nreq * sizeof(*list));
	if (copy == NULL)
		ERETURN(stat, FDC_ERR_NOMEM, 0);

	for (i = 0; i < nreq; i++) {
		fio = GETIOB(list[i].li_fildes);
		llfio = fio->fioptr;

		if (i == 0)
			first_llfio = llfio;
		else if (llfio != first_llfio) {
			_SETERROR(list[i].li_status, FDC_ERR_LSTIO, 0);
			continue;
		}

		copy[i] = list[i];		/* copy the entry */
		copy[i].li_fildes = (int)llfio;	/* pass it on to lower layer */

		_trace_listio(fio, i, cmd, &copy[i], nreq);
	}

	ret = XRCALL(llfio, listiortn) cmd, copy, nreq, stat);

	free(copy);

	return(ret);
}

void
_trace_listio(
struct fdinfo		*fio,
int			which,
int			cmd,
struct fflistreq        *lr,
int			nreq)
{
	struct trace_f	*trc_info;

	trc_info = (struct trace_f *)fio->lyr_info;

	if (lr->li_flags & LF_LSEEK) {
		trc_info->lseeks++;
		trc_info->lastseek = YES;
		trc_info->curpos = lr->li_offset;
	}

	_trc_enter(fio, TRC_LISTIO);

	_trc_info(fio, " req %d of %d :",which+1, nreq);
	_trc_info(fio, " cmd=%s",
		cmd==LC_START? "LC_START": (cmd==LC_WAIT? "LC_WAIT" : "???"));
	_trc_info(fio, " li_opcode=%s" ,
		lr->li_opcode==LO_READ? "LO_READ":
		(lr->li_opcode==LO_WRITE? "LO_WRITE" : "???"));
	_trc_info(fio, " li_flags=%s" ,
		lr->li_flags==0? "0":
		((lr->li_flags == LF_LSEEK)? "LF_LSEEK": "???"));
	_trc_info(fio, " li_offset=%d",lr->li_offset);
	_trc_info(fio, " li_fildes=%d",lr->li_fildes);
	_trc_info(fio, " li_buf=%d",lr->li_buf);
	_trc_info(fio, " li_nbyte=%d",lr->li_nbyte);
	_trc_info(fio, " li_status=%d",lr->li_status);
	_trc_info(fio, " li_signo=%d",lr->li_signo);
	_trc_info(fio, " li_nstride=%d",lr->li_nstride);
	_trc_info(fio, " li_filstride=%d",lr->li_filstride);
	_trc_info(fio, " li_memstride=%d",lr->li_memstride);

	trc_info->lastseek = NO;
	trc_info->last_stpos = -1;		/* unknown */
	trc_info->last_endpos = -1;		/* unknown */
	trc_info->curpos = -1;			/* unknown */

	_trc_simple_exit(fio);
}
