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
#include <string.h>

#include "ht_hashtable.h"
#include "ct_trie.h"
#include "xxhash.h"


/* ---------------------------------------------------------------------
 * Method: ht_lookup (..)
 * Scope: Global
 * 
 * Description:
 * Look up a given name, based on the first component of the nodes.
 * --------------------------------------------------------------------- */
struct bucket_t*
ht_lookup (struct ct_instance* ct, struct node_t* node, char* first_comp, bool print_flag)
{
    assert (ct);

    unsigned long long key = ht_keygen (first_comp);
    unsigned int index;
    struct bucket_t* bucket_walker; 
    bool equal;

    if (!key)
    {
        fprintf (stderr, "[ht_lookup] ERROR: Key generating has been failed.\n");
        return 0;
    }
    if (!node->hash_table || !node->hash_table->size)
        return 0;
    if (!node->hash_table->buckets)
    {
        fprintf (stderr, "[ht_lookup] WARNING: An initialized ht without bucket.\n");
        return 0;
    }

    index = key % node->hash_table->size;
    bucket_walker = &node->hash_table->buckets[index];   
    while (bucket_walker->next_bucket)
    {
        bucket_walker = bucket_walker->next_bucket;
        if (bucket_walker->key == key)
        {
            // -- keys are equal, make sure components are NOT equal before insertion -- //
            equal = true;
            if (bucket_walker->next_node->comps[0].len != strlen(first_comp))
                equal = false;
            if (equal)
            { 
                for (int i=0; i < strlen(first_comp); i++)
                {
                    if (bucket_walker->next_node->comps[0].bytes[i] != first_comp[i]) 
                    {
                        equal = false; 
                        break;
                    }
                }
            }
            if (!equal)
            {
                // -- THIS IS WARNING [KEYs are equal BUT contents are different] -- //
                fprintf (stderr, "[ht_lookup] WARNING: HT COLLISION, FIX IT.\n"); 
                return 0;
            }
            else
            {
                // -- FOUND! -- //
                return bucket_walker;
            }
        }
    }

    // --NOT FOUND -- //
    return 0;
} /* -- end of ht_lookup(..) -- */


/* ---------------------------------------------------------------------
 * Method: ht_insert (..)
 * Scope: Global
 * 
 * Description:
 * Insert a new record to the hash table of the corresponded node.
 * NOTE:
 *     Duplicate records will not be added.
 * --------------------------------------------------------------------- */
