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
/* $Id$ */


#ifndef _HDECODER_H_
#define _HDECODER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smapi/msgapi.h>
#include <fidoconf/fidoconf.h>
#include <fidoconf/common.h>
#include <huskylib/xstr.h>
#include <huskylib/log.h>
#include <huskylib/tree.h>

#define MAX 64
#define MAX_SECTIONS 10000

#define DECODE_BYTE(b) ((b == 0x60) ? 0 : b - 0x20)

typedef struct _DelCut
{
    dword nDelMsg;
    UINT  nBegCut;
    UINT  nEndCut;
} DelCutStruct;

typedef struct _UUEFile
{
    char *         ID;
    char *         m_fname;
    int            m_nSections;
    int            m_nAdded;
    int            m_nPerm;
    char **        UUEparts;
    DelCutStruct * toBeDeleted;
    char *         description;
    hs_addr        origin;
} UUEFile;

extern tree * UUEFileTree;

int processMsg(HAREA hArea, dword msgNumb, int scan_cut, UINT nBegCut, UINT nEndCut);
UUEFile * MakeUUEFile(int nsec, char * name, char * ID);
int FreeUUEFile(char *);
int CompareUUEFile(char *, char *);
void AddPart(UUEFile * uuc, char * msgBody, char * uuepart, int section, int slen);

extern int nDelMsg;
extern int nCutMsg;
extern DelCutStruct * toBeDeleted;
extern dword nMaxDeleted;
extern s_area * currArea;
extern s_fidoconfig * config;
extern XMSG xmsg;
extern dword currMsgUid;
extern char * versionStr;
extern int lock_fd;


#endif // ifndef _HDECODER_H_
