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
 * Defintition of main structures and functions of component-based trie
 */

#include "db_debug_struct.h"
#ifndef CT_TRIE_H
#define CT_TRIE_H

#ifndef SLASH
#define SLASH 0x2F
#endif
#ifndef MAX_NUM_OF_COMPS
#define MAX_NUM_OF_COMPS 100
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

/* ----------------------------------------------------------------------------------------
 * structure of a node in component-based trie
 *
 *    [NODE] --> [Components]: <1st_comp><2nd_comp>...<nth_comp>
 *    |
 *    1st_child --> 2nd_child --> 3rd_child --> .. ->nth_child
 *    |
 *    |
 *    [Next_Node]-> [Components]: ..
 *    |
 *    1st_child --> ..
 * ---------------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------------
 * structure of hash tables
 *
 *     [HT] => [  b_0   |  b_1  |  b_2   |  b_3  | ... |  b_n  ] & [size] & [used]
 *                 \
 *              [next_bucket] & [key] & [next_node] ...
 *                  \
 *                [next_bucket] ..
 *
 * NOTE:
 *     The first bucket is used JUST as a poitner, it means its next_node is always NULL, as
 *     it is not used for storing any info. We use it as the head of the linked list at each
 *     cell of this array (i.e. HT).
 * ----------------------------------------------------------------------------------------- */

struct node_t {
    struct comp_t* comps;    // -- context of a node (i.e. a given component) -- //
    int num_of_comp;         // -- number of components which are included by this node -- //
    struct ht_t* hash_table; // -- pointer to children -- //   
    struct node_t* parent;
    /**
     * Optional fields
     *    ...
     */
};

struct ht_t {
    struct bucket_t* buckets;
    int size;
    int used;
};

struct comp_t {
    char* bytes;  // -- characters of this component -- //
    int len;      // -- length of this component -- //
};

struct bucket_t {
    struct node_t* next_node;    // -- the node which is pointed by this child (i.e. pointer) -- //
    unsigned long long key;
    // -- we use chaining for collision resolution -- //
    struct bucket_t* next_bucket; 
    struct bucket_t* pre_bucket;
    bool head;
};

struct ct_instance {
    struct node_t root;
    struct t_stat* trie_stat;
    struct bucket_t** visitedChildren; // -- used by remove function -- //
    char** comps_array;                // -- extracted name components (used by lookup) -- //
    int ht_init_size;                  // -- the initial size of hash tables -- //
};

/* -------------- main functions ---------------*/
struct node_t* trie_insert (struct ct_instance*, const char*, bool);   // -- insert a name if it is not already there -- //
struct node_t* trie_do_insert (struct ct_instance*, struct node_t*, char**, int /*comp_walker*/, int /*all_comp*/, bool);
struct node_t* trie_node_partition (struct ct_instance*, struct bucket_t* /*pointer to the node to partition*/, char**, int /*num of comps*/, int /*comp_walker*/, int/*node_comp_walker*/, bool);

struct node_t* trie_node_merge (struct ct_instance*, struct bucket_t* /*child which points to the parent*/);
struct node_t* trie_lookup (struct ct_instance*, const char*, bool /*printf_flag*/, bool /*exact_match*/, struct bucket_t** /*visitedChildren*/);   // -- lookup a given name -- //
int trie_remove (struct ct_instance*, const char*, bool);   // -- remove a given name -- //

void trie_free_node (struct node_t*);
void trie_do_free_node (struct node_t*);
#endif /* ct_TRIE_H */


