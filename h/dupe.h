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

#ifndef DUPE_H
#define DUPE_H

#include <fidoconf/fidoconf.h>
#include <huskylib/typesize.h>
#include <huskylib/tree.h>

/* This header file contains the structures of the dupe file */

struct textDupeEntry {
  char *filename, *from, *areaname;
  time_t timeCreated;
};

typedef struct textDupeEntry s_textDupeEntry;


typedef struct hashMDupeEntry s_hashMDupeEntry;

struct dupeMemory {
  tree *avlTree;
};

typedef struct dupeMemory s_dupeMemory;


int writeToDupeFile();
int dupeDetection(s_textDupeEntry *msg);

#endif /* DUPE_H */
