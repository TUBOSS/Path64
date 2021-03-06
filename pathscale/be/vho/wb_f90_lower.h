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


#ifndef wb_f90_lower_INCLUDED
#define wb_f90_lower_INCLUDED

extern void WB_F90_Lower_Initialize(WN* wn_global, WN_MAP Prompf_Id_Map);

extern void WB_F90_Lower_Set_Parent_Map(WN_MAP Parent_Map); 

extern void WB_F90_Lower_Set_Prompf_Info(PROMPF_INFO* prompf_info); 

extern void WB_F90_Lower_Terminate();

extern void s_f90_lower_debug(const char init_buffer[]);

#endif /* wb_f90_lower_INCLUDED */
