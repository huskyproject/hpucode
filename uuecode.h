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
dword   currMsgUid;



#endif
