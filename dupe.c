/*****************************************************************************
 * HPT --- FTN NetMail/EchoMail Tosser
 *****************************************************************************
 * Copyright (C) 1997-2000
 *
 * Matthias Tichy
 *
 * Fido:     2:2433/1245 2:2433/1247 2:2432/605.14
 * Internet: mtt@tichy.de
 *
 * Grimmestr. 12         Buchholzer Weg 4
 * 33098 Paderborn       40472 Duesseldorf
 * Germany               Germany
 *
 * Hash Dupe and other typeDupeBase (C) 2000 
 *
 * Alexander Vernigora
 *
 * Fido:     2:4625/69              
 * Internet: alexv@vsmu.vinnica.ua
 *
 * Yunosty 79, app.13 
 * 287100 Vinnitsa   
 * Ukraine
 *
 * This file is part of HPT.
 *
 * HPT is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * HPT is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HPT; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/
#include <smapi/unused.h>
#include <errno.h>
#include "uuecode.h"
#include "dupe.h"


FILE *fDupe;

s_dupeMemory *CommonDupes=NULL;

int compareEntriesBlank(char *e1, char *e2) {
   int rc=1;
   unused(e1); unused(e2);
   return rc;
}


int compareEntries(char *p_e1, char *p_e2) {
   const s_textDupeEntry  *atxt,   *btxt;
   int rc = 1;
   const void *e1 = (const void *)p_e1, *e2 = (const void *)p_e2;
   
   atxt = e1; btxt = e2;
   rc = strcmp(atxt->filename, btxt->filename);
   if (rc == 0) rc = strcmp(atxt->from, btxt->from);
   if (rc == 0) rc = strcmp(atxt->areaname, btxt->areaname);
   
   return rc;
}

int writeEntry(char *p_entry) {
   const s_textDupeEntry  *entxt;
   const void *entry = (const void *)p_entry;
   
   entxt = entry;

   fprintf(fDupe,"%s %s %s\n",entxt->filename,entxt->areaname,entxt->from);

   return 1;
}

int deleteEntry(char *entry) {
   s_textDupeEntry  *entxt;

   
   entxt = (s_textDupeEntry *)entry;
   nfree(entxt->from);
   nfree(entxt->filename);
   nfree(entxt->areaname);
   nfree(entxt);

   return 1;
}

void doReading(FILE *f, s_dupeMemory *mem) {
   s_textDupeEntry  *entxt;
   char *line, *token;
   char white[]=" \t";
   while( (line = readLine(f)) != NULL ) 
   {
      entxt = (s_textDupeEntry*) smalloc(sizeof(s_textDupeEntry));

      token = strtok( line, white );
      if(!token)
      { nfree(entxt); continue; }
      
      entxt->filename = sstrdup(token);

      token = strtok( NULL, white );
      if(!token)
      { nfree(entxt); continue; }

      entxt->areaname = sstrdup(token);

      token = strtok( NULL, white );
      if(!token)
      { nfree(entxt); continue; }

      entxt->from = sstrdup(token);
      
      /*
      linelen = fnlen = 0;
      // try to find begin of filename
      while(line[fnlen] !='\0' && isspace(line[fnlen])) fnlen++;
      if(line[fnlen] =='\0')
      { nfree(entxt); continue; }

      strspn(line,white);

      prevpos = fnlen;
//      prevpos = strspn(line,white);

      // try to find end of filename
      while(line[fnlen] !='\0' && !isspace(line[fnlen])) fnlen++;
      if(line[fnlen] =='\0')
      { nfree(entxt); continue; }
      
      linelen = fnlen - prevpos;
      entxt->filename = scalloc(linelen+1,sizeof(char));
      // try to find begin of areaname
      while(line[fnlen] !='\0' && isspace(line[fnlen])) fnlen++;
      if(line[fnlen] =='\0')
      { nfree(entxt); continue; }
      
      prevpos = fnlen;
      // try to find end of areaname
      while(line[fnlen] !='\0' && !isspace(line[fnlen])) fnlen++;
      if(line[fnlen] =='\0')
      { nfree(entxt); continue; }

      linelen = fnlen - prevpos;
      entxt->areaname = scalloc(linelen+1,sizeof(char));
      
      // try to find begin of from
      while(line[fnlen] !='\0' && isspace(line[fnlen])) fnlen++;
      if(line[fnlen] =='\0')
      { nfree(entxt); continue; }
      
      prevpos = fnlen;
      // try to find end of from
      while(line[fnlen] !='\0' && !isspace(line[fnlen])) fnlen++;

      linelen = fnlen - prevpos;
      entxt->from = scalloc(linelen+1,sizeof(char));
*/      
      tree_add(&(mem->avlTree), compareEntriesBlank, (char *) entxt, deleteEntry);
      
   }
}

s_dupeMemory *readDupeFile() {
   FILE *f;
   char *fileName=NULL;
   s_dupeMemory *dupeMemory;

   dupeMemory = smalloc(sizeof(s_dupeMemory));
   tree_init(&(dupeMemory->avlTree));
   
   xstrscat(&fileName, config->dupeHistoryDir, "uuecode.dup", NULL);
   w_log('2', "Reading dupes from %s", fileName);

   f = fopen(fileName, "rb");
   if (f != NULL) { w_log(LL_FILE,"dupe.c:readDupeFile(): opened %s (\"rb\" mode)",fileName);
       // readFile
       doReading(f, dupeMemory);
       fclose(f);
   } else {
       if (fexist(fileName)) w_log('2', "Error reading dupes");
       else if( errno != ENOENT)
         w_log('2', "Dupe base read error: %s", strerror(errno) );
   }

   nfree(fileName);

   return dupeMemory;
}

void freeDupeMemory() {
   
   if (CommonDupes != NULL) {
      tree_mung(&(CommonDupes -> avlTree), deleteEntry);
      nfree(CommonDupes);
   };
}

int createDupeFile(char *name) {
   FILE *f;

//   w_log(LL_SRCLINE,"dupe.c:%u:createDupeFile() name='%s'", __LINE__, name);

   f = fopen(name, "wb");
   if (f!= NULL) {
      w_log(LL_FILE,"dupe.c:createDupeFile(): opened %s (\"wb\" mode)",name);
      
      fDupe = f;
      tree_trav(&(CommonDupes->avlTree), writeEntry);
      fDupe = NULL;
      
      fclose(f);
      freeDupeMemory();

      return 0;
   } else return 1;
}


int writeToDupeFile() {
   char *fileName=NULL;

   int  rc = 0;          
   
   xstrscat(&fileName, config->dupeHistoryDir, "uuecode.dup", NULL);
   
   if (CommonDupes != NULL) {
      if (tree_count(&(CommonDupes->avlTree)) > 0) {
         rc = createDupeFile(fileName);
      }
   }
   nfree(fileName);
   return rc;
}


int dupeDetection(s_textDupeEntry *msg) {
   
   int pos=0;
   
   if (CommonDupes == NULL)
      CommonDupes = readDupeFile(); //read Dupes
   
   while ( msg->from[pos] != '\0' )
   {
      if( (msg->from[pos] == ' ') || (msg->from[pos] == '\t'))
         msg->from[pos] = '_';
      pos++;
   }
   
   if (tree_srchall(&(CommonDupes->avlTree), compareEntries, (char *) msg)) {
      tree_add(&(CommonDupes->avlTree), compareEntriesBlank, (char *) msg, deleteEntry);
      return 1;
   }
   else {
      deleteEntry((char *)msg);
      return 0;
   }
   
   return 0;
}

