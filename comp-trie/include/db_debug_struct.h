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
 * This file contains main structures of debugging section.
 */

#ifndef DB_DEBUG_STRUCT_H
#define DB_DEBUG_STRUCT_H
#ifndef DOT_DIR_PATH
#define DOT_DIR_PATH "./dot"
#endif
#ifndef DOT_FILE_PATH
#define DOT_FILE_PATH "./dot/graph.dot"
#endif
#ifndef PDF_FILE_PATH
#define PDF_FILE_PATH "./dot/graph.pdf"
#endif

struct t_stat {
    int max;    // -- branch with max length -- //
    int num;    // -- number of leaves (i.e. branches) -- //
    long long sum;    // -- sum of lengths of branches -- //
    struct linkedList_t* node;
    signed int id;     // -- dot_node id (increase by one after visiting a node) -- //
    int* width;        // -- number of nodes at each level -- //
    float chain_length; // -- sum of lengths of chainis --//
    long long ht_size; //-- sum of hash table sizes--// 
};

struct linkedList_t {
    struct node_t* current;      // -- current node -- //
    struct node_t* next;         // -- the next node -- //
};


#endif /* db_DEBUG_STRUCT_H */
