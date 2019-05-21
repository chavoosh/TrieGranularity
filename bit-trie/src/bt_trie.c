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

#include "bt_trie.h"
#include "bt_struct.h"

/* -----------------------------------------------------------------
 * Method: bt_insert (..)
 * Scope: Protected
 *
 * Description:
 * This function insert a name into the bit-trie. If the corresponded
 * name is found, we do nothing and the leaf will be returned.
 * Otherwise, we will inset the name (except the bit-based LPM) and 
 * finally the leaf will be returned.  
 * ------------------------------------------------------------------ */
struct node_t*
bt_insert (struct bt_instance* bt, const char* name, bool print_flag)
{
    int bit_walker = 0;       // -- index of the input name (in terms of bit) -- //
    int node_bit_walker = 0;  // -- index of content of the current node (in terms of bit) -- //
    char slider;              // -- extracted byte of the input byte (i.e. slider) -- //
    int ret_compare;          // -- result of byte comparison -- //
    int name_len = strlen(name);
    int child;                // -- 0 or 1 -- //
    int num_of_bits;          // -- number of compared bits at each step -- //
    struct node_t* node_walker;    // -- node traverser -- //
    struct node_t* parent;         // -- parent of the current node -- //

    // -- check the MSB of the first byte of the name -- //
    if (BIT(name[0],7) == ZERO)
    {
        node_walker = bt->root.child_0;
        child = 0;
    }
    else
    {
        node_walker = bt->root.child_1;
        child = 1;
    }
    parent = &(bt->root); 

    // -- welcome to loop party! -- //
    while (node_walker) 
    {
        node_bit_walker = 0;
        // -- slide the slider over the name -- //
        while (CURRENT_BYTE(bit_walker) < name_len)
        {
            if ( (CURRENT_BYTE(bit_walker) < name_len - 1) )
            {
                // -- middle byte -- //
                slider = (char)BIT_SLIDER(name, CURRENT_BYTE(bit_walker), CURRENT_BIT(bit_walker));
                if (node_walker->len - node_bit_walker >= BYTE_LEN)
                    num_of_bits = BYTE_LEN;
                else
                    num_of_bits = node_walker->len - node_bit_walker;
                if (num_of_bits <= 0)
                    break; // -- end of the noed, search its child -- //
                if ((ret_compare=(bt_byte_compare(slider, node_walker->bytes[CURRENT_BYTE(node_bit_walker)], num_of_bits))) == -1)
                {
                    bit_walker+=num_of_bits;
                    node_bit_walker+=num_of_bits;
                    continue;  // -- go to while -- //
                }
                else
                {
                    bit_walker+=ret_compare;
                    node_bit_walker+=ret_compare;
                    // -- if this is the end of the node, search its children -- //
                    if (node_bit_walker == node_walker->len)
                    {
                        break;
                    }
                    else
                    {
                        // -- partition the node -- //
                        return (bt_node_partition (bt, node_walker, child, name, node_bit_walker, bit_walker, print_flag));
                    }
                } 
            }
            else
            {
                // -- the last byte -- //            
                slider = BIT_SLIDER_LAST(name, CURRENT_BYTE(bit_walker), CURRENT_BIT(bit_walker)); 
                if (node_walker->len - node_bit_walker >= BYTE_LEN)
                    num_of_bits = BYTE_LEN;
                else
                    num_of_bits = node_walker->len - node_bit_walker;
                if (num_of_bits <= 0)
                    break; // -- end of the noed, search its child -- //
                if ((ret_compare=(bt_byte_compare(slider, node_walker->bytes[CURRENT_BYTE(node_bit_walker)], num_of_bits))) == -1)
                {
                    int min = (num_of_bits < ((CURRENT_BIT(bit_walker) + 1))) ? num_of_bits : (CURRENT_BIT(bit_walker) + 1); 
                    node_bit_walker += min;
                    bit_walker += min;
                    continue;  // -- go to while -- //
                }
                else
                {
                    node_bit_walker+=ret_compare;
                    bit_walker+=ret_compare;
                    // -- if this is the end of the node, search its children -- //
                    if (node_bit_walker == node_walker->len)
                    {
                        break;
                    }
                    // -- if this is the end of the name -- //
                    if (CURRENT_BYTE(bit_walker) >= name_len)
                    {
                        break;
                    }
                    else
                    {
                        // -- partition the name -- //
                        return (bt_node_partition (bt, node_walker, child, name, node_bit_walker, bit_walker, print_flag));
                    }
                } 
            }   
        }
        // -- if this is not the end of the name, go to the next node -- //
        if (CURRENT_BYTE(bit_walker) >= name_len)
        {
            /**
             * Possible cases:
             *     1- End of the node + EON ON  => Do nothing
             *     2- End of the node + EON OFF => Turn ON EON
             *     3- NOT end of name + EON ON  => Partition the node
             *     4- NOT end of name + EON OFF => Partition the node
             */
            if (node_bit_walker >= node_walker->len)
            {
                // -- end of the node and name -- //
                if (node_walker->EON_flag)
                {
                    // -- do nothing -- //
                    return node_walker;
                }
                else
                {
                    // -- Turn ON EON -- //
                    node_walker->EON_flag = true;
                    return node_walker;
                }
            }
            else
            {
                // -- middle of the name -- //
                return (bt_node_partition (bt, node_walker, child, name, node_bit_walker, bit_walker, print_flag));
            }
        }
        // -- name is NOT found, use the current bit to find the next child -- //
        if (BIT(name[CURRENT_BYTE(bit_walker)], CURRENT_BIT(bit_walker)) == ZERO)
        {
            parent = node_walker;
            node_walker = node_walker->child_0;
            child = 0;
        }
        else 
        {
            parent = node_walker;
            node_walker = node_walker->child_1;
            child = 1;
        }
    }

    return (bt_do_insert (bt, parent, child, name, 0 /*node_bit_walker*/, bit_walker, print_flag));
} /* -- end of bt_insert(..) -- */