struct bucket_t*
ht_insert (struct ct_instance* ct, struct node_t* node, char* first_comp, bool print_flag)
{
    assert (ct);
    struct bucket_t* bucket_walker;

    unsigned long long key = ht_keygen (first_comp);
    unsigned int index;
    bool equal;

    if (!key)
    {
        fprintf (stderr, "[ht_insert] ERROR: Key generating has been failed.\n");
        return 0;
    }

    // ** BEGIN - nothing is in the HT ** //
    if (!node->hash_table)
    {
        node->hash_table = (struct ht_t*)malloc(sizeof(struct ht_t));
        node->hash_table->size = ct->ht_init_size;
        node->hash_table->used = 0;
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size);
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].key = 1;  // -- key of head is one -- //
            node->hash_table->buckets[i].next_node = 0;
            node->hash_table->buckets[i].head = true;
            node->hash_table->buckets[i].next_bucket = 0;
            node->hash_table->buckets[i].pre_bucket = 0;
        }
        // -- insert -- //
        index = key % node->hash_table->size;
        node->hash_table->buckets[index].next_bucket = (struct bucket_t*)malloc(sizeof(struct bucket_t));
        node->hash_table->buckets[index].next_bucket->key = key;
        node->hash_table->buckets[index].next_bucket->next_node = 0;
        node->hash_table->buckets[index].next_bucket->next_bucket = 0;
        node->hash_table->buckets[index].next_bucket->pre_bucket = &node->hash_table->buckets[index];
        node->hash_table->buckets[index].next_bucket->head = false;
        node->hash_table->used++;
        return node->hash_table->buckets[index].next_bucket;
    }
    if (!node->hash_table->size)
    { 
        //fprintf (stderr, "[ht_insert] ERROR: Hash table with size ZERO.\n");
        node->hash_table->size = ct->ht_init_size;
        node->hash_table->used = 0;
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size);
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].key = 1;  // -- key of head is one -- //
            node->hash_table->buckets[i].next_node = 0;
            node->hash_table->buckets[i].head = true;
            node->hash_table->buckets[i].next_bucket = 0;
            node->hash_table->buckets[i].pre_bucket = 0;
        }
        // -- insert -- //
        index = key % node->hash_table->size;
        node->hash_table->buckets[index].next_bucket = (struct bucket_t*)malloc(sizeof(struct bucket_t));
        node->hash_table->buckets[index].next_bucket->key = key;
        node->hash_table->buckets[index].next_bucket->next_node = 0;
        node->hash_table->buckets[index].next_bucket->next_bucket = 0;
        node->hash_table->buckets[index].next_bucket->pre_bucket = &node->hash_table->buckets[index];
        node->hash_table->buckets[index].next_bucket->head = false;
        node->hash_table->used++;
        return node->hash_table->buckets[index].next_bucket;
    }
    if (!node->hash_table->buckets) 
    {
        node->hash_table->buckets = (struct bucket_t*)malloc(sizeof(struct bucket_t) * node->hash_table->size); 
        for (int i=0; i<node->hash_table->size; i++)
        {
            node->hash_table->buckets[i].key = 1;  // -- key of head is one -- //
            node->hash_table->buckets[i].next_node = 0;
            node->hash_table->buckets[i].head = true;
            node->hash_table->buckets[i].next_bucket = 0;
            node->hash_table->buckets[i].pre_bucket = 0;
        }
        // -- insert -- //
        index = key % node->hash_table->size;
        node->hash_table->buckets[index].next_bucket = (struct bucket_t*)malloc(sizeof(struct bucket_t));
        node->hash_table->buckets[index].next_bucket->key = key;
        node->hash_table->buckets[index].next_bucket->next_node = 0;
        node->hash_table->buckets[index].next_bucket->next_bucket = 0;
        node->hash_table->buckets[index].next_bucket->pre_bucket = &node->hash_table->buckets[index];
        node->hash_table->buckets[index].next_bucket->head = false;
        node->hash_table->used++;
        return node->hash_table->buckets[index].next_bucket;
    }
    // ** END - HT is empty ** //

    // -- rehash if it is necessary -- //
    if ((double)((double)node->hash_table->used/node->hash_table->size) > 0.5)
    {
        // -- increase the size of corresponded HT by TWO, before inserting the record -- //
        ht_rehash(ct, node, print_flag);
    }
    index = key % node->hash_table->size;
    bucket_walker = &node->hash_table->buckets[index];   
    while (bucket_walker->next_bucket)
    {
        bucket_walker = bucket_walker->next_bucket;
        if (bucket_walker->key == key)
        {
            // -- keys are equal, make sure components are NOT equal before insertion -- //
            equal = true;
            if (bucket_walker->next_node->comps[0].len != strlen(first_comp))
                equal = false;
            if (equal)
            { 
                for (int i=0; i < strlen(first_comp); i++)
                {
                    if (bucket_walker->next_node->comps[0].bytes[i] != first_comp[i]) 
                    {
                        equal = false; 
                        break;
                    }
                }
            }
            if (!equal)
            {
                // -- THIS IS A WARNING [KEYs are equal BUT contents are different] -- //
                fprintf (stderr, "[ht_insert] WARNING: HT COLLISION, FIX IT.\n");
            }
            else
            {
                if (print_flag)
                    printf ("Trying to add duplicate key in the hash table.\n");
                return 0;
            }
        }
    }
    // -- insert it here (at the end of linked list) -- // 
    bucket_walker->next_bucket = (struct bucket_t*)malloc(sizeof(struct bucket_t));
    bucket_walker->next_bucket->key = key;
    bucket_walker->next_bucket->next_node = 0;
    bucket_walker->next_bucket->pre_bucket = bucket_walker;
    bucket_walker->next_bucket->next_bucket = 0;
    bucket_walker->next_bucket->head = false; 
    node->hash_table->used++;
    return bucket_walker->next_bucket;
} /* -- end of ht_insert (..) -- */

