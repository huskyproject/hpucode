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

struct ticfiletype {
                char file[50];      // Name of the file affected by Tic
                char area[50];      // Name of File Area
                char areadesc[100]; // Description of File Area
                char **desc;        // Short Description of file
                int anzdesc;        // Number of Desc Lines
                char replaces[50];  // Replaces File
                int size;           // Size of file
                unsigned long crc;  // CRC of File
                unsigned long date; // Date
                s_addr from;        // From Addr
                s_addr to;          // To Addr
                s_addr origin;      // Origin
                char password[50];  // Password
                char **ldesc;       // Array of Pointer to Strings with ldescs
                int anzldesc;       // Number of Ldesc Lines
                s_addr *seenby;     // Array of Pointer to Seenbys
                int anzseenby;      // Number of seenbys
                char **path;        // Array of Pointer to Strings with Path
                int anzpath;        // Numer of Path lines
                };

typedef struct ticfiletype s_ticfile;

typedef struct _UUEFile
{
    char            m_fname[MAX];
    int             m_nSections;
    int             m_nAdded;
    int             m_nPerm;
   char**           UUEparts;
   dword*           toBeDeleted;
  struct _UUEFile*  next;
  struct _UUEFile*  prev;
} UUEFile ;


int processMsg(HAREA oldArea, dword msgNumb);


UUEFile* MakeUUEFile(int nsec, char *name);
UUEFile* FindUUEFile(char *name);
void FreeUUEChain();
void AddPart(UUEFile* uuc, char* uuepart, int section, int slen);

int      nDelMsg;
dword*   toBeDeleted;
dword    nMaxDeleted;
UUEFile  *UFilesHead;
s_area   *currArea;
s_fidoconfig *config;
XMSG     xmsg;
dword   currMsgUid;
char *description;


#endif