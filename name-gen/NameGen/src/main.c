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

char* _args = "inmo";
/* --------------------------------------
 * Method: print_inst()
 * Scope: private
 * 
 * Description:
 * Prints how to use the program.
 * -------------------------------------- */
void print_inst (char* prg)
{
    printf ("Usage: %s [-%s] [file...]\n", prg, _args);
    printf ("\t-i:   input file\n");
    printf ("\t-n:   number of records (=<input file size)\n");
    printf ("\t-m:   number of new names to be generated\n");
    printf ("\t-o:   optional characteristics (number and length of components) as input\n");

} /* -- end of print_inst () -- */


/* ---------------------------------------------------
 * Method: main()
 * Scope: Public 
 * 
 * Description:
 * URL statistics analyzer 
 * --------------------------------------------------- */

int main (int argc, char** argv)
{ 
    srand((unsigned)time(NULL));
    /* ------------------------------ BEGIN Parsing Terminal------------------------ */
    /**
     * OPTIONS:
     *     -i: input file
     *     -n: number of records
     *     -m: number of new names to be generated
     *     -o: optional characteristics (number and length of components) as input
     */
    char* input_file = NULL;
    int num_of_rec = 0;
    long num_of_new_names = 0;
    int sw = 0;
    long ret;
    char* rem;   // -- after ret in strtol -- //
    bool optional_flag = false;

    while ((sw = getopt (argc, argv, "i:n:m:o")) != -1)
    switch (sw)
    {
        case 'i':
            input_file = optarg;
            break;
        case 'n':
            ret = strtol (optarg, &rem, 10); 
            if (ret < 1)
            {
                fprintf (stderr, "[main] ERROR: Option -%c requires an integer argument, greater than ZERO.\n", sw);
                return 1;
            }
            num_of_rec = (int)ret;
            break;
        case 'm':
            ret = strtol (optarg, &rem, 10); 
            if (ret < 1)
            {
                fprintf (stderr, "[main] ERROR: Option -%c requires an integer argument, greater than ZERO.\n", sw);
                return 1;
            }
 
            num_of_new_names = ret;
            break;
        case 'o':
            optional_flag = true;
            break;
        case '?':
            if (optopt=='i' || optopt=='n' || optopt=='m' || optopt=='o')
                fprintf (stderr, "[main] ERROR: Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
            {
                fprintf (stderr, "[main] ERROR: Unknown option `-%c'.\n", optopt);
                print_inst(argv[0]);
            }
            else
            {
                fprintf (stderr,"[main] ERROR: Unknown option character `\\x%x'.\n", optopt);
                print_inst(argv[0]);
            }
            return 1;
        default:
            abort ();
    }

    // -- remaining -- //
    if (optind < argc)
    {
        printf ("[main] ERROR: Non-option argument %s\n", argv[optind]);
        print_inst(argv[0]);
        return 1;
    }
/* ------------------------------ END Parsing Terminal-------------------------- */

/* ------------------------ BEGIN General INFO Initialization----------------------- */
    struct general_info *gen = (struct general_info*)malloc(sizeof(struct general_info));
    gen->num_of_names = 0;
    gen->avg_name_length = 0;
    gen->max_length_of_names = 0;
    gen->avg_comp_length = 0;
    gen->max_length_of_comps = 0;
    for (int i=0; i<MAX_NUM_OF_COMPS; i++) 
        gen->num_of_comps[i] = 0;
    gen->max_num_of_comps = 0;
    for(int i=0; i<MAX_NUM_OF_COMPS; i++)
    {
        gen->component[i] = (struct general_comp_info*)malloc(sizeof(struct general_comp_info));
        gen->component[i]->max_comp_length = 0;
        gen->component[i]->avg_comp_length = 0;
        for(int j=0; j<MAX_NUM_OF_CHARS_IN_COMP; j++)
            gen->component[i]->comp_length[j]=0;    
    }
    for(int i=0; i<MAX_NUM_OF_COMPS; i++)
        gen->comp_count_presence[i]=0;
    for (int k=0; k<MAX_NUM_OF_COMPS; k++)
        for (int i=0; i<NUM_OF_POSSIBLE_CHARS+1; i++)
            for (int j=0; j<NUM_OF_POSSIBLE_CHARS+1; j++)
                {
                gen->component[k]->markov_matrix_o1[i][j] = 0;
                for (int m=0; m<NUM_OF_POSSIBLE_CHARS+1; m++)
                    gen->component[k]->markov_matrix_o2[i][j][m] = 0;
                } 
    gen->opt_comp_length = 0;
    gen->opt_num_of_comps = 0; 
/* ------------------------ END General INFO Initialization----------------------- */

// -- if arguments are not provided, stop --//
    if (input_file == NULL || num_of_rec == 0 || num_of_new_names == 0)
    {
        printf("./ni [-i URL file path] [-n number of input names] [-m number of new names to be generated][-o  optional]");
        return 0;
    }
    printf("==========================NAMES============================\n");
    FILE* input = fopen(input_file, "r");
    FILE *f0 = fopen("../output/names_after_parsing.txt", "w");
    if (f0 == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    char* str = malloc (MAX_LENGTH_OF_NAME * sizeof(char));  
            /* ------------------------ BEGIN Name INFO malloc----------------------- */
	    struct name_info *info = (struct name_info*)malloc(sizeof(struct name_info));
	    for(int i=0; i<MAX_NUM_OF_COMPS; i++)
	    {
		info->component[i] = (struct comp_info*)malloc(sizeof(struct comp_info));
		info->component[i]->comp_chars = (char *) malloc (MAX_NUM_OF_CHARS_IN_COMP * sizeof (char));
	    }
            /* ------------------------ END Name INFO malloc----------------------- */
    for (int i=0; i < num_of_rec; i++)
    {
        if (fscanf (input, "%s", str) != EOF)
        {
            /* ------------------------ BEGIN Name INFO Initialization----------------------- */
	    info->name_length = 0;
	    info->num_of_comp = 0;
	    info->avg_comp_length = 0;
            info->max_com_length = 0;
	    for(int i=0; i<MAX_NUM_OF_COMPS; i++)
	    {
		for(int j=0; j<MAX_NUM_OF_CHARS_IN_COMP; j++)
		    info->component[i]->comp_chars [j] = 0 ; //--[TODO] how to initialize component characters--//
		info->component[i]->comp_length = 0;
	    }
            /* ------------------------ END Name INFO Initialization----------------------- */
            if (!name_parsing (str))
            {
                fprintf (stderr, "ERROR in Name Parsing");
            }
            //printf("name %s\n", str);
            fprintf(f0,"%s\n", str);
            if (!name_proc (str, info))
            {
                fprintf (stderr, "ERROR in Name Statistics Analysis");
            }
            else
            {
	        //print_info (info);
		update_general_info (gen, info);
            }
        }
        else
            fprintf (stderr, "[main] WARNING: Check the input file.\n");
    }
    printf("======================GENERAL INFO=========================\n");
    print_general_info (gen); //-- Besides printing, it will complete parameters calculation--//

    if (optional_flag)
    {
    printf("=================Name Generation Inputs====================\n");
    printf("Enter # of components less than %d (0 if you want to ignore): ", gen->max_num_of_comps +1);
    scanf("%d",&gen->opt_num_of_comps);
    printf("Enter length of components (0 if you want to ignore): ");
    scanf("%d",&gen->opt_comp_length);
    }

    printf("===========NEW GENERATED NAMES (Order-1 MARKOV)============\n");
    gen_new_names_o1 (gen, num_of_new_names);
    printf("===========NEW GENERATED NAMES (Order-2 MARKOV)============\n");
    gen_new_names_o2 (gen, num_of_new_names);
    printf("                 CHECK ../OUTPUT FOLDER                     \n");
 return 0;
} /* -- end of main(..) -- */
