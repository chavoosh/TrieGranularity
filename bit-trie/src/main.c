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
 * Program driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "bt_trie.h"
#include "bt_struct.h"
#include "db_debug.h"
#include "main.h"

char* _args = "intprxRhe";
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
    printf ("\t-R:   generate trie statistical information and its final graph \n");
    printf ("\t-e:   speed evaluation mode (enter random names file) \n");
    printf ("\t-h:   Print help \n");
} /* -- end of print_inst () -- */

/* ------------------------------------------------
 * Method: print_summary
 * Scope: Public 
 * 
 * Description:
 * To print a summary of the program after running 
 * ------------------------------------------------- */
void
print_summary (struct bt_instance* bt, double insert_time, double  lookup_time, double remove_time, bool print_flag, bool dfs_flag)
{
    printf ("\n============ SUMMARY ===========\n");
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
    if (dfs_flag)
    {
        printf ("---------- Trie stat -----------\n");
        printf ("\tMAX Hieght=   %u\n", bt->trie_stat->max);
        printf ("\tNUM Leaves=   %u\n", bt->trie_stat->num);
        printf ("\tSUM Height=   %lld\n", bt->trie_stat->sum);
        if (bt->trie_stat->num != 0)
            printf ("\tAVE Height=   %f\n", (float)bt->trie_stat->sum/(float)bt->trie_stat->num);
        int c = 0;
        while (bt->trie_stat->width[c] != 0)
            c++;
        if (c > 0)
        {
            int all_nodes = 0;
            for (int i=0; i<c; i++)
                all_nodes += bt->trie_stat->width[i];
            printf ("\tALL Nodes=     %d\n", all_nodes);
            printf ("\tAVE Width=     %f\n", (float)((float)(all_nodes-1)/(all_nodes-bt->trie_stat->num)));
        }
        
        printf (ANSI_COLOR_RED "\nTo see the final Patricia Trie run below command:\n");
        printf ("    $ bash render.sh");
        printf (ANSI_COLOR_RESET "\n");
    }
} /* -- end of print_summary (..) -- */


/* --------------------------------------
 * Method: warmup()
 * Scope: public 
 * 
 * Description:
 * Just to test the main functions.
 * -------------------------------------- */
void
warmup (struct bt_instance* bt, bool print_flag, bool remove_flag, bool dfs_flag)
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
        bt_insert (bt, (const char*)names[i], print_flag);
    }
    end = clock();
    insert_cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    start = clock();
    for (int i=0; i<num_of_names; i++)
    {
        // -- lookup some names -- //
        if (bt_lookup(bt, (const char*)names[i], print_flag, 0))
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
            if (bt_remove(bt, (const char*)names[i], print_flag) == 0)
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
        db_dfs (bt, print_flag);
    }
    // -- summary -- //    
    print_summary (bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
    return;
} /* -- end of warmup(..) function -- */

/* --------------------------------------------------------
 * Method: free_bt()
 * Scope: Public 
 * 
 * Description:
 * By this function the main bt instance will be free'd.
 * This function should be called at the end of the program.
 * -------------------------------------------------------- */
void
free_bt (struct bt_instance* bt)
{
    assert (bt);
    
    // -- unchained Django! -- //
    free (bt->root.child_0);
    free (bt->root.child_1);
    free (bt->trie_stat->width);
    free (bt->trie_stat);
    free (bt->root.bytes);
    bt->root.parent = 0;
    bt->root.child_0 = 0;
    bt->root.child_1 = 0;
    bt->trie_stat = 0;
    free (bt); 
} /* end of free_bt(..) -- */


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

    // -- flags -- //
    bool warmup_flag = false;
    bool print_flag = false;
    bool remove_flag = false;
    bool to_mem_flag = false;
    bool dfs_flag = false;
    bool help_flag = false;
    bool eval_flag = false;
    char* rand_file = NULL;

    while ((sw = getopt (argc, argv, "ri:n:tpxRhe:")) != -1)
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
        case '?':
            if (optopt=='i' || optopt=='n' || optopt=='p' || optopt=='t' || optopt=='r' || optopt=='x' || optopt=='R' || optopt=='h' || optopt=='e')
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
    struct bt_instance* bt;
    bt = (struct bt_instance*)malloc(sizeof(struct bt_instance));
    assert (bt);
    bt->root.bytes = (char*)malloc(2);
    bt->root.bytes[0] = (char)SLASH;
    bt->root.bytes[1] = '\0';
    bt->root.len = 8;   // -- in terms of bit -- //
    bt->root.child_0 = 0;
    bt->root.child_1 = 0;
    bt->root.parent = 0;
    bt->root.EON_flag = false;    // -- this is not EON -- //
    bt->trie_stat = (struct t_stat*)malloc(sizeof(struct t_stat));
    bt->trie_stat->max = 0;
    bt->trie_stat->sum = 0;
    bt->trie_stat->num = 0;
    bt->trie_stat->id = 0;
    bt->trie_stat->width = (int*)malloc(MAX_HEIGHT * sizeof(int));
    for (int i=0; i<MAX_HEIGHT; i++)
        bt->trie_stat->width[i] = 0;
    /* --------------------------- END Initialize ------------------------ */


    /* --------------------------- BEGIN TEST Area ------------------------ */
