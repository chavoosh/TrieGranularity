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
 * This file contains necessary functions for monitoring and debugging the trie. 
 */ 

#include "db_debug_struct.h"
#include "ct_trie.h"

#ifndef DB_DEBUG_H
#define DB_DEBUG_H

void db_dfs (struct ct_instance*, bool);
int db_do_dfs (struct ct_instance*, struct node_t* /*next_node*/, struct node_t* /*parent node*/, int /*height*/, struct t_stat*, signed int/*p_id*/, bool);
void db_print_node (struct node_t*);
void db_print_node_to_file (struct node_t* /*next_node*/, struct node_t* /*parent_node*/, signed int /*next_node id*/, signed int /*parent id*/);

#endif /* -- db_DEBUG_H -- */
