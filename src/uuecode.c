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

#include <errno.h>
#include <time.h>
#include <huskylib/huskylib.h>
#include "uuecode.h"
#include "dupe.h"
#include "cvsdate.h"
#include "version.h"

#ifdef UNIX
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef __TURBOC__
#include <share.h>
#endif

#ifdef __EMX__
#include <share.h>
#include <sys/types.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>


#if defined ( __WATCOMC__ )
#include <share.h>
#endif

char* versionStr      = NULL;
int  nDelMsg, nCutMsg, nAllAreas;

typedef struct {
    int   Negative;
    char* Mask;
} sFilters;

sFilters* Filters=NULL; 
int nFilters = 0;

static tree* FilteredAreas = NULL;

void start_help(void) {

    fprintf(stdout, "%s by Max Chernogor\n",versionStr);
    fprintf(stdout,"Usage: hpucode [ -del|-cut|-all ] [areamask1 !areamask2 ...] \n\n");
    fprintf(stdout,"Options:  -del, -cut, -all\n");
    fprintf(stdout,"          -del - delete decoded messages\n");
    fprintf(stdout,"          -cut - cut UUE code from decoded messages\n");
    fprintf(stdout,"          -all - scans all areas defined by areamsks in command line;\n");
    fprintf(stdout,"                 by default hpucode scans areas that match areamasks\n");
    fprintf(stdout,"                 from importlog file\n");
}


int processCommandLine(int argc, char **argv)
{
    int i = 0;

    if (argc == 1)
    {
        start_help();
        return 0;
    }

    Filters = scalloc(sizeof(sFilters),argc);

    while (i < argc-1) {
        i++;
        if        (stricmp(argv[i], "-del") == 0) {
            nDelMsg = 1;
            continue;
        } else if (stricmp(argv[i], "-cut") == 0) {
            nCutMsg = 1;
            continue;
        } else if (stricmp(argv[i], "-all") == 0) {
            nAllAreas = 1;
            continue;
        } else if (stricmp(argv[i], "-h") == 0) {
            start_help();
            continue;
        } else {
            Filters[nFilters].Mask = argv[i];
            if (argv[i][0] == '!')
                Filters[nFilters].Negative = 1;
            nFilters++;
        }
    } /* endwhile */

    return nFilters;
}


int fc_compareEntries(char *p_e1, char *p_e2)
{
    ps_area e1 = (ps_area)p_e1;
    ps_area e2 = (ps_area)p_e2;
    if(stricmp(e1->areaName,e2->areaName) < 0)
        return -1;
    else if(stricmp(e1->areaName,e2->areaName) > 0)
        return 1;
    return 0;
}
int fc_deleteEntry(char *p_e1) {
    return 1;
}

void ApplyFilter(ps_area area) 
{
    int i;
    for(i = 0; i < nFilters; i++)
    {
        if( (Filters[i].Negative == 0) && (patimat(area->areaName,Filters[i].Mask)) )
            break;
    }
    if (i == nFilters)
        return;

    for(i = 0; i < nFilters; i++)
    {
        if( (Filters[i].Negative == 1) && (patimat(area->areaName,(Filters[i].Mask+1))) )
            return;
    }
    tree_add(&FilteredAreas, fc_compareEntries, (char *)(area), fc_deleteEntry );
}

int ApplyFilters() 
{
    FILE  *impLog = NULL;
    ps_area  area = NULL;

    tree_init(&FilteredAreas, 1);

    if(!nAllAreas)
    {
        char* buff=NULL;

        impLog = fopen(config->importlog, "r");
        if(impLog) 
        {
            while (!feof(impLog)) {
                buff = readLine(impLog);
                if(!buff) break;
                if ((area = getNetMailArea(config, buff)) == NULL) {
                    area = getArea(config, buff);
                } 
                ApplyFilter(area);
                nfree(buff);
            }
            fclose(impLog);
        }
    }
    if(impLog) {
        w_log(LL_SCANNING,
        "Using importlogfile -> scanning only listed areas that match the mask");
    }  else {
        unsigned i;

        w_log(LL_SCANNING,
        "Scanning all areas that match the mask");

        for (i=0; i < config->netMailAreaCount; i++)
            ApplyFilter(&(config->netMailAreas[i]));

        for (i=0; i < config->echoAreaCount; i++)
            ApplyFilter(&(config->echoAreas[i]));

        for (i=0; i < config->localAreaCount; i++)
            ApplyFilter(&(config->localAreas[i]));

    }
    return tree_count(&FilteredAreas);
}


