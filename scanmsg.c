#include "uuecode.h"
//#include "dupe.h"


void addPart(char *text, int section, int amount, char* name)
{
    char *begin = NULL, *end= NULL, *end2 =NULL;
    int partlen = 0;
    UUEFile* node = NULL;
    begin = strchr(text, '\r');
    if(!begin) return;
    
    while(begin[0] == '\r')
        begin++;

    end = begin;
    while( end && end[0] < '\x0061' )
        end++;

    if(end)
        partlen = end-begin;
    else
        return;

    node = FindUUEFile(name);
    if(!node)
    {
        node = MakeUUEFile(amount,name);
        node->prev = UFilesHead->prev;
        UFilesHead->prev->next = node; // add to end
        UFilesHead->prev       = node; // save end pos
    }
    AddPart(node, begin, section, partlen);
}

int scan4UUE(char* text, dword textLen)
{
    int nRet = 0;
    int linelen=80;
    char name[MAX];
    int perms;
    int section;
    int amount;
    int multi = 0;
    char *szSection = NULL;
    char *szBegin   = NULL;
    
    szSection = strstr(text, "section ");
    while(szSection)
    {
        if(sscanf(szSection,"section %d of %d of file %s",&section, &amount, name) == 3)
        {
            multi = 1;
            if(section == 1)
            {
                szBegin = strstr(szSection, "begin ");
                while(szBegin)
                {
                    if(sscanf(szBegin, "begin %o %s", &perms, name) == 2) {
                        addPart(szBegin, section, amount, name);
                        description = sstrdup(xmsg.subj);
                    }
                    szBegin = strstr(szBegin+1, "begin ");
                }
            }
            else
            {
                addPart(szSection, section, amount, name);
            }

        }
        szSection = strstr(szSection+1, "section ");
    }
    if(!multi)
    {
        szBegin = strstr(text, "begin ");
        while(szBegin)
        {
            if(sscanf(szBegin, "begin %o %s", &perms, name) == 2) {
                nRet = 0;
            }
            szBegin = strstr(szBegin+1, "begin ");
        }
    }
    return nRet;
}

int processMsg(HAREA oldArea)
{
   HMSG msg;
   char *text;
   dword  textLen;
   int unsent, rc = 0;


   msg = MsgOpenMsg(oldArea, MOPEN_RW, currMsgNumb);
   if (msg == NULL) return rc;

   if (MsgReadMsg(msg, &xmsg, 0, 0, NULL, 0, NULL)<0) {
      MsgCloseMsg(msg);
      return rc;
   }

   unsent = (xmsg.attr & MSGLOCAL) && !(xmsg.attr & MSGSENT);
   
   textLen = MsgGetTextLen(msg);
   text = (char *) malloc(textLen+1);
   text[textLen] = '\0';
   
   MsgReadMsg(msg, NULL, 0, textLen, (byte*)text, 0, NULL);
   MsgCloseMsg(msg);

   scan4UUE(text,textLen);
   free(text);
   rc = 1;
   
   return rc;
}