/* -----------------------------------------------------------------
 * Method: bt_do_insert (..)
 * Scope: Protected
 * ------------------------------------------------------------------ */
struct node_t*
bt_do_insert (struct bt_instance* bt, struct node_t* parent, int child, const char* name, int node_bit_walker, int bit_walker, bool print_flag)
{
    assert (bt);
    assert (name);
   
    int name_len = strlen(name);
    struct node_t* node;
    if (!child)
        node = parent->child_0; 
    else
        node = parent->child_1;

    // -- if node is null, just add the rest of the name -- //
    if (!node)
    {
        if (!child)
        {
            parent->child_0 = (struct node_t*)malloc(sizeof(struct node_t));
            node = parent->child_0;
        }
        else
        {        
            parent->child_1 = (struct node_t*)malloc(sizeof(struct node_t));
            node = parent->child_1;
        }
        node->child_0 = 0;
        node->child_1 = 0;
        node->len = (name_len*BYTE_LEN) - bit_walker;
        node->EON_flag = true;
        node->bytes = (char*)malloc(node->len/BYTE_LEN + 1);
        node_bit_walker = 0;
        // -- add remaining bits -- //
        if (bt_byte_cpy (name, node, bit_walker, (name_len*BYTE_LEN)-1))
        {
            fprintf (stderr, "[bt_do_insert] ERROR: An error occured while copying bytes.\n");
            return 0;
        }

        // -- add parent -- //
        node->parent = parent;
    }
    else
    {
        // -- we should not be here -- //
        fprintf (stderr, "[bt_do_insert] ERROR: There is a problem in do_insertion.\n");
        return 0;
    }

    // -- insertion is done -- //
    return node;
} /* -- end of bt_do_insert (..) -- */


/* -----------------------------------------------------------------
 * Method: bt_node_partition (..)
 * Scope: Protected
 *
 * Description:
 * Parition a name from a given bit number. The content of the current
 * node will be splitted. So that, the upper part will remain in the same
 * node, while the lower part will shifted to a new node.
 * Moreover, a new node will be added to the current node which contains 
 * all remaining bits of the current name.
 * ------------------------------------------------------------------ */
