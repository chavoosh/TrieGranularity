# About NameGen:
NameGen uses a Markov model for learning real datasets, extracting information, and generating close-to-real names randomly. 
It enables end-users to tune the name characteristic (e.g., number and length of components). 

It removes the need to rely on available small datasets or to generate huge datasets of fully random names for network evaluation.
Without loss of generality, we focus on NDN names as an important use-case for future Internet architectures.

#### Note
The implementation of NameGen has not been optimized yet, but we open-sourced it to help the community for speeding up the realization of NDN.

## How to run the program:

Go to directory `src` and compile the program by running:
    
    $make

In directory `src`, use the following command to run NameGen

    $./ni -i <input file> -n <number of records/URLs to learn> -m <number of new names to be generated> -o <optional characteristics (number and length of components) as input>

For example, one may run `$./ni -i ../dataset/test_URL_input.txt -n 2 -m 8`

Then, NameGen is trained by 2 names from dataset `test_URL_input.txt` and generates a new dataset of 8 NDN-like names.

For more information on the structure of NameGen, see Sec. 3 in our recent paper [1]:

1. [Chavoosh Ghasemi*, Hamed Yousefi*, Kang G. Shin, and Beichuan Zhang (*co-first authors), _On the Granularity of Trie-Based Data Structures for Name Lookups and Updates_, in IEEE/ACM Transactions on Networking, vol. 27, no. 2, pp. 777-789, April 2019.](https://ieeexplore.ieee.org/document/8673766/)

