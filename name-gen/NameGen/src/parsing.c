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
 * Method: name_parsing ()
 * Scope: Public
 *
 * Description:
 * url parsing
 * --------------------------------------------------- */
char* name_parsing (char* str)
{
/* -----------------remove protocol (http://www., https://www., www., ftp://, etc.)------------------*/
int index=0, i=0, j=0, k=0, l=0;

if (strncmp(str, "http://www.", strlen("http://www.")) == 0)
    index= strlen("http://www.");
else if (strncmp(str, "https://www.", strlen("https://www.")) == 0)
    index= strlen("https://www.");
else if (strncmp(str, "http://", strlen("http://")) == 0)
    index= strlen("http://");
else if (strncmp(str, "https://", strlen("https://")) == 0)
    index= strlen("https://");
else if(strncmp(str, "www.", strlen("www.")) == 0)
    index= strlen("www.");
    /* ------------------
     * [TODO] ....... : more patterns to be added
     *     ....;
     * -----------------
     */
if (index != 0)
{
    for(i=index; i<strlen(str); i++)
        str[i-index]=str[i];
    str[i-index]='\0';
} 

/* ------------------------put '/' at the beginning and remove '/' from the end------------------------*/
if (str[strlen(str)-1] == '/')
    str[strlen(str)-1]='\0';

if (str[0]!='/')
{
    for (i=strlen(str); i>-1; i--)
        str[i+1]=str[i];
    str[0]='/';
}

/* -------------------------------------change "//" to "/" in name--------------------------------------*/
for(i=0; i<strlen(str); i++)
    if ((str[i] == '/') & (str[i+1] =='/'))
    {
        //str[i] = str[i+1]= '.';
        for(j=i; j<strlen(str); j++)
            str[j]=str[j+1];
        str[j]='\0';
    }
/* -----------------change format of TLD: '.' in .com, .net, .org, .edu, etc. to '/'---------------------*/
FILE* f_TLD = fopen("../src/TLD_list.txt", "r");
if (f_TLD == NULL)
{
    printf("Error opening TLD file!\n");
    exit(1);
}
char* str_tmp = malloc (MAX_LENGTH_OF_NAME * sizeof(char));        
while (fscanf (f_TLD, "%s", str_tmp) != EOF) //-- check TLDs one by one everywhere in the name --//
{
    for (i=0; i<strlen(str); i++)
    {
        j=0;
        k=i;
        while((str_tmp[0]==str[i]) && (j < strlen(str_tmp)) && (str_tmp[j] == str[k]))
        {
            k++;
            j++;
        }
        if ((j==strlen(str_tmp)) && (str[k] == '/' || str[k] == '\0'))
            //-- if we are here, it means that we have found this TLD in the input name --//
            str[i]='/';
    }
}

    /* ------------------
     * [TODO] ....... : more patterns to be added to the TLD_list file if necessary
     *(.us .com .ca .co.uk .net .info .biz .eu .edu .org .name .me .tv 
     .at .be .ch .cn .cz .de .dk .fr .it .in .nl .no .pt .ru .se .jp)
     *     ....;
     * -----------------
     */
fclose(f_TLD);

/* -------------------------------move TLD to the beginning of the name---------------------------------*/
FILE* f_TLD1 = fopen("../src/TLD_list.txt", "r");
if (f_TLD1 == NULL)
{
    printf("Error opening TLD file!\n");
    exit(1);
}
for (i=1; (str[i]!='/' && i<strlen(str)); i++); //-- Our target TLD can only be the second component --//
if (str[i]=='/')
    while (fscanf (f_TLD1, "%s", str_tmp) != EOF) //-- check TLDs one by one everywhere in the name --//
    {
        j=1;
        k=i+1;
        while((str_tmp[j] == str[k]) && (j < strlen(str_tmp)))
        {
            k++;
            j++;
        }
        if ((j==strlen(str_tmp)) && (str[k] == '/' || str[k] == '\0'))
            //-- if we are here, it means that we have found this TLD in the input name --//
        {
             for(l=i-1; l>-1; l--)
                 str[l+strlen(str_tmp)]=str[l];
             for (l=strlen(str_tmp)-1; l>-1; l--)
                 str[l]=str_tmp[l];
             str[0]='/';
             break;
        }
    }
fclose(f_TLD1);
return(str);
}/* -- end of name_parsing () -- */

