/* -*- Mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2019
 * Regents of the University of Arizona & University of Michigan.
 *
 * TrieGranularity is a free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * TrieGranularity source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with TrieGranularity, e.g., in COPYING.md or LICENSE file.
 * If not, see <http://www.gnu.org/licenses/>.
 * 
 * For list of authors, please see AUTHORS.md file.
 * 
 * Description:
 * Each node in character-level trie has an instance of hash table.
 */

#ifndef HT_HASHTABLE_H
#define HT_HASHTABLE_H

#ifndef HT_INIT_SIZE
#define HT_INIT_SIZE  1
#endif
#include "Bt_trie.h"

struct bucket_t* ht_lookup (struct Bt_instance*, struct node_t*, char /*first_byte*/, bool print_flag);
struct bucket_t* ht_insert (struct Bt_instance*, struct node_t*, char /*first_bytes*/, bool print_flag);
void ht_rehash (struct Bt_instance*, struct node_t*);

#endif /* -- end of ht_HASHTABLE_H -- */
