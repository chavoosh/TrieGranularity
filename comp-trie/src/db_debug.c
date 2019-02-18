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
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "ct_trie.h"
#include "cm_component.h"
#include "db_debug.h"
#include "db_debug_struct.h"


/* -----------------------------------------------------------------------------------
 * Method: db_dfs(..)
 * Scope: private
 * 
 * Description:
 * dfs driver.
 * ----------------------------------------------------------------------------------- */
void
db_dfs (struct ct_instance* ct, bool print_flag)
{
    assert (ct);
    ct->trie_stat->max = 0;
    ct->trie_stat->sum = 0;
    ct->trie_stat->num = 0;
    ct->trie_stat->id = 0;
    ct->trie_stat->chain_length = 0;
    ct->trie_stat->ht_size = 0;

    // -- take the root and start -- //
    if (ct->root.hash_table == 0)
    {
        // -- the trie is empty -- //
        fprintf (stderr, "[db_dfs] WARNING: The trie is empty.\n");
    }

    DIR* dir = opendir(DOT_DIR_PATH);
    if (dir)
        closedir(dir);
    else if (ENOENT == errno)
        mkdir(DOT_DIR_PATH, 0700);
    else
    {
        printf ("failed to open dot dir\n");
        return;
    }

    // -- check the files -- //
    if (access( DOT_FILE_PATH, F_OK ) != -1)
        remove (DOT_FILE_PATH);
    if (access( PDF_FILE_PATH, F_OK ) != -1)
        remove (PDF_FILE_PATH);

    FILE* dot = fopen (DOT_FILE_PATH, "a");
    fprintf (dot, "digraph {\n");
    fclose(dot);

    if (print_flag)
        printf ("----------- DFS ----------\n");
    db_do_dfs (ct, &(ct->root), 0, 0, ct->trie_stat, -1, print_flag);

    dot = fopen(DOT_FILE_PATH, "a");
    fprintf (dot, "}");
    fclose (dot);

    if (print_flag)
    {
        // -- some statistical info -- //
        /*
        printf (" ----------- Trie stat ---------------\n");
        printf ("\tMAX Hieght=   %u\n", trie_stat->max);
        printf ("\tNUM Leaves=   %u\n", trie_stat->num);
        printf ("\tSUM Height=   %lld\n", trie_stat->sum);
        if (trie_stat->num != 0)
            printf ("\tAVE Height=   %f\n", (float)trie_stat->sum/(float)trie_stat->num);
        */
    }
    return;

} /* -- end of db_dfs(..) -- */

/* -----------------------------------------------------------------------------------
 * Method: db_do_dfs(..)
 * Scope: private
 * 
 * Description:
 * Traverse the trie in DFS manner.
 * ----------------------------------------------------------------------------------- */
int
db_do_dfs (struct ct_instance* ct, struct node_t* node, struct node_t* parent, int height, struct t_stat* trie_stat, signed int p_id, bool print_flag)
{
    assert (ct);
    assert (trie_stat);
    int id;
    struct bucket_t* bucket_walker;

    // -- claim your own id -- //
    trie_stat->id++;
    id = trie_stat->id;
    // -- open dot file -- //
    FILE* dot = fopen (DOT_FILE_PATH, "a"); 

    // -- if there is ONLY root in the trie -- //
    if (p_id == -1)
    {
        if (!node->hash_table || !node->hash_table->size)
        {
           fprintf (dot, "\t{\"<%u><%s>\" [label=\"<%s>\"]};", p_id, node->comps[0].bytes, node->comps[0].bytes);
           fclose (dot);
           return 0;
        } 
    }
    else
    {
        db_print_node_to_file (node, parent, trie_stat->id, p_id);
    }
    fclose(dot);

    if (height > MAX_HEIGHT)
    {
        fprintf (stderr, "[db_do_dfd]: MAX_HEIGH is reached.\n");
        exit(0);
    }
    trie_stat->width[height] = trie_stat->width[height] + 1;

    if (!node->hash_table)
    {
        // -- this is a leaf -- //
        trie_stat->max = (trie_stat->max < height) ? height : trie_stat->max;
        trie_stat->num += 1;
        trie_stat->sum += height;
        return height;
    }
    else
    {
        trie_stat->ht_size += node->hash_table->size;
        // --here we calculate the avg length of chains (buckets with head=0 are not counted since they have chain length 0) -- //
        int counter1 = 0; //-- number of all buckets --//
        int counter2 = 0; //--number of chains --//
        for (int i=0; i<node->hash_table->size; i++)
        {
            bucket_walker = &node->hash_table->buckets[i];
            if (bucket_walker->next_bucket !=0)
            {
                counter2 += 1;    
                while (bucket_walker->next_bucket)
                {
                    counter1 += 1;
                    bucket_walker = bucket_walker->next_bucket;
                }
            }   
        }
        trie_stat->chain_length += (float)((float)counter1) / counter2;
    }
    for (int i=0; i<node->hash_table->size; i++)
    {
        bucket_walker=&node->hash_table->buckets[i];
        while (bucket_walker->next_bucket) 
        {
            bucket_walker = bucket_walker->next_bucket;
            db_do_dfs (ct, bucket_walker->next_node, node, height + 1, trie_stat, id, print_flag); 
            if (print_flag)
            {
                printf ("H:%u   ",height);
                db_print_node (bucket_walker->next_node);
            }
        }
    }
    // -- this is not a leaf -- //
    return 0;
} /* -- end of db_do_dfs(..) -- */


