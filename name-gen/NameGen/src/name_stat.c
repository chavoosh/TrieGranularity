/* -*- Mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
 * Regents of the University of Arizona & University of Michigan.
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
 * NameGen tries to analyze the URLs statistics and generate NDN names accordingly.
 * NameGen is a free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>

#include "name_stat.h"

/* ---------------------------------------------------
 * Method: print_info()
 * Scope: Public 
 * 
 * Description:
 * print statistical info of a single name
 * --------------------------------------------------- */
void print_info (struct name_info *info)
{
    printf("Name Length: %d\n", info->name_length);
    printf("# of components: %d\n", info->num_of_comp);
    printf("Avg. length of components: %f\n", info->avg_comp_length);
    printf("Max length of components: %d\n", info->max_com_length);
    for(int i=0; i<MAX_NUM_OF_COMPS; i++)
        if (info->component[i]->comp_chars[0] != 0)
            printf("Componet %d-th is %s\n", i+1, info->component[i]->comp_chars);
}/* -- end of print_info () -- */

/* ---------------------------------------------------
 * Method: print_general_info()
 * Scope: Public 
 * 
 * Description:
 * print general statistical info (all names analysis)
 * --------------------------------------------------- */
void print_general_info (struct general_info *gen)
{
    FILE *f1 = fopen("../output/num_of_comp_dist.txt", "w");
    if (f1 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    FILE *f2 = fopen("../output/comp_length_dist.txt", "w");
    if (f2 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    FILE *f3 = fopen("../output/markov_o1_prob_log.txt", "w");
    if (f3 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    printf("# of names: %ld\n", gen->num_of_names);
    printf("Avg. length of names: %f\n", gen->avg_name_length / gen->num_of_names);
    printf("Max length of names: %d\n", gen->max_length_of_names);
    printf("Avg. length of components: %f\n", gen->avg_comp_length / gen->num_of_names);
    printf("Max length of components: %d\n", gen->max_length_of_comps);
    double tmp=0;
    for (int i=0; i<MAX_NUM_OF_COMPS; i++)
    {
        gen->num_of_comps[i] /= gen->num_of_names;
        fprintf(f1, " %f", gen->num_of_comps[i]); 
    }
    for (int i=0; i<MAX_NUM_OF_COMPS; i++)
        tmp += (i+1) * gen->num_of_comps[i];  
    printf("Avg. # of components: %f\n", tmp);
    printf("Max # of components: %d\n", gen->max_num_of_comps);


    for (int index_comp=0; index_comp<MAX_NUM_OF_COMPS; index_comp++)
    {
        fprintf(f2,"============%d-th Component=====================\n", index_comp+1);
        for(int i=0; i<MAX_NUM_OF_CHARS_IN_COMP; i++)
        {
            gen->component[index_comp]->comp_length[i] /= gen->comp_count_presence[index_comp];
            fprintf(f2, " %f", gen->component[index_comp]->comp_length[i]); 
        }
        /*
        printf("=================INFO of %d-th Component=====================\n", index_comp+1);
        printf("Avg. length of %d-th component: %f\n", index_comp+1, gen->component[index_comp]->avg_comp_length / gen->comp_count_presence[index_comp]);
        printf("Max length of %d-th component: %d\n", index_comp+1, gen->component[index_comp]->max_comp_length);  
        */
        fprintf(f3,"============%d-th Component=====================\n", index_comp+1);
        int tmp = 0;
        for (int i=0; i<NUM_OF_POSSIBLE_CHARS+1; i++)
        {   
            fprintf(f3, " %d", i);
            tmp=0;
            for (int j=0; j<NUM_OF_POSSIBLE_CHARS+1; j++)
                tmp += gen->component[index_comp]->markov_matrix_o1[i][j];
            for (int j=0; j<NUM_OF_POSSIBLE_CHARS+1; j++)
                if (tmp != 0)           
                    fprintf(f3, " %f", gen->component[index_comp]->markov_matrix_o1[i][j] / tmp);
                else
                    fprintf(f3, " %f", 0.0);
            fprintf(f3,"\n");
        }
    } 
    fclose(f1);
    fclose(f2);
    fclose(f3);
}/* -- end of print_general_info () -- */

/* ---------------------------------------------------
 * Method: update_general_info()
 * Scope: Public
 *
 * Description:
 * Extract general statistical info (all names analysis)
 * --------------------------------------------------- */
void update_general_info (struct general_info *gen, struct name_info *info)
{
    gen->num_of_names++;
    gen->avg_name_length += info->name_length;
    if (gen->max_length_of_names < info->name_length)
        gen->max_length_of_names = info->name_length;
    gen->avg_comp_length += info->avg_comp_length; //--[TODO] Here, we report per-name length.//
    if (gen->max_length_of_comps < info->max_com_length)
        gen->max_length_of_comps = info->max_com_length;
    gen->num_of_comps [(info->num_of_comp)-1]++;
    if (gen->max_num_of_comps < info->num_of_comp)
        gen->max_num_of_comps = info->num_of_comp;

    for (int i=0; i<info->num_of_comp; i++)
    {
        gen->component[i]->comp_length[strlen(info->component[i]->comp_chars)-1]++;
        gen->component[i]->avg_comp_length += strlen(info->component[i]->comp_chars);
        if (gen->component[i]->max_comp_length < strlen(info->component[i]->comp_chars))
            gen->component[i]->max_comp_length = strlen(info->component[i]->comp_chars);
        gen->comp_count_presence[i]++;
        //===============Train MARKOV (Order-1 model): Transition Probability Calc==================
        if (strlen(info->component[i]->comp_chars) > 1)
            for (int j=0; j<(strlen(info->component[i]->comp_chars)-1); j++)
                gen->component[i]->markov_matrix_o1[(int)info->component[i]->comp_chars[j]][(int)info->component[i]->comp_chars[j+1]] ++;
        gen->component[i]->markov_matrix_o1[128][(int)info->component[i]->comp_chars[0]] ++; //--for transition to the first char --//
       //===============Train MARKOV (Order-2 model): Transition Probability Calc==================
        if (strlen(info->component[i]->comp_chars) > 2)
            for (int j=0; j<(strlen(info->component[i]->comp_chars)-2); j++)
                gen->component[i]->markov_matrix_o2[(int)info->component[i]->comp_chars[j]][(int)info->component[i]->comp_chars[j+1]] [(int)info->component[i]->comp_chars[j+2]] ++;
        if (strlen(info->component[i]->comp_chars) > 1)
            gen->component[i]->markov_matrix_o2[128][(int)info->component[i]->comp_chars[0]][(int)info->component[i]->comp_chars[1]] ++; //--for transition to the second char --//  
        gen->component[i]->markov_matrix_o2[128][128][(int)info->component[i]->comp_chars[0]] ++; //--for transition to the first char --//      
    }
}/* -- end of update_general_info () -- */

/* ---------------------------------------------------
 * Method: name_proc()
 * Scope: Public
 *
 * Description:
 * Extract statistical info of a name
 * --------------------------------------------------- */
struct name_info* name_proc (const char *name, struct name_info* info)
{
    int num_of_component = 0;	
    int i = 0,j = 0;
    while (i < strlen(name))
    {
	if (name[i]=='/')
        {
	    num_of_component++;
	    j = 0;
        }
        else
        {
            info->component[num_of_component-1]->comp_chars[j] = name[i];
            j++;
        }
        i++;
    }
    info->name_length = strlen(name);
    info->num_of_comp = num_of_component;
    for(i=0; i<MAX_NUM_OF_COMPS; i++)
    {
        info->component[i]->comp_length = strlen(info->component[i]->comp_chars);
        info->avg_comp_length += strlen(info->component[i]->comp_chars);
        if (info->max_com_length < info->component[i]->comp_length)
            info->max_com_length = info->component[i]->comp_length;
    }
    info->avg_comp_length /= info->num_of_comp;
 return (info);
}/* -- end of name_proc () -- */

/* ---------------------------------------------------
 * Method: gen_new_names_o1()
 * Scope: Public
 *
 * Description:
 * generate new names based on the trained Markov (ORDER 1)
 * --------------------------------------------------- */
void gen_new_names_o1 (struct general_info *gen, long num_of_new_names) 
{
    FILE *f4 = fopen("../output/o1_generated_names.txt", "w");
    if (f4 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    char * new_name = malloc(MAX_LENGTH_OF_NAME * sizeof(char));
    for (int i=0; i<num_of_new_names; i++)
    {
        for (int j=0; j<MAX_LENGTH_OF_NAME; j++)
            new_name[j] = 0;
        int index_in_new_name = 0;
        int new_num_of_comp = 0;
        int new_comp_length = 0;
        int j = 0;
        double sum_prob = 0;
        //============== BEGIN: calculation of number of components=============
        if(gen->opt_num_of_comps == 0)
        {
            while (j<MAX_NUM_OF_COMPS)
            {
                double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accourate if necessary --//
                sum_prob += gen->num_of_comps[j];
                if (sum_prob >= random)
                    break;
                j++;
            }
            new_num_of_comp = j;
            new_num_of_comp++; //-- j is in [0:MAX_NUM_OF_COMPS-1], but number of components is in [1:MAX_NUM_OF_COMPS] --//
        }
        else
            new_num_of_comp = gen->opt_num_of_comps;
        //============== END: calculation of number of components=============
        int index_comp = 0;
        for (index_comp=0; index_comp<new_num_of_comp; index_comp++)
        {
            int k = 0;
            double sum_prob = 0;
        //============== BEGIN: calculation of length of a component=============
            if(gen->opt_comp_length == 0)
            {
                while (k<MAX_NUM_OF_CHARS_IN_COMP)
                {
                    double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accourate if necessary --//
                    sum_prob += gen->component[index_comp]->comp_length[k];
                    if (sum_prob >= random)
                        break;
                    k++;
                }
                new_comp_length = k;
                new_comp_length++; //-- k is in [0:MAX_NUM_OF_CHARS_IN_COMP-1], but length is in [1:MAX_NUM_OF_CHARS_IN_COMP] --//
            }
            else
                new_comp_length = gen->opt_comp_length;
        //============== END: calculation of length of a component=============
            int state = 128; //-- starting state is NULL (matrix[128][] in our order-1 Markov model) --//
            index_in_new_name = strlen (new_name);
            new_name[index_in_new_name] = '/';
            index_in_new_name++;
        //============== BEGIN: generating characters for a component=============
            for (int l=index_in_new_name; l<(index_in_new_name + new_comp_length); l++)
            {
                double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accurate if necessary --//
                double sum_markov_o1 = 0;
                for (int m=0; m<NUM_OF_POSSIBLE_CHARS+1; m++)
                    sum_markov_o1 += gen->component[index_comp]->markov_matrix_o1[state][m];
                //printf("sum %c %f\n", state, sum_markov_o1); 
                double sum_prob = 0;
                int s = 0;
                while (s<128)
                {
                    sum_prob += (gen->component[index_comp]->markov_matrix_o1[state][s] / sum_markov_o1);
                    if (sum_prob >= random)
                        break;
                    s++;
                }
                //--[TODO] here if there is no pattern to continue, we simply return to the first state (s=128) and repeat character generation
                if (s != 128) 
                    new_name[l] = (char) (s);
                else
                    l--;
                state = s ;
            }
        //============== END: generating characters for a component=============
        }
    //printf("%s\n", new_name);
    fprintf(f4,"%s\n", new_name);
    }
    fclose(f4);
}/* -- end of gen_new_names_o1 () -- */

/* ---------------------------------------------------
 * Method: gen_new_names_o2()
 * Scope: Public
 *
 * Description:
 * generate new names based on the trained Markov (ORDER 2)
 * --------------------------------------------------- */
void gen_new_names_o2 (struct general_info *gen, long num_of_new_names) 
{
    FILE *f5 = fopen("../output/o2_generated_names.txt", "w");
    if (f5 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    char * new_name = malloc(MAX_LENGTH_OF_NAME * sizeof(char));
    for (int i=0; i<num_of_new_names; i++)
    {
        for (int j=0; j<MAX_LENGTH_OF_NAME; j++)
            new_name[j] = 0;
        int index_in_new_name = 0;
        int new_num_of_comp = 0;
        int new_comp_length = 0;
        int j = 0;
        double sum_prob = 0;
        //============== BEGIN: calculation of number of components=============
        if (gen->opt_num_of_comps == 0)
        {
            while (j<MAX_NUM_OF_COMPS)
            {
                double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accourate if necessary --//
                sum_prob += gen->num_of_comps[j];
                if (sum_prob >= random)
                    break;
                j++;
            }
            new_num_of_comp = j;
            new_num_of_comp++; //-- j is in [0:MAX_NUM_OF_COMPS-1], but number of components is in [1:MAX_NUM_OF_COMPS] --//
        }
        else
            new_num_of_comp = gen->opt_num_of_comps;
        
        //============== END: calculation of number of components=============
        int index_comp = 0;
        for (index_comp=0; index_comp<new_num_of_comp; index_comp++)
        {
            int k = 0;
            double sum_prob = 0;
        //============== BEGIN: calculation of length of a component=============
            if(gen->opt_comp_length == 0)
            {
                while (k<MAX_NUM_OF_CHARS_IN_COMP)
                {
                    double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accourate if necessary --//
                    sum_prob += gen->component[index_comp]->comp_length[k];
                    if (sum_prob >= random)
                        break;
                    k++;
                }
                new_comp_length = k;
                new_comp_length++; //-- k is in [0:MAX_NUM_OF_CHARS_IN_COMP-1], but length is in [1:MAX_NUM_OF_CHARS_IN_COMP] --//
            }
            else
                new_comp_length = gen->opt_comp_length;
        //============== END: calculation of length of a component=============
            int state1 = 128; //-- starting state is NULL (matrix[128][128][] in our order-2 Markov model) --//
            int state2 = 128; //-- starting state is NULL (matrix[128][128][] in our order-2 Markov model) --//
            index_in_new_name = strlen (new_name);
            new_name[index_in_new_name] = '/';
            index_in_new_name++;
        //============== BEGIN: generating characters for a component=============
            for (int l=index_in_new_name; l<(index_in_new_name + new_comp_length); l++)
            {
                double random=(rand() % 1000000000) / 1000000000.0; //-- [TODO] make it more accourate if necessary --//
                double sum_markov_o2 = 0;
                for (int m=0; m<NUM_OF_POSSIBLE_CHARS+1; m++)
                    sum_markov_o2 += gen->component[index_comp]->markov_matrix_o2[state1][state2][m];
                double sum_prob = 0;
                int s = 0;
                while (s<128)
                {
                    sum_prob += (gen->component[index_comp]->markov_matrix_o2[state1][state2][s]/sum_markov_o2);
                    if (sum_prob >= random)
                        break;
                    s++;
                }
                //--[TODO] here if there is no pattern to continue, we simply return to the first state (s=128) and repeat character generation
                if (s != 128)
                    new_name[l] = (char) (s);
                else
                    l--;
                state1 = state2 ;
                state2 = s ;
            }
        //============== END: generating characters for a component=============
        }
    //printf("%s\n", new_name);
    fprintf(f5,"%s\n", new_name);
    }
    fclose(f5);
}/* -- end of gen_new_names_o2 () -- */
