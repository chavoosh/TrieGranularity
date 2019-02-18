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
 * Defintition of main structures and functions of bit-level trie
 */

#include "bt_struct.h"
#include "db_debug_struct.h"

#ifndef BT_TRIE_H
#define BT_TRIE_H
#ifndef MAX_HEIGHT
#define MAX_HEIGHT 10000
#endif
#ifndef SLASH
#define SLASH 0x2F
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* ---------------------------------------------------------
 * structure of a node in bit-based trie
 *
 *                 [ N O D E ] --> bytes[]
 *                  |       |
 *    1st_child (node)     2nd_child (node)
 * --------------------------------------------------------- */
struct node_t {
    char* bytes;       // -- content of a node -- //
    /**
     * length of node's content, in terms of "BIT"
     * E.g. the node may contain 2 bytes, but the
     * length be 6. It means just 6 bits of these
     * 2 bytes (i.e. 8 bits) should be checked. 
     */
    int len;                  
    struct node_t* child_0;   // -- the first child (i.e. 0-based child) -- //
    struct node_t* child_1;   // -- the second child (i.e. 1-based child) -- //
    struct node_t* parent;    // -- the parent of the current node -- //
    bool EON_flag;            // -- whether this is node is the end of a name -- //
    /**
     * Optional fields
     *    ...
     */
};

struct bt_instance {
    struct node_t root;
    struct t_stat* trie_stat;
};

/* -------------- main functions ---------------*/
struct node_t* bt_insert (struct bt_instance*, const char*, bool);   // -- insert a name if it is not already there -- //
struct node_t* bt_do_insert (struct bt_instance*, struct node_t* /*parent*/, int /*which child?*/, const char*, int /*node_bit_walker*/, int /*bit_walker*/, bool);
struct node_t* bt_node_partition (struct bt_instance*, struct node_t*, int /*which child*/, const char*, int /*node_bit_walker*/, int /*bit_walker*/, bool);

//struct node_t* bt_node_merge (struct bt_instance*, struct child_t* /*child which points to the parent*/);
struct node_t* bt_lookup (struct bt_instance*, const char*, bool /*printf_flag*/, bool /*exact_match*/);   // -- lookup a given name -- //
int bt_remove (struct bt_instance*, const char*, bool);   // -- remove a given name -- //
struct node_t* bt_node_merge (struct bt_instance*, struct node_t* /*parent*/, int /*child*/, bool);
signed int bt_byte_compare (char, char, int /*number of bits to compare (NON-ZERO-based)*/);
int bt_byte_cpy (const char* /*src*/, struct node_t* /*dst node*/, int /*start index (ZERO_Based)*/, int /*end index ZERO-Based*/);
int bt_byte_cpy_index (const char* /*src*/, struct node_t* /*dst node*/, int /*dst start index (ZERO-based)*/, int /*src start index (ZERO_Based)*/, int /*end index ZERO-Based*/);    // -- accept start index for dst -- //

void bt_free_node (struct node_t*);
void bt_do_free_node (struct node_t*);
 
#endif /* bt_TRIE_H */
