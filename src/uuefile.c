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

#include "uuecode.h"
#include "dupe.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <huskylib/huskylib.h>


static char *invalidExt[] = {"*.mo?", "*.tu?", "*.we?", "*.th?", "*.fr?", 
                                   "*.sa?", "*.su?", "*.pkt", "*.tic"};


int  isReady(UUEFile* uuc);
void MakeFile(UUEFile* uuc);
void MakeTicFile(UUEFile* uuc);

typedef struct  {
    char* areamask;
    char* fileEcho;
}   uueFileGroup;

unsigned int fileGroupscount = 0;
uueFileGroup *uueFileGroups  = NULL;

void initFileGroups()
{
    unsigned int i = 0;
	char *tok;

    if(!config->numuuEGrp || uueFileGroups)
        return;
    for(i = 0; i < config->numuuEGrp; i++)
    {
        tok = strtok(config->uuEGrp[i], " \t,");
        tok = strtok(NULL, " \t,");
        while (tok) {
            uueFileGroups = srealloc(uueFileGroups, sizeof(uueFileGroup)*(fileGroupscount+1));
            uueFileGroups[fileGroupscount].areamask = sstrdup(tok);
            uueFileGroups[fileGroupscount].fileEcho = config->uuEGrp[i];
            fileGroupscount++;
            tok = strtok(NULL, " \t,");
        }
    }
}

char* findFileGroup(char* areaMask)
{
    unsigned int i = 0;
    char* areagroup = NULL;
    initFileGroups();

    if(!uueFileGroups)
        return xstrscat(&areagroup,"uue.",areaMask,NULL);
    for(i = 0; i < fileGroupscount; i++) {
        if (patimat(areaMask,uueFileGroups[i].areamask))
            return sstrdup(uueFileGroups[i].fileEcho);
    }
    return xstrscat(&areagroup,"uue.",areaMask,NULL);
}

int DecodePart(char *text, FILE *outfile)
{
    char *linep	= NULL;

    size_t   linelen	= 0;
    int      linecnt	= 0;
    UCHAR outbyte	[3];
    char* endl =NULL;  
    char* linebuf = text;
    int lastlen = 0;
    int nRet = 0;

    if(!text) return 0;
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
                /* outbyte [2] |= DECODE_BYTE (linep [3]) & 0x3F; */
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
        nRet = 1;
        do
        {    endl++;   }
        while(endl[0] == '\r');
        linebuf = endl;
        linelen = strlen(linebuf);
    } while (linelen > 1);
    
    return nRet;
}

int  isReady(UUEFile* uuc)
{
    int i;
    if(!uuc) return 0; /* Check pointers before use */

    for(i = 0; i < uuc->m_nSections; i++)
    {
        if(uuc->UUEparts[i] == NULL)
            return 0;
    }
    return (i == uuc->m_nSections) && uuc->m_fname ;
}

UUEFile* MakeUUEFile(int nsec, char *name, char* ID)
{
    UUEFile* uuc = scalloc(1,sizeof(UUEFile));
    uuc->m_nSections = nsec;
    if(nsec>0){
        uuc->UUEparts = (char**)scalloc( nsec, sizeof(char*) );
        if(nDelMsg || nCutMsg)
            uuc->toBeDeleted = (dword*)scalloc(nsec,sizeof(dword));
    }
    if(name)
    uuc->m_fname = sstrdup(name);
    uuc->ID = ID ? sstrdup(ID) : uuc->m_fname; 
    return uuc;
}


int  CompareUUEFile(char *p_e1, char *p_e2)
{
   UUEFile  *a, *b;
   if(!p_e1 || !p_e2) return p_e1>p_e2? 1: p_e1==p_e2? 0:-1;

   a = (UUEFile*)p_e1; 
   b = (UUEFile*)p_e2;
   return strcmp(a->ID,b->ID);
}

int  FreeUUEFile(char *entry)
{
    if(entry)
    {
        UUEFile  *uuc;
        int i;
        uuc = (UUEFile *)entry;
        for(i = 0; i < uuc->m_nSections; i++)
            nfree(uuc->UUEparts[i]);
        nfree(uuc->toBeDeleted);
        nfree(uuc->description);
        if(uuc->m_fname != uuc->ID)
            nfree(uuc->ID);
        nfree(uuc->m_fname);
    }
    return 1;
}

void AddPart(UUEFile* uuc, char* uuepart, int section, int slen)
{
    if(!uuc || !uuepart) return; /* Check pointers before use */

    if(section > uuc->m_nSections || uuc->m_nAdded == uuc->m_nSections || 
       uuc->UUEparts[section-1])
        return;

    if(section == 1)
    {
        uuc->description = sstrdup((char*)xmsg.subj);
        uuc->origin.zone  = xmsg.orig.zone;
        uuc->origin.net   = xmsg.orig.net;
        uuc->origin.node  = xmsg.orig.node;
        uuc->origin.point = xmsg.orig.point;
    }

    uuc->UUEparts[section-1] = scalloc( slen+1, sizeof(char) );
    strncpy(uuc->UUEparts[section-1],uuepart,slen);
    if(nDelMsg || nCutMsg)
        uuc->toBeDeleted[uuc->m_nAdded] = currMsgUid;
    uuc->m_nAdded++;
    if((uuc->m_nAdded == uuc->m_nSections) && isReady(uuc))
    {
        MakeFile(uuc);
    }
}
#if !defined(UNIX) && !defined(OS2) && !defined(__DJGPP__)
#   include <windows.h>
#endif

