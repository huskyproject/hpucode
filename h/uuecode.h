/* $Id$ */

/*****************************************************************************
 * HPUCODE --- Uuencoded files from FTN messagebase extractor
 *****************************************************************************
 * Copyright (C) 2002  Max Chernogor & Husky Team
 *
 * http://husky.sf.net
 *
 * This file is part of HUSKY fidonet package.
 *
 * HUSKY is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * HUSKY is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HPT; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#ifndef _HDECODER_H_
#define _HDECODER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smapi/msgapi.h>
#include <smapi/prog.h>
#include <fidoconf/fidoconf.h>
#include <fidoconf/common.h>
#include <fidoconf/xstr.h>
#include <fidoconf/log.h>

#define MAX              64

typedef struct _UUEFile
{
    char            *m_fname;
    int             m_nSections;
    int             m_nAdded;
    int             m_nPerm;
   char**           UUEparts;
   dword*           toBeDeleted;
    char            *description;
  hs_addr           origin;
  struct _UUEFile*  next;
  struct _UUEFile*  prev;
} UUEFile ;


int processMsg(HAREA hArea, dword msgNumb, int scan_cut);

UUEFile* MakeUUEFile(int nsec, char *name);
UUEFile* FindUUEFile(char *name);
void FreeUUEChain();
void AddPart(UUEFile* uuc, char* uuepart, int section, int slen);

int      nDelMsg;
int      nCutMsg;
dword*   toBeDeleted;
dword    nMaxDeleted;
UUEFile  *UFilesHead;
s_area   *currArea;
s_fidoconfig *config;
XMSG     xmsg;
dword    currMsgUid;
char*    versionStr;



#endif
