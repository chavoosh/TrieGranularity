# CHARACTER-LEVEL TRIE

Character-level (or Byte-level) trie defines a name as a sequence of US-ASCII printable
characters (i.e., bytes). Thus, the number of children at each node cannot exceed 
94 (codes 33–126). To keep track of children, i.e., to implement the trie edges
we employ a hash table at each node, as using linked list drastically decreases
the speed due to linear probing (i.e., checking the children, one-by-one, at each
node until finding the target child). The ASCII codes are used as keys, while the
hash function is simply modulus.
To minimize the memory footprint, the initial size of hash table is 1 while expanding
gradually for collision resolution. In case of a collision, a re-hash function is called,
doubling the hash table size until the mapped bucket becomes empty.


How to run the program:
-----------------------
First compile the program. To do this, go to /src directory and run:
    
    $ make

In `/src` directory, use the following command to run warmup (test the program):
    
    $ ./Bt -t

#### NOTE:
- The program can be evaluated based on an input file. Names in the input file can appear in
  any form, with no restriction on their structure (i.e., it can be either a hierarichal, like
  `/test/char/level/trie`, or a flat name, like SHA1 hash value).
- Also, make sure there is no empty line in the input file and each name appears in a separate line.

Use the below command to feed the program a file of names:
    
    $ ./Bt -i <file_path> -n <number_of_records_to_process>

If you need to print out the output of functions (e.g., insertion, lookup, trie_traversal, etc.),
use [-p] option. For instance, to see the output of functions in test scenario, run the following
command:
    
    $ ./Bt -tp

By default, the program only insert and lookup the names. If you need to remove the names after insertion
and lookup, then enable [-r] option:
    
    $ ./Bt -i <file_path> -n <number_of_records_to_process> -r

If you need to cut off disk I/O delay from evaluation you can copy all input names to the main memory before performaing any task.
To do that use [-x] option:
    
    $ ./Bt -i <file_path> -n <number_of_records_to_process> -x

#### NOTE: 
- If you have enough memory on your machine you can do this, otherwise let the program read 
  names directly from the file on the disk.

If you want to evaluate the real speed of each function (i.e. insert, lookup, and remove),
run the program with [-e] option:
    
    $ ./Bt -i <file_path> -n <number_of_records_to_process> -e <file_path>

#### NOTE:
- By enabling this option the program will automatically switch to [-x] mode (even if you did not enable it).
  
- In evaluation mode, the program needs 2 input files. The first one is the list of all names that should 
  be inserted in the empty trie (pass by [-i] option). The second file is a **partial** subset of the first
  input file, including the names that need to be looked up, inserted, and removed in/from the trie (which is
  not empty).
  The members of a **partial** subset of a collection of objects, like names can belong to the target collection
  or not.
  E.g., assuming the target collection is like this:
  
        /test/trie/granularity/1
        /test/trie/2
        /test/granularity/0

  Then, either of the following collections is a possible partial subset of above collection:
 
        /test/trie/2
        /foo/pub
  &
  
        /foo/pub
        /foo/trie

- In the report section (when the program is done) what you see is an average amount of time of a single
  operation that each function needs. Thus, not surprisingly, we SHOULD see numebrs in order of miliseconds.

- Evaluation mode (i.e. [-e] mode) is NOT good for memory usage evaluation.

- In [-e] mode, the [-r] and [-x] options will be enabled automatically.


By using [-H] option you can set the initial size of hash tables at each node. E.g. here we set it to '16'.
    
    $ ./Bt -i <file_path> -n <number_of_records_to_process> -H 16


## Additiional Notes:
- You can draw a graph of generated trie by enabling [-R] option (report mode). After running the
  program in report mode, run `render.sh` script to see the visualized representation of generated
  trie.
- To show the graph we use **dot** and **xpdf** tools. Here are some useful tips about **xpdf**:  
    - ZOOM IN/OUT = `Ctrl` + `+`/`-`
    - Slide = Keep the screen with mouse and slide
