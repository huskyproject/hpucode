#include <errno.h>
#include <time.h>
#include <string.h>
#include "uuecode.h"
#include "dupe.h"

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



unsigned long msgCopied, msgProcessed; // per Area
unsigned long totaloldMsg, totalmsgCopied;


void ScanArea(s_area *area)
{
   char* areaName;
   HAREA oldArea;
   dword highMsg, numMsg/*, hw*/;
   word areaType = area -> msgbType & (MSGTYPE_JAM | MSGTYPE_SQUISH | MSGTYPE_SDM);
   
   
   currArea = area;
   areaName = area -> fileName;
   
   oldArea = MsgOpenArea((byte *) areaName, MSGAREA_NORMAL, areaType);
   
   if (oldArea != NULL) {
       
       
       highMsg = MsgGetHighMsg(oldArea);
       numMsg = MsgGetNumMsg(oldArea);

       toBeDeleted = nDelMsg ? (dword*)smalloc(highMsg * sizeof(dword)) : NULL;
       
       for (currMsgNumb = 1; currMsgNumb <= highMsg; currMsgNumb++) {
           processMsg(oldArea);
       };
      
       if(toBeDeleted && nMaxDeleted)
       {
           w_log(LL_INFO, "Deleting decoded messages...");
           numMsg = 0;
           for (currMsgNumb = 0; currMsgNumb < nMaxDeleted; currMsgNumb++) {
               if(MsgKillMsg(oldArea, toBeDeleted[currMsgNumb]-currMsgNumb) == 0)
                    numMsg++;
           }
           w_log(LL_INFO, "Deleted:%u of decoded messages:%u",numMsg,nMaxDeleted);
           nMaxDeleted=0;
       }
       nfree(toBeDeleted);
       MsgCloseArea(oldArea);
       FreeUUEChain();
   }
   else {
       if (oldArea) MsgCloseArea(oldArea);
       printf("Could not open %s ", areaName);
   }
}
   
void handleArea(s_area *area)
{
   if ((area -> msgbType & MSGTYPE_SQUISH) == MSGTYPE_SQUISH ||
      (area -> msgbType & MSGTYPE_JAM) == MSGTYPE_JAM ||
      (area -> msgbType & MSGTYPE_SDM) == MSGTYPE_SDM) {
      w_log(LL_INFO, "Scan area: %s", area -> areaName);
      msgCopied = 0;
      msgProcessed = 0;
      ScanArea(area);
   };
}

void doArea(s_area *area, char *cmp)
{
   if (patimat(area->areaName,cmp)) handleArea(area);
}

int main(int argc, char **argv) {
   
   unsigned int i;
   int          k;
   struct _minf m;
   char* buff=NULL;
   
   printf(  "\n::  uuecode v0.1\n");
   if( argc < 2 ) {
  	   printf ("::  usage: uuecode [ -del ] [areamask1 areamask2 ...] \n");
   } else {
      if((argc > 2) && (strcmp(argv[1], "-del") == 0))
         nDelMsg = 1;
      else
         nDelMsg = 0;
      
      setvar("module", "uuecode");
      config = readConfig(NULL);
      
      if (config != NULL ) {
         if (config->logFileDir) {
            xstrscat(&buff, config->logFileDir, "uuecode.log", NULL);
            openLog(buff, "uuecode", config);
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
         description = NULL;
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
         k = nDelMsg == 1 ? 2 : 1;
         while(k < argc)           
         {
            for (i=0; i < config->netMailAreaCount; i++)
               // scan netmail areas
               doArea(&(config->netMailAreas[i]), argv[k]);
            
            for (i=0; i < config->echoAreaCount; i++)
               // scan echomail areas
               doArea(&(config->echoAreas[i]), argv[k]);
/*            
            for (i=0; i < config->localAreaCount; i++)
               // scan local areas
               doArea(&(config->localAreas[i]), argv[k]);
*/
            k++;
         }
         writeToDupeFile();
         disposeConfig(config);
         FreeUUEChain();
         nfree(UFilesHead);
         nfree(description);
         w_log(LL_STOP, "End");
         closeLog();
         return 0;
      } else {
         printf("Could not read fido config\n");
         return 1;
      }
   }
   return 0;
}
