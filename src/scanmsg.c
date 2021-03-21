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

#include "uuecode.h"

/*  type == 0 - fixed amount */
/*  type == 1 - undefined amount */
void _addPart(char * text,
              int section,
              int amount,
              char * name,
              char * ID,
              int type,
              char * msgBody)
{
    char * begin = NULL, * end = NULL;
    char * endstr  = NULL;
    void * tmp     = NULL;
    size_t partlen    = 0;
    UUEFile * node = NULL;
    UUEFile nfnd;

    if(!text)
    {
        return;
    }

    w_log(LL_FUNC, "%s::_addPart()", __FILE__);
    begin = text;

    while(*begin)
    {
        endstr = strchr(begin, '\r');

        if(!endstr)
        {
            return;
        }

        partlen = 3 * (endstr - begin - 1) / 4;

        if(DECODE_BYTE(begin[0]) == partlen)
        {
            break;
        }

        if(((*(endstr - 1) == '`') && (DECODE_BYTE(begin[0]) == partlen - 1)))
        {
            break;
        }

        while(*endstr++ == '\r')
        {}
        begin = endstr - 1;
    }
    end = begin;

    while(*end)
    {
        endstr = strchr(end, '\r');

        if(!endstr)
        {
            break;
        }

        partlen = 3 * (endstr - end - 1) / 4;

        if(DECODE_BYTE(end[0]) != partlen)
        {
            if(!(((*(endstr - 2) == '`') || (*(endstr - 1) == '`') || (*(endstr + 1) == '`')) &&
                 ((DECODE_BYTE(end[0]) == partlen - 1) || (DECODE_BYTE(end[0]) == partlen - 2))))
            {
                /*
                   while(*endstr++ == '\r') { };
                   end = endstr-1;
                 */
                break;
            }
        }

        while(*endstr++ == '\r')
        {}
        end = endstr - 1;
    }
    partlen = end - begin;

    if(partlen < 2)
    {
        return;
    }

    nfnd.ID = ID ? ID : name;
    node    = (UUEFile *)tree_srch(&UUEFileTree, CompareUUEFile, (char *)&nfnd);

    if(!node)
    {
        if(type && !endstr)
        {
            amount++;
        }

        node = MakeUUEFile(amount, name, ID);
        tree_add(&UUEFileTree, CompareUUEFile, (char *)node, FreeUUEFile);
    }
    else
    {
        if(!node->m_fname && name)
        {
            node->m_fname = sstrdup(name);
        }

        if(type && section == node->m_nSections && !endstr)
        {
            node->m_nSections = section + 1;
            tmp = scalloc(1, node->m_nSections * sizeof(char *));
            memcpy(tmp, node->UUEparts, (node->m_nSections - 1) * sizeof(char *));
            nfree(node->UUEparts);
            node->UUEparts = (char **)tmp;

            if(nDelMsg || nCutMsg)
            {
                tmp = scalloc(1, node->m_nSections * sizeof(DelCutStruct));
                memcpy(tmp, node->toBeDeleted, (node->m_nSections - 1) * sizeof(DelCutStruct));
                nfree(node->toBeDeleted);
                node->toBeDeleted = (DelCutStruct *)tmp;
            }
        }
    }

    AddPart(node, msgBody, begin, section, (int)partlen);
} /* _addPart */