int ScanArea(char *carea)
{
   char* areaName;
   HAREA oldArea;
   dword highMsg, numMsg,nMN;
   word areaType;
   ps_area area = (ps_area)carea;

   if(!area) return 1;

   w_log(LL_SCANNING, "Scan area: %s", area -> areaName);

   areaType = area -> msgbType & (MSGTYPE_JAM | MSGTYPE_SQUISH | MSGTYPE_SDM);

   currArea = area;
   areaName = area -> fileName;
   
   oldArea = MsgOpenArea((byte *) areaName, MSGAREA_NORMAL, areaType);
   
   if (oldArea != NULL) {
       
       tree_init(&UUEFileTree,1);
       
       highMsg = MsgGetHighMsg(oldArea);

       if(highMsg && (nDelMsg || nCutMsg))
          toBeDeleted = (dword*)smalloc(highMsg * sizeof(dword));
       else
          toBeDeleted = NULL;
       
       for (nMN = 1; nMN <= highMsg; nMN++) {
           processMsg(oldArea,nMN,0);
       };
      
       if(nDelMsg && nMaxDeleted)
       {
           w_log(LL_INFO, "Deleting decoded messages...");
           numMsg = 0;
           for (nMN = 0; nMN < nMaxDeleted; nMN++) {
               if(MsgKillMsg(oldArea, MsgUidToMsgn(oldArea,toBeDeleted[nMN], UID_EXACT)) == 0)
                   numMsg++;
           }
           w_log(LL_INFO, "Deleted:%u of decoded messages:%u",numMsg,nMaxDeleted);
           nMaxDeleted=0;
       }
       if(nCutMsg && nMaxDeleted)
       {
           w_log(LL_INFO, "Cuting UUE code from  messages...");
           numMsg = 0;
           for (nMN = 0; nMN < nMaxDeleted; nMN++) {
               processMsg(oldArea,MsgUidToMsgn(oldArea,toBeDeleted[nMN], UID_EXACT),1);
           }
           nMaxDeleted=0;
       }

       nfree(toBeDeleted);
       MsgCloseArea(oldArea);
       tree_mung(&UUEFileTree, FreeUUEFile);
   }
   else {
       if (oldArea) MsgCloseArea(oldArea);
       w_log(LL_ERROR, "Could not open %s ", areaName);
   }
   return 1;
}
   
int main(int argc, char **argv) {
    
    s_area       *area   = NULL;
    struct _minf m;
    char* buff=NULL;

    xscatprintf(&buff, "%u.%u.%u", VER_MAJOR, VER_MINOR, VER_PATCH);
    setvar("version", buff);
    
    versionStr = GenVersionStr( "hpuCode", VER_MAJOR, VER_MINOR, VER_PATCH,
                               VER_BRANCH, cvs_date );
    nDelMsg = nCutMsg = nAllAreas = 0;

    if (processCommandLine(argc, argv) == 0) 
        return 0;

#ifdef __NT__
    SetFileApisToOEM();
#endif

    setvar("module", "hpucode");
    config = readConfig(NULL);

    if (!config) {
        printf("Could not read fido config\n");
        return 1;
    }

    if (config->lockfile) {
        lock_fd = lockFile(config->lockfile, config->advisoryLock);
        if( lock_fd < 0 )
        {
            disposeConfig(config);
            exit(EX_CANTCREAT);
        }
    }
    /*  load recoding tables */
    initCharsets();
    getctabs(config->intab,config->outtab);

    if (config->logFileDir) {
        nfree(buff);
        xstrscat(&buff, config->logFileDir, "hpucode.log", NULL);
        initLog(config->logFileDir, config->logEchoToScreen, config->loglevels, config->screenloglevels);
        openLog(buff, versionStr);
        nfree(buff);
    } 
    if(config->protInbound)
        _createDirectoryTree(config->protInbound);
    else
    {
        printf("\n\tProtInbound not defined\n");
        if (config->lockfile) {
            FreelockFile(config->lockfile ,lock_fd);
        }
        return 1;
    }
    w_log(LL_START, "Start");

    if(ApplyFilters())
    {
        currArea = NULL;
        toBeDeleted = NULL;
        nMaxDeleted = 0;
        m.req_version = 2;
        m.def_zone = config->addr[0].zone;
        if (MsgOpenApi(&m)!= 0) {
            printf("MsgOpenApi Error.\n");
            if (config->lockfile) {
                FreelockFile(config->lockfile ,lock_fd);
            }
            exit(1);
        }
        tree_trav( &FilteredAreas, &ScanArea);
        writeToDupeFile();
        MsgCloseApi();
    }  else {
        w_log(LL_STOP, "Nothing to scan");
    }
    w_log(LL_STOP, "End");
    closeLog();
    doneCharsets();
    if (config->lockfile) {
        FreelockFile(config->lockfile ,lock_fd);
    }
    disposeConfig(config);
    return 0;
}
