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

// type == 0 - fixed amount
// type == 1 - undefined amount
void _addPart(char *text, int section, int amount, char* name, int type)
{
    char *begin = NULL, *end = NULL; 
    char *endstr = NULL;
    void *tmp = NULL;
    int partlen = 0;
    int rr = 0;
    UUEFile* node = NULL;
    w_log(LL_FUNC,"%s::addPart()", __FILE__);
    begin = strchr(text, '\r');
    if(!begin) return;
    
    while(begin[0] == '\r')
        begin++;
    
    end = begin;
    while( *end  && 
                   (unsigned char)(*end) < '\x0061' && 
                  ((unsigned char)(*end) > '\x0020' || (unsigned char)(*end) == '\r') && 
          rr < 3)
    {
        rr = (end[0] == '\r') ?  rr+1 : 0;
        end++;
        if((rr == 2) && (strncmp(end,"---",3) == 0))
            break;
    }

    if(end)
    {
        if( rr > 1 ) end--; 
        partlen = end-begin;
        if(partlen < 2)
            return;

        endstr = strstr(end-1, "\rend\r");
    }
    else
        return;

    if(type == 0 && !endstr && amount == 1)
        return;


    node = FindUUEFile(name);
    if(!node)
    {
        if( type && !endstr )
        {
            amount++;
        }
        node = MakeUUEFile(amount,name);
        node->prev = UFilesHead->prev;
        UFilesHead->prev->next = node; // add to end
        UFilesHead->prev       = node; // save end pos
    }
    else
    {
        if(type && section == node->m_nSections && !endstr)
        {
            node->m_nSections = section+1;
            tmp = scalloc(1,node->m_nSections*sizeof(char*));
            memcpy(tmp,node->UUEparts,(node->m_nSections-1)*sizeof(char*));
            nfree(node->UUEparts);
            node->UUEparts = (char**)tmp;
            if(nDelMsg || nCutMsg)
            {
                tmp = scalloc(1,node->m_nSections*sizeof(dword));
                memcpy(tmp,node->toBeDeleted,(node->m_nSections-1)*sizeof(dword));
                nfree(node->toBeDeleted);
                node->toBeDeleted = (dword*)tmp;
            }
        }
    }
    AddPart(node, begin, section, partlen);
}

int scan4UUE(const char* text)
{
    int nRet = 0;
    char name[MAX];
    int perms = 0;
    int section = 0;
    int amount = 0;
    int atype = 0; 
    float ff = 0.0;
    int multi = 0;
    char *szSection = NULL;
    char *szBegin   = NULL;
    
    szSection = strstr(text, "section ");
    while(szSection)
    {
        if(sscanf(szSection,"section %d of %d of file %s",&section, &amount, name) == 3)
        {
            w_log(LL_FUNC,"%s::scan4UUE(), section %d of %d detected", __FILE__,section,amount);
            multi = 1;
            atype = 0;
        }
        else
        {
            if(sscanf(szSection,"section %d of uuencode %f of file %s",&section,&ff,name) == 3)
            {
                w_log(LL_FUNC,"%s::scan4UUE(), section %d detected", __FILE__, section);
                amount = section;
                multi = 1;
                atype = 1;
            }
            else
            {
                amount = 0;
            }
        }
        if(amount == 0) 
        {
            szSection = strstr(szSection+1, "section ");
            continue;
        }
        if(section == 1)
        {
            szBegin = strstr(szSection, "begin ");
            if(szBegin)
            {
                w_log(LL_FUNC,"%s::scan4UUE(), first section detected", __FILE__);
                _addPart(szBegin, section, amount, name, atype);
            }
        }
        else
        {
            _addPart(szSection, section, amount, name, atype);
        }
        szSection = strstr(szSection+1, "section ");
    }
    if(!multi)
    {
        szBegin = strstr(text, "begin ");
        while(szBegin)
        {
            if(strchr(szBegin,'\r') - szBegin > 10)
            {
                if(sscanf(szBegin, "begin %o %s", &perms, name) == 2) {
                    w_log(LL_FUNC,"%s::scan4UUE(), single message uue detcted", __FILE__);
                    _addPart(szBegin, 1, 1, name, 0);
                }
            }
            szBegin = strstr(szBegin+1, "begin ");
        }
    }
    return nRet;
}

char* cutUUEformMsg(char *text)
{
   int rr = 0;
   char *end = NULL;
   char *szBegin = strstr(text, "begin ");
   if(!szBegin) return NULL;
    
   szBegin = strchr(szBegin, '\r');
    if(!szBegin) return  NULL;
    
    while(szBegin[0] == '\r')
        szBegin++;

    end = szBegin;
    while( end && end[0] < '\x0061' && rr < 3)
    {
        rr = (end[0] == '\r') ?  rr+1 : 0;
        end++;
    }

    if(end)
    {
       if( rr > 1 ) end--; 
       memmove(szBegin,end,strlen(end)+1);
    }

   return text;
}

int processMsg(HAREA hArea, dword msgNumb, int scan_cut)
{
   HMSG msg;
   char *text = NULL;
   dword  textLen = 0;
   int rc = 0;

   msg = MsgOpenMsg(hArea, MOPEN_RW, msgNumb);
   if (msg == NULL) return rc;

   currMsgUid = MsgMsgnToUid(hArea, msgNumb);
   textLen = MsgGetTextLen(msg);
   text = (char *) scalloc(1,(textLen+1)*sizeof(char));

   if (MsgReadMsg(msg, &xmsg, 0, textLen, (byte*)text, 0, NULL)<0) {
      rc = 0;
   } else {
      if(scan_cut == 0)
      {
         scan4UUE(text);
      }
      else
      {
         text = cutUUEformMsg(text);
         if(text)
         {
            textLen = strlen(text)+1;
            MsgWriteMsg(msg, 0, &xmsg, (byte*)text, textLen, textLen, 0, NULL);
         }
      }
      rc = 1;
   }
   MsgCloseMsg(msg);
   if(!text)
   MsgKillMsg(hArea, msgNumb);
   nfree(text);
   return rc;
}

