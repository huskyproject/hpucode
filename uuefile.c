#include "uuecode.h"
#include "dupe.h"
#include <sys/stat.h>
#include <fidoconf/crc.h>

int  isReady(UUEFile* uuc);
void MakeFile(UUEFile* uuc);
void MakeTicFile(UUEFile* uuc);
void FreeUUEFile(UUEFile* uuc);


#define DECODE_BYTE(b) ((b == 0x60) ? 0 : b - 0x20)

typedef unsigned char BYTE;

void DecodePart(char *text, FILE *outfile)
{
    char *linep	= NULL;
    size_t   linelen	= 0;
    int      linecnt	= 0;
    BYTE outbyte	[3];
    char* endl =NULL;  
    char* linebuf = text;
    int lastlen = 0;
    do
    {
        endl = strchr(linebuf,'\r');  

        if(endl) endl[0] = '\0';
        else     break;

        /* The first byte of the line represents the length of the DECODED
        line */
        linelen = DECODE_BYTE (linebuf [0]);
        linep = linebuf + 1;
        for (linecnt = linelen; linecnt > 0; linecnt -= 3, linep += 4)
        {
            lastlen = strlen(linep);
            if(lastlen > 3)
            {
                /* Decode the 4-byte block */
                outbyte [0] = DECODE_BYTE (linep [0]);
                outbyte [1] = DECODE_BYTE (linep [1]);
                outbyte [0] <<= 2;
                outbyte [0] |= (outbyte [1] >> 4) & 0x03;
                outbyte [1] <<= 4;
                outbyte [2] = DECODE_BYTE (linep [2]);
                outbyte [1] |= (outbyte [2] >> 2) & 0x0F;
                outbyte [2] <<= 6;
                outbyte [2] |= DECODE_BYTE (linep [3]) & 0x3F;
            }
            else
            {
                outbyte [0] = DECODE_BYTE (linep [0]);
                if(lastlen > 1)
                {
                    outbyte [1] = DECODE_BYTE (linep [1]);
                    outbyte [0] <<= 2;
                    outbyte [0] |= (outbyte [1] >> 4) & 0x03;
                    outbyte [1] <<= 4;
                }
                if(lastlen > 2)
                {
                    outbyte [2] = DECODE_BYTE (linep [2]);
                    outbyte [1] |= (outbyte [2] >> 2) & 0x0F;
                    outbyte [2] <<= 6;
                }
                //outbyte [2] |= DECODE_BYTE (linep [3]) & 0x3F;
            }
            /* Write the decoded bytes to the output file */
            
            if (linecnt > 3)
            {
                if (fwrite (outbyte, 1, 3, outfile) != 3)
                {
                    fprintf (stderr, "uudecode: Error writing to output file\n");
                    exit (1);
                }
            }
            else
            {
                if (fwrite (outbyte, 1, linecnt, outfile) != (size_t)linecnt)
                {
                    fprintf (stderr, "uudecode: Error writing to output file\n");
                    exit (1);
                }
                linecnt = 3;
            }
            
        }
        do
        {    endl++;   }
        while(endl[0] == '\r');
        linebuf = endl;
        linelen = strlen(linebuf);
    } while (linelen > 1);
}

int  isReady(UUEFile* uuc)
{
    int i;
    for(i = 0; i < uuc->m_nSections; i++)
    {
        if(uuc->UUEparts[i] == NULL)
            return 0;
    }
    return i == uuc->m_nSections;
}

UUEFile* MakeUUEFile(int nsec, char *name)
{
    UUEFile* uuc = scalloc(1,sizeof(UUEFile));
    uuc->m_nSections = nsec;
    if(nsec>0){
        uuc->UUEparts = (char**)scalloc( nsec, sizeof(char*) );
        if(nDelMsg)
            uuc->toBeDeleted = (dword*)scalloc(nsec,sizeof(dword));
    }
    if(name)
    strncpy(uuc->m_fname,name,MAX-1);
    return uuc;
}

void FreeUUEFile(UUEFile* uuc)
{
    int i;
    for(i = 0; i < uuc->m_nSections; i++)
    nfree(uuc->UUEparts[i]);
    nfree(uuc->toBeDeleted);
}

void FreeUUEChain()
{
    UUEFile* nodeToDel;
    UUEFile* node = UFilesHead->next;
    UFilesHead->next = NULL;
    UFilesHead->prev = UFilesHead;
    while(node)
    {
        nodeToDel = node;
        FreeUUEFile(nodeToDel);
        node = node->next;
        nfree(nodeToDel);
    }
}