/*
    char nn[] = {0x2c, 0xec};
    struct node_t* ret_insert = (struct node_t*)malloc(sizeof(struct node_t));
    printf ("__%02x\n", (char)BIT_SLIDER(nn, 0, 4));
    printf ("__%02x\n", (char)BIT_SHIFT_UP(nn, 0, 8));
    const char* na = "/mp/ll_432/ds$/mpold%25_arm/index_file/home.php";
    if ((ret_insert=(bt_insert (bt, na, print_flag))))
        db_print_node (ret_insert);


    const char* na = "abcd";
    const char* nb = "abd";
    const char* nc = "abe";
    const char* nd = "ace";
    const char* ne = "abc";
    const char* nf = "ab";
    if ((ret_insert=(bt_insert (bt, na, print_flag))))
        db_print_node (ret_insert);
    if ((ret_insert=(bt_insert (bt, nb, print_flag))))
        db_print_node (ret_insert);
    if ((ret_insert=(bt_insert (bt, nc, print_flag))))
        db_print_node (ret_insert);
    if ((ret_insert=(bt_insert (bt, nd, print_flag))))
        db_print_node (ret_insert);
    if ((ret_insert=(bt_insert (bt, ne, print_flag))))
        db_print_node (ret_insert);
    if ((ret_insert=(bt_insert (bt, nf, print_flag))))
        db_print_node (ret_insert);

    db_dfs(bt, print_flag);
*/
    /* --------------------------- END TEST Area ------------------------ */

    // -- warmup -- //
    if (warmup_flag)
        warmup (bt, print_flag, remove_flag, dfs_flag);
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
    struct node_t* ret_insert = 0;
    char* str = malloc (MAX_NAME_LEN);
    FILE* input = fopen(input_file, "r");
    if (input == NULL)
    {
        fprintf (stderr, "[main] ERROR: Failed to open MAIN File\n");
        return 1;
    }
    clock_t start, end;
    double insert_cpu_used = 0;
    double lookup_cpu_used = 0;
    double remove_cpu_used = 0;


    if (eval_flag)
    {
        int rand_size = 0;
        time_t seed;
        srand((unsigned) time(&seed));   
        int rand_tmp = rand()%num_of_rec;
        rand_size = (rand_tmp < 1000000) ? rand_tmp : 1000000;
        rand_size = 1000000;

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

        FILE* rand_file_input = fopen(rand_file, "r");
        if (rand_file_input == NULL)
        {
            fprintf (stderr, "[main] ERROR: Failed to open RANDOM File\n");
            return 1;
        }

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

        // -- mass insert -- //
        printf ("MASS INSERTION:\n");
        for (int i = 0; i < num_of_rec-rand_size; i++)
        {
            if (!bt_insert (bt, (const char*)all_input[i], print_flag))
            {
                if (print_flag)
                    printf ("Duplicate name OR Insertion error.\n");
                continue;
            }
        }

        // ======= EVAL PART ======== // 
      
        // -- eval lookup speed -- //
        start = clock();
        printf ("EVAL LOOKUP:\n");
        for (int i = 0; i < rand_size; i++)
        {
            if (!bt_lookup (bt, (const char*)rand_input[i], print_flag, 0)) 
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
            if (!bt_insert (bt, (const char*)rand_input[i], print_flag))
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
            if (!bt_remove (bt, (const char*)rand_input[i], print_flag)) 
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

        printf ("OK\n");
        if (dfs_flag)
        {
            // -- before summary we need to run dfs -- //
            db_dfs(bt, print_flag);
        }

        // -- summary -- //
        print_summary (bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
        free(str);
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        for (int i=0; i<rand_size; i++)
            free(rand_input[i]);
        free(all_input);
        free(rand_input);
        free_bt(bt);
        return 0; 
        // -- END OF EVAL PART -- //
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
 
        // -- mass insertion -- //
        printf ("MASS INSERTION:\n");
        start = clock();
        for (int i = 0; i < num_of_rec; i++)
        {
            if ((ret_insert=(bt_insert (bt, (const char*)all_input[i], print_flag))))
            {
                //db_print_node(ret_insert); 
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
            if (!bt_lookup (bt, (const char*)all_input[i], print_flag, 0)) 
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
            for (int i = 0; i < num_of_rec; i++)
            {
                if (!bt_remove (bt, (const char*)all_input[i], print_flag)) 
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
            db_dfs (bt, print_flag);
        }
        // -- summary -- //    
        print_summary (bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);

        // -- END OF MASS PART -- //
        for (int i=0; i<num_of_rec; i++)
            free(all_input[i]);
        free(all_input);
        free(str);
        free_bt(bt);
        return 0; 
    }

    // -- if to_mem_flag is NOT set -- //
    // -- mass insertion -- //
    start = clock();
    printf ("MASS INSERTION:\n");
    for (int i = 0; i < num_of_rec; i++)
    {
        if (fscanf (input, "%s", str) != EOF)
        {
            //printf ("Insert name:  %s\n", str);
            if ((ret_insert=(bt_insert (bt, (const char*)str, print_flag))))
            {
                //db_print_node(ret_insert); 
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
            if (!bt_lookup (bt, (const char*)str, print_flag, 0)) 
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
            //printf ("Remove number:  %u\n", i);
            if (fscanf (input, "%s", str) != EOF)
            {
                if (!bt_remove (bt, (const char*)str, print_flag)) 
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

    if (dfs_flag)
    {
        // -- before summary we need to run dfs -- //
        db_dfs (bt, print_flag);
    }
    // -- summary -- //    
    print_summary (bt, insert_cpu_used, lookup_cpu_used, remove_cpu_used, print_flag, dfs_flag);
 
    /* ---------------------------  END Mass part ------------------------- */
    free(str);
    free_bt(bt);
    return 0;
} /* -- end of main(..) function -- */

