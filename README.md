# TrieGranularity

[![DOI](https://zenodo.org/badge/171184745.svg)](https://zenodo.org/badge/latestdoi/171184745)


**TrieGranularity** is a free open source project, including four main programs:
- bit-level trie
- character-level trie
- component-level trie
- name generator

The main goal of this project is to provide a fair environment to compare three well-known trie granularities with
each other and clear their strengthes and weaknesses. Each trie, tries to make a trade off between speed and memory
usage as well as structural complextity.

We also introduce NameGen, a program to generate a dataset of NDN/CCN-like names. This tool provides end-users with numbers of options to tune the characterstics of the generated dataset.

For details of implementation of each tool please read:

[Chavoosh Ghasemi*, Hamed Yousefi*, Kang G. Shin, and Beichuan Zhang (*co-first authors), _On the Granularity of Trie-Based Data Structures for Name Lookups and Updates_, in IEEE/ACM Transactions on Networking, vol. 27, no. 2, pp. 777-789, April 2019.](https://ieeexplore.ieee.org/document/8673766/)