struct node_t*
bt_node_partition (struct bt_instance* bt, struct node_t* node, int child, const char* name, int node_bit_walker, int bit_walker, bool print_flag)
{
    assert (bt);
    assert (name);

    // -- split the current node -- //
    if (!node)
    {
        // -- something is wrong -- //
        fprintf (stderr, "[bt_node_parition] ERROR: A NULL node.\n");  
        return 0;
    }
    int name_len = strlen(name);
    int org_EON_flag = node->EON_flag;  // -- before paritioning the node, remember its flag -- //
    // -- the parent will point to this child at the end -- //
    struct node_t* new_node;
    struct node_t* parent;
    // -- make a copy of the current node -- //
    struct node_t* node_tmp = (struct node_t*)malloc(sizeof(struct node_t));
    *node_tmp = *node;
    parent = (node->parent);

    // -- now remove the node -- //
    if (!child) 
    {
        if (!parent || !parent->child_0)
        {
            // -- something is wrong -- //
            fprintf (stderr, "[bt_node_partition] ERROR: An error occured while partitioning the node.\n");
            free (node_tmp->bytes);
            free (node_tmp);
            return 0;
        }
        parent->child_0->parent = 0;
        free(parent->child_0);
        parent->child_0 = (struct node_t*)malloc(sizeof(struct node_t));
        new_node = parent->child_0;  
    }
    else
    {
        if (!parent || !parent->child_1)
        {
            // -- something is wrong -- //
            fprintf (stderr, "[bt_node_partition] ERROR: An error occured while partitioning the node.\n");
            free (node_tmp->bytes);
            free (node_tmp);
            return 0;
        }
        parent->child_1->parent = 0;
        free(parent->child_1);
        parent->child_1 = (struct node_t*)malloc(sizeof(struct node_t));
        new_node = parent->child_1;
    }

    // -- allocate two children for the future two nodes -- //
    new_node->child_0 = 0;
    new_node->child_1 = 0;
  
    // -- set the parent -- //
    new_node->parent = node_tmp->parent; 

    if (CURRENT_BYTE(bit_walker) < name_len)
        new_node->EON_flag = false;
    else
        new_node->EON_flag = true;
 
    // -- allocate the bytes -- //
    new_node->len = node_bit_walker;
    new_node->bytes = (char*)malloc(node_bit_walker/BYTE_LEN + 1); 

    // -- copy the upper part of the content, byte-by-byte -- //
    if (bt_byte_cpy ((const char*)node_tmp->bytes, new_node, 0, node_bit_walker-1 /*zero-based*/))
    {
        fprintf (stderr, "[bt_node_partition] ERROR: An error occured while copying bytes.\n");
        free (node_tmp->bytes);
        free (node_tmp);
        return 0;
    }

    // -- build children -- //
    char which_child;
    if (CURRENT_BYTE(bit_walker) < name_len)
        which_child = BIT(name[CURRENT_BYTE(bit_walker)], CURRENT_BIT(bit_walker));
    else
        which_child = !(BIT(node_tmp->bytes[CURRENT_BYTE(node_bit_walker)], CURRENT_BIT(node_bit_walker)));
    

    if (which_child == ZERO)
    {
        // -- ONLY if there is some bit of the input name to insert, build this node -- //
        if (CURRENT_BYTE(bit_walker) < name_len)
        {
            // -- build child_0 -- //
            new_node->child_0 = (struct node_t*)malloc(sizeof(struct node_t));
            new_node->child_0->EON_flag = true;
            new_node->child_0->len = (name_len*BYTE_LEN) - bit_walker;
            new_node->child_0->child_0 = 0;
            new_node->child_0->child_1 = 0;
            new_node->child_0->bytes = (char*)malloc(new_node->child_0->len/BYTE_LEN + 1);
            new_node->child_0->parent = new_node;
            // -- copy the remainig of the name -- //
            if (bt_byte_cpy (name, new_node->child_0, bit_walker, (name_len*BYTE_LEN)-1))
            {
                fprintf (stderr, "[bt_node_partition] ERROR: An error occured while copying bytes.\n");
                free (node_tmp->bytes);
                free (node_tmp);
                return 0;
            } 
        }
        else
        {
            // -- do NOT initialize it -- //
            new_node->child_0 = 0;
        }
        // -- now build child_1 -- //
        new_node->child_1 = (struct node_t*)malloc(sizeof(struct node_t));
        if (org_EON_flag)
            new_node->child_1->EON_flag = true;
        else
            new_node->child_1->EON_flag = false;
        new_node->child_1->len = node_tmp->len - node_bit_walker;
        new_node->child_1->child_0 = 0;
        new_node->child_1->child_1 = 0;
        new_node->child_1->bytes = (char*)malloc(new_node->child_1->len/BYTE_LEN + 1);
        new_node->child_1->parent = new_node;
        // -- copy the remainig of node's content -- //
        if (bt_byte_cpy ((const char*)node_tmp->bytes, new_node->child_1, node_bit_walker, node_tmp->len-1))
        {
            fprintf (stderr, "[bt_node_partition] ERROR: An error occured while copying bytes.\n");
            free (node_tmp->bytes);
            free (node_tmp);
            return 0;
        }
        // -- copy children -- //
        if (node_tmp->child_0 != 0)
        {
            new_node->child_1->child_0 = node_tmp->child_0;
            new_node->child_1->child_0->parent = new_node->child_1;
            // -- make the children of this child to point to the new child -- //
            if (new_node->child_1->child_0->child_1 != 0)
                new_node->child_1->child_0->child_1->parent = new_node->child_1->child_0;
            if (new_node->child_1->child_0->child_0 != 0)
                new_node->child_1->child_0->child_0->parent = new_node->child_1->child_0;
        }
        if (node_tmp->child_1 != 0)
        {            
            new_node->child_1->child_1 = node_tmp->child_1;
            new_node->child_1->child_1->parent = new_node->child_1;
            // -- make the children of this child to point to the new child -- //
            if (new_node->child_1->child_1->child_1 != 0)
                new_node->child_1->child_1->child_1->parent = new_node->child_1->child_1;
            if (new_node->child_1->child_1->child_0 != 0)
                new_node->child_1->child_1->child_0->parent = new_node->child_1->child_1;
        }
    }
    else
    {
        if (CURRENT_BYTE(bit_walker) < name_len)
        {
            // -- build child_1 -- //  
            new_node->child_1 = (struct node_t*)malloc(sizeof(struct node_t));
            new_node->child_1->EON_flag = true;
            new_node->child_1->len = (name_len*BYTE_LEN) - bit_walker;
            new_node->child_1->child_0 = 0;
            new_node->child_1->child_1 = 0;
            new_node->child_1->bytes = (char*)malloc(new_node->child_1->len/BYTE_LEN + 1);
            new_node->child_1->parent = new_node;
        
            // -- copy the remainig of the name -- //
            if (bt_byte_cpy (name, new_node->child_1, bit_walker, (name_len*BYTE_LEN)-1))
            {
                fprintf (stderr, "[bt_node_partition] ERROR: An error occured while copying bytes.\n");
                free (node_tmp->bytes);
                free (node_tmp);
                return 0;
            } 
        }
        else
        {
            // -- do NOT initialize it -- //
            new_node->child_1 = 0;
        }
        // -- now build child_0 -- //
        new_node->child_0 = (struct node_t*)malloc(sizeof(struct node_t));
        if (org_EON_flag)
            new_node->child_0->EON_flag = true;
        else
            new_node->child_0->EON_flag = false;
        new_node->child_0->len = node_tmp->len - node_bit_walker;
        new_node->child_0->child_0 = 0;
        new_node->child_0->child_1 = 0;
        new_node->child_0->bytes = (char*)malloc(new_node->child_0->len/BYTE_LEN + 1);
        new_node->child_0->parent = new_node;

        // -- copy the remainig of node's content -- //
        if (bt_byte_cpy ((const char*)node_tmp->bytes, new_node->child_0, node_bit_walker, node_tmp->len-1))
        {
            fprintf (stderr, "[bt_node_partition] ERROR: An error occured while copying bytes.\n");
            free (node_tmp->bytes);
            free (node_tmp);
            return 0;
        } 
        // -- copy children -- //
        if (node_tmp->child_0 != 0)
        {
            new_node->child_0->child_0 = node_tmp->child_0;
            new_node->child_0->child_0->parent = new_node->child_0;
            // -- make the children of this child to point to the new child -- //
            if (new_node->child_0->child_0->child_1 != 0)
                new_node->child_0->child_0->child_1->parent = new_node->child_0->child_0;
            if (new_node->child_0->child_0->child_0 != 0)
                new_node->child_0->child_0->child_0->parent = new_node->child_0->child_0;
        }
        if (node_tmp->child_1 != 0)
        {            
            new_node->child_0->child_1 = node_tmp->child_1;
            new_node->child_0->child_1->parent = new_node->child_0;
            // -- make the children of this child to point to the new child -- //
            if (new_node->child_0->child_1->child_1 != 0)
                new_node->child_0->child_1->child_1->parent = new_node->child_0->child_1;
            if (new_node->child_0->child_1->child_0 != 0)
                new_node->child_0->child_1->child_0->parent = new_node->child_0->child_1;
        }
    }
    free (node_tmp->bytes);
    free (node_tmp);
    return new_node;
} /* -- end of bt_node_partition(..) -- */


