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
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "ct_trie.h"
#include "cm_component.h"
#include "db_debug.h"
#include "db_debug_struct.h"
#include "main.h"
#include "ht_hashtable.h"

char* _args = "intprxRhHe";
/* --------------------------------------
 * Method: print_inst()
 * Scope: Public 
 * 
 * Description:
 * Prints how to use the program.
 * -------------------------------------- */
void
print_inst (char* prg)
{
    printf ("Usage: %s [-%s] [file...]\n", prg, _args);
    printf ("\t-i:   input file\n");
    printf ("\t-n:   number of records (=<input file size)\n");
    printf ("\t-t:   run the warmup scenarios (use this solely)\n");
    printf ("\t-p:   print out functions output (e.g. insertion) \n");
    printf ("\t-r:   remove the names after insertion \n");
    printf ("\t-x:   copy names in memory before any task (more memory, less delay) \n");
    printf ("\t-R:   Generate trie statistical information and its final graph \n");
    printf ("\t-h:   Print help \n");
    printf ("\t-e:   speed evaluation mode (enter random names file) \n");
    printf ("\t-H:   Set the initial size of hash tables at nodes \n");
} /* -- end of print_inst () -- */

/* ------------------------------------------------
 * Method: print_summary
 * Scope: Public 
 * 
 * Description:
 * To print a summary of the program after running 
 * ------------------------------------------------- */
void
print_summary (struct ct_instance* ct, double insert_time, double  lookup_time, double remove_time, bool print_flag, bool dfs_flag)
{
    printf ("============ SUMMARY ===========\n");
    printf ("------------- TIME -------------\n");
    printf ("Insertion time:    %f\n", insert_time);
    printf ("Lookup time:       %f\n", lookup_time);
    printf ("Removal time:      %f\n", remove_time);
    if (!dfs_flag)
    {
        printf (ANSI_COLOR_RED "\nTo see more statistical info of the final trie use [-R] tag\n");
        printf (ANSI_COLOR_RESET "\n");
    }

    // -- some statistical info -- //
    else
    {
        printf ("---------- Trie stat -----------\n");
        printf ("\tMAX Hieght=   %u\n", ct->trie_stat->max);
        printf ("\tNUM Leaves=   %u\n", ct->trie_stat->num);
        printf ("\tSUM Height=   %lld\n", ct->trie_stat->sum);
        if (ct->trie_stat->num != 0)
            printf ("\tAVE Height=   %f\n", (float)ct->trie_stat->sum/(float)ct->trie_stat->num);
        int c = 0;
        while (ct->trie_stat->width[c] != 0)
            c++;
        if (c > 0)
        {
            int all_nodes = 0;
            for (int i=0; i<c; i++)
                all_nodes += ct->trie_stat->width[i]; 
            printf ("\tALL Nodes=    %d\n", all_nodes);
            printf ("\tAVE Width=    %f\n", (float)((float)(all_nodes-1)/(all_nodes-ct->trie_stat->num)));
            printf ("\tAVE Hash Table Size=   %f\n",(float)((float)ct->trie_stat->ht_size / (float)(all_nodes-ct->trie_stat->num)));
            printf ("\tAVE Chain Length=      %f\n", (float)((float)ct->trie_stat->chain_length / (float)(all_nodes-ct->trie_stat->num)));
        }    
        
        printf (ANSI_COLOR_RED "\nTo see the final Patricia Trie run below command:\n");
        printf ("    $ bash render.sh");
        printf (ANSI_COLOR_RESET "\n");
    }
} /* -- end of print_summary (..) -- */

/* --------------------------------------
 * Method: warmup()
 * Scope: Public 
 * 
 * Description:
 * Just to test the main functions.
 * -------------------------------------- */
