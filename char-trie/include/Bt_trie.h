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
 */

#include "db_debug_struct.h"
#ifndef BT_TRIE_H
#define BT_TRIE_H

#ifndef SLASH
#define SLASH 0x2F
#endif
#ifndef EON
#define EON 0x01  // -- end of each name -- //
#endif
#ifndef MAX_HEIGHT
#define MAX_HEIGHT 100
#endif
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef int bool;
#define true 1
#define false 0

/* ---------------------------------------------------------
 * structure of a node in Byte-level trie
 *
 *    [NODE] --> bytestream | len
 *    |
 *    hash_table (children) --> buckets | size
 *    |
 *    bucket--> first_byte 
 *    |
 *    [Next_Node] ..
 * --------------------------------------------------------- */
struct ht_t {
    struct bucket_t* buckets;   // -- buckets of the hash table -- //
    int size;
    int used;  // -- number of used buckets -- //
};

struct bucket_t {
    char first_byte;  // -- a child stores only the first byte of the next node -- //
    struct node_t* next_node;
};

struct node_t {
    struct ht_t* hash_table;   // -- each node has a hash_table to store the children -- //
    char* bytes;   // -- contet of the node -- // 
    int len;       // -- len of content -- //
    struct node_t* parent;  
};

struct Bt_instance {
    struct node_t root;
    struct t_stat* trie_stat;
    struct node_t** visitedNodes;  // -- used by remove function -- //
    int ht_init_size;              // -- initial size of hash table at each node -- //
};

/* -------------- main functions ---------------*/
char* Bt_en_name (const char*, char** /*output*/);
struct node_t* Bt_insert (struct Bt_instance*, const char*, bool);   // -- insert a name if it is not already there -- //
struct node_t* Bt_do_insert (struct Bt_instance*, struct node_t*, const char* /*name*/, int/*byte_walker*/, bool);
struct node_t* Bt_node_partition (struct Bt_instance*, struct bucket_t*, const char* /*name*/, char /*first_byte*/, int /*byte_walker*/, int /*node_byte_walker*/, bool);

struct node_t* Bt_lookup (struct Bt_instance*, const char*, bool /*printf_flag*/, bool /*exact_match*/, struct node_t** /*visitedChildren*/);   // -- lookup a given name -- //

int Bt_remove (struct Bt_instance*, const char*, bool);   // -- remove a given name -- //
struct node_t* Bt_node_merge (struct Bt_instance*, struct node_t* /*parent of node to remove*/);

void Bt_free_node (struct node_t*);
void Bt_do_free_node (struct node_t*);
#endif /* bt_TRIE_H */