/* -----------------------------------------------------------------
 * Method: bt_byte_cpy (..)
 * Scope: Protected
 *
 * Description:
 * Copy content of the first argument to the second one (bit-by-bit).
 *
 * NOTE:
 *     The src_start and src_end bytes will be copied, as well. 
 * RETURN:
 *    0: DONE!
 *    1: ERROR
 * ------------------------------------------------------------------ */
int
bt_byte_cpy (const char* src, struct node_t* dst_node, int src_start, int src_end)
{

    int src_bit_walker = src_start;
    int dst_bit_walker = 0;
    char slider;
    int rem_bits;   // -- number of bits of the last extracted byte -- // 

    if (!src || !dst_node)
    {
        // -- bad input -- //
        fprintf (stderr, "[bt_byte_cpy] ERROR: Bad input.\n");
        return 1;
    }

    while ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end) + 1) &&
            (src_bit_walker < src_end + 1) )
    {
        if ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end)) && (src_end - src_bit_walker > 6) )
        {
            // -- middle byte -- //
            slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            memcpy (dst_node->bytes + CURRENT_BYTE(dst_bit_walker), &slider, 1);
            src_bit_walker+=BYTE_LEN;
            dst_bit_walker+=BYTE_LEN;
        }
        else
        {
            if (CURRENT_BYTE(src_bit_walker) == CURRENT_BYTE(src_end))
            {
                // -- we are the at last byte -- //
                slider = (char)BIT_SLIDER_LAST(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            }
            else  // -- we are at the second last byte -- //
            {
                if (src_end - src_bit_walker > 6)
                {
                    // -- something is wrong -- //
                    fprintf (stderr, "[bt_byte_cpy] ERROR: A wrong broken loop.\n");
                    return 1; 
                }
                slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            }

            // -- mask all unwanted bits  -- // 
            char mask;
            if (CURRENT_BYTE(src_end) == CURRENT_BYTE(src_bit_walker))
            {
                rem_bits = src_end - (src_bit_walker-1);
                mask = (char)BIT_MASK_LOW_ZERO((BYTE_LEN-rem_bits));
            }
            else
            {
                rem_bits = (BYTE_LEN - CURRENT_BIT(src_end)) + (CURRENT_BIT(src_bit_walker) + 1); 
                mask = (char)BIT_MASK_LOW_ZERO(BYTE_LEN-rem_bits);
            }
            slider &= mask;
            memcpy (dst_node->bytes + CURRENT_BYTE(dst_bit_walker), &slider, 1);
            src_bit_walker += rem_bits;
            dst_bit_walker += rem_bits;
        }
    }
    return 0;   
} /* -- end of bt_byte_cpy(..) -- */

