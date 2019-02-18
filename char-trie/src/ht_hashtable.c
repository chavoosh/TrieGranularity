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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ht_hashtable.h"
#include "Bt_trie.h"

/* ----------------------------------------------------------------
 * Method: ht_lookup (..)
 * Scope: Global
 *
 * Description:
 * Lookup in a hash table of a given node to find corresponded key.
 * ---------------------------------------------------------------- */
struct bucket_t*
ht_lookup (struct Bt_instance* Bt, struct node_t* node, char first_byte, bool print_flag)
{
    assert (Bt);

    char key;  // -- key is the first byte -- //
    int index;

    if (!node->hash_table)
        return 0;
    if (!node->hash_table->size)
    {
        //fprintf (stderr, "[ht_lookup] ERROR: Hash table with size ZERO.\n");
        return 0;
    }
    key = first_byte;   // -- key is the first byte -- //
    index = ((int)key)%node->hash_table->size;
    if (!node->hash_table->buckets) 
        return 0;
    if (node->hash_table->buckets[index].first_byte != key)
        return 0;
    else
        return &node->hash_table->buckets[index]; 
} /* -- end of ht_lookup (..) -- */ 


/* ----------------------------------------------------------------
 * Method: ht_insert (..)
 * Scope: Global
 *
 * Description:
 * Insert a new key into a hash table.
 * ---------------------------------------------------------------- */
struct
bucket_t* ht_insert (struct Bt_instance* Bt, struct node_t* node, char first_byte, bool print_flag)
{
    assert (Bt);

    char key;  // -- key is the first byte -- //
    int index;
    if (!node->hash_table)
    {
        node->hash_table = (struct ht_t*)malloc(sizeof(struct ht_t));
        node->hash_table->size = Bt->ht_init_size;
        node->hash_table->used = 0;
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size);
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].first_byte = 0;
            node->hash_table->buckets[i].next_node = 0;
        }
        // -- insert -- //
        key = first_byte;
        index = ((int)key)%node->hash_table->size;
        node->hash_table->buckets[index].first_byte = key;
        node->hash_table->used++;
        return &node->hash_table->buckets[index];
    }
    if (!node->hash_table->size)
    { 
        //fprintf (stderr, "[ht_insert] WARNING: Hash table with size ZERO.\n");
        node->hash_table->size = Bt->ht_init_size;
        node->hash_table->used = 0;
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size);
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].first_byte = 0;
            node->hash_table->buckets[i].next_node = 0;
        }
        // -- insert -- //
        key = first_byte;
        index = ((int)key)%node->hash_table->size;
        node->hash_table->buckets[index].first_byte = key;
        node->hash_table->used++;
        return &node->hash_table->buckets[index];
    }
    if (!node->hash_table->buckets) 
    {
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size); 
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].first_byte = 0;
            node->hash_table->buckets[i].next_node = 0;
        }
        // -- insert -- //
        key = first_byte;
        index = ((int)key)%node->hash_table->size;
        node->hash_table->buckets[index].first_byte = key;
        node->hash_table->used++;
        return &node->hash_table->buckets[index];
    }
    key = first_byte;   // -- key is the first byte -- //
    index = ((int)key)%node->hash_table->size;

    if (node->hash_table->buckets[index].first_byte != key)
    {
        if (!node->hash_table->buckets[index].first_byte)
        {
            // -- insert it here -- //
            node->hash_table->buckets[index].first_byte = key;
            node->hash_table->used++;
            return &node->hash_table->buckets[index];
        }
        // -- rehash until we can add this key -- //
        while (node->hash_table->buckets[index].first_byte != 0)
        {           
            ht_rehash (Bt, node);
            index = ((int)key)%node->hash_table->size;
        } 
        node->hash_table->buckets[index].first_byte = key;
        node->hash_table->used++;
        return &node->hash_table->buckets[index];
    }
    else
    {
        if (print_flag)
            printf ("Trying to add duplicate key in the hash table.\n");
        return 0;
    }
} /* -- end of ht_insert (..) -- */ 

/* ----------------------------------------------------------------
 * Method: ht_rehash (..)
 * Scope: Global
 *
 * Description:
 * Double the size of hash table and re-arrange the keys.
 * ---------------------------------------------------------------- */
void
ht_rehash (struct Bt_instance* Bt, struct node_t* node)
{
    assert (Bt);
   
    char key;
    int index;
    if (!node->hash_table)
    {
        fprintf (stderr, "[ht_rehash] ERROR: HT is not initialized.\n");
        return;
    }
    struct ht_t* hash_table_tmp = (struct ht_t*)malloc(sizeof(struct ht_t));

    *hash_table_tmp = *node->hash_table;
    free(node->hash_table);
    node->hash_table = (struct ht_t*)malloc(sizeof(struct ht_t));
    node->hash_table->size = hash_table_tmp->size * 2;
    node->hash_table->used = hash_table_tmp->used;
    //printf ("Rehash to size:  %u\n", node->hash_table->size);
    node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size); 
    for (int i=0; i < node->hash_table->size; i++)
    {
        node->hash_table->buckets[i].first_byte = 0;
        node->hash_table->buckets[i].next_node = 0;
    }
    for (int i=0; i < hash_table_tmp->size; i++)
    {
        if (hash_table_tmp->buckets[i].first_byte != 0)
        {
            key = hash_table_tmp->buckets[i].first_byte;
            index = ((int)key) % node->hash_table->size; 
            node->hash_table->buckets[index].first_byte = key;
            node->hash_table->buckets[index].next_node = hash_table_tmp->buckets[i].next_node; 
            // -- free -- //
            hash_table_tmp->buckets[i].first_byte = 0;
        }
    }
    // -- do not touch the next_nodes -- //
    free(hash_table_tmp->buckets);
    hash_table_tmp->size = 0;
    free(hash_table_tmp);
} /* -- end of ht_rehash(..) -- */
