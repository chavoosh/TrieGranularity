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

#include "Bt_trie.h"
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
    printf ("\t-H:   Initial size of hash tables \n");
} /* -- end of print_inst () -- */

/* ------------------------------------------------
 * Method: print_summary
 * Scope: Public 
 * 
 * Description:
 * To print a summary of the program after running 
 * ------------------------------------------------- */
void
print_summary (struct Bt_instance* Bt, double insert_time, double  lookup_time, double remove_time, bool print_flag, bool dfs_flag)
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
        printf ("\tMAX Hieght=   %u\n", Bt->trie_stat->max);
        printf ("\tNUM Leaves=   %u\n", Bt->trie_stat->num);
        printf ("\tSUM Height=   %lld\n", Bt->trie_stat->sum);
        if (Bt->trie_stat->num != 0)
            printf ("\tAVE Height=   %f\n", (float)Bt->trie_stat->sum/(float)Bt->trie_stat->num);
        int c = 0;
        while (Bt->trie_stat->width[c] != 0)
            c++;
        if (c > 0)
        {
            int all_nodes = 0;
            for (int i=0; i < c; i++)
                all_nodes += Bt->trie_stat->width[i];
            printf ("\tALL Nodes=    %d\n", all_nodes);
            printf ("\tAVE Width=    %f\n", (float)((float)(all_nodes-1)/(all_nodes-Bt->trie_stat->num)));
            printf ("\tAVE Hash Table Size=   %f\n",(float)((float)Bt->trie_stat->ht_size / (float)(all_nodes-Bt->trie_stat->num)));
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
warmup (struct Bt_instance* Bt, bool print_flag, bool remove_flag, bool dfs_flag)
{

    clock_t start, end;
    double insert_cpu_used = 0;
    double lookup_cpu_used = 0;
    double remove_cpu_used = 0;
 
    char* names[] = {"/ndn/uofa/cs/department/pub","/ndn/uofa/cs/department","/ndn/uofa/ece/department","/ndn/uofa/cs/department/pub/icn/","/ndn/uofa/cs/icn/"};
    int num_of_names = 5;    
    char* name = (char*)malloc(1);

    start = clock();
    for (int i=0; i<num_of_names; i++)
    {
        // -- insert some names -- //
        Bt_en_name ((const char*)names[i], &name);
        Bt_insert (Bt, (const char*)name, print_flag);
    }
    end = clock();
    insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    for (int i=0; i<num_of_names; i++)
    {
        // -- lookup some names -- //
        Bt_en_name ((const char*)names[i], &name);
        if (Bt_lookup(Bt, (const char*)name, print_flag, 0, 0))
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
            Bt_en_name ((const char*)names[i], &name);
            if (Bt_remove(Bt, (const char*)name, print_flag) == 0)
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
        db_dfs (Bt, print_flag);
    }
    // -- summary -- //    
    print_summary (Bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
    free_Bt(Bt);
    free(name);
} /* -- end of warmup(..) function -- */

/* --------------------------------------------------------
 * Method: free_Bt()
 * Scope: Public 
 *
 * Description:
 * By this function the main Bt instance will be free'd.
 * This function should be called at the end of the program.
 * --------------------------------------------------------- */
void
free_Bt (struct Bt_instance* Bt)
{
    assert (Bt);
    Bt_free_node (&Bt->root);
    Bt->root.len = 0;
    free(Bt->visitedNodes);
    free(Bt->trie_stat->width);
    free(Bt->trie_stat);
    free(Bt);
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
int
main (int argc, char** argv)
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
    struct Bt_instance* Bt;
    Bt = (struct Bt_instance*)malloc(sizeof(struct Bt_instance));
    assert (Bt);
    if (hash_init_size_flag)
        Bt->ht_init_size = hash_init_size;
    else
        Bt->ht_init_size = HT_INIT_SIZE;
    Bt->root.len = 2;  // -- SLASH & EON -- //
    Bt->root.bytes = (char*)malloc(Bt->root.len + 1); // -- '\0' -- //
    Bt->root.bytes[0] = (char)SLASH;
    Bt->root.bytes[1] = (char)EON;
    Bt->root.bytes[2] = '\0';
    
    // -- initialize the hash table at the first use -- //
    Bt->root.hash_table = 0;
    Bt->root.parent = 0;
    Bt->trie_stat = (struct t_stat*)malloc(sizeof(struct t_stat));
    Bt->trie_stat->max = 0;
    Bt->trie_stat->sum = 0;
    Bt->trie_stat->num = 0;
    Bt->trie_stat->id = 0;
    Bt->trie_stat->ht_size = 0;
    Bt->visitedNodes = (struct node_t**)malloc(MAX_HEIGHT * sizeof(struct node_t*));
    for (int i=0; i<MAX_HEIGHT; i++)
        Bt->visitedNodes[i] = 0;
    Bt->trie_stat->width = (int*)malloc(MAX_HEIGHT * sizeof(int));
    for (int i=0; i<MAX_HEIGHT; i++)
        Bt->trie_stat->width[i] = 0;
    /* --------------------------- END Initialize ------------------------ */

    // -- warmup -- //
    if (warmup_flag)
        warmup (Bt, print_flag, remove_flag, dfs_flag);

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
    double insert_cpu_used = 0;
    double lookup_cpu_used = 0;
    double remove_cpu_used = 0;
    char* name = (char*)malloc(1);

    /**
     * How this evaluation works:
     * The input names will be saved into an array
     * A random number of them will be selected (i.e. random indexes of the array)
     * The randomly selected names will not be inserted into the trie
     * Lookup is performed
     * Insert the randomly selected names, now 
     * Remove the selected the names
     */
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
            {
                fprintf (stderr, "[Main] WARNINIG: EOF has been met.\n");
                break;
            }
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
            {
                fprintf (stderr, "[Main] WARNINIG: EOF has been met.\n");
                break;
            }
        } 
        fclose(rand_file_input);

        printf ("rand_size:  %u\n", rand_size); 

        printf ("MASS INSERTION:\n");
        for (int i = 0; i < num_of_rec - rand_size; i++)
        {
            Bt_en_name((const char*)all_input[i], &name);
            if (!Bt_insert (Bt, (const char*)name, print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
                continue;
            }
        }


        // =============== EVAL PART ============== //

        // -- eval lookup speed -- //
        start = clock();
        printf ("EVAL LOOKUP:\n");
        for (int i = 0; i < rand_size; i++)
        {
            Bt_en_name((const char*)rand_input[i], &name);
            if (!Bt_lookup (Bt, (const char*)name, print_flag, 0, 0)) 
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
            Bt_en_name((const char*)rand_input[i], &name);
            if (!Bt_insert (Bt, (const char*)name, print_flag))
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
            Bt_en_name((const char*)rand_input[i], &name);
            if (!Bt_remove (Bt, (const char*)name, print_flag)) 
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
            db_dfs(Bt, print_flag);
        }
        // -- summary -- //
        print_summary (Bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
        free(str);
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        free(all_input);
        for (int i=0; i<rand_size; i++)
            free(rand_input[i]);
        free(rand_input);
        free_Bt(Bt);
        free(name);
        return 0; 
        // -- END OF EVAL PART -- //
    }


    if (to_mem_flag)
    {
        char** all_input = (char**)malloc((sizeof(char*) * num_of_rec)); 
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

        start = clock();
        printf ("MASS INSERTION:\n");
        for (int i = 0; i < num_of_rec; i++)
        {
            Bt_en_name((const char*)all_input[i], &name);
            if (!Bt_insert (Bt, (const char*)name, print_flag))
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
            Bt_en_name((const char*)all_input[i], &name);
            if (!Bt_lookup (Bt, (const char*)name, print_flag, 0, 0)) 
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
                Bt_en_name((const char*)all_input[i], &name);
                if (!Bt_remove (Bt, (const char*)name, print_flag)) 
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
            db_dfs(Bt, print_flag);
        }
        // -- summary -- //
        print_summary (Bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
        free(str);
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        free(all_input);
        free_Bt(Bt);
        free(name);
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
            Bt_en_name((const char*)str, &name); 
            //printf ("Insert name: %s\n", name);
            if (!Bt_insert (Bt, (const char*)name, print_flag))
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
            Bt_en_name((const char*)str, &name);
            if (!Bt_lookup (Bt, (const char*)name, print_flag, 0, 0)) 
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
                Bt_en_name((const char*)str, &name);
                if (!Bt_remove (Bt, (const char*)name, print_flag)) 
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
        db_dfs (Bt, print_flag);
    }
    // -- summary -- //    
    print_summary (Bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
    /* ---------------------------  END Mass part ------------------------- */
    free(str);
    free(name);
    free_Bt(Bt);
    return 0;
} /* -- end of main(..) function -- */