/* -----------------------------------------------------------------
 * Method: bt_byte_cpy_index (..)
 * Scope: Protected
 *
 * Description:
 * Copy content of the first argument to the second one (bit-by-bit).
 * Here we can specify the start point at the destinatoin (bit-based).
 *
 * NOTE:
 *     The dst_start will be used.
 *     The src_start and src_end bytes will be copied, as well. 
 * RETURN:
 *    0: DONE!
 *    1: ERROR
 * ------------------------------------------------------------------ */
int
bt_byte_cpy_index (const char* src, struct node_t* dst_node, int dst_start, int src_start, int src_end)
{

    int src_bit_walker = src_start;
    int dst_bit_walker = dst_start;
    char slider;
    int rem_bits;   // -- number of bits of the last extracted byte from src -- // 
    int pad;        // -- number of free bits of the first byte of dst -- //
    if (!src || !dst_node)
    {
        // -- bad input -- //
        fprintf (stderr, "[bt_byte_cpy] ERROR: Bad input.\n");
        return 1;
    }

    // -- first make sure there is not any free space in the first byte of dst -- //
    if (dst_start % BYTE_LEN != 0)
    {
        if ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end) + 1) &&
             (src_bit_walker < src_end + 1) )
        {
            if ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end)) && (src_end - src_bit_walker > 6) )
            {
                pad = BYTE_LEN - (dst_start%BYTE_LEN);
                slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
                slider = slider>>(BYTE_LEN - pad);
                dst_node->bytes[CURRENT_BYTE(dst_bit_walker)] |= slider;
                src_bit_walker+=pad;
                dst_bit_walker+=pad; 
            }
            else
            {
                pad = BYTE_LEN - (dst_start%BYTE_LEN);

                if (CURRENT_BYTE(src_bit_walker) == CURRENT_BYTE(src_end))
                {
                    // -- we are the at last byte -- //
                    slider = (char)BIT_SLIDER_LAST(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
                }
                else  // -- we are at the second last byte -- //
                {
                    if (src_end - src_bit_walker > 6)
                    {
                        // -- something is wrong -- //
                        fprintf (stderr, "[bt_byte_cpy] ERROR: A wrong broken loop.\n");
                        return 1; 
                    }
                    slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
                }
                // -- mask all unwanted bits  -- // 
                char mask;
                if (CURRENT_BYTE(src_end) == CURRENT_BYTE(src_bit_walker))
                {
                    rem_bits = src_end - (src_bit_walker-1);
                    mask = (char)BIT_MASK_LOW_ZERO(BYTE_LEN-rem_bits);
                }
                else
                {
                    rem_bits = (BYTE_LEN - CURRENT_BIT(src_end)) + (CURRENT_BIT(src_bit_walker) + 1); 
                    mask = (char)BIT_MASK_LOW_ZERO(BYTE_LEN-rem_bits);
                }
                slider &= mask;
                slider = slider>>(BYTE_LEN - pad);
                dst_node->bytes[CURRENT_BYTE(dst_bit_walker)] |= slider;
                src_bit_walker+= (pad < rem_bits) ? pad : rem_bits;
                dst_bit_walker+= (pad < rem_bits) ? pad : rem_bits;
            }
        }
        else
        {
            return 0; 
        }
    } // -- now dst_bit_walker starts from the next byte -- //

    while ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end) + 1) &&
            (src_bit_walker < src_end + 1) )
    {
        if ( (CURRENT_BYTE(src_bit_walker) < CURRENT_BYTE(src_end)) && (src_end - src_bit_walker > 6) )
        {
            // -- middle byte -- //
            slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            memcpy (dst_node->bytes + CURRENT_BYTE(dst_bit_walker), &slider, 1);
            src_bit_walker+=BYTE_LEN;
            dst_bit_walker+=BYTE_LEN;
        }
        else
        {
            if (CURRENT_BYTE(src_bit_walker) == CURRENT_BYTE(src_end))
            {
                // -- we are the at last byte -- //
                slider = (char)BIT_SLIDER_LAST(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            }
            else  // -- we are at the second last byte -- //
            {
                if (src_end - src_bit_walker > 6)
                {
                    // -- something is wrong -- //
                    fprintf (stderr, "[bt_byte_cpy] ERROR: A wrong broken loop.\n");
                    return 1; 
                }
                slider = (char)BIT_SLIDER(src, CURRENT_BYTE(src_bit_walker), CURRENT_BIT(src_bit_walker));
            }
            // -- mask all unwanted bits  -- // 
            char mask;
            if (CURRENT_BYTE(src_end) == CURRENT_BYTE(src_bit_walker))
            {
                rem_bits = src_end - (src_bit_walker-1);
                mask = (char)BIT_MASK_LOW_ZERO(BYTE_LEN-rem_bits);
            }
            else
            {
                rem_bits = (BYTE_LEN - CURRENT_BIT(src_end)) + (CURRENT_BIT(src_bit_walker) + 1); 
                mask = (char)BIT_MASK_LOW_ZERO(BYTE_LEN-rem_bits);
            }
            slider &= mask;
            memcpy (dst_node->bytes + CURRENT_BYTE(dst_bit_walker), &slider, 1);
            src_bit_walker += rem_bits;
            dst_bit_walker += rem_bits;
        }
    }
    return 0;   
} /* -- end of bt_byte_cpy_index(..) -- */



