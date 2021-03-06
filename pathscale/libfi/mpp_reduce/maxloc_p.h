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


/* USMID @(#) libfi/mpp_reduce/maxloc_p.h	92.0	10/08/98 14:37:14 */


#define GLUE(X,Y) X ## Y
#define XGLUE(X,Y) GLUE(X,Y)
#define Pragma_CRI(s) _Pragma(#s)
#define Pragma(s) Pragma_CRI(_CRI s)

Pragma(duplicate XGLUE(_MAXLOC_JP,RANK) as XGLUE(_MAXLOC_SP,RANK))

void
XGLUE(_MAXLOC_JP,RANK)(DopeVectorType * result,
	   DopeVectorType * source,
	   DopeVectorType * mask)
{
	long i, extents[RANK], mask_flag;
	long dim1_tmp, dim2_tmp;
	void *result_base_ptr;
	void *result_sdd_ptr, *source_sdd_ptr, *mask_sdd_ptr;

	/*
	 * If any of the extents for the source or mask array 
	 * arguments are zero, then return to the caller. Since
	 * we allocate the result array, there is no need to
	 * check that.
	 */

	EXTENT_CHK(source);

	if (mask) {
	    EXTENT_CHK(mask);
	}

	/*
	 *  Allocate the result shared temporary and fill in the
	 *  dope vector.
	 */

	_shmalloc_loc_result(result,source,&result_base_ptr,RANK);

	/*
	 * Initialize the sdd for the shared result temporary so
	 * that it describes one distributed array that is
	 * dimensioned (_N_PES,source->n_dim) and distributed
	 * (:BLOCK(1),:). This macro uses variables result, source,
	 * dim1_tmp, dim2_tmp, and result_base_ptr.
	 */

	INIT_LOC_SDD();

	/*
	 * The result sdd pointer in the dope vector does not have the
	 * sign bit set. Set up sdd pointer so it does. This is the
	 * way Fortran can tell that the pointer points to shared data.
	 */

	i = (long)result->base_addr.a.ptr | SIGN_BIT;
	result_sdd_ptr = (void *)i;
	source_sdd_ptr = source->base_addr.a.ptr;

	/*
	 * If a mask argument was provided by the user, then check
	 * to see if it is an array or a scalar. If the mask is a
	 * non-zero scalar, then treat as if no mask was provided.
	 * If the mask is a scalar with a value of zero, then set the
	 * result to 0 and return to caller. If the mask is a shared
	 * array, then set up the mask's sdd pointer.
	 */ 

	CHECK_MASK(mask_sdd_ptr, mask_flag);

	/*
	 * Call the Fortran work routine.
	 */

	for (i=0; i < RANK; i++) {
	    extents[i] = source->dimension[i].extent;
	}
	if (mask_flag != 0) {
	    if (source->type_lens.type == DVTYPE_REAL) {
	        XGLUE(XGLUE(MAXLOC_SP,RANK),@)(result_sdd_ptr, source_sdd_ptr,
		       mask_sdd_ptr, &mask_flag, extents);
	    } else {
	        XGLUE(XGLUE(MAXLOC_JP,RANK),@)(result_sdd_ptr, source_sdd_ptr,
		       mask_sdd_ptr, &mask_flag, extents);
	    }
	} else {
	    if (source->type_lens.type == DVTYPE_REAL) {
	        XGLUE(XGLUE(MAXLOC_NM_SP,RANK),@)(result_sdd_ptr,
			source_sdd_ptr, extents);
	    } else {
	        XGLUE(XGLUE(MAXLOC_NM_JP,RANK),@)(result_sdd_ptr,
			source_sdd_ptr, extents);
	    }
	}
}
