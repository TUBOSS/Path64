/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

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


/* ====================================================================
 * ====================================================================
 *
 * Module: ipa_option.c
 * $Revision: 1.4 $
 * $Date: 04/12/21 14:57:40-08:00 $
 * $Author: bos@eng-25.internal.keyresearch.com $
 * $Source: /home/bos/bk/kpro64-pending/ipa/main/analyze/SCCS/s.ipa_option.cxx $
 *
 * Revision history:
 *  24-Jan-96 - Original Version
 *
 * Description:
 *
 * Handle option processing for IPA.
 *
 * ====================================================================
 * ====================================================================
 */

#ifdef _KEEP_RCS_ID
static char *rcs_id = "$Source: /home/bos/bk/kpro64-pending/ipa/main/analyze/SCCS/s.ipa_option.cxx $ $Revision: 1.4 $";
#endif /* _KEEP_RCS_ID */

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "linker.h"

#pragma weak outfilename	/* So elfdump doesn't barf */

#define USE_STANDARD_TYPES
#include "defs.h"
#include "config.h"
#include "erglob.h"		/* Include the error tables */
#include "file_util.h"		/* for creating trace filename */
#include "flags.h"		/* for OPTION_LIST, etc. */
#include "glob.h"		/* for timer output file */
#include "mempool.h"		/* for MEM_Tracing_Enable () */
#include "timing.h"		/* for Initialize_Timing() */
#include "tracing.h"		/* to set up TFile for trace */
#include "strtab.h"		/* for ipc_option.h */
#include "config_lno.h"		// for LNO_Prompl
#include "ipc_option.h"		/* for -INLINE options */
#include "ipa_option.h"		/* to see if trace was specified */

/* Copied from config.c: */
#define MAX_OLIMIT		INT32_MAX

/* IPA file variables declared in glob.h: */
char *Ipa_File_Name = NULL;	/* IPAA summary file name */
FILE *Ipa_File = NULL;		/* IPAA summary file (if open) */

/* Skiplist support from config.h: */
typedef struct skiplist SKIPLIST;
SKIPLIST *Build_Skiplist ( OPTION_LIST *olist );
SKIPLIST *IPA_Skip_List = NULL;	/* List of skip options */

BOOL Trace_IPA = FALSE;		/* Main IPA progress trace */
BOOL Trace_Perf = FALSE;	/* performance trace */
BOOL Trace_IPALNO = FALSE;    /* IPA to LNO correctness trace */

/* flags that do not fall into the option group */
BOOL Verbose = FALSE;
BOOL Demangle = FALSE;

BOOL ProMP_Listing = FALSE;		// TURE means passing
					// -PROMP:next_id=<n> to backend


/* ====================================================================
 *
 * Process_IPA_Options
 *
 * Process the IPA options from an argument list (pre-filtered by ld).
 *
 * ====================================================================
 */