/* -----------------------------------------------------------------
 * Method: bt_remove (..)
 * Scope: Protected
 *
 * Description:
 * Remove a given name from the trie. To remove a name, we have to make
 * sure that the whole name exists. After finding the name, JUST the last
 * node will be removed. After removing the last node, maybe merging the 
 * parent and remaining node is necessary.
 *
 * It is important to mention that there cannot be any node with one 
 * child, except the root or EON node. So, we have to merge some nodes if
 * it is necessary.
 *
 * Return:
 *     1: name is not found (Not removed)
 *     2: ERROR (Not remove)
 *     0: DONE! (removed)
 *
 * NOTE:
 *     Removing each node -> should be done by their parent.
 * ------------------------------------------------------------------ */
int
bt_remove (struct bt_instance* bt, const char* name, bool print_flag)
{
    assert (bt); 
    assert (name);

    struct node_t* node;            // -- working node -- //
    struct node_t* parent;          // -- parent of the working node -- //
    int child;

    // -- start exact name lookup -- //
    if (!(node = bt_lookup(bt, name, 0, 1)))
    {
        // -- name is NOT found -- //
        if (print_flag)
            fprintf (stderr, "[bt_remove] ERROR: Name lookup failed.\n");
        return 1; 
    }

    // ======= start removing procedure ====== //
    parent = node->parent; 
    // -- which child -- //
    if (parent->child_0 == node)
        child = 0;
    else
    {
        if (parent->child_1 != node)
        {
            fprintf (stderr, "[bt_remove] ERROR: An error has been occured while removeing.\n");
            return 2;
        }
        child = 1;
    }
    // -- some pre-checks -- //
    if (!node->EON_flag)
    {
        // -- lookup ended up with a non EON node -- //
        fprintf (stderr, "[bt_remove] WARNING: Lookup eneded up with a non EON node.\n");
        return 2;
    }
    /**
     * Cases:
     *     1- node has two children -> Set EON to OFF
     *     2- node has one child    -> Set EON to OFF + Merge
     *     3- node has no child     -> Remove node + Check parent
     *     4- parent is EON         -> Do nothing
     *     5- parent is NOT EON     -> Merge 
     *
     * NOTE:
     *     Root should be ignored
     */
    // -- check children of this node -- //
    if (node->child_0 !=0 && node->child_1 != 0)
    {            
        node->EON_flag = false;
        return 0;
    }

    if (!node->child_0 && !node->child_1)
    {
        // -- This is a LEAF, remove it -- //
        if (!child)
        {
            bt_free_node (parent->child_0);
            free (parent->child_0);
            parent->child_0 = 0;
        }
        else
        {
            bt_free_node (parent->child_1);
            parent->child_1 = 0;
        }
        // -- check the parent -- //
        if (parent == &(bt->root))
        {
            // -- do not merge the root -- //
            return 0;
        }
        if (parent->EON_flag)
        {
            return 0;
        }
        else  // -- parent is NOT EON -- //
        {
            // -- merge the parent with the remaining child -- //
            if (!bt_node_merge (bt, parent, !child, print_flag))
            {
                fprintf (stderr, "[bt_remove] ERROR: An error occured while merging.\n");
                return 2;
            }
            return 0;
        } 
    }
    else // -- node with one child -- //
    {
        node->EON_flag = false;
        if (!node->child_0)
            child = 1;
        else 
            child = 0;
        if (!bt_node_merge(bt, node, child, print_flag))
        {
            fprintf (stderr, "[bt_remove] ERROR: An error occured while merging.\n");
            return 2;
        }
        return 0;
    }
    return 0;
} /* -- end of bt_remove(..) -- */


/* -----------------------------------------------------------------
 * Method: bt_node_merge (..)
 * Scope: Protected
 *
 * Description:
 * Merge a node with one of its leaves.
 * ------------------------------------------------------------------ */
