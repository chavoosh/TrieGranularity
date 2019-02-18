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

#include "bt_trie.h"
#include "db_debug.h"


/* -----------------------------------------------------------------------------------
 * Method: db_dfs(..)
 * Scope: private
 * 
 * Description:
 * dfs driver.
 * ----------------------------------------------------------------------------------- */
void
db_dfs (struct bt_instance* bt, bool print_flag)
{
    assert (bt);
    bt->trie_stat->max = 0;
    bt->trie_stat->sum = 0;
    bt->trie_stat->num = 0;
    bt->trie_stat->id = 0;

    // -- take the root and start -- //
    if (!bt->root.child_0 && !bt->root.child_1)
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
    db_do_dfs (bt, &(bt->root), 0, 0, bt->trie_stat, -1, 0 /*useless here*/, print_flag);

    dot = fopen(DOT_FILE_PATH, "a");
    fprintf (dot, "}");
    fclose (dot);

    if (print_flag)
    {
        // -- some statistical info -- //
        /*
        printf (" ----------- Trie stat ---------------\n");
        printf ("\tMAX Hieght=   %u\n", bt->trie_stat->max);
        printf ("\tNUM Leaves=   %u\n", bt->trie_stat->num);
        printf ("\tSUM Height=   %lld\n", bt->trie_stat->sum);
        if (bt->trie_stat->num != 0)
            printf ("\tAVE Height=   %f\n", (float)bt->trie_stat->sum/(float)bt->trie_stat->num);
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
db_do_dfs (struct bt_instance* bt, struct node_t* node, struct node_t* parent, int height, struct t_stat* trie_stat, signed int p_id, int child, bool print_flag)
{
    assert (bt);
    assert (trie_stat);
    int id;
    struct node_t* node_walker;     // -- to traverse children of a node -- // 

    // -- claim your own id -- //
    trie_stat->id++;
    id = trie_stat->id;
    // -- open dot file -- //
    FILE* dot = fopen (DOT_FILE_PATH, "a"); 

    // -- if there is ONLY root in the trie -- //
    if (p_id == -1)
    {
        if (!node->child_0 && !node->child_1)
        {
           fprintf (dot, "\t{\"<%u><%02x>\" [label=\"<%02x>\"]};", p_id, node->bytes[0], node->bytes[0]);
           fclose (dot);
           return 0;
        } 
    }
    else
    {
        db_print_node_to_file (node, parent, trie_stat->id, child, p_id);
    }
    fclose(dot);
    if (height > MAX_HEIGHT)
    {
        fprintf (stderr, "[db_do_dfd]: MAX_HEIGH is reached.\n");
        exit(0);
    }
    trie_stat->width[height] = trie_stat->width[height] + 1;
    if (node->EON_flag)
        trie_stat->num += 1;   // -- a node with EON_flag ON is a leaf -- //
    if (!node->child_0 && !node->child_1)
    {
        // -- this is a leaf -- //
        trie_stat->max = (trie_stat->max < height) ? height : trie_stat->max;
        trie_stat->sum += height;
        return height;
    }

    // -- travese the children -- //
    for (int i=0; i<2; i++)
    {
        if (i==0)
            node_walker = node->child_0;
        if (i==1)
            node_walker = node->child_1;

        if (!node_walker)
            continue;
        if (!(node_walker->bytes))
        {
            fprintf (stderr, "[db_do_dfs] WARNING: A null active node.\n");
            return 0;
        } 
        db_do_dfs (bt, node_walker, node_walker->parent, height + 1, trie_stat, id, i, print_flag); 
        if (print_flag)
        {
            printf ("H:%u   ",height);
        }
    }
    // -- this is not a leaf -- //
    return 0;
} /* -- end of db_do_dfs(..) -- */


/* -----------------------------------------------------------------------------------
 * Method: db_print_node_to_file (..)
 * Scope: private
 * 
 * Description:
 * Print byte(s) of node to dot file.
 * ----------------------------------------------------------------------------------- */
void
db_print_node_to_file (struct node_t* node, struct node_t* parent, signed int id, int child, signed int p_id)
{
    FILE* dot = fopen(DOT_FILE_PATH, "a");
    int bit_walker;

    // -- add parent -- //
    fprintf (dot,"\t{\"<%u>", p_id);
    bit_walker=0;
    while (bit_walker < parent->len)
    {
        fprintf (dot, "<%02x>", parent->bytes[CURRENT_BYTE(bit_walker)]);
        bit_walker+=BYTE_LEN;
    }
    fprintf (dot, "\" ");
    fprintf (dot, "[label=\"");
    bit_walker=0;
    while (bit_walker < parent->len)
    {
        fprintf (dot, "<%02x>", parent->bytes[CURRENT_BYTE(bit_walker)]);
        bit_walker+=BYTE_LEN;
    }
    fprintf (dot, ":[%u]\"]", parent->len); 
    if (parent->EON_flag)
        fprintf (dot, " [color=lightblue, style=filled]");
    fprintf (dot, "}");

    // -- add node -- //
    fprintf (dot, " -> {\"<%u>",id);
    bit_walker=0;
    while (bit_walker < node->len)
    {
        fprintf (dot, "<%02x>", node->bytes[CURRENT_BYTE(bit_walker)]);
        bit_walker+=BYTE_LEN;
    }
    fprintf (dot, "\" ");
    fprintf (dot, "[label=\"");
    bit_walker=0;
    while (bit_walker < node->len)
    {
        fprintf (dot, "<%02x>", node->bytes[CURRENT_BYTE(bit_walker)]);
        bit_walker+=BYTE_LEN;
    }
    fprintf (dot, ":[%u]\"] \n", node->len);
    if (node->EON_flag)
        fprintf (dot, " [color=lightblue, style=filled]");
    fprintf (dot, "}");

    // -- label the link -- // 
    fprintf (dot, "[label=\"%u\"];\n", child);
    fclose (dot);
    return;
} /* -- end of db_print_node_to_file (..) -- */


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
    int bit_walker = 0;
    int node_len = node->len;

    while (bit_walker < node_len)
    {
        printf ("<%02x>", node->bytes[CURRENT_BYTE(bit_walker)]); 
        bit_walker += (node_len - bit_walker < BYTE_LEN) ? node_len-bit_walker : BYTE_LEN;
    }
    printf (":[%u]\n", node_len);
    return;
} /* -- end of db_print_node (..) -- */