void MakeFile(UUEFile* uuc)
{
    FILE *out;
    char *fname = NULL;
    int i, nodel=0;
    s_textDupeEntry *msg;

    if(!uuc) return; /* Check pointers before use */
    
    msg = scalloc(sizeof(s_textDupeEntry),1);
    
    msg->filename = sstrdup(uuc->m_fname);
    msg->areaname = sstrdup(currArea->areaName);
    msg->from     = sstrdup((char*)xmsg.from);

    if(dupeDetection(msg) == 1)
    {
        for(i = 0; i < 9; i++)
        {
            if(patimat(uuc->m_fname,invalidExt[i]))
                xstrcat(&uuc->m_fname,"_");
        }
        xstrscat(&fname,config->protInbound,uuc->m_fname,NULL);
        if(!fexist(fname)) {
            out = fopen(fname, "wb");
            if(out) {
                for(i = 0; i < uuc->m_nSections; i++)
                {
                    if( DecodePart(uuc->UUEparts[i], out) == 0)
                        break;
                }
                fclose(out);
                if(i == uuc->m_nSections)
                {
                    w_log(LL_INFO, "file: %15s has %d complete sections is saved",
                        uuc->m_fname,uuc->m_nSections);
                    MakeTicFile(uuc);
                    nodel = 0;
                }
                else
                {
                    remove(fname);
                    nodel = 1;
                }
            } else {
                w_log(LL_ERROR,"file: %15s can not be created", uuc->m_fname);
            }
        } else {
            w_log(LL_CREAT,"file: %15s detected in inbound", uuc->m_fname);
        }
    }
    else
    {
        w_log(LL_FUNC,"%s::MakeFile(), dupe file %15s detected", __FILE__,uuc->m_fname);
    }
    if( (!nodel) && (nDelMsg || nCutMsg) )
    {
        for( i = 0; i < uuc->m_nSections; i++ )
            toBeDeleted[nMaxDeleted+i] = uuc->toBeDeleted[i];
        nMaxDeleted += uuc->m_nSections;
    }
    tree_delete(&UUEFileTree, CompareUUEFile,(char*)uuc, FreeUUEFile);
    nfree(fname);
    w_log(LL_FUNC,"%s::MakeFile(), exit", __FILE__);
}

void MakeTicFile(UUEFile* uuc)
{
   unsigned int i=0;
   struct stat stbuf;
   FILE *tichandle  = NULL;
   char *newticfile = NULL;
   char *fname      = NULL;
   char *areagroup  = NULL;
   s_link* link;
   
   if(!uuc) return; /* Check pointers before use */

   link = getLinkFromAddr( config,*(currArea->useAka) );
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

   xstrscat(&fname,config->protInbound,uuc->m_fname,NULL);

   stat(fname,&stbuf);
   if(stbuf.st_size == 0)
   {
       remove(fname);
       return;
   }

   newticfile=makeUniqueDosFileName(config->protInbound,"tic",config);
   
   tichandle=fopen(newticfile,"wb");

   areagroup = findFileGroup(currArea->areaName);


   if (config->outtab != NULL)
       recodeToTransportCharset((CHAR*)uuc->description);

   fprintf(tichandle, "Created by %s, written by Max Chernogor\r\n",versionStr);
   fprintf(tichandle, "File %s\r\n", uuc->m_fname);
   fprintf(tichandle, "Area %s\r\n", areagroup);
   if(currArea->areaName)
     fprintf(tichandle, "Areadesc UUE decoded from %s\r\n", currArea->areaName);
   if(uuc->description)
     fprintf(tichandle, "Desc %s\r\n", uuc->description);
   fprintf(tichandle, "From %s\r\n", aka2str(link->hisAka));
   fprintf(tichandle, "To %s\r\n",   aka2str(link->hisAka));
   fprintf(tichandle, "Origin %s\r\n", aka2str(uuc->origin));
   fprintf(tichandle, "Size %lu\r\n", (unsigned long)stbuf.st_size);
   fprintf(tichandle, "Crc %08lX\r\n", (unsigned long) filecrc32(fname));
   if(link->fileFixPwd)
     fprintf(tichandle, "Pw %s\r\n", link->fileFixPwd);

   fclose(tichandle);
   w_log(LL_CREAT, "tic file:%s created for file:%s",newticfile, uuc->m_fname);   
   nfree(newticfile);
   nfree(areagroup);
}