struct node_t*
bt_node_merge (struct bt_instance* bt, struct node_t* parent, int child, bool print_flag)
{
    assert (bt);
    
    struct node_t* tmp_node = (struct node_t*)malloc(sizeof(struct node_t));
    struct node_t* child_node = (struct node_t*)malloc(sizeof(struct node_t));
    int dst_start = 0;

    // -- some pre-checks -- //
    if (!parent)
    {
        fprintf (stderr, "[bt_node_merge] ERROR: Parent is NULL.\n");
        return 0;
    } 
    // -- start merging -- //
    if (!child)
        *child_node = *parent->child_0; 
    else
        *child_node = *parent->child_1; 
    tmp_node->len = parent->len + child_node->len;
    tmp_node->bytes = (char*)malloc((tmp_node->len/BYTE_LEN)+1);

    // -- just initialize to make sure node_remove works properly -- //
    tmp_node->child_0 = 0;
    tmp_node->child_1 = 0;
    tmp_node->parent = 0;

    if (bt_byte_cpy_index ((const char*)parent->bytes, tmp_node, dst_start, 0, (parent->len-1)))
    {
        fprintf (stderr, "[bt_node_merge] ERROR: An error has been occured while copying parent bytes.\n");
        bt_free_node(tmp_node);
        free(tmp_node);
        free(child_node->bytes);
        free(child_node);
        return 0;
    }
    dst_start += parent->len;
    if (bt_byte_cpy_index ((const char*)child_node->bytes, tmp_node, dst_start, 0, (child_node->len-1)))
    {
        fprintf (stderr, "[bt_node_merge] ERROR: An error has been occured while copying child bytes.\n");
        bt_free_node(tmp_node);
        free(tmp_node);
        free(child_node->bytes);
        free(child_node);
        return 0;
    }
    // -- update the parent -- //
    parent->len = tmp_node->len;
    parent->bytes = realloc(parent->bytes, (((parent->len)/BYTE_LEN) + 1));
    memcpy (parent->bytes, tmp_node->bytes, (((parent->len)/BYTE_LEN) + 1)); 

    // -- copy children -- //
    if (child_node->child_0 != 0)
    {
        if (parent->child_0 != 0)
            free (parent->child_0);
        parent->child_0 = child_node->child_0;
        parent->child_0->parent = parent; 
    }
    else
    {
        if (parent->child_0)
        {
            //bt_free_node (parent->child_0);
            parent->child_0->parent = 0;
            free(parent->child_0);
        }
        parent->child_0 = 0;
    }
    if (child_node->child_1 != 0)
    {
        if (parent->child_1 != 0)
            free (parent->child_1);
        parent->child_1 = child_node->child_1;
        parent->child_1->parent = parent;
    }
    else
    {
        if (parent->child_1)
        {
            //bt_free_node (parent->child_1);
            parent->child_1->parent = 0; 
            free(parent->child_1);
        }
        parent->child_1 = 0;
    }
    if (child_node->EON_flag)
        parent->EON_flag = true;
    else
        parent->EON_flag = false;
    bt_free_node(tmp_node);
    free(tmp_node);
    free (child_node->bytes);
    free(child_node);
    return parent;
} /* -- end of bt_node_merge(..) -- */

/* -----------------------------------------------------------------
 * Method: bt_byte_compare (..)
 * Scope: Protected
 *
 * Description:
 * Compare two bytes with each other. Comparison starts from MSB.
 * RETURN:
 *    -1: Matched
 *     #: number of matched bits (between 1 to 7-one bit matches at least)
 * ------------------------------------------------------------------ */
signed int
bt_byte_compare (char byte_1, char byte_2, int len)
{
    for (int i=7; i>=(BYTE_LEN-len); i--)
    {
        if ((char)BIT(byte_1,i) == (char)BIT(byte_2,i))
        {
            continue;
        }
        else
            return BYTE_LEN-i-1;
    }
    // -- match -- //
    return -1;
} /* -- end of bt_byte_compare(..) -- */


/* -----------------------------------------------------------------
 * Method: bt_lookup (..)
 * Scope: Protected
 *
 * Description:
 * Lookup a given name in the bit-level trie
 * ------------------------------------------------------------------ */
struct node_t* bt_lookup (struct bt_instance* bt, const char* name, bool print_flag, bool exact)
{
    int bit_walker = 0;       // -- index of the input name (in terms of bit) -- //
    int node_bit_walker = 0;  // -- index of content of the current node (in terms of bit) -- //
    char slider;              // -- extracted byte of the input byte (i.e. slider) -- //
    int ret_compare;          // -- result of byte comparison -- //
    int name_len = strlen(name);
    int num_of_bits;            // -- number of compared bits at each step -- //
    struct node_t* node_walker;      // -- node traverser -- //
    int visited_walker;              // -- index of visitedNodes array -- //
    struct node_t* lastVisitedNode = 0;  // -- if exact flag is ON -- //

    // -- check the MSB of the first byte of the name -- //
    if (BIT(name[0],7) == ZERO)
    {
        node_walker = bt->root.child_0;
    }
    else
    {
        node_walker = bt->root.child_1;
    }

    visited_walker = 0;