extern "C" void
Process_IPA_Options ( INT argc, char **argv )
{
    int i;

    for (i = 0; i < argc; i++) {
	if (argv[i] != NULL && *(argv[i]) == '-' ) {
	    if (Process_Command_Line_Group (argv[i]+1, Common_Option_Groups))
		continue;

	    switch (argv[i][1]) {

	    case 'I':
		/* We support a -IPA group option with no sub-options, which
		 * won't be recognized by Process_Command_Line_Group, so we
		 * need to tolerate it here to avoid error messages:
		 */
		if ( strcmp ( argv[i], "-IPA" ) != 0 ) {
		    ErrMsg ( EC_Unknown_Flag, argv[i][0], argv[i] );
		}
		break;

	    case 't':
		Process_Trace_Option ( argv[i] );
		break;

	    case 'Y':
		/* Just toss it for now... */
		break;

	    default:			/* What's this? */
		ErrMsg ( EC_Unknown_Flag, argv[i][0], argv[i] );
		break;
	    }
	}
    }

    if (LNO_Prompl)
	ProMP_Listing = TRUE;

    /* Configure Olimit to be max if set to 0: */
    if ( Olimit == 0 ) Olimit = MAX_OLIMIT;

    /* Specfile- and post-processing of -INLINE options: */
    Process_Inline_Options ();

#ifndef _STANDALONE_INLINER
    /* Specfile- and post-processing of GP partition options: */
    Process_IPA_Specfile_Options ();
#endif

    /* check consistency for Hard/Soft PU Limits */
    if (IPA_PU_Limit_Set) {
	if (IPA_PU_Hard_Limit_Set) {
	    if (IPA_PU_Hard_Limit < IPA_PU_Limit)
		IPA_PU_Hard_Limit = IPA_PU_Limit;
	} else
	    IPA_PU_Hard_Limit = IPA_PU_Limit + (IPA_PU_Limit >> 2);
    } else if (IPA_PU_Hard_Limit_Set) {
	IPA_PU_Limit = IPA_PU_Hard_Limit;
	IPA_PU_Limit_Set = TRUE;
    }

    if (IPA_Force_Depth_Set) {
	IPA_PU_Limit = UINT32_MAX;
	IPA_Bloat_Factor = UINT32_MAX;
	IPA_Max_Depth = IPA_Force_Depth;
    }

    IPA_Enable_Opt_Alias = FALSE;	/* turn this off for 7.2 */
    /* If WOPT uses alias info, we need to produce it: */
    if ( IPA_Enable_Opt_Alias ) {
	IPA_Enable_Simple_Alias = TRUE;
    }

    if (IPA_Output_File_Size != 0) {
	IPA_Max_Output_File_Size +=
	    IPA_Max_Output_File_Size / 100 * IPA_Output_File_Size;
    }

    if ( Get_Trace ( TKIND_ALLOC, TP_IPA ) ) {
	IPA_Enable_Memtrace = TRUE;
	MEM_Tracing_Enable ();
    }

    /* -tt1:1 requests all of the performance trace flags: */
    if ( Get_Trace ( TP_PTRACE1, 1 ) ) {
	Set_Trace ( TP_PTRACE1, 0xffffffff );
	Set_Trace ( TP_PTRACE2, 0xffffffff );
    }

    /* Any IPA TLOG trace flags set? */
    Trace_Perf = Get_Trace (TP_PTRACE1, TP_PTRACE1_INL|TP_PTRACE1_IPA);
    Trace_IPA = Get_Trace ( TP_IPA, IPA_TRACE_IPA );
    Verbose = ld_ipa_opt[LD_IPA_SHOW].flag;
    Demangle = ld_ipa_opt[LD_IPA_DEMANGLE].flag;

    if (ld_ipa_opt[LD_IPA_SHARABLE].flag & F_STATIC) {
	IPA_Enable_Picopt = FALSE;
	IPA_Enable_AutoGnum = FALSE;
    }

    if (!outfilename) {
    	outfilename = (char *)MALLOC(20);
	strcat(outfilename,"a.out");
    }
    /* Build a skip list: */
    IPA_Skip_List = Build_Skiplist ( IPA_Skip );

    /* Get IPAA summary file name if required: */
    if ( IPA_Enable_Simple_Alias && Ipa_File_Name == NULL ) {
	Ipa_File_Name = concat_names ( outfilename, (const string)".ipaa" );
    }

    /* Set up for tracing -- file, timers, etc.: */
    if ( Tracing_Enabled ) {
	char * cmd_file_name = concat_names ( outfilename, (const string)".ipa.t" );

	Set_Trace_File ( cmd_file_name );

	/* -ti64 requests a listing of all the -tt flags: */
	if ( Get_Trace ( TKIND_INFO, TINFO_TFLAGS ) ) {
	    List_Phase_Numbers ();
	}

	/* Initialize timers: */
	if ( Get_Trace ( TKIND_INFO, TINFO_TIME ) ) {
	    Tim_File = TFile;
	    Initialize_Timing ( TRUE );
	}
    }
   
    /* Transformation log file */
    if ( Get_Trace ( TP_PTRACE1, TP_PTRACE1_IPA) ||
	 Get_Trace ( TP_PTRACE1, TP_PTRACE1_IPA_CPROP) ||
	 Get_Trace ( TP_PTRACE1, TP_PTRACE1_IPALNO)) {
	if ( Tlog_File_Name == NULL ) {
	    /* Replace source file extension to get trace file: */
	    Tlog_File_Name =  concat_names ( outfilename, (const string)".tlog" );
	}
	if ( (Tlog_File = fopen ( Tlog_File_Name, "w" ) ) == NULL ) {
	    ErrMsg ( EC_Tlog_Open, Tlog_File_Name, errno );
	    Tlog_File_Name = NULL;
	    Tlog_File = stdout;
	}
    }

    /* Trace options: */
    if ( Get_Trace ( TP_MISC, 128 ) ) {
	Trace_Option_Groups ( TFile, Common_Option_Groups, TRUE );
    } else if ( Trace_IPA || Trace_Perf
	       || Get_Trace ( TP_MISC, 32 ) )
	{
	    Trace_Option_Groups ( TFile, Common_Option_Groups, FALSE );
	}
} /* Process_IPA_Options */
