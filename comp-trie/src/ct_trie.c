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

#include "ct_trie.h"
#include "cm_component.h"
#include "db_debug.h"
#include "ht_hashtable.h"

/* -----------------------------------------------------------------
 * Method: trie_insert (..)
 * Scope: Protected
 *
 * Description:
 * This function insert a name into the trie. If the corresponded
 * name is found, we do nothing and the leaf will be returned.
 * Otherwise, we inset the name (except the LPM) and finally
 * the leaf will be returned.  
 * ------------------------------------------------------------------ */
struct node_t*
trie_insert (struct ct_instance* ct, const char* name, bool print_flag)
{
    assert (ct);
    assert (name);

    int byte_walker = 0;       // -- index of the name -- //
    int comp_walker = 0;       // -- index of the current component (i.e. node) -- //
    int node_comp_walker = 0;  // -- index of node's component -- //
    bool in_node = false;      // -- iterate through node's components, or search children? -- //
    char* c_component;         // -- the current component -- //
    struct node_t* node;       // -- node traverser -- // 
    struct bucket_t* child;    // -- return value of ht_lookup -- //
    int all_comp = 0;          // -- number of extracted components from the input name -- //
    bool go_next = false;      // -- if a node was match, but more components need to be matched -- //

    while (ct->comps_array[comp_walker])
    {
        free(ct->comps_array[comp_walker]);
        ct->comps_array[comp_walker] = 0;
        comp_walker++;
    }
    comp_walker = 0;
    if (strlen(name) < 2)
    {
        // -- a name with length of ONE? -- //
        fprintf (stderr, "[trie_insert] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }

    // -- extract all name components -- //
    if (cm_extract_comps (name, ct->comps_array, print_flag))
    {
        fprintf (stderr, "[trie_insert] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }
    if (!ct->comps_array[0])
    {
        // -- no component -- //
        fprintf (stderr, "[trie_insert] WARNING: Bad input name:  %s\n", name);
        return 0;
    }

    // -- figure out the number of components -- //
    while (ct->comps_array[all_comp])
    {
        all_comp++; 
    }
    if (all_comp == MAX_NUM_OF_COMPS)
    {
        fprintf (stderr, "[trie_insert] WARNING: MAX_NUM_OF_COMPS is reached.\n");
        return 0;
    }
    c_component = ct->comps_array[comp_walker];

    /* ----------- Welcome to loop party ----------- */
    node = &(ct->root);
    // -- Look up extracted components. If mismatch occured, insert them one-by-one -- //
    while (c_component)
    {
        // -- search node's components? -- //
        if (in_node)
        {
            node_comp_walker = 1;  // -- zero is check beforehand -- //            
            if (child->next_node->num_of_comp < 2)
            {
                fprintf (stderr, "[trie_insert] WARNING: in_node flag is wrongly ON.\n");
                return 0;
            }
            if (node_comp_walker >= child->next_node->num_of_comp)   
            {
                // -- something is wrong with the previous iteration -- //
               fprintf (stderr, "[trie_insert] WARNING: node_comp_walker > # of comps.\n");
               return 0;
            }

            while (node_comp_walker < child->next_node->num_of_comp)
            {
                // -- if their lengths are different, this cannot be a match -- //
                if (strlen(c_component) != child->next_node->comps[node_comp_walker].len)
                {
                    // -- get one step back to the last matched component -- //
                    return (trie_node_partition (ct, child, ct->comps_array, all_comp, comp_walker, node_comp_walker, print_flag));
                }

                byte_walker = 0;
                while (byte_walker < strlen(c_component))
                {
                    // -- [TODO] We can use memcmp -- //
                    if (c_component[byte_walker] == child->next_node->comps[node_comp_walker].bytes[byte_walker])
                    {
                        byte_walker++;
                    }
                    else
                    {
                        // -- get one step back to the last matched component -- //
                        return (trie_node_partition (ct, child, ct->comps_array, all_comp, comp_walker, node_comp_walker, print_flag));
                    }
                } // -- this component was a match, go for the next component -- //

                comp_walker++;
                node_comp_walker++;
                if (comp_walker >= all_comp)
                {
                    /**
                     * The input name is found, so this should be
                     * the last component of the current node. 
                     */
                    if (node_comp_walker == child->next_node->num_of_comp)
                    {
                        return child->next_node;
                    }
                    else
                    {
                        fprintf (stderr, "[trie_insert] WARNING: Found name does not hit the end of the node.\n");
                        return 0;
                    } 
                }
                if (comp_walker == MAX_NUM_OF_COMPS)
                {
                    fprintf (stderr, "[trie_insert] WARNING: MAX_NUM_OF_COMPS is reached.\n");
                    return 0;
                }
                c_component = ct->comps_array[comp_walker]; 
                if (node_comp_walker >= child->next_node->num_of_comp)   
                {
                    // -- this was the last component of this name, continue with out_name search (children lookup) -- //
                    node = child->next_node; // -- now it's ready to get out of the loop -- //
                    in_node = false;
                    break;
                }
                else
                    continue; // -- go to the inner while loop -- //
            } // -- end of inner while loop -- //
        } // -- end of if (in_node) -- //
 
        // -- search for this component among the children -- //
        if (!(child=ht_lookup(ct, node, c_component, print_flag)))
        {
            // -- no child is available, so run do_insert -- //
            return (trie_do_insert (ct, node, ct->comps_array, comp_walker-1, all_comp, print_flag)); 
        }
        byte_walker = 0;
        while (byte_walker < strlen(c_component))
        {
            if (strlen(c_component) != child->next_node->comps[0].len)
            {
                // -- this child is not a match, go for the next child -- //
                break;
            }
            if (c_component[byte_walker] == child->next_node->comps[0].bytes[byte_walker])
            {
                byte_walker++;
            }
            else
            {
                // -- this child is not a match, go for the next child -- //
                break;
            }
        }
        if (byte_walker ==  strlen(c_component))
        {
            // -- we found the matched child. Jump to the corresponded node -- //
            /**
             * If there is more than one component in this node, check the
             * rest of them. Maybe there are some other components which can
             * be matched.
             *
             * NOTE:
             *     Do not change the node, as if it comes to node partitionting,
             *     we need to know the parent of this node.
             */

            // -- is this the last component of the name? -- //
            comp_walker++;
            if (comp_walker >= all_comp)
            {
                if (child->next_node->num_of_comp > 1)
                {
                    fprintf (stderr, "[trie_insert] WARNING: Found name does hit the end of the node.\n");
                    return 0;
                }
                return child->next_node;
            } 

            // -- this is NOT the last component of the input name -- //
            if (comp_walker == MAX_NUM_OF_COMPS)
            {
                fprintf (stderr, "[trie_insert] WARNING: MAX_NUM_OF_COMPS is reached.\n");
                return 0;
            }
            c_component = ct->comps_array[comp_walker]; 
            if (child->next_node->num_of_comp > 1)
            {
                in_node = true;
                go_next = false;
            }
            else
            {
                // -- the child->next_node was a match, but this is not the end, so jump to it and continue -- //
                node = child->next_node;
                go_next = true;
                in_node = false;
            }
        }
        else
        {
            // -- The node pointed by this child is not a match, so call do_insert(..) function -- //
            go_next = false;
            in_node = false;
        } 
        // -- name is not found, yet -- //
        if (in_node || go_next)
           continue;   // -- go to the out while loop -- //

        // -- none of the available children are the match, so insertion should be triggered -- //
        return (trie_do_insert (ct, node, ct->comps_array, comp_walker-1, all_comp, print_flag));
    } // -- end of while (c_component) -- */

    // -- unreachable point -- //
    return 0;  
} /* -- end of trie_insert(..) -- */


/* -----------------------------------------------------------------
 * Method: trie_do_insert (..)
 * Scope: Protected
 *
 * Description:
 * Insert the remaining components of a name (after doing LPM) in the
 * trie (EON is added to the name when components were extracted).
 * ------------------------------------------------------------------ */
struct node_t*
trie_do_insert (struct ct_instance* ct, struct node_t* node, char** comps_array, int comp_walker, int all_comp, bool print_flag)
{
    assert (ct);
    struct bucket_t* child;
    comp_walker++;
    // -- check whether we should be here or not -- //
    if ((child=ht_lookup(ct, node, comps_array[comp_walker], print_flag)) != 0)
    {
        // -- we could continue with insertion function -- //
        fprintf (stderr, "[trie_do_insert] ERROR: do_insert should not have been called.\n");
        return 0;
    } 
    if (!(child=ht_insert(ct, node, comps_array[comp_walker], print_flag)))
    {
        fprintf (stderr, "[trie_do_insert] ERROR: ht_lookup failed.\n");
        return 0;
    } 
 
    // -- insert the remaining components in the node pointed by this child -- //
    child->next_node = (struct node_t*)malloc(sizeof(struct node_t));
    child->next_node->num_of_comp = all_comp - comp_walker;
    child->next_node->comps = (struct comp_t*)malloc(sizeof(struct comp_t) * (all_comp - comp_walker));
    child->next_node->hash_table = 0;
    child->next_node->parent = node;
    for (int i=0; i<child->next_node->num_of_comp; i++)
    {
        if (comp_walker >= all_comp)
        { 
            // -- something is wrong here -- //
            fprintf (stderr, "[trie_do_insert] WARNING: bad loop condition.\n");
            return 0; 
        }
        child->next_node->comps[i].bytes = (char*)malloc(strlen(comps_array[comp_walker]) + 1);
        child->next_node->comps[i].len = strlen(comps_array[comp_walker]);
        memcpy (child->next_node->comps[i].bytes, comps_array[comp_walker], strlen (comps_array[comp_walker]) + 1); // -- copy null terminator -- //
        comp_walker++;
    }
    if (print_flag)
    {
        printf ("Inserted node:  ");
        db_print_node (child->next_node);     
    }
    return child->next_node;
} /* -- end of trie_do_insert (..) -- */

/* -----------------------------------------------------------------
 * Method: trie_lookup (..)
 * Scope: Protected
 *
 * Description:
 * Lookup a given name.
 * ------------------------------------------------------------------ */
struct node_t*
trie_lookup (struct ct_instance* ct, const char* name, bool print_flag, bool exact_match, struct bucket_t** visitedChildren)
{
    assert (ct);
    assert (name);

    int byte_walker = 0;       // -- index of the name -- //
    int comp_walker = 0;       // -- index of the current component (i.e. node) -- //
    int node_comp_walker = 0;  // -- index of node's component -- //
    bool in_node = false;      // -- iterate through node's components, or search children? -- //
    char* c_component;         // -- the current component -- //
    struct node_t* node;       // -- node traverser -- // 
    struct bucket_t* child;    // -- return value of ht_lookup -- //
    int all_comp = 0;          // -- number of extracted components from the input name -- //
    bool go_next = false;      // -- if a node was match, but more components need to be matched -- //
    int visited_walker = 0;    // -- index of comps_array (in case of exact match) -- //

    if (exact_match)
    {
        if (!visitedChildren)
        {
            fprintf (stderr, "[trie_lookup] ERROR: Exact match does not work without input array.\n");
            return 0;
        }
    }

    while (ct->comps_array[comp_walker] != 0)
    {
        free(ct->comps_array[comp_walker]);
        ct->comps_array[comp_walker] = 0;
        comp_walker++;
    }

    comp_walker = 0;
    if (strlen(name) < 2)
    {
        // -- a name with length of ONE? -- //
        fprintf (stderr, "[trie_lookup] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }

    // -- extract all name components -- //
    if (cm_extract_comps (name, ct->comps_array, print_flag))
    {
        fprintf (stderr, "[trie_lookup] ERROR: A name with len of ONE or ZERO.\n");
        return 0;
    }

    if (!ct->comps_array[0])
    {
        // -- no component -- //
        fprintf (stderr, "[trie_lookup] WARNING: Bad input name:  %s\n", name);
        return 0;
    }

    // -- figure out the number of components -- //
    while (ct->comps_array[all_comp])
    {
        all_comp++; 
    }
    if (all_comp == MAX_NUM_OF_COMPS)
    {
        fprintf (stderr, "[trie_lookup] WARNING: MAX_NUM_OF_COMPS is reached.\n");
        return 0;
    }
    c_component = ct->comps_array[comp_walker];

    /* ----------- Welcome to loop party ----------- */
    node = &(ct->root);
    // -- Look up extracted components. If mismatch occured, insert them one-by-one -- //
    while (c_component)
    {
        // -- search node's components? -- //
        if (in_node)
        {
            node_comp_walker = 1;  // -- zero is check beforehand -- //            
            if (child->next_node->num_of_comp < 2)
            {
                fprintf (stderr, "[trie_lookup] WARNING: in_node flag is wrongly ON.\n");
                return 0;
            }
            if (node_comp_walker >= child->next_node->num_of_comp)   
            {
                // -- something is wrong with the previous iteration -- //
               fprintf (stderr, "[trie_lookup] WARNING: node_comp_walker > # of comps.\n");
               return 0;
            }

            while (node_comp_walker < child->next_node->num_of_comp)
            {
                // -- if their lengths are different, this cannot be a match -- //
                if (strlen(c_component) != child->next_node->comps[node_comp_walker].len)
                {
                    // -- lookup failed -- //
                    return 0;
                }

                byte_walker = 0;
                while (byte_walker < strlen(c_component))
                {
                    // -- [TODO] We can use memcmp -- //
                    if (c_component[byte_walker] == child->next_node->comps[node_comp_walker].bytes[byte_walker])
                    {
                        byte_walker++;
                    }
                    else
                    {
                        // -- lookup failed -- //
                        return 0;
                    }
                } // -- this component was a match, go for the next component -- //

                node_comp_walker++;
                comp_walker++;
                if (comp_walker >= all_comp)
                {
                    /**
                     * The input name is found, so this should be
                     * the last component of the current node. 
                     */
                    if (node_comp_walker == child->next_node->num_of_comp)
                    {
                        if (exact_match)
                        {
                            visitedChildren[visited_walker] = child;
                            visited_walker++;
                        }
                        return child->next_node;
                    }
                    else
                    {
                        fprintf (stderr, "[trie_lookup] WARNING: Found name does not hit the end of the node.\n");
                        return 0;
                    } 
                }
                if (comp_walker == MAX_NUM_OF_COMPS)
                {
                    fprintf (stderr, "[trie_lookup] WARNING: MAX_NUM_OF_COMPS is reached.\n");
                    return 0;
                }
                c_component = ct->comps_array[comp_walker]; 
                if (node_comp_walker >= child->next_node->num_of_comp)   
                {
                    // -- this was the last component of this name, continue with out_name search (children lookup) -- //
                    node = child->next_node; // -- now it's ready to get out of the loop -- //
                    in_node = false;
                    break;
                }
                else
                    continue; // -- go to the inner while loop -- //
            } // -- end of inner while loop -- //
        } // -- end of if (in_node) -- //
 
        if (exact_match)
        { 
            if (comp_walker != 0)
            {
                // -- remember all visited nodes, by the children which point them -- //
                visitedChildren[visited_walker] = child;
                visited_walker++;
            }
        }
        // -- search for this component among the children -- //
        if (!(child=ht_lookup(ct, node, c_component, print_flag)))
        {
            // -- no child is available, so lookup failed -- //
            return 0;
        }
        byte_walker = 0;
        while (byte_walker < strlen(c_component))
        {
            if (strlen(c_component) != child->next_node->comps[0].len)
            {
                // -- this child is not a match, go for the next child -- //
                break;
            }
            if (c_component[byte_walker] == child->next_node->comps[0].bytes[byte_walker])
            {
                byte_walker++;
            }
            else
            {
                // -- this child is not a match, go for the next child -- //
                break;
            }
        }
        if (byte_walker ==  strlen(c_component))
        {
            // -- we found the matched child. Jump to the corresponded node -- //
            /**
             * If there is more than one component in this node, check the
             * rest of them. Maybe there are some other components which can
             * be matched.
             *
             * NOTE:
             *     Do not change the node, as if it comes to node partitionting,
             *     we need to know the parent of this node.
             */

            // -- is this the last component of the name? -- //
            comp_walker++;
            if (comp_walker >= all_comp)
            {
                if (child->next_node->num_of_comp > 1)
                {
                    fprintf (stderr, "[trie_lookup] WARNING: Found name does hit the end of the node.\n");
                    return 0;
                }
                if (exact_match)
                {
                    visitedChildren[visited_walker] = child;
                    visited_walker++;
                }
                return child->next_node;
            } 

            // -- this is NOT the last component of the input name -- //
            if (comp_walker == MAX_NUM_OF_COMPS)
            {
                fprintf (stderr, "[trie_lookup] WARNING: MAX_NUM_OF_COMPS is reached.\n");
                return 0;
            }
            c_component = ct->comps_array[comp_walker]; 
            if (child->next_node->num_of_comp > 1)
            {
                in_node = true;
                go_next = false;
            }
            else
            {
                // -- the child->next_node was a match, but this is not the end, so jump to it and continue -- //
                node = child->next_node;
                go_next = true;
                in_node = false;
            }
        }
        else
        {
            // -- The node pointed by this child is not a match, so call lookup failed -- //
            go_next = false;
            in_node = false;
        } 
        // -- name is not found, yet -- //
        if (in_node || go_next)
           continue;   // -- go to the out while loop -- //

        // -- none of the available children are the match, so lookup failed -- //
        return 0;
    } // -- end of while (c_component) -- */

    // -- unreachable point -- //
    return 0;  
} /* -- end of trie_lookup (..) -- */

/* -----------------------------------------------------------------
 * Method: trie_node_partition (..)
 * Scope: Protected
 *
 * Description:
 * Partition a name from a given point, so that its components will 
 * splitted into two parts (i.e. parent and first_child). Then the 
 * second part of splited node (i.e. first_child) and the remaining 
 * components of the name should be added to the parent.
 * ------------------------------------------------------------------ */
struct node_t*
trie_node_partition (struct ct_instance* ct, struct bucket_t* child, char** comps_array, int all_comp, int comp_walker, int node_comp_walker, bool print_flag)
{
    assert (ct);
    assert (comps_array);

    if (node_comp_walker < 0)
    {
        fprintf (stderr, "[trie_node_partition] ERROR: Negative partitioning point.\n");
        return 0;
    }
    // -- partition given node from the last matched component -- //
    else
    {
        /**
         * Node partitioning:
         *    1- find the last matched component
         *    2- we keep the prvious node (with all children)
         *    3- a new node is created between previous node and parent
         *    4- this new node has two children:
         *        a) the prvious node
         *        b) the rest of input name
         */ 
        // -- the current child should point to a new node, so store the "next_node" pointer of the current child -- //
        struct bucket_t* bucket_walker;
        struct node_t* parent;
        struct node_t* first_node;
        struct node_t* second_node;
        struct bucket_t* in_ret;   // -- store the returned value from ht_insert -- //
        struct node_t* next_node_tmp = (struct node_t*)malloc(sizeof(struct node_t));
        // -- [TODO] Do we need to copy these info??? Because maybe it gets lost after removing corresponded pointer -- //
        *next_node_tmp = *(child->next_node);
        free(child->next_node);
        // -- partition the corresponded node into parent and first_node -- //
        child->next_node = (struct node_t*)malloc(sizeof(struct node_t));
        parent = child->next_node;  // -- agent is set -- //
        parent->num_of_comp = node_comp_walker;  // -- at the parent we do not have EON -- //
        parent->comps = (struct comp_t*)malloc(sizeof(struct comp_t) * parent->num_of_comp);
        parent->parent = next_node_tmp->parent;
        parent->hash_table = 0;
        for (int i=0; i<parent->num_of_comp; i++)
        { 
            // -- copy each component to a new component in the new node -- //
            parent->comps[i].bytes = (char*)malloc(next_node_tmp->comps[i].len + 1);
            parent->comps[i].len = next_node_tmp->comps[i].len;
            memcpy(parent->comps[i].bytes, next_node_tmp->comps[i].bytes, next_node_tmp->comps[i].len + 1); // -- copy null terminator -- //
        }
        // -- parent received its components -- //

        // -- for the first node -- //
        if (!(in_ret=ht_insert(ct, parent, next_node_tmp->comps[node_comp_walker].bytes, print_flag)))
        {
            fprintf (stderr, "[trie_node_partition] ERROR: HT insertion has been failed.\n");
            return 0;
        }
        in_ret->next_node = (struct node_t*)malloc(sizeof(struct node_t));
        first_node = in_ret->next_node;  // -- agent is set -- //
        first_node->num_of_comp = next_node_tmp->num_of_comp - node_comp_walker; // -- [TODO] check zero condition -- //
        first_node->comps = (struct comp_t*)malloc(sizeof(struct comp_t) * first_node->num_of_comp);
        first_node->parent = parent;
        for (int i=0; i<first_node->num_of_comp; i++)
        { 
            // -- copy each component to a new component in the new node -- //
            first_node->comps[i].bytes = (char*)malloc(next_node_tmp->comps[node_comp_walker + i].len + 1);
            first_node->comps[i].len = next_node_tmp->comps[node_comp_walker + i].len;
            memcpy(first_node->comps[i].bytes, next_node_tmp->comps[node_comp_walker + i].bytes, next_node_tmp->comps[node_comp_walker + i].len + 1); // -- copy null terminator -- //
        }
        // -- first node received its components -- //
        if (!next_node_tmp->hash_table ||
            !next_node_tmp->hash_table->size ||
            !next_node_tmp->hash_table->used)
        {
            first_node->hash_table = 0;
        }
        else
        {
            first_node->hash_table = next_node_tmp->hash_table;
            for (int i=0; i < first_node->hash_table->size; i++)
            {
                bucket_walker = &first_node->hash_table->buckets[i];
                while (bucket_walker->next_bucket)
                {
                    bucket_walker = bucket_walker->next_bucket;
                    if (bucket_walker->key) 
                        bucket_walker->next_node->parent = first_node;
                }
            }
        }
        // -- first node is DONE -- //
 
        // -- for the second node -- //
        if (!(in_ret=ht_insert(ct, parent, comps_array[comp_walker], print_flag)))
        {
            fprintf (stderr, "[trie_node_partition] ERROR: HT insertion has been failed.\n");
            return 0;
        }
        in_ret->next_node = (struct node_t*)malloc(sizeof(struct node_t));
        second_node = in_ret->next_node;  // -- agent is set -- //
        second_node->num_of_comp = all_comp - comp_walker;  
        second_node->comps = (struct comp_t*)malloc(sizeof(struct comp_t) * second_node->num_of_comp);
        second_node->hash_table = 0;
        second_node->parent = parent;
        for (int i=0; i<second_node->num_of_comp; i++)
        {
            second_node->comps[i].bytes = (char*)malloc(strlen(comps_array[comp_walker]) + 1);
            second_node->comps[i].len = strlen(comps_array[comp_walker]);
            memcpy (second_node->comps[i].bytes, comps_array[comp_walker], strlen(comps_array[comp_walker]) + 1);
            comp_walker++;
        }

        for (int i=0; i<next_node_tmp->num_of_comp; i++)
        {
            free(next_node_tmp->comps[i].bytes);
            next_node_tmp->comps[i].bytes = 0;
            next_node_tmp->comps[i].len = 0;           
        }
        free(next_node_tmp->comps);
        next_node_tmp->comps = 0;
        free(next_node_tmp);
        // -- do not touch the children -- //  

        if (print_flag)
        {
            printf ("Inserted node: ");
            db_print_node (second_node);
        }
        // -- second child is done -- //
        // -- Partiotioning is DONE! END of insertion -- //
        return second_node; 
    }

    // -- unreachable point -- //
    return 0;
} /* -- end of trie_node_partition(..) -- */

/* -----------------------------------------------------------------
 * Method: trie_remove (..)
 * Scope: Protected
 *
 * Description:
 * Remove a given name from the trie. To remove a name, we have to make
 * sure that the whole name exists. After finding the name, JUST the last
 * node will be removed. After removing the last node, maybe merging the 
 * parent and remaining node be necessary.
 *
 * It is important to mention that there cannot be any node with one 
 * child, except the root. So, we have to merge some nodes if it was
 * necessary.
 *
 * Return:
 *     1: nam is not found (Not removed)
 *     2: ERROR (Not remove)
 *     0: DONE! (removed)
 *
 * NOTE:
 *     Removing first child -> should be done from the node.
 *     Removing other children -> should be done by the previous child.
 * ------------------------------------------------------------------ */
int
trie_remove (struct ct_instance* ct, const char* name, bool print_flag)
{
    assert (ct);
    assert (name);
   
    int visited_walker = 0; // -- index of visitedChildren array -- //
    struct bucket_t* child;
    struct bucket_t* bucket_tmp = (struct bucket_t*)malloc(sizeof(struct bucket_t));
 
    for (int i=0; i<MAX_HEIGHT; i++)
        ct->visitedChildren[i] = 0;

    // -- start exact name lookup -- //
    if (!trie_lookup (ct, name, 0, 1, ct->visitedChildren))
    {
        // -- name is NOT found -- //
        free(bucket_tmp);
        return 1; 
    }
    if (!ct->visitedChildren[0]) 
    {
        fprintf (stderr, "[trie_remove] ERROR: No node is visited while exact matching.\n");
        free(bucket_tmp);
        return 2;
    }
    // -- number of visited nodes -- // 
    while (ct->visitedChildren[visited_walker])
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
        fprintf (stderr, "[trie_remove] ERROR: An error has been occured while removing.\n");
        free(bucket_tmp);
        return 2;
    }

    if (ct->visitedChildren[visited_walker]->next_node->hash_table)
    {
        // -- exact lookup ended up with a non-leaf node -- //
        fprintf (stderr, "[trie_remove] ERROR: Exact lookup has been ended up with a non-leaf node.\n");
        free(bucket_tmp);
        return 2;
    }

    if (visited_walker == 0)
    {
        // -- there is just one node (regardless of the root) to remove -- //
        child = ct->visitedChildren[visited_walker];
        if (ct->root.hash_table->used == 1)
        {
             // -- this the last child of the root, safely remove it and then remove the whole hash table -- //
             trie_free_node(child->next_node);
             free(child->next_node);
             child->next_node = 0;
             child->key = 0; 
             if (child->next_bucket)
                 free(child->next_bucket);
             child->next_bucket = 0;
             for (int i=0; i<ct->root.hash_table->size; i++)
             {
                 if (ct->root.hash_table->buckets[i].next_bucket)
                     free(ct->root.hash_table->buckets[i].next_bucket);
                 ct->root.hash_table->buckets[i].next_bucket = 0; 
             }
             free(ct->root.hash_table->buckets);
             ct->root.hash_table->buckets = 0;
             ct->root.hash_table->size = 0;
             ct->root.hash_table->used = 0;
             free(ct->root.hash_table);
             ct->root.hash_table = 0;
             free(bucket_tmp);
             return 0;
         }
         else
         {
             // -- just remove the child, do not touch anything else -- //
             trie_free_node(child->next_node);
             free(child->next_node);
             child->next_node = 0;
             child->key = 0; 
             *bucket_tmp = *child;
             free(child->pre_bucket->next_bucket);  // -- child is deallocated -- //
             bucket_tmp->pre_bucket->next_bucket = bucket_tmp->next_bucket;
             if (bucket_tmp->next_bucket)
                 bucket_tmp->next_bucket->pre_bucket = bucket_tmp->pre_bucket;
             bucket_tmp->pre_bucket = 0;
             bucket_tmp->next_bucket = 0;
             ct->root.hash_table->used--;
             // -- we do not care of merging at root -- //
             free(bucket_tmp);
             return 0;
         }
    }
         
    // -- the last node is not a root's leaf -- //
    if (visited_walker > 0)
    {
        // -- get the child which points to the next node -- //
        child = ct->visitedChildren[visited_walker];
        // -- check number of children of the second last visited node -- //
        if (ct->visitedChildren[visited_walker-1]->next_node->hash_table->used == 1)
        {
            // -- an intermediate node with just one node is not normal -- //
            fprintf (stderr, "[trie_remove] WARNING: An intermediate node with one child.\n"); 
            return 1;
        }
        if (ct->visitedChildren[visited_walker-1]->next_node->hash_table->used == 2)
        {
            trie_free_node(child->next_node);
            free(child->next_node);
            child->next_node = 0;
            child->key = 0;
            *bucket_tmp = *child;
            free(child->pre_bucket->next_bucket);  // -- child is deallocated -- //
            bucket_tmp->pre_bucket->next_bucket = bucket_tmp->next_bucket;
            if (bucket_tmp->next_bucket)
                bucket_tmp->next_bucket->pre_bucket = bucket_tmp->pre_bucket;
            bucket_tmp->pre_bucket = 0;
            bucket_tmp->next_bucket = 0;
            ct->visitedChildren[visited_walker-1]->next_node->hash_table->used--;
            // -- now merge -- // 
            trie_node_merge (ct, ct->visitedChildren[visited_walker - 1]);
            free(bucket_tmp);
            return 0;
        } 
        else
        {
            trie_free_node(child->next_node);
            free(child->next_node);
            child->next_node = 0;
            child->key = 0;
            *bucket_tmp = *child;
            free(child->pre_bucket->next_bucket);  // -- child is deallocated -- //
            bucket_tmp->pre_bucket->next_bucket = bucket_tmp->next_bucket;
            if (bucket_tmp->next_bucket)
                bucket_tmp->next_bucket->pre_bucket = bucket_tmp->pre_bucket;
            bucket_tmp->pre_bucket = 0;
            bucket_tmp->next_bucket = 0;
            ct->visitedChildren[visited_walker-1]->next_node->hash_table->used--;
            free(bucket_tmp);
            // -- do not merge -- //
            return 0; 
        }
    }
    // -- unreachable part -- //
    return 0;
} /* -- end of trie_remove(..) -- */

/* -----------------------------------------------------------------
 * Method: trie_node_merge (..)
 * Scope: Protected
 *
 * Description:
 * Merge a node with one leaf with its child.
 * ------------------------------------------------------------------ */
struct node_t*
trie_node_merge (struct ct_instance* ct, struct bucket_t* parent_pointer)
{
    assert (ct);
    if (!parent_pointer || !parent_pointer->next_node || !parent_pointer->next_node->hash_table || 
        !parent_pointer->next_node->hash_table->size || !parent_pointer->next_node->hash_table->used)
    {
        fprintf (stderr, "[trie_node_merge] ERROR: An error has been occured while merging.\n");
        return 0;
    }

    int used = 0;
    struct node_t* parent = (struct node_t*)malloc(sizeof(struct node_t));
    struct node_t* n_parent;  // -- new parent -- //
    int num_of_merged_comp = 0;
    struct node_t* node_tmp = (struct node_t*)malloc(sizeof(struct node_t));
    struct bucket_t* bucket_walker;
    *parent = *(parent_pointer->next_node);
   
    for (int i=0; i < parent->hash_table->size; i++)
    {
        bucket_walker = &parent->hash_table->buckets[i];
        while (bucket_walker->next_bucket)
        {
            bucket_walker = bucket_walker->next_bucket;
            used++;
            // -- copy the node to remove -- //
            *node_tmp = *bucket_walker->next_node;
            free(bucket_walker->next_node);
        }
    } 
    if (used > 1)
    {
        fprintf (stderr, "[trie_node_merge] ERROR: Trying to merge a node with more than one child.\n");
        // -- [TODO] free -- //
        return 0;
    }
    for (int i=0; i < parent->hash_table->size; i++)
    { 
        if (parent->hash_table->buckets[i].next_bucket)
            free(parent->hash_table->buckets[i].next_bucket);
        parent->hash_table->buckets[i].next_bucket = 0;
    }
    free(parent->hash_table->buckets);
    parent->hash_table->buckets = 0;
    parent->hash_table->size = 0; 
    parent->hash_table->used = 0;
    free(parent->hash_table);
    parent->hash_table = 0;

    // -- copy the parent -- //
    free(parent_pointer->next_node);
    parent_pointer->next_node = (struct node_t*)malloc(sizeof(struct node_t));
    n_parent = parent_pointer->next_node;

    // -- ready to merge -- //
    num_of_merged_comp = parent->num_of_comp + node_tmp->num_of_comp; 
    n_parent->comps = (struct comp_t*)malloc(sizeof(struct comp_t) * num_of_merged_comp);
    n_parent->num_of_comp = num_of_merged_comp;
    n_parent->parent = parent->parent;

    // -- copy parent's components -- //
    for (int i=0; i<parent->num_of_comp; i++)
    {
        n_parent->comps[i].bytes = (char*)malloc(parent->comps[i].len + 1);  // -- null terminator -- //
        n_parent->comps[i].len = parent->comps[i].len;
        memcpy (n_parent->comps[i].bytes, parent->comps[i].bytes, parent->comps[i].len + 1);
    }
    // -- copy next_node's components -- //
    for (int i=0; i<node_tmp->num_of_comp; i++)
    {
        n_parent->comps[parent->num_of_comp + i].bytes = (char*)malloc(node_tmp->comps[i].len + 1);  // -- null terminator -- //
        n_parent->comps[parent->num_of_comp + i].len = node_tmp->comps[i].len;
        memcpy (n_parent->comps[parent->num_of_comp + i].bytes, node_tmp->comps[i].bytes, node_tmp->comps[i].len + 1);
    }

    // -- copy all children of the next_node (just point to the head!)-- //
    if (node_tmp->hash_table && node_tmp->hash_table->size)
    {
        n_parent->hash_table = node_tmp->hash_table;
        for (int i=0; i<node_tmp->hash_table->size; i++)
        {
            bucket_walker = &node_tmp->hash_table->buckets[i];
            while (bucket_walker->next_bucket)
            { 
                bucket_walker = bucket_walker->next_bucket;
                if (bucket_walker->key)
                    bucket_walker->next_node->parent = n_parent;
            }
        }
    }
    else
        n_parent->hash_table = 0;

    // -- node merge is DONE!-- //
    // -- free the parent, next_child, and its next_node (do not touch children of the next_node) -- //
    for (int i=0; i<node_tmp->num_of_comp; i++)
    {
        free(node_tmp->comps[i].bytes);
        node_tmp->comps[i].bytes = 0;
        node_tmp->comps[i].len = 0;
    }
    free(node_tmp->comps);
    node_tmp->num_of_comp = 0;
    node_tmp->parent = 0;
    // -- we did not touch children of node_tmp -- //
    for (int i=0; i<parent->num_of_comp; i++)
    {
        free(parent->comps[i].bytes);
        parent->comps[i].bytes = 0;
        parent->comps[i].len = 0;
    }
    free(node_tmp);
    free(parent->comps);
    free(parent);
    // -- do not touch parent's children -- //
    return n_parent;
} /* -- end of trie_node_merge(..) -- */

/* -----------------------------------------------------------------
 * Method: trie_free_node (..)
 * Scope: Protected
 *
 * Description:
 * This function deallocates a given node and its subtrees.
 * ------------------------------------------------------------------ */
void
trie_free_node (struct node_t* node)
{
    assert (node);
    struct bucket_t* bucket_walker;

    if (node->hash_table)
    {
        if (!node->hash_table->size)
            fprintf (stderr, "[trie_free_node] WARNING: An initialized ht with size ZERO\n");
        for (int i=0; i<node->hash_table->size; i++)
        {
            bucket_walker = &node->hash_table->buckets[i];
            while (bucket_walker->next_bucket)
               bucket_walker = bucket_walker->next_bucket;
            while (!bucket_walker->head)
            {
                trie_free_node(bucket_walker->next_node);
                if (bucket_walker->next_bucket)
	            free(bucket_walker->next_bucket);
                bucket_walker->next_bucket = 0;
                free(bucket_walker->next_node);
                bucket_walker->next_node = 0;
                bucket_walker->key = 0; 
                bucket_walker = bucket_walker->pre_bucket;
            }
            if (bucket_walker->next_bucket)
                free(bucket_walker->next_bucket);  // -- the head -- //
            bucket_walker->next_bucket = 0;
        }
        free(node->hash_table->buckets);
        node->hash_table->buckets = 0;
        node->hash_table->size = 0;
        node->hash_table->used = 0;
        free(node->hash_table); 
    }
    trie_do_free_node (node);
    return;    
} /* -- end of trie_free_node (..) -- */

/* -----------------------------------------------------------------
 * Method: trie_do_free_node (..)
 * Scope: Protected
 *
 * Description:
 * Freeing all memories allocated to a node and its elements.
 * ------------------------------------------------------------------ */
void
trie_do_free_node (struct node_t* node)
{
    assert (node);

    /* -- COMPS PART -- */
    for (int i=0; i<node->num_of_comp; i++)
    {
        free (node->comps[i].bytes);
        node->comps[i].bytes = 0;
        node->comps[i].len = 0;
    }
    free (node->comps);
    node->comps = 0;
    node->num_of_comp = 0;
    node->hash_table = 0;
    node->parent = 0;
    return;
} /* -- end of trie_do_free_node (..) -- */
