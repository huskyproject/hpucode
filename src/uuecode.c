/*
    $Id$
*/

#include <errno.h>
#include <time.h>
#include <string.h>
#include <fidoconf/recode.h>
#include "uuecode.h"
#include "dupe.h"
#include "cvsdate.h"

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
#include <string.h>
#include <stdlib.h>
#include <smapi/prog.h>
#include <share.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#include <share.h>
#endif
#ifdef _MAKE_DLL_MVC_    
#define SH_DENYNO   _SH_DENYNO
#define S_IREAD     _S_IREAD
#define S_IWRITE    _S_IWRITE
#endif

#ifdef __MINGW32__
int __stdcall SetFileApisToOEM(void);
#endif

const int VER_MAJOR   = 0;
const int VER_MINOR   = 31;
char* versionStr      = NULL;

void ScanArea(s_area *area)
{
   char* areaName;
   HAREA oldArea;
   dword highMsg, numMsg,nMN;
   word areaType = area -> msgbType & (MSGTYPE_JAM | MSGTYPE_SQUISH | MSGTYPE_SDM);
   
   
   currArea = area;
   areaName = area -> fileName;
   
   oldArea = MsgOpenArea((byte *) areaName, MSGAREA_NORMAL, areaType);
   
   if (oldArea != NULL) {
       
       
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
       FreeUUEChain();
   }
   else {
       if (oldArea) MsgCloseArea(oldArea);
       w_log(LL_ERROR, "Could not open %s ", areaName);
   }
}
   
void doArea(s_area *area, char *cmp)
{
    if(area->scn == 1) // do not scan area twice
        return;

    if (patimat(area->areaName,cmp)) 
    {
        if ((area -> msgbType & MSGTYPE_SQUISH) == MSGTYPE_SQUISH ||
            (area -> msgbType & MSGTYPE_JAM) == MSGTYPE_JAM ||
            (area -> msgbType & MSGTYPE_SDM) == MSGTYPE_SDM) {
            w_log(LL_INFO, "Scan area: %s", area -> areaName);
            ScanArea(area);
            area->scn = 1;
        }
    }
}

int main(int argc, char **argv) {
    
    unsigned int i;
    int          k;
    FILE         *impLog = NULL;
    s_area       *area   = NULL;

    struct _minf m;
    char* buff=NULL;
    
    xscatprintf(&buff, "%u.%u", VER_MAJOR, VER_MINOR);
    
#ifdef __linux__
    xstrcat(&buff, "/lnx");
#elif defined(__FreeBSD__) || defined(__NetBSD__)
    xstrcat(&buff, "/bsd");
#elif defined(__OS2__) || defined(OS2)
    xstrcat(&buff, "/os2");
#elif defined(__NT__)
    xstrcat(&buff, "/w32");
#elif defined(__sun__)
    xstrcat(&buff, "/sun");
#elif defined(MSDOS)
    xstrcat(&buff, "/dos");
#elif defined(__BEOS__)
    xstrcat(&buff, "/beos");
#endif
    xscatprintf(&buff, " %s", cvs_date);
    xscatprintf(&versionStr,"hpuCode %s", buff);
    nfree(buff);
    
    printf(  "\n::  %s by Max Chernogor\n",versionStr);
    
    if( argc < 2 ) {
        printf ("::  usage: hpucode [ -del|-cut ] [areamask1 areamask2 ...] \n");
        return 0;
    }
    nDelMsg = nCutMsg = 0;
    if(argc > 2)
    { 
        if(strcmp(argv[1], "-del") == 0)
            nDelMsg = 1;
        if(strcmp(argv[1], "-cut") == 0)
            nCutMsg = 1;
    }

#ifdef __NT__
    SetFileApisToOEM();
#endif

    setvar("module", "hpucode");
    config = readConfig(NULL);

    if (config != NULL ) {
        // load recoding tables
        initCharsets();
        if (config->intab != NULL) getctab(intab, (unsigned char*) config->intab);
        if (config->outtab != NULL) getctab(outtab, (unsigned char*) config->outtab);

        if (config->logFileDir) {
            xstrscat(&buff, config->logFileDir, "hpucode.log", NULL);
            openLog(buff, "hpucode", config);
            nfree(buff);
        } 
        if(config->protInbound)
            _createDirectoryTree(config->protInbound);
        else
        {
            printf("\n\tProtInbound not defined\n");
            return 1;
        }
        w_log(LL_START, "Start");
        currArea = NULL;
        toBeDeleted = NULL;
        nMaxDeleted = 0;
        UFilesHead = scalloc(1,sizeof(UUEFile));
        UFilesHead->prev = UFilesHead;
        m.req_version = 2;
        m.def_zone = config->addr[0].zone;
        if (MsgOpenApi(&m)!= 0) {
            printf("MsgOpenApi Error.\n");
            exit(1);
        }
        k = (nDelMsg || nCutMsg) ? 2 : 1;
        impLog = fopen(config->importlog, "r");

        if(impLog)
            w_log(LL_INFO, 
            "Using importlogfile -> scanning only listed areas that match the mask");
        else
            w_log(LL_INFO, 
            "Scanning all areas that match the mask");

        while(k < argc)           
        {
            if(impLog)
            {
                rewind(impLog);
                while (!feof(impLog)) {
                    buff = readLine(impLog);
                    if(!buff) break;
                    if ((area = getNetMailArea(config, buff)) == NULL) {
                        area = getArea(config, buff);
                    } 
                    doArea(area, argv[k]);
                    nfree(buff);
                }
            } else {
                for (i=0; i < config->netMailAreaCount; i++)
                    // scan netmail areas
                    doArea(&(config->netMailAreas[i]), argv[k]);
                
                for (i=0; i < config->echoAreaCount; i++)
                    // scan echomail areas
                    doArea(&(config->echoAreas[i]), argv[k]);
            }
            k++;
        }
        if(impLog) fclose(impLog);
        writeToDupeFile();
        MsgCloseApi();
        FreeUUEChain();
        w_log(LL_STOP, "End");
        closeLog();
        doneCharsets();
        disposeConfig(config);
        nfree(UFilesHead);
        return 0;
    } else {
        printf("Could not read fido config\n");
        return 1;
    }
    return 0;
}