/* -----------------------------------------------------------------------------------
 * Method: db_print_node (..)
 * Scope: private
 * 
 * Description:
 * Print component(s) of node.
 * ----------------------------------------------------------------------------------- */
void
db_print_node (struct node_t* node)
{
    for (int i=0; i<node->num_of_comp; i++)
    {
        if (isprint(node->comps[i].bytes[0]))
            printf ("<%s>", node->comps[i].bytes);
        else
        {
            if (node->comps[i].len > 1)
            {
                fprintf (stderr, "[db_print_node] WARNING: Too long EON.\n");
                return;
            }
            printf ("<%u>", node->comps[i].bytes[0]);
        }
    }
    printf ("\n");
    return;
} /* -- end of db_print_node (..) -- */


/* -----------------------------------------------------------------------------------
 * Method: db_print_node_to_file (..)
 * Scope: private
 * 
 * Description:
 * Print component(s) of node to dot file.
 * ----------------------------------------------------------------------------------- */
void
db_print_node_to_file (struct node_t* node, struct node_t* parent, signed int id, signed int p_id)
{
    FILE* dot = fopen(DOT_FILE_PATH, "a");

    // -- add parent -- //
    fprintf (dot,"\t{\"<%u>", p_id);
    for (int i=0; i<parent->num_of_comp; i++)
    {
        if (isprint(parent->comps[i].bytes[0]))
            fprintf (dot, "<%s>", parent->comps[i].bytes);
        else
        {
            if (parent->comps[i].len > 1)
            {
                fprintf (stderr, "[db_print_node] WARNING: Too long EON.\n");
                fclose(dot);
                return;
            }
            fprintf (dot, "<%u>", parent->comps[i].bytes[0]);
        }
    }
    fprintf (dot, "\" ");
    fprintf (dot, "[label=\"");
    for (int i=0; i<parent->num_of_comp; i++)
    {
        if (isprint(parent->comps[i].bytes[0]))
            fprintf (dot, "<%s>", parent->comps[i].bytes);
        else
        {
            if (parent->comps[i].len > 1)
            {
                fprintf (stderr, "[db_print_node] WARNING: Too long EON.\n");
                fclose(dot);
                return;
            }
            fprintf (dot, "<%u>", parent->comps[i].bytes[0]);
        }
    }
    fprintf (dot, "\"]}");

    // -- add node -- //
    fprintf (dot, " -> {\"<%u>",id);
    for (int i=0; i<node->num_of_comp; i++)
    {
        if (isprint(node->comps[i].bytes[0]))
            fprintf (dot, "<%s>", node->comps[i].bytes);
        else
        {
            if (node->comps[i].len > 1)
            {
                fprintf (stderr, "[db_print_node] WARNING: Too long EON.\n");
                fclose(dot);
                return;
            }
            fprintf (dot, "<%u>", node->comps[i].bytes[0]);
        }
    }
    fprintf (dot, "\" ");
    fprintf (dot, "[label=\"");
    for (int i=0; i<node->num_of_comp; i++)
    {
        if (isprint(node->comps[i].bytes[0]))
            fprintf (dot, "<%s>", node->comps[i].bytes);
        else
        {
            if (node->comps[i].len > 1)
            {
                fprintf (stderr, "[db_print_node] WARNING: Too long EON.\n");
                fclose(dot);
                return;
            }
            fprintf (dot, "<%u>", node->comps[i].bytes[0]);
        }
    }
    fprintf (dot, "\"]};\n");
    fclose (dot);
    return;
} /* -- end of db_print_node_to_file (..) -- */
