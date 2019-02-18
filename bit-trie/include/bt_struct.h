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
 * The main macros and bitfields of bit-level Patricia trie. 
*/

#ifndef BT_STRUCT_H
#define BT_STRUCT_H

#ifndef BYTE_LEN
#define BYTE_LEN 8
#endif

#ifndef ONE
#define ONE 0x01
#endif
#ifndef ZERO
#define ZERO 0x00
#endif

typedef int bool;
#define true 1
#define false 0

/**
 * Definition of bit-wise macros
 */
// -- to create bit mask -- //
#define BIT_MASK(n)    ( 1<<(n) )
#define BIT(a, n)      ( a & BIT_MASK(n) ) 

// -- create n-low-bit zero/one mask -- //
#define BIT_MASK_LOW_ZERO(n)        ( 0xFF<<(n) ) 
#define BIT_MASK_LOW_ONE(n)         ( 0xFF & ~BIT_MASK_LOW_ZERO(n) )

// -- shift a given cell of a char array up/down -- //
#define BIT_SHIFT_UP(a, i, n)       ( a[i]<<(n) & BIT_MASK_LOW_ZERO(n) )
#define BIT_SHIFT_DOWN(a, i, n)     ( a[i]>>(n) & BIT_MASK_LOW_ONE(BYTE_LEN-n) )

// -- move the slider one byte forward -- //
#define BIT_SLIDER(a, i, current_bit)    ( BIT_SHIFT_UP(a, i, (BYTE_LEN-current_bit-1)) | BIT_SHIFT_DOWN(a, (i+1), current_bit + 1) )
#define BIT_SLIDER_LAST(a, i, current_bit)  ( BIT_SHIFT_UP(a, i, (BYTE_LEN - (current_bit+1))) & BIT_MASK_LOW_ZERO(BYTE_LEN - (current_bit + 1)) )
#define CURRENT_BIT(n)              ( BYTE_LEN-((n)%BYTE_LEN)-1 )
#define CURRENT_BYTE(n)             ( (n)/BYTE_LEN )

/**
 * EXAMPLE:
 * Here is a byte and we assume the current
 * bit is the 3rd bit (index 2) and this is
 * the 16th byte of the corresponded name.
 * So the params will be as below:
 *     CURRENT_BIT= 2
 *     CURRENT_BYTE= 16
 *     
 * -----------------------------------
 * | 7 | 6 | 5 | 4 | 3 |<<2>>| 1 | 0 |
 * -----------------------------------
 *  
 * If this byte is the last byte of the name:
 *     --------------------------------------
 *     |<<2'>>| 1' | 0' | - | - | - | - | - |
 *     --------------------------------------
 *     As we can see the rest of the byte is 
 *     filled,using ZERO.
 *
 * If this byte is NOT the last byte of the name:
 *     -------------------------------------------
 *     |<<2'>>| 1' | 0' | 7" | 6" | 5" | 4" | 3" |
 *     -------------------------------------------
 *     As we can see, the rest of the byte is filled,
 *     using the upper part of the next byte (i.e. 17th
 *     byte).
 *
 * IMPORTANT NOTE:
 *     All the indexes are ZERO-based.
 */



#endif /* bt_STRUCT_H */
