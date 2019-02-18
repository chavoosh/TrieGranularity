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
#include <ctype.h>
#include "cm_component.h"
#include "ct_trie.h"

/* --------------------------------------------------------------------------
 * Method: cm_extract_comps
 * Scope: private
 *
 * Description:
 * This function extracts all components of a given name, where the delimiter
 * is SLASH. After extracting components, the component array will be ended 
 * with EON component (i.e. /0x01). So, all the names in this trie have this
 * component as their last component.
 *
 * NOTE:
 *    If you want to print the output, turn ON the flag!
 *
 * RETURN:
 *    1: ERROR
 *    0: DONE!
 * -------------------------------------------------------------------------- */
int
cm_extract_comps (const char* name, char** comps_array, bool print_flag)
{
    assert (name);

    int comp_walker = 0;    // -- index of the current component (i.e. node) -- //
    const char del[2] = "/";
    char *token;
    char *t_name;

    if (strlen(name) < 2)
    {
        // -- a name with length of ONE? -- //
        fprintf (stderr, "[cm_extract_comps] ERROR: A name with len of ONE or ZERO.\n");
        return 1;
    }

    // -- extracting components -- //
    if (name[0] != (char)SLASH)
    {
        // -- all names should start with SLASH -- //
        fprintf (stderr, "[cm_extract_comps] ERROR: All names should start with slash:   %s\n", name);
        return 1;
    }
    t_name = (char*)malloc(strlen(name) + 1);
    strcpy (t_name, name);

    if (print_flag)
    { 
        printf ("===============\n");
        printf ("Name:   %s::", name);
    }

    token = strtok (t_name, del);  
    while (token != NULL)
    {
        comps_array[comp_walker] = (char*)realloc(comps_array[comp_walker], (strlen(token) + 1)); 
        strcpy(comps_array[comp_walker], token);
        comps_array[comp_walker][strlen(token)] = '\0'; 
        comp_walker++;
        if (comp_walker == MAX_NUM_OF_COMPS)
        {
            fprintf (stderr, "[cm_extract_comps] WARNING: Number of components is greater than set MAX.\n\t%s", t_name);
            free(t_name);
            return 1;
        }
        token = strtok(NULL, del); 
    }      
    if (!comp_walker)
    {
        fprintf (stderr, "[cm_extract_comps] WARNING: A name with no valid component.\n\t%s", t_name);
        free(t_name);
        return 1;
    }

    // -- we add another component to all names to mark their ending -- //
    comps_array[comp_walker] = (char*)realloc(comps_array[comp_walker], 2);
    comps_array[comp_walker][0] = (char)EON;
    comps_array[comp_walker][1] = '\0';
    comp_walker++;

    if (print_flag)
    { 
        printf ("%u\n", comp_walker);
        printf ("Comps:  ");
        for (int i=0; i<comp_walker; i++)
        {
            if (isprint(comps_array[i][0]))
                printf("<%s>", comps_array[i]);
            else
                printf ("<%u>", (int)comps_array[i][0]);
        }
        printf ("\n===============\n");
    }
    free(t_name);
    return 0;
} /* -- end of cm_extract_comps (..) -- */