void
warmup (struct ct_instance* ct, bool print_flag, bool remove_flag, bool dfs_flag)
{

    clock_t start, end;
    double insert_cpu_used = 0;
    double lookup_cpu_used = 0;
    double remove_cpu_used = 0;
 
    char* names[] = {"/ndn/uofa/cs/department/pub","/ndn/uofa/cs/department","/ndn/uofa/ece/department","/ndn/uofa/cs/department/pub/icn/","/ndn/uofa/cs/icn/"};
    int num_of_names = 5;    

    start = clock();
    for (int i=0; i<num_of_names; i++)
    {
        // -- insert some names -- //
        trie_insert (ct, (const char*)names[i], print_flag);
    }
    end = clock();
    insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    for (int i=0; i<num_of_names; i++)
    {
        // -- lookup some names -- //
        if (trie_lookup(ct, (const char*)names[i], print_flag, 0, 0))
        {
            if (print_flag)
                printf ("Name is found:   %s\n", names[i]);
        }
        else
        {
            if (print_flag)
                printf ("Name is NOT found:   %s\n", names[i]);
        }
    }
    end = clock();
    lookup_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    start = clock();
    if (remove_flag)
    {
        for (int i=0; i<num_of_names; i++)
        {
            // -- remove some names -- // 
            if (trie_remove(ct, (const char*)names[i], print_flag) == 0)
            {
                if (print_flag)
                    printf ("Name is removed:   %s\n", names[i]);
            }
            else
            {
                if (print_flag)
                    printf ("Name is NOT removed:   %s\n", names[i]);
            }
        }
    }
    end = clock();
    remove_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    if (dfs_flag)
    {
        // -- before summary we need to run dfs -- //
        db_dfs (ct, print_flag);
    }
    // -- summary -- //    
    print_summary (ct, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
    free_ct(ct);
} /* -- end of warmup(..) function -- */


/* ---------------------------------------------------
 * Method: main()
 * Scope: Public 
 * 
 * Description:
 * Program driver. Here the main instance of
 * the trie will be defined.
 * --------------------------------------------------- */
void free_ct (struct ct_instance* ct)
{
    assert (ct);
    trie_free_node (&ct->root);
    free(ct->visitedChildren);
    ct->visitedChildren = 0;
    for (int i=0; i<MAX_NUM_OF_COMPS; i++)
    {
        free(ct->comps_array[i]); 
        ct->comps_array[i] = 0;
    }
    free(ct->comps_array);
    ct->comps_array = 0;
    free(ct->trie_stat->width);
    free(ct->trie_stat);
    return; 
} /* -- end of free_ct (..) -- */

/* ---------------------------------------------------
 * Method: main()
 * Scope: Public 
 * 
 * Description:
 * Program driver. Here the main instance of
 * the trie will be defined.
 * --------------------------------------------------- */
int main (int argc, char** argv)
{
    /* ------------------------------ BEGIN Parsing ------------------------ */
    char* input_file = NULL;
    int num_of_rec = 0;
    int sw = 0;
    long ret;
    char* rem;   // -- after ret in strtol -- //
    bool warmup_flag = false;
    bool print_flag = false;
    bool remove_flag = false;
    bool to_mem_flag = false;
    bool dfs_flag = false;
    bool help_flag = false;
    bool hash_init_size_flag = false;
    int hash_init_size = 0;
    bool eval_flag = false;
    char* rand_file = NULL;
    
    while ((sw = getopt (argc, argv, "ri:n:tpxRhH:e:")) != -1)
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
        case 't':
            warmup_flag = true;
            break;
        case 'p':
            print_flag = true;
            break;
        case 'r':
            remove_flag = true;
            break;
        case 'x':
            to_mem_flag = true;
            break;
        case 'R':
            dfs_flag = true;
            break;
        case 'h':
            help_flag = true;
            break;
        case 'e':
            eval_flag = true;
            rand_file = optarg;
            break;
        case 'H':
            ret = strtol (optarg, &rem, 10); 
            if (ret < 1)
            {
                fprintf (stderr, "[main] ERROR: Option -%c requires an integer argument, greater than ZERO.\n", sw);
                return 1;
            }
            hash_init_size = ret;
            hash_init_size_flag = true;
            break;
        case '?':
            if (optopt=='i' || optopt=='n' || optopt=='p' || optopt=='t' || optopt=='r' || optopt=='x' || optopt=='R' || optopt=='h' || optopt=='H' || optopt=='e')
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
    /* ------------------------------ END Parsing -------------------------- */

    if (help_flag)
    {
        print_inst(argv[0]); 
        return 0;
    } 
    /* --------------------------- Begin Initialize ------------------------ */
    struct ct_instance* ct;
    ct = (struct ct_instance*)malloc(sizeof(struct ct_instance));
    assert (ct);
    if (hash_init_size_flag)
        ct->ht_init_size = hash_init_size;
    else
        ct->ht_init_size = HT_INIT_SIZE;
    ct->root.comps = (struct comp_t*)malloc(sizeof(struct comp_t));
    ct->root.comps[0].bytes = (char*)malloc(2);
    ct->root.comps[0].bytes[0] = (char)SLASH;
    ct->root.comps[0].bytes[1] = '\0';
    ct->root.comps[0].len = 1; 
    ct->root.num_of_comp = 1;
    ct->root.hash_table = 0;  // -- initialize it at the first use -- //
    ct->root.parent = 0;
    ct->trie_stat = (struct t_stat*)malloc(sizeof(struct t_stat));
    ct->trie_stat->max = 0;
    ct->trie_stat->sum = 0;
    ct->trie_stat->num = 0;
    ct->trie_stat->id = 0;
    ct->trie_stat->chain_length = 0;
    ct->trie_stat->ht_size = 0;
    ct->visitedChildren = (struct bucket_t**)malloc(MAX_HEIGHT * sizeof(struct bucekt_t*));
    for (int i=0; i<MAX_HEIGHT; i++)
        ct->visitedChildren[i] = 0;
    ct->comps_array = (char**)malloc(MAX_NUM_OF_COMPS * sizeof(char*));  // -- [TODO] assume number of components in a name cannot exceed MAX_NUM_OF_COMPS -- // 
    for (int i=0; i<MAX_NUM_OF_COMPS; i++)
    {
        //ct->comps_array[i] = (char*)malloc(1);
        ct->comps_array[i] = 0;
    }
    ct->trie_stat->width = (int*)malloc(MAX_HEIGHT * sizeof(int));
    for (int i=0; i<MAX_HEIGHT; i++)
        ct->trie_stat->width[i] = 0;
    /* --------------------------- END Initialize ------------------------ */

    // -- warmup -- //
    if (warmup_flag)
        warmup (ct, print_flag, remove_flag, dfs_flag);

    /* --------------------------- BEGIN Mass part ------------------------ */
    // -- if arguments are not provided, stop --//
    if (input_file != NULL && num_of_rec == 0)
    {
        fprintf (stderr, "[main] ERROR: Specify number of records.\n");
        return 0;
    }
    if (input_file == NULL || num_of_rec == 0)
        return 0;

    // -- mass insertion -- //
    char* str = malloc (MAX_NAME_LEN);
    FILE* input = fopen(input_file, "r");
    if (input == NULL)
    {
        fprintf (stderr, "[main] ERROR: Failed to open the MAIN File\n");
        return 1;
    }
    clock_t start, end;
    double insert_cpu_used;
    double lookup_cpu_used;
    double remove_cpu_used;

    if (eval_flag)
    {
        int rand_size = 0;
        time_t seed;
        srand((unsigned) time(&seed));   
        int rand_tmp = rand()%num_of_rec;
        rand_size = (rand_tmp < 1000000) ? rand_tmp : 1000000;
        rand_size = 1000000;

        FILE* rand_file_input = fopen(rand_file, "r");
        if (rand_file_input == NULL)
        {
            fprintf (stderr, "[main] ERROR: Failed to open the RAND File\n");
            return 1;
        }
        char** all_input = (char**)malloc((sizeof(char*) * num_of_rec)); 
        char** rand_input = (char**)malloc((sizeof(char*) * rand_size)); 
        for (int i=0; i<num_of_rec; i++)
        {
            if (fscanf(input, "%s", str) != EOF)
            {
                all_input[i] = (char*)malloc(strlen(str) + 1);
                strcpy (all_input[i], str);
            }
            else
                break;
        } 
        fclose(input);

        for (int i=0; i<rand_size; i++)
        {
            if (fscanf(rand_file_input, "%s", str) != EOF)
            {
                rand_input[i] = (char*)malloc(strlen(str) + 1);
                strcpy (rand_input[i], str);
            }
            else 
                break;
        } 
        fclose(rand_file_input);

        printf ("rand_size:  %u\n", rand_size);
        printf ("MASS INSERTION:\n");
        for (int i = 0; i < num_of_rec-rand_size; i++)
        {
            if (!trie_insert (ct, (const char*)all_input[i], print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
                continue;
            }
        }

        // ============= EVAL PART =============== //
        // -- eval lookup speed -- //
        start = clock();
        printf ("EVAL LOOKUP:\n");
        for (int i = 0; i < rand_size; i++)
        {
            if (!trie_lookup (ct, (const char*)rand_input[i], print_flag, 0, 0)) 
            {
                if (print_flag)
                    printf ("Name is NOT found:\t%s\n", rand_input[i]);                 
            }
            else
            {
                if (print_flag)
                    printf ("Name is found:\t%s\n", rand_input[i]);
            }
        }
        end = clock();
        lookup_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // -- eval insertion speed -- //
        start = clock();
        printf ("EVAL INSERTION:\n");
        for (int i = 0; i < rand_size; i++)
        {
            if (!trie_insert (ct, (const char*)rand_input[i], print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
            }
            else
            {
                if (print_flag)
                    printf ("Name is inserted:\t%s\n", rand_input[i]);
            }
        }
        end = clock();
        insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // -- eval remove speed -- //
        start = clock();
        printf ("EVAL REMOVE:\n");
        for (int i = 0; i < rand_size; i++)
        { 
            if (!trie_remove (ct, (const char*)rand_input[i], print_flag)) 
            {
                if (print_flag)
                    printf ("Name is removed:\t%s\n", rand_input[i]);                 
            }
            else
            {
                if (print_flag)
                    printf ("Name is NOT removed:\t%s\n", rand_input[i]);
            }
        }
        end = clock();
        remove_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;



        if (dfs_flag)
        {
            // -- before summary we need to run dfs -- //
            db_dfs(ct, print_flag);
        }
        // -- summary -- //
        print_summary (ct, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
        free(str);
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        for (int i=0; i<rand_size; i++)
            free(rand_input[i]);
        free(rand_input);
        free(all_input);
        free_ct(ct);
        return 0; 
        // -- END OF MASS PART -- //
    }

    if (to_mem_flag)
    {
        char** all_input = (char**)malloc((sizeof(char*) * num_of_rec)); 
        for (int i=0; i<num_of_rec; i++)
        {
            if (fscanf(input, "%s", str) != EOF)
                all_input[i] = (char*)malloc(strlen(str) + 1);
                strcpy (all_input[i], str);
        } 
        fclose(input);

        start = clock();
        printf ("MASS INSERTION:\n");
        for (int i = 0; i < num_of_rec; i++)
        {
            if (!trie_insert (ct, (const char*)all_input[i], print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
                continue;
            }
        }
        end = clock();
        insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // -- mass lookup -- //
        start = clock();
        printf ("MASS LOOKUP:\n");
        for (int i = 0; i < num_of_rec; i++)
        {
            if (!trie_lookup (ct, (const char*)all_input[i], print_flag, 0, 0)) 
            {
                if (print_flag)
                    printf ("Name is NOT found:\t%s\n", all_input[i]);                 
            }
            else
            {
                if (print_flag)
                    printf ("Name is found:\t%s\n", all_input[i]);
            }
        }
        end = clock();
        lookup_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // -- mass remove -- //
        start = clock();
        if (remove_flag)
        {
            printf ("MASS REMOVE:\n");
            for (int i = num_of_rec-1; i >= 0; i--)
            {
                if (!trie_remove (ct, (const char*)all_input[i], print_flag)) 
                {
                    if (print_flag)
                        printf ("Name is removed:\t%s\n", all_input[i]);                 
                }
                else
                {
                    if (print_flag)
                        printf ("Name is NOT removed:\t%s\n", all_input[i]);
                }
            }
        }
        end = clock();
        remove_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        if (dfs_flag)
        {
            // -- before summary we need to run dfs -- //
            db_dfs(ct, print_flag);
        }
        // -- summary -- //
        print_summary (ct, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
        free(str);
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        free(all_input);
        free_ct(ct);
        free(ct);
        return 0; 
        // -- END OF MASS PART -- //
    }

    // -- if to_mem_flag is NOT set -- //
    start = clock();
    printf ("MASS INSERTION:\n");
    for (int i = 0; i < num_of_rec; i++)
    {
        if (fscanf (input, "%s", str) != EOF)
        {
            //printf ("Name:  %s\n", str);
            if (!trie_insert (ct, (const char*)str, print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
                continue;
            }
        }
        else
        {
            if (print_flag)
                fprintf (stderr, "[main] WARNING: Check the input file.\n");
        }
    }
    end = clock();
    insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    fclose(input);

    // -- mass lookup -- //
    input = fopen(input_file, "r");
    start = clock();
    printf ("MASS LOOKUP:\n");
    for (int i = 0; i < num_of_rec; i++)
    {
        if (fscanf (input, "%s", str) != EOF)
        {
            if (!trie_lookup (ct, (const char*)str, print_flag, 0, 0)) 
            {
                if (print_flag)
                    printf ("Name is NOT found:\t%s\n", str);                 
            }
            else
            {
                if (print_flag)
                    printf ("Name is found:\t%s\n", str);
            }
        }
        else
        {
            if (print_flag)
                fprintf (stderr, "[main] WARNING: Check the input file.\n");
        }
    }
    end = clock();
    lookup_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    fclose(input);

    // -- mass remove -- //
    input = fopen(input_file, "r");
    start = clock();
    if (remove_flag)
    {
        printf ("MASS REMOVE:\n");
        for (int i = 0; i < num_of_rec; i++)
        {
            if (fscanf (input, "%s", str) != EOF)
            {
                if (!trie_remove (ct, (const char*)str, print_flag)) 
                {
                    if (print_flag)
                        printf ("Name is removed:\t%s\n", str);                 
                }
                else
                {
                    if (print_flag)
                        printf ("Name is NOT removed:\t%s\n", str);
                }
            } 
            else
            {
                if (print_flag)
                    fprintf (stderr, "[main] WARNING: Check the input file.\n"); 
            }
        }
    }
    end = clock();
    remove_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    fclose(input);
    if (remove_flag)
    {
        printf (ANSI_COLOR_RED "\nTo see the the real performance of remove function, run the program in [-x] mode.");
        printf (ANSI_COLOR_RESET "\n");
    }

    if (dfs_flag)
    {
        // -- before summary we need to run dfs -- //
        db_dfs (ct, print_flag);
    }
    // -- summary -- //    
    print_summary (ct, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
    /* ---------------------------  END Mass part ------------------------- */
    free(str);
    free_ct(ct);
    free(ct);
    return 0;
} /* -- end of main(..) function -- */