/* ---------------------------------------------------------------------
 * Method: ht_rehash (..)
 * Scope: Global
 * 
 * Description:
 * Double the size of the hash table of the corresponded node and rearrange
 * the keys, consequently.
 * --------------------------------------------------------------------- */
void
ht_rehash (struct ct_instance* ct, struct node_t* node, bool print_falg)
{
    assert (ct);
    
    unsigned long long key; 
    unsigned int index;
    struct bucket_t* bucket_walker_org;
    struct bucket_t* bucket_walker_rehash;

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
        node->hash_table->buckets[i].key = 0;
        node->hash_table->buckets[i].next_node = 0;
        node->hash_table->buckets[i].head = true;
        node->hash_table->buckets[i].next_bucket = 0;
        node->hash_table->buckets[i].pre_bucket = 0;
    }

    // -- know rearrange the keys (here we are sure there is no duplicate key) --//
    for (int i=0; i < hash_table_tmp->size; i++)
    { 
        bucket_walker_org = &hash_table_tmp->buckets[i];        
        while (bucket_walker_org->next_bucket)
        {
            bucket_walker_org = bucket_walker_org->next_bucket;
            key = bucket_walker_org->key;
            index = key % node->hash_table->size;  // -- find the new location -- //
            
            bucket_walker_rehash = &node->hash_table->buckets[index];
            // -- find an empty slot for this item -- //
            while (bucket_walker_rehash->next_bucket)
                bucket_walker_rehash = bucket_walker_rehash->next_bucket;
            // -- insert it here -- //
            bucket_walker_rehash->next_bucket = (struct bucket_t*)malloc(sizeof(struct bucket_t));
            bucket_walker_rehash->next_bucket->key = bucket_walker_org->key;
            bucket_walker_rehash->next_bucket->next_node = bucket_walker_org->next_node;
            bucket_walker_rehash->next_bucket->next_bucket = 0; 
            bucket_walker_rehash->next_bucket->pre_bucket = bucket_walker_rehash; 
            bucket_walker_rehash->next_bucket->head = false;
        }
        // -- free -- // 
        // -- we have reached the end of this linked list -- //
        if (bucket_walker_org->head)
            continue;
        while (!bucket_walker_org->head)
        {
            bucket_walker_org->key = 0;
            if (bucket_walker_org->next_bucket)
                free(bucket_walker_org->next_bucket);
            bucket_walker_org->next_bucket = 0;
            bucket_walker_org = bucket_walker_org->pre_bucket;           
        }
        // -- remove the head -- //
        if (bucket_walker_org->next_bucket)
            free(bucket_walker_org->next_bucket);
        bucket_walker_org->next_bucket = 0;
    }
    // -- do not touch the next_nodes -- //
    free(hash_table_tmp->buckets);
    hash_table_tmp->size = 0;
    free(hash_table_tmp);
}

/* ---------------------------------------------------------------------
 * Method: ht_keygen (..)
 * Scope: Global
 * 
 * Description:
 * Generate a key based on xxhash hash function. The input is a string and
 * the output will be the key. This key will be devided by size of the ht
 * (not in this function) to figure out the corresponded bucket.
 * --------------------------------------------------------------------- */
unsigned long long
ht_keygen (char* comp)
{
    
    int len=strlen(comp);
    unsigned long long seed=1234;
    unsigned long long key= XXH64 (comp, len, seed);
    //printf ("key:  %llu\n", key);
    return key;
} /* -- end of ht_keygen(..) -- */