UUEFile* FindUUEFile(char *name)
{
    UUEFile* node = UFilesHead->next;
    while(node)
    {
        if( stricmp(name, node->m_fname) == 0)
            break;
        node = node->next;
    }
    return node;
}


void AddPart(UUEFile* uuc, char* uuepart, int section, int slen)
{
    if(section > uuc->m_nSections || uuc->m_nAdded == uuc->m_nSections)
        return;
    uuc->UUEparts[section-1] = scalloc( slen+1, sizeof(char) );
    strncpy(uuc->UUEparts[section-1],uuepart,slen);
    if(nDelMsg)
        uuc->toBeDeleted[uuc->m_nAdded] = currMsgUid;
    uuc->m_nAdded++;
    if((uuc->m_nAdded == uuc->m_nSections) && isReady(uuc))
    {
        MakeFile(uuc);
    }
}
#ifdef WINNT
#   ifdef __MINGW32__
    typedef int   BOOL;
    typedef char *LPWSTR;
    typedef const char *LPCWSTR;
    BOOL    __stdcall OemToCharA(LPCWSTR,LPWSTR);
    #define OemToChar OemToCharA
#   endif    
#else
#   include <windows.h>
#endif

void MakeFile(UUEFile* uuc)
{
    FILE *out;
    UUEFile* prevUUEF;
    char fname[256] = "";
    int i;
    
    s_textDupeEntry *msg = scalloc(sizeof(s_textDupeEntry),1);
    
    msg->filename = sstrdup(uuc->m_fname);
    msg->areaname = sstrdup(currArea->areaName);
    msg->from     = sstrdup(xmsg.from);

    if(dupeDetection(msg) == 1)
    {
       w_log(LL_INFO, "file: %15s has %d complete sections is saved",
          uuc->m_fname,uuc->m_nSections);
       strcpy(fname,config->protInbound);
       strcat(fname,uuc->m_fname);          
#ifdef WINNT
        OemToChar(fname, fname);
#endif    
        out = fopen(fname, "wb");
        for(i = 0; i < uuc->m_nSections; i++)
        {
            DecodePart(uuc->UUEparts[i], out);
        }
        fclose(out);
        MakeTicFile(uuc);
    }
    else
    {
    }
    if(nDelMsg)
    {
        for( i = 0; i < uuc->m_nSections; i++ )
            toBeDeleted[nMaxDeleted+i] = uuc->toBeDeleted[i];
        nMaxDeleted += uuc->m_nSections;
    }
    FreeUUEFile(uuc);
    prevUUEF = uuc->prev;
    prevUUEF->next = uuc->next;
    if(uuc->next)
        uuc->next->prev = prevUUEF;
    if(prevUUEF->next == NULL)
        UFilesHead->prev = prevUUEF;
    nfree(uuc);
}

void MakeTicFile(UUEFile* uuc)
{
   unsigned int i=0;
   struct stat stbuf;
   FILE *tichandle;
   char *newticfile;
   char fname[256] = "";
   s_link* link = getLinkFromAddr( config,*(currArea->useAka) );
   
   while( !link && i < config->addrCount )
   {
      link = getLinkFromAddr( config, config->addr[i] );
      i++;
   }
   if(!link)
   {
      w_log(LL_INFO, "tic file not created for file %s", uuc->m_fname);
      return ;
   }

   strcpy(fname,config->protInbound);
   strcat(fname,uuc->m_fname);          

   stat(fname,&stbuf);

   newticfile=makeUniqueDosFileName(config->protInbound,"tic",config);

   tichandle=fopen(newticfile,"wb");

   fprintf(tichandle,"Created by uuecode, written by Max Chernogor\r\n");
   fprintf(tichandle,"File %s\r\n", uuc->m_fname);
   fprintf(tichandle,"Area %s\r\n", currArea->areaName);
   fprintf(tichandle,"Desc %s\r\n", description);
   fprintf(tichandle,"From %s\r\n", aka2str(link->hisAka));
   fprintf(tichandle,"To %s\r\n",   aka2str(link->hisAka));
   fprintf(tichandle,"Origin %s\r\n",aka2str(link->hisAka));
   fprintf(tichandle,"Size %lu\r\n", stbuf.st_size);
   fprintf(tichandle,"Crc %08lX\r\n",filecrc32(fname));
   fprintf(tichandle,"Pw %s\r\n",link->fileFixPwd);

   fclose(tichandle);
   nfree(newticfile);
}