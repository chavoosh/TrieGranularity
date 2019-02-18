#############################
### COMPONENT-LEVEL TRIE ####
############################# 

============
Description:
============
Component-level trie defines a name as a sequence of components which are delimited by slashes
’/’. Like in character-level trie, we utilize a hash table starting with the size of 1 at each node
to keep track of its children. In this implementation we use xxhash function.
Unlike in character trie, the number of children of each node is not bounded in component trie.
A re-hashing function is called whenever the load factor, i.e., the number of records (equal to
the number of children of the node) divided by the number of buckets—exceeds a specified threshold.
To control frequent rehashing which is time consuming and wastes a lot of memory space, we also employ
chaining for collision resolution. Hence, linear probing is likely to happen in a given bucket.
Unlike character trie, in this structure, finding the child to branch to at each level needs at least
two off-chip memory accesses (to reach the very first element of the chain at the mapped bucket).
Actually, the mapped bucket that stores the address of the first element in the chain is fetched
via the first off-chip memory access. The second access comes with fetching and reading the first
element. From there on, traversing any other element in the chain also requires one off-chip
memory access.


=======================
How to run the program:
=======================
First compile the program. To do this, go to `/src` directory and run:
    $ make

In `/src` directory, use the following command to run warmup (test the program):
    $ ./ct -t

NOTE:
  The program can be evaluated based on an input file. Names MUST be hierarchically structured
  and composed of multiple components separated by a delimiter (default is slash ’/’). Flat names
  are a special case of hierarchical names with only one component.
  E.g.,
      `/test/trie/granularity` has 3 components including `test`, `trie`, and `granularity`.

  Also, make sure there is no empty line in the input file and each name appears in a separate line.

Use the following command to feed the program an input file including some names:
    $ ./ct -i <file_path> -n <number_of_records_to_process>

If you need to print out the output of functions (e.g., insertion, lookup, trie_traversal, etc.),
use [-p] option. For instance, to see the output of functions in test scenario, run the following
command:
    $ ./ct -tp

By default, the program only insert and lookup the names. If you need to remove the names after insertion
and lookup, then enable [-r] option:
    $ ./ct -i <file_path> -n <number_of_records_to_process> -r
 
To see the formed component-based Patricia trie, run the following command:
    $ bash render.sh


If you need to cut off disk I/O delay from evaluation you can copy all input names to the main memory before performaing any task.
To do that use [-x] option:
    $ ./ct -i <file_path> -n <number_of_records_to_process> -x
NOTE: 
    If you have enough memory on your machine you can do this, otherwise let the program read 
    names directly from the file on the disk.

If you want to evaluate the real speed of each function (i.e. insert, lookup, and remove),
run the program with [-e] option:
    $ ./ct -i <file_path> -n <number_of_records_to_process> -e <file_path>

NOTE:
  * By enabling this option the program will automatically switch to [-x] mode (even if you did not enable it).
  
  * In evaluation mode, the program needs 2 input files. The first one is the list of all names that should 
    be inserted in the empty trie (pass by [-i] option). The second file is a **partial** subset of the first
    input file, including the names that need to be looked up, inserted, and removed in/from the trie (which is
    not empty).
    The members of a **partial** subset of a collection of objects, like names can belong to the target collection
    or not.
    E.g., assuming the target collection is like this:
        ------------------------
        /test/trie/granularity/1
        /test/trie/2
        /test/granularity/0
        ------------------------
    Then, either of the following collections is a possible partial subset of above collection:
        ------------
        /test/trie/2
        /foo/pub
        ------------
        /foo/pub
        /foo/trie

  * In the report section (when the program is done) what you see is an average amount of time of a single
    operation that each function needs. Thus, not surprisingly, we SHOULD see numebrs in order of miliseconds.

  * Evaluation mode (i.e. [-e] mode) is NOT good for memory usage evaluation.

  * In [-e] mode, the [-r] and [-x] options will be enabled automatically.


In this version of component-level trie, the edges are implemented by using hash table at each node.
So, instead of using linked list (which incures linear search at each node) we have used exact match.
This behvior increases the spead of the program significantly.

By using [-H] option you can set the initial size of hash tables at each node. E.g. here we set it to '16'.
    $ ./ct -i <file_path> -n <number_of_records_to_process> -H 16

NOTE:
  - For the hash table we have used xxhash (you can find documentation in the current folder)

=====================================================================================================
NOTES:
  You can draw a graph of generated trie by enabling [-R] option (report mode). After running the
  program in report mode, run `render.sh` script to see the visualized representation of generated
  trie.
  To show the graph we use `dot` and `xpdf` tools. Here are some useful tips about `xpdf`:  
    - ZOOM IN/OUT = Ctrl + '+'/'-'
    - Slide = Keep the screen with mouse and slide