    // -- welcome to loop party! -- //
    while (node_walker) 
    {
        if (exact)
        {
            // -- remember this node -- //
            lastVisitedNode = node_walker;
            visited_walker++;
            if (visited_walker == MAX_HEIGHT)
            {
                fprintf (stderr, "[bt_lookup] ERROR: MAX_HEIGHT is reached, increase it.\n");
                return 0;
            }
        }

        node_bit_walker = 0;
        // -- slide the slider over the name -- //
        while (CURRENT_BYTE(bit_walker) < name_len)
        {
            if (CURRENT_BYTE(bit_walker) < name_len - 1)
            {
                // -- middle byte -- //
                slider = (char)BIT_SLIDER(name, CURRENT_BYTE(bit_walker), CURRENT_BIT(bit_walker));
                if (node_walker->len - node_bit_walker >= BYTE_LEN)
                    num_of_bits = BYTE_LEN;
                else
                    num_of_bits = node_walker->len - node_bit_walker;
                if (num_of_bits <= 0)
                    break; // -- end of the node, search its child -- //
                if ((ret_compare=(bt_byte_compare(slider, node_walker->bytes[CURRENT_BYTE(node_bit_walker)], num_of_bits))) == -1)
                {
                    bit_walker+=num_of_bits;
                    node_bit_walker+=num_of_bits;
                    continue;  // -- go to while -- //
                }
                else
                {
                    bit_walker+=ret_compare;
                    node_bit_walker+=ret_compare;
                    // -- if this is the end of the node, search its children -- //
                    if (num_of_bits <= 0)
                    {
                        break;
                    }
                    else
                    {
                        // -- name is not found -- //
                        if (print_flag)
                            printf ("Name is NOT found.\n"); 
                        return 0;
                    }
                } 
            }
            else
            {
                // -- the last byte -- //            
                slider = BIT_SLIDER_LAST(name, CURRENT_BYTE(bit_walker), CURRENT_BIT(bit_walker)); 
                if (node_walker->len - node_bit_walker >= BYTE_LEN)
                    num_of_bits = BYTE_LEN;
                else
                    num_of_bits = node_walker->len - node_bit_walker;
                if (num_of_bits <= 0)
                    break; // -- end of the node, search its children -- //
                if ((ret_compare=(bt_byte_compare(slider, node_walker->bytes[CURRENT_BYTE(node_bit_walker)], num_of_bits))) == -1)
                {
                    int min = (num_of_bits < ((CURRENT_BIT(bit_walker) + 1))) ? num_of_bits : (CURRENT_BIT(bit_walker) + 1); 
                    node_bit_walker += min;
                    bit_walker += min;
                    continue;  // -- go to while -- //
                }
                else
                {
                    node_bit_walker+=ret_compare;
                    bit_walker+=ret_compare;
                    // -- if this is the end of the node, search its children -- //
                    if (num_of_bits <= 0)
                    {
                        break;
                    }
                    // -- if this is the end of the name -- //
                    if (CURRENT_BYTE(bit_walker) >= name_len)
                    {
                        break;
                    }
                    else
                    {
                        // -- name is not found -- //
                        if (print_flag)
                            printf ("Name is NOT found.\n");
                        return 0;
                    }
                } 
            } 
        }
        // -- if this is not the end of the name, go to the next node -- //
        if (CURRENT_BYTE(bit_walker) >= name_len)
        {
            /**
             * Possible cases:
             *     1- End of the node + EON ON  => Name is found 
             *     2- End of the node + EON OFF => Name is NOT found 
             *     3- NOT end of name + EON ON  => Name is NOT found
             *     4- NOT end of name + EON OFF => Name is NOT found
             */
            if (node_bit_walker >= node_walker->len)
            {
                // -- end of the name -- //
                if (node_walker->EON_flag)
                {
                    // -- do nothing -- //
                    if (print_flag)                  
                        //printf ("Name is found.\n");
                    if (exact)
                    {
                        if (print_flag)
                            printf ("Number of visited nodes:  %u\n", visited_walker);
                        return lastVisitedNode;
                    }
                    return node_walker;
                }
                else
                { 
                    if (print_flag)
                        printf ("Name is NOT found.\n");                   
                    return 0;
                }
            }
            else
            {
                if (print_flag)
                    printf ("Name is NOT found.\n");                   
                return 0;
            }
        }
        // -- name is NOT found, use the current bit to find the next child -- //
        if (BIT(name[CURRENT_BYTE(bit_walker)], CURRENT_BIT(bit_walker)) == ZERO)
        {
            node_walker = node_walker->child_0;
        }
        else 
        {
            node_walker = node_walker->child_1;
        }
    }
    if (print_flag)
        printf ("Name is NOT found.\n");
    return 0;
} /* -- end of bt_lookup(..) -- */

/* -----------------------------------------------------------------
 * Method: bt_free_node (..)
 * Scope: Protected
 *
 * Description:
 * Free a node and its subtree (do not touch the parent). 
 * ------------------------------------------------------------------ */
void
bt_free_node (struct node_t* node)
{
    assert (node);
    if (node->child_0)
    {
        bt_free_node (node->child_0);
        free(node->child_0);
        node->child_0 = 0;
    }
    if (node->child_1)
    {
        bt_free_node (node->child_1);
        free(node->child_1);
        node->child_1 = 0;
    }
    bt_do_free_node (node);
} /* -- end of bt_free_node (..) -- */

/* -----------------------------------------------------------------
 * Method: bt_do_free_node (..)
 * Scope: Protected
 *
 * Description:
 * Free all allocated memories to a node.  
 * ------------------------------------------------------------------ */
void
bt_do_free_node (struct node_t* node)
{
    assert (node);

    free (node->bytes);
    if (node->child_0)
        free (node->child_0);
    if (node->child_1)
        free (node->child_1);
    node->bytes = 0; 
    node->child_0 = 0;
    node->child_1 = 0;
    node->parent = 0;
    node->EON_flag = false;
    node->len = 0;
    return;
} /* -- end of bt_do_free_node (..) -- */