int scan4UUE(char * text, const char * ctl)
{
    int nRet = 0;
    char name[MAX];
    unsigned int perms = 0;
    int section        = 0;
    int amount         = 0;
    int atype          = 0;
    float ff           = 0.0;
    char * szSection   = NULL;
    char * szBegin     = NULL;

    if(!text)
    {
        return 0;
    }

    szSection = strstr(text, "section ");

    while(szSection)
    {
        if(sscanf(szSection, "section %d of %d of file %s", &section, &amount, name) == 3)
        {
            w_log(LL_FUNC, "%s::scan4UUE(), section %d of %d detected", __FILE__, section, amount);
            atype = 0;
        }
        else if(sscanf(szSection, "section %d of uuencode %f of file %s", &section, &ff,
                       name) == 3)
        {
            w_log(LL_FUNC, "%s::scan4UUE(), section %d detected", __FILE__, section);
            amount = section;
            atype  = 1;
        }
        else
        {
            amount = 0;
        }

        if(amount == 0)
        {
            szSection = strstr(szSection + 1, "section ");
            continue;
        }

        if(amount > MAX_SECTIONS)
        {
            w_log(LL_WARN, "Number of sections:%d too much for decoding", amount);
            szSection = strstr(szSection + 1, "section ");
            continue;
        }

        if(section == 1)
        {
            szBegin = strstr(szSection, "begin ");

            if(szBegin)
            {
                w_log(LL_FUNC, "%s::scan4UUE(), first section detected", __FILE__);
                _addPart(szBegin, section, amount, OS_independed_basename(name), NULL, atype,
                         text);
            }
        }
        else
        {
            _addPart(szSection, section, amount, OS_independed_basename(name), NULL, atype, text);
        }

        nRet      = 1;
        szSection = strstr(szSection + 1, "section ");
    }

    if(nRet)
    {
        return nRet;
    }

    szBegin = strstr(text, "begin ");

    while(szBegin)
    {
        if(strchr(szBegin, '\r') - szBegin > 10)
        {
            if(sscanf(szBegin, "begin %o %s", &perms, name) == 2)
            {
                char * SPLIT = (char *)MsgGetCtrlToken((byte *)ctl, (byte *)"SPLIT");

                if(SPLIT)
                {
                    w_log(LL_FUNC, "%s::scan4UUE(), SPLITed message uue detcted", __FILE__);
                    section = 10 * (SPLIT[45] - '0') + SPLIT[46] - '0';
                    amount  = 10 * (SPLIT[48] - '0') + SPLIT[49] - '0';

                    if((section > amount) || (section <= 0) || (amount <= 0))
                    {
                        w_log(LL_WARN, "Incorrect number of sections [%d/%d]", section, amount);
                        szBegin = strstr(szBegin + 1, "begin ");
                        continue;
                    }

                    if(amount > MAX_SECTIONS)
                    {
                        w_log(LL_WARN, "Number of sections:%d too much for decoding", amount);
                        szBegin = strstr(szBegin + 1, "begin ");
                        continue;
                    }

                    SPLIT[44] = '\0';
                    stripRoundingChars(SPLIT + 5, " \t");
                    _addPart(szBegin,
                             section,
                             amount,
                             OS_independed_basename(name),
                             SPLIT + 5,
                             0,
                             text);
                    nfree(SPLIT);
                }
                else
                {
                    w_log(LL_FUNC, "%s::scan4UUE(), single message uue detcted", __FILE__);
                    _addPart(szBegin, 1, 1, OS_independed_basename(name), NULL, 0, text);
                }

                nRet = 1;
            }
        }

        szBegin = strstr(szBegin + 1, "begin ");
    }

    if(nRet)
    {
        return nRet;
    }
    else
    {
        char * SPLIT = (char *)MsgGetCtrlToken((byte *)ctl, (byte *)"SPLIT");

        /* FSC-0047 */
        if(SPLIT && SPLIT[47] == '/')
        {
            w_log(LL_FUNC, "%s::scan4UUE(), SPLITed message uue detcted", __FILE__);
            section = 10 * (SPLIT[45] - '0') + SPLIT[46] - '0';
            amount  = 10 * (SPLIT[48] - '0') + SPLIT[49] - '0';

            if(amount > MAX_SECTIONS)
            {
                w_log(LL_WARN, "Number of sections:%d too much for decoding", amount);
                return nRet;
            }

            SPLIT[44] = '\0';
            stripRoundingChars(SPLIT + 5, " \t");
            _addPart(text, section, amount, NULL, SPLIT + 5, 0, text);
            nfree(SPLIT);
        }
    }

    return nRet;
} /* scan4UUE */

#if 0
char* cutUUEformMsg(char *text)
{
    int rr = 0;
    char *end = NULL;
    char *p   = NULL;
    char *szBegin;
    int DelMsg = 1;

    if(!text) return NULL;

    p = text;

    while(p)
    {

        szBegin = strstr(p, "begin ");
        if ((szBegin) && (DelMsg == 1)) { /* Is this first section or single UUE ? */
            DelMsg = 0;
        }
        /* if(!szBegin) return NULL; */
        if( szBegin )
        {
            szBegin = strchr(szBegin, '\r');
            if(!szBegin) return  NULL;

            while(szBegin[0] == '\r')
                szBegin++;
        }
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
        p = end;
    }
    return  DelMsg == 0 ? text : NULL;
}
#endif

int processMsg(HAREA hArea, dword msgNumb, int scan_cut, UINT nBegCut, UINT nEndCut)
{
    HMSG msg;
    char * text   = NULL;
    char * ctl    = NULL;
    char * p      = NULL;
    dword textLen = 0;
    dword ctlen   = 0;
    int rc        = 0;

    msg = MsgOpenMsg(hArea, MOPEN_RW, msgNumb);

    if(msg == NULL)
    {
        return rc;
    }

    currMsgUid = MsgMsgnToUid(hArea, msgNumb);
    textLen    = MsgGetTextLen(msg);
    ctlen      = MsgGetCtrlLen(msg);
    text       = (char *)scalloc(1, (textLen + 1) * sizeof(char));
    ctl        = (char *)scalloc(1, (ctlen + 1) * sizeof(char));
    p          = text;
    memset(&xmsg, 0, sizeof(xmsg));

    if(MsgReadMsg(msg, &xmsg, 0, textLen, (byte *)text, ctlen, (byte *)ctl) == (dword)(-1L))
    {
        rc = 0;
    }
    else
    {
        if(scan_cut == 0)
        {
            ctl[ctlen] = '\0';
            scan4UUE(text, ctl);
        }
        else if(nBegCut > 0)
        {
            char * szBeg = text + nBegCut;
            char * szEnd = text + nBegCut + nEndCut;
            memmove(szBeg, szEnd, strlen(szEnd) + 1);
            textLen = (dword)strlen(text) + 1;

            if(hArea->type == MSGTYPE_SQUISH)
            {
                MsgCloseMsg(msg);
                msg = MsgOpenMsg(hArea, MOPEN_CREATE, msgNumb);
            }

            MsgWriteMsg(msg, 0, &xmsg, (byte *)text, textLen, textLen, ctlen, (byte *)ctl);
        }
        else
        {
            p = NULL; /*  Kill Msg */
        }

        rc = 1;
    }

    MsgCloseMsg(msg);

    if(!p)
    {
        MsgKillMsg(hArea, msgNumb);
    }

    nfree(text);
    nfree(ctl);
    return rc;
} /* processMsg */
