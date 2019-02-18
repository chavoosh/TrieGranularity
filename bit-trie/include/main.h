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

#include "bt_trie.h"
#ifndef MAIN_H
#define MAIN_H

#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN 10000 // set a upper bound for name length
#endif

void print_inst (char*);     // -- program help -- //
void print_summary (struct bt_instance*, double, double, double, bool, bool);   // -- summary of program after running -- //
void warmup (struct bt_instance*, bool, bool, bool);   // -- a group of test cases -- //

void free_bt (struct bt_instance*);
#endif /* MAIN_H */
