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

#include "Bt_trie.h"
#include "db_debug.h"
#include "ht_hashtable.h"

/* -----------------------------------------------------------------
 * Method: Bt_en_name (..)
 * Scope: Protected
 *
 * Description:
 * Add EON at end of each name.
 * ------------------------------------------------------------------ */
char*
Bt_en_name (const char* name, char** en_name)
{
    assert (name);
    *en_name = realloc(*en_name, strlen(name) + 2);
    memcpy (*en_name, name, strlen(name));
    *(*en_name + strlen(name)) = (char)EON;
    *(*en_name + strlen(name) + 1) = '\0';
    return *en_name; 
} /* -- end of Bt_en_name(..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_insert (..)
 * Scope: Protected
 *
 * Description:
 * This function insert a name into the Byte-level trie. If the corresponded
 * name is found, we do nothing and the leaf will be returned.
 * Otherwise, we inset the name (except the LPM) and finally
 * the leaf will be returned.  
 *
 * RETURN:
 *     0:   ERROR | Duplicate
 *     OTW: DONE
 * ------------------------------------------------------------------ */
struct node_t*
Bt_insert (struct Bt_instance* Bt, const char* name, bool print_flag)
{
    assert (Bt);
    assert (name);

    int byte_walker = 0;           // -- index of the name -- //
    int node_byte_walker = 0;      // -- index of the working node's content -- //
    struct node_t* node;           // -- node traverser -- // 
    struct bucket_t* child = NULL; // -- child traverser -- //
    char first_byte;               // -- change it when the working node changes -- //

    if (strlen(name) < 2)
    {
        // -- a name with length of ONE? -- //
        fprintf (stderr, "[trie_insert] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }

    /* ----------- Welcome to the loop party ----------- */
    node = &(Bt->root);
    if (name[byte_walker] != (char)SLASH)
    {
        fprintf (stderr, "[Bt_insert] WARNING: NDN names should start with SLASH.\n");
        return 0;
    }
    byte_walker = 1;   // -- Assuming all names start with SLASH "/" -- //
    
    // -- Look up the name. If mismatch occured, insert remaining bytes one-by-one -- //
    if (!(child = ht_lookup(Bt, node, name[byte_walker], print_flag)))
    {
        // -- none of the children start with this byte -- //
        return (Bt_do_insert (Bt, node, name, byte_walker, print_flag));
    } 

    while (byte_walker < strlen(name))
    {
        // -- Look up the name. If mismatch occured, insert remaining bytes one-by-one -- //
        if (!(child = ht_lookup(Bt, node, name[byte_walker], print_flag)))
        {
            // -- none of the children match, so INSERT it -- //
            return (Bt_do_insert (Bt, node, name, byte_walker, print_flag));
        } 
        // -- child is set, now set the node -- //
        node = child->next_node;
        first_byte = (char)name[byte_walker];
        if (!node)
        {
            fprintf (stderr, "[Bt_insert] ERROR: A null next_node.\n");
            return 0;
        }
        // -- match the node content -- //
        while (node_byte_walker < node->len && byte_walker < strlen(name))
        {
            if (node->bytes[node_byte_walker] == name[byte_walker])
            {
                byte_walker++;
                node_byte_walker++;
                continue;
            }
            if (!node_byte_walker)
            {
                // -- The previous child specified the first byte and now it does no match -- //
                fprintf (stderr, "[Bt_insert] ERROR: The first byte of the node does match.\n");
                return 0;
            }
            // -- search the children of this node for remaining bytes -- //
            break;
        } // -- end of content match -- //
        if (byte_walker == strlen(name))
        {
            if (node_byte_walker == node->len)
            {
                // -- name is found -- //
                if (print_flag)
                    printf ("Name is found:  %s\n", name);
                return 0;
            }
            else
            {
                // -- the name has been ended at the middle of the current node, so partition the node -- //
                return (Bt_node_partition (Bt, child, name, first_byte, byte_walker, node_byte_walker, print_flag));
            }
        }
        if (node_byte_walker == node->len)
        {
            // -- continue searching among the children of this node -- //
            node_byte_walker = 0;
            continue;
        }
        else
        {
            // -- the name has been ended at the middle of the current node, so partition the node -- //
            return (Bt_node_partition (Bt, child, name, first_byte, byte_walker, node_byte_walker, print_flag));
        }
    } // -- end of while loop -- //
    fprintf (stderr, "[Bt_insert] WARNINIG: Unreachable part is acccessed.\n");
    return 0;  
} /* -- end of Bt_insert(..) -- */


/* -----------------------------------------------------------------
 * Method: Bt_do_insert (..)
 * Scope: Protected
 *
 * Description:
 * Insert the remaining components of a name (after doing LPM) in the
 * Byte-level trie (EON has been already added to the name).
 * ------------------------------------------------------------------ */
struct node_t*
Bt_do_insert (struct Bt_instance* Bt, struct node_t* node, const char* name, int byte_walker, bool print_flag)
{
    assert (Bt);
    struct bucket_t* child;
    // -- check whether we should be here or not -- //
    if ((child=ht_lookup(Bt, node, name[byte_walker], print_flag)))
    {
        fprintf (stderr, "[Bt_do_insert] ERROR: The child is occupied.\n");
        return 0;
    }
    
    // -- create a new record in the ht and use the corresponded node to insert the remaining bytes -- // 
    if (!(child=ht_insert(Bt, node, name[byte_walker], print_flag)))
    {
        fprintf (stderr, "[Bt_do_insert] ERROR: HT insertion has been failed.\n");
        return 0;
    }
    // -- use this child to insert the bytes -- //
    child->next_node = (struct node_t*)malloc(sizeof(struct node_t));
    child->next_node->len = strlen(name)-byte_walker;
    child->next_node->bytes = (char*)malloc(child->next_node->len + 1);
    child->next_node->parent = node;
    // -- do not initialize the ht, until we need it -- // 
    child->next_node->hash_table = 0;
 
    // -- copy the remaining bytes -- //
    memcpy(child->next_node->bytes, name + byte_walker, child->next_node->len);  
    child->next_node->bytes[child->next_node->len] = '\0';
 
    if (print_flag)
    {
        printf ("Inserted node:  ");
        db_print_node (child->next_node);    
    }
    return child->next_node;
} /* -- end of Bt_do_insert (..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_lookup (..)
 * Scope: Protected
 *
 * Description:
 * Lookup a given name.
 * ------------------------------------------------------------------ */
struct node_t* Bt_lookup (struct Bt_instance* Bt, const char* name, bool print_flag, bool exact_match, struct node_t** visitedNodes)
{
    assert (Bt);
    assert (name);

    int byte_walker = 0;           // -- index of the name -- //
    int node_byte_walker = 0;      // -- index of the working node's content -- //
    struct node_t* node;           // -- node traverser -- // 
    struct bucket_t* child = 0;    // -- child traverser -- //
    int visited_walker = 0;        // -- index of visitedNodes -- //

    if (strlen(name) < 2)
    {
        // -- a name with length of ONE? -- //
        fprintf (stderr, "[Bt_lookup] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }

    /* ----------- Welcome to the loop party ----------- */
    node = &(Bt->root);
    if (name[byte_walker] != (char)SLASH)
    {
        fprintf (stderr, "[Bt_lookup] WARNING: NDN names should start with SLASH.\n");
        return 0;
    }
    byte_walker = 1;   // -- Assuming all names start with SLASH "/" -- //
    
    // -- Look up the name. If mismatch occured, lookup fails -- //
    if (!(child = ht_lookup(Bt, node, name[byte_walker], print_flag)))
    {
        // -- none of the children start with this byte -- //
        return 0;
    } 

    while (byte_walker < strlen(name))
    {
        // -- Look up the name. If mismatch occured, insert remaining bytes one-by-one -- //
        if (!(child = ht_lookup(Bt, node, name[byte_walker], print_flag)))
        {
            // -- none of the children match, so INSERT it -- //
            return 0;
        } 
        // -- child is set, now set the node -- //
        node = child->next_node;
        if (!node)
        {
            fprintf (stderr, "[Bt_lookup] ERROR: A null next_node.\n");
            return 0;
        }
        if (exact_match)
        {
            // -- this array does not store root as a visited node -- //
            visitedNodes[visited_walker] = node;
            visited_walker++;
        }
        // -- match the node content -- //
        while (node_byte_walker < node->len && byte_walker < strlen(name))
        {
            if (node->bytes[node_byte_walker] == name[byte_walker])
            {
                byte_walker++;
                node_byte_walker++;
                continue;
            }
            if (!node_byte_walker)
            {
                // -- The previous child specified the first byte and now it does no match -- //
                fprintf (stderr, "[Bt_lookup] ERROR: The first byte of the node did not match.\n");
                return 0;
            }
            // -- search the children of this node for remaining bytes -- //
            break;
        } // -- end of content match -- //
        if (byte_walker == strlen(name))
        {
            if (node_byte_walker == node->len)
            {
                // -- name is found -- //
                return node;
            }
            else
            {
                // -- the name has been ended at the middle of the current node, lookup failed -- //
                return 0;
            }
        }
        if (node_byte_walker == node->len)
        {
            // -- continue searching among the children of this node -- //
            node_byte_walker = 0;
            continue;
        }
        else
        {
            // -- the name has been ended at the middle of the current node, so partition the node -- //
            return 0;
        }
    } // -- end of while loop -- //
    fprintf (stderr, "[Bt_lookup] WARNINIG: Unreachable part is acccessed.\n");
    return 0;  
} /* -- end of Bt_lookup (..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_node_partition (..)
 * Scope: Protected
 *
 * Description:
 * Partition a name from a given point, so that its bytes will 
 * splitted into two parts (i.e. parent and first_child). Then the 
 * second part of splited node (i.e. first_child) and the remaining 
 * bytes of the name should be added to the parent, as children.
 * ------------------------------------------------------------------ */
struct node_t*
Bt_node_partition (struct Bt_instance* Bt, struct bucket_t* child, const char* name, char first_byte, int byte_walker, int node_byte_walker, bool print_flag)
{
    assert (Bt);
    struct node_t* parent;
    struct node_t* first_node;
    struct node_t* second_node;
    struct bucket_t* in_ret;    // -- stores the returned value by insertion -- //
    if (!node_byte_walker)
    {
        fprintf (stderr, "[Bt_node_partition] ERROR: node_byte_walker == 0.\n");
        return 0;
    }

    // -- partition given node from the last matched byte (i.e. byte_walker--) -- //
    /**
     * Node partitioning:
     *    1- find the last matched byte 
     *    2- we keep the previous node (with all children)
     *    3- a new node is created instead of the previous node
     *    4- this new node has two children:
     *        a) remaining bytes of the previous node 
     *        b) the rest of input name
     */ 
    // -- the current child should point to a new node, so store the "next_node" pointer of the current child -- //
    struct node_t* next_node_tmp = (struct node_t*)malloc(sizeof(struct node_t));
    *next_node_tmp = *(child->next_node);
    free(child->next_node);
    // -- partition the corresponded node into parent and first_node -- //
    child->next_node = (struct node_t*)malloc(sizeof(struct node_t));

    parent = child->next_node;  // -- agent is set -- //
    parent->parent = next_node_tmp->parent;
    parent->hash_table = 0;     // -- hash table will be initialized by ht_insert -- //
    parent->len = node_byte_walker;
    parent->bytes = (char*)malloc(parent->len + 1);
    memcpy (parent->bytes, next_node_tmp->bytes, parent->len);
    parent->bytes[parent->len] = '\0';
    // -- parent has received its content, now we add its two children -- //

    if (next_node_tmp->bytes[node_byte_walker] == name[byte_walker])
    {
        // -- so why are we here!!? -- //
        fprintf (stderr, "[Bt_node_merge] ERROR: Node merging has been failed.\n");
        return 0;
    }

    // -- first node -- //
    if (!(in_ret=ht_insert(Bt, parent, next_node_tmp->bytes[node_byte_walker], print_flag)))
    {
        fprintf (stderr, "[Bt_do_insert] ERROR: HT insertion has been failed.\n");
        return 0;
    }
    // -- use the return child to insert the bytes -- //
    in_ret->next_node = (struct node_t*)malloc(sizeof(struct node_t));
    first_node = in_ret->next_node;   // -- agent is set -- //
    first_node->parent = parent;
    first_node->len = next_node_tmp->len - node_byte_walker;
    first_node->bytes = (char*)malloc(first_node->len + 1);
    memcpy(first_node->bytes, next_node_tmp->bytes + node_byte_walker, first_node->len);
    first_node->bytes[first_node->len] = '\0';
    first_node->hash_table = next_node_tmp->hash_table;
    // -- first node is DONE -- //

    // -- second node -- //
    if (!(in_ret=ht_insert(Bt, parent, name[byte_walker], print_flag)))
    {
        fprintf (stderr, "[Bt_do_insert] ERROR: HT insertion has been failed.\n");
        return 0;
    }
    // -- use the return child to insert the bytes -- //
    in_ret->next_node = (struct node_t*)malloc(sizeof(struct node_t));
    second_node = in_ret->next_node;   // -- agent is set -- //
    second_node->parent = parent;
    second_node->len = strlen(name) - byte_walker;
    second_node->bytes = (char*)malloc(second_node->len + 1);
    memcpy(second_node->bytes, name + byte_walker, second_node->len);
    second_node->bytes[second_node->len] = '\0';
    second_node->hash_table = 0;
    // -- second node is DONE -- // 

    free(next_node_tmp->bytes);
    next_node_tmp->len = 0;
    next_node_tmp->parent = 0;
    next_node_tmp->hash_table = 0;  // -- do not touch the children -- //
    free(next_node_tmp);

    if (print_flag)
    {
        printf ("Inserted node: ");
        db_print_node (second_node);
    }
    // -- second child is done -- //
    // -- Partiotioning is DONE! END of insertion -- //
    return second_node; 
} /* -- end of trie_node_partition(..) -- */

/* -----------------------------------------------------------------
 * Method: trie_remove (..)
 * Scope: Protected
 *
 * Description:
 * Remove a given name from the trie. To remove a name, we have to make
 * sure that the whole name exists. After finding the name, JUST the last
 * node will be removed. After removing the last node, maybe merging the 
 * parent and remaining node is necessary.
 *
 * It is important to mention that there cannot be any node with one 
 * child, except the root. So, we have to merge some nodes if it is 
 * necessary.
 *
 * Return:
 *     1: name is not found (Not removed)
 *     2: ERROR (Not remove)
 *     0: DONE! (removed)
 *
 * NOTE:
 *     Removing first child -> should be done from the node.
 *     Removing other children -> should be done by the previous child.
 * ------------------------------------------------------------------ */
int
Bt_remove (struct Bt_instance* Bt, const char* name, bool print_flag)
{
    assert (Bt);
    assert (name);
   
    int visited_walker = 0;  // -- index of visitedNodes array -- //
    struct node_t* node;     // -- working node -- //
    char first_byte;
    struct bucket_t* lo_ret; // -- return value of ht_lookup -- //
 
    for (int i=0; i<MAX_HEIGHT; i++)
        Bt->visitedNodes[i] = 0;

    // -- start exact name lookup -- //
    if (!Bt_lookup (Bt, name, 0, 1, Bt->visitedNodes))
    {
        // -- name is NOT found -- //
        return 1; 
    }
    if (!Bt->visitedNodes[0]) 
    {
        fprintf (stderr, "[Bt_remove] ERROR: No node is visited while exact matching.\n");
        return 2;
    }
    // -- number of visited nodes -- // 
    while (Bt->visitedNodes[visited_walker])
    {
        visited_walker++;
    }
    if (print_flag)
        printf ("Number of visited nodes:  %u\n", visited_walker);
 
    // -- jumpt to the last visited child  -- //
    visited_walker-- ;
    if (visited_walker < 0)
    {
        // -- something is wrong -- //
        fprintf (stderr, "[Bt_remove] ERROR: An error has been occured while removing.\n");
        return 2;
    }

    if (Bt->visitedNodes[visited_walker]->hash_table != 0) 
    {
        // -- exact lookup ended up with a non-leaf node -- //
        fprintf (stderr, "[Bt_remove] ERROR: Exact lookup has been ended up with a non-leaf node.\n");
        return 2;
    }
   

    if (visited_walker == 0)
    {       
        // -- there is just one node (regardless of the root) to remove -- //
        node = &Bt->root;
        if (node->hash_table->used == 1)
        {
            // -- the last visited node is the last child of its parent, so just remove it -- //
            first_byte = Bt->visitedNodes[visited_walker]->bytes[0];
            Bt_free_node (Bt->visitedNodes[visited_walker]);
            if (!(lo_ret=ht_lookup(Bt, &Bt->root, first_byte, print_flag)))
            {
                fprintf (stderr, "[Bt_remove] ERROR: An error has been occured while name removal.\n");
                return 1;
            }    
            free(lo_ret->next_node);
            lo_ret->next_node = 0;
            lo_ret->first_byte = 0;
            free(node->hash_table->buckets);
            node->hash_table->size = 0;
            node->hash_table->used = 0;
            free(node->hash_table);
            node->hash_table=0;
        } 

        else  // -- for root we do not merge anything -- //
        {
            // -- remove the corresponded node and bucket -- //
            first_byte = Bt->visitedNodes[visited_walker]->bytes[0];
            Bt_free_node (Bt->visitedNodes[visited_walker]);
            if (!(lo_ret=ht_lookup(Bt, &Bt->root, first_byte, print_flag)))
            {
                fprintf (stderr, "[Bt_remove] ERROR: An error has been occured while name removal.\n");
                return 1;
            }    
            free(lo_ret->next_node);
            lo_ret->next_node = 0;
            lo_ret->first_byte = 0;
            node->hash_table->used--;
        }
        return 0;
    }

    // -- the last node is not a root's leaf -- //
    if (visited_walker > 0)
    {
        node = Bt->visitedNodes[visited_walker-1];
        if (node->hash_table->used == 1)
        {
            // -- an intermediate node with just one node is not normal -- //
            fprintf (stderr, "[Bt_remove] WARNING: An intermediate node with one child.\n"); 
            return 1;
        }
        if (node->hash_table->used == 2)
        { 
            first_byte = Bt->visitedNodes[visited_walker]->bytes[0];
            Bt_free_node (Bt->visitedNodes[visited_walker]);
            if (!(lo_ret=ht_lookup(Bt, Bt->visitedNodes[visited_walker-1], first_byte, print_flag)))
            {
                fprintf (stderr, "[Bt_remove] ERROR: An error has been occured while name removal.\n");
                return 1;
            }    
            free(lo_ret->next_node);
            lo_ret->next_node = 0;
            lo_ret->first_byte = 0;
            node->hash_table->used--;

            // -- now merge -- //
            Bt_node_merge (Bt, node);
            return 0;
        }
        else
        {
            first_byte = Bt->visitedNodes[visited_walker]->bytes[0];
            Bt_free_node (Bt->visitedNodes[visited_walker]);
            if (!(lo_ret=ht_lookup(Bt, Bt->visitedNodes[visited_walker-1], first_byte, print_flag)))
            {
                fprintf (stderr, "[Bt_remove] ERROR: An error has been occured while name removal.\n");
                return 1;
            }    
            free(lo_ret->next_node);
            lo_ret->next_node = 0;
            lo_ret->first_byte = 0;
            node->hash_table->used--;
        }
    }
    // -- unreachable part -- // 
    return 0;
} /* -- end of Bt_remove(..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_node_merge (..)
 * Scope: Protected
 *
 * Description:
 * Merge a node with one of its leaves.
 * ------------------------------------------------------------------ */
struct node_t*
Bt_node_merge (struct Bt_instance* Bt, struct node_t* parent)
{
    assert (Bt);
    if (!parent || !parent->hash_table || !parent->hash_table->size || !parent->hash_table->used)
    {
        fprintf (stderr, "[Bt_node_merge] ERROR: An error has been occured while merging.\n");
        return 0;
    }

    int used = 0;
    struct node_t* node_tmp = (struct node_t*)malloc(sizeof(struct node_t)); 
    char* parent_byte;
    int parent_len;
    // -- check whether this node is elgible for merging, then copy the node to remove -- //
    for (int i=0; i<parent->hash_table->size; i++)
    {
        if (parent->hash_table->buckets[i].first_byte != 0)
        {
            used++; 
            // -- copy the node to remove -- // 
            *node_tmp = *parent->hash_table->buckets[i].next_node;
            free(parent->hash_table->buckets[i].next_node);
        }
    }
    if (used > 1)
    {
        // -- this node has more than one child -- //
        fprintf (stderr, "[Bt_node_merge] ERROR: Trying to merge a node with more than one child.\n");
        return 0;
    }
    free(parent->hash_table->buckets);
    parent->hash_table->buckets = 0;
    parent->hash_table->size = 0;
    parent->hash_table->used = 0;
    free(parent->hash_table);
    parent->hash_table = 0;

    // -- copy the parent -- //
    parent_byte = (char*)malloc(parent->len);
    memcpy (parent_byte, parent->bytes, parent->len);
    parent_len = parent->len;
    // -- realloc parent bytes -- //
    parent->len = parent->len + node_tmp->len;
    parent->bytes = realloc(parent->bytes, parent->len + 1);
    memcpy(parent->bytes, parent_byte, parent_len);
    memcpy(parent->bytes + parent_len, node_tmp->bytes, node_tmp->len);
    parent->bytes[parent->len] = '\0';
    // -- parent received its content -- //
    parent->hash_table = node_tmp->hash_table;   
    // -- change parent of children of node_tmp -- //
    if (node_tmp->hash_table != 0 && 
        node_tmp->hash_table->size != 0 && 
        node_tmp->hash_table->used != 0)
    {
        for (int i=0; i < node_tmp->hash_table->size; i++)
        {
            if (node_tmp->hash_table->buckets[i].first_byte)
                node_tmp->hash_table->buckets[i].next_node->parent = parent;
        }   
    }
    // -- free -- //
    free(node_tmp->bytes);
    node_tmp->bytes = 0;
    node_tmp->len = 0;
    node_tmp->parent = 0;
    free(node_tmp);
    free(parent_byte);
    // -- do not touch parent's children -- //
    return parent;
} /* -- end of Bt_node_merge(..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_free_node (..)
 * Scope: Protected
 *
 * Description:
 * This function deallocates a given node and its subtrees.
 * ------------------------------------------------------------------ */
void
Bt_free_node (struct node_t* node)
{
    assert (node);
    if (!node->hash_table)
    {
        Bt_do_free_node (node);
        return;
    }
    for (int i=0; i<node->hash_table->size; i++)
    {
        if (!node->hash_table->buckets[i].first_byte)
            continue;
        Bt_free_node(node->hash_table->buckets[i].next_node);
        node->hash_table->buckets[i].first_byte = 0;
        free(node->hash_table->buckets[i].next_node);
        node->hash_table->buckets[i].next_node = 0;
    }
    free(node->hash_table->buckets);
    node->hash_table->size=0;
    node->hash_table->used=0;
    Bt_do_free_node (node);
    return;    
} /* -- end of trie_free_node (..) -- */

/* -----------------------------------------------------------------
 * Method: Bt_do_free_node (..)
 * Scope: Protected
 *
 * Description:
 * Free all memories allocated to a node and its elements.
 * ------------------------------------------------------------------ */
void
Bt_do_free_node (struct node_t* node)
{
    if (!node)
    {
        fprintf (stderr, "[Bt_do_free_node] ERROR: A null node to free.\n");
        return;
    }
    free(node->hash_table); 
    node->hash_table = 0;
    free(node->bytes);
    node->bytes = 0;
    node->len = 0;
    node->parent = 0;
    return;
} /* -- end of Bt_do_free_node (..) -- */
