/* -*- Mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
 * Regents of the University of Arizona & University of Michigan.
 *
 * Description:
 * Statistical name info.
 * NameGen is a free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * NameGen source code is distributed in the hope that it will be useful,
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

#ifndef NAME_INFO_H
#define NAME_INFO_H

#ifndef MAX_NUM_OF_COMPS
#define MAX_NUM_OF_COMPS 20 //-- we assume that a name has at most 20 components --//
#endif

#ifndef MAX_NUM_OF_CHARS_IN_COMP
#define MAX_NUM_OF_CHARS_IN_COMP 1000 //-- we assume that a component has at most 1000 characters --//
#endif

#ifndef MAX_LENGTH_OF_NAME
#define MAX_LENGTH_OF_NAME 1000 //-- we assume that a name has at most 1000 characters --//
#endif

#ifndef NUM_OF_POSSIBLE_CHARS
#define NUM_OF_POSSIBLE_CHARS 128 //-- we have 128 possible characters --//
#endif

typedef int bool;
#define true 1
#define false 0

struct general_info{
    long num_of_names; //-- number of names --//    
    double avg_name_length; //-- average length of all names --//
    int max_length_of_names; //-- max length of all names --//
    double avg_comp_length; //-- average component length of all names --//
    int max_length_of_comps; //-- max length of a component --//
    double num_of_comps[MAX_NUM_OF_COMPS]; //-- average number of components --//
    int max_num_of_comps; //-- max number of components --//
    long comp_count_presence[MAX_NUM_OF_COMPS]; //-- number of names having component i [1:MAX_NUM_OF_COMPS] --//
    struct general_comp_info* component[MAX_NUM_OF_COMPS]; //-- general info of components --//
    int opt_comp_length; //-- this optional parameter reads as input and determines the length of all components of a new name to be generated --//
    int opt_num_of_comps; //-- this optional parameter reads as input and determines the number of components of a new name to be generated --//
    
    /* ------------------
     * optioinal fields:
     *     ....;
     * -----------------
     */
};

struct general_comp_info{
    int max_comp_length; //-- max length of i-th components --//
    double avg_comp_length; //-- average length of i-th components --//
    double comp_length[MAX_NUM_OF_CHARS_IN_COMP]; //-- length of i-th components --// 
    double markov_matrix_o1[NUM_OF_POSSIBLE_CHARS+1][NUM_OF_POSSIBLE_CHARS+1]; //-- this matrix records the probability of occurrence of each char after another one in i-th components, which is necessary for Markov. we consider one more character as NULL as the start state in our Markov model where each state is a character --//
    double markov_matrix_o2[NUM_OF_POSSIBLE_CHARS+1][NUM_OF_POSSIBLE_CHARS+1][NUM_OF_POSSIBLE_CHARS+1];//-- the same as the previous Markov matrix, but Markov is of order 2 (i.e, occurence of each char depends on its two previous chars) --//
    /* ------------------
     * optioinal fields:
     *     ....;
     * -----------------
     */
};

struct name_info{
     int name_length; //-- length of a name --//
     int num_of_comp; //-- number of components of a name --//
     double avg_comp_length; //-- average length of components of a name --//
     int max_com_length; //-- max length of components of a name --//
     struct comp_info* component[MAX_NUM_OF_COMPS]; //-- components of a name --//
    /* ------------------
     * optioinal fields:
     *     ....;
     * -----------------
     */
     };

struct comp_info {
    char* comp_chars;  // -- characters of a component -- //
    int comp_length;   // -- length of a component -- //
    /* ------------------
     * optioinal fields:
     *     ....;
     * -----------------
     */
};

//-----------------------FUNCTIONS-------------------------//
char* name_parsing (char*); // -- parse/pre-process a name -- //
void print_info (struct name_info*); // -- print statistical info of a name -- //
struct name_info* name_proc (const char *, struct name_info* ); // -- retrieve statistical info of a name -- //
void print_general_info (struct general_info*); // -- print statistical info of total names -- //
void update_general_info (struct general_info*, struct name_info*); // -- retrieve statistical info of total names -- //
void gen_new_names_o1 (struct general_info*, long); // -- generate new names based on the trained order_1 Markov -- //
void gen_new_names_o2 (struct general_info*, long); // -- generate new names based on the trained order-2 Markov -- //

#endif /* -- NAME_INFO_H -- */
