# Corpus Search

A high-performance tool for querying annotated text corpora using optimized set operations and indexing techniques.
## References

This project is inspired by the paper: *Efficient corpus search using unary and binary indexes* by Peter Ljunglöf and Nicholas Smallbone.

## Overview

This project implements an efficient corpus search engine inspired by the paper "Efficient corpus search using unary and binary indexes" by Peter Ljunglöf and Nicholas Smallbone. It allows users to query annotated text corpora with a simple but powerful query language, using advanced data structures and algorithms for optimal performance.

## Features

- **Efficient Indexing**: Pre-compiles indexes for fast attribute-based searches
- **Optimized Set Operations**: Uses specialized intersection and difference algorithms
- **Powerful Query Language**: Simple syntax for complex queries
- **Multiple Search Strategies**: Automatically chooses the most efficient search algorithm based on set sizes
- **Memory Optimization**: Uses integer encoding to minimize memory usage

## Corpus Format

The tool works with annotated text corpora where each token has four attributes:

- **word**: The actual characters in the word
- **c5**: A fine-grained word class tag
- **lemma**: The lemmatized (dictionary) form of the word
- **pos**: A coarse-grained part-of-speech tag (e.g., noun, verb, adjective)

Supported part-of-speech tags include SUBST (noun), ADJ, VERB, ADV, ART, PREP, PRON, CONJ, INTERJ, and various punctuation tags.
### Example corpus sentence
```
# sentence 14, Texts/A/A0/A00.xml
there   EX0     there   PRON
is      VBZ     be      VERB
no      AT0     no      ART
vaccine NN1     vaccine SUBST
or      CJC     or      CONJ
cure    VVB-NN1 cure    VERB
currently       AV0     currently       ADV
available       AJ0     available       ADJ
.       PUN     .       PUN
```
## Query Language

The query language follows this grammar:

```
<query>     ::= <clause> { <clause> } 
<clause>    ::= '[' , { <literal> } , ']' 
<literal>   ::= <attribute> , ( '=' | '!=' ) , <value> 
<attribute> ::= 'word' | 'c5' | 'lemma' | 'pos'
<value>     ::= '"' , <string> , '"'
```

Examples of valid queries:
- `[]` (matches any token)
- `[lemma="house"]` (matches tokens with lemma "house")
- `[lemma="house" pos!="VERB"]` (matches non-verb tokens with lemma "house")
- `[pos="ART"] [lemma="house"]` (matches an article followed by "house")

## Implementation Details

### Optimized Data Structures

- **Token Representation**: Integer-encoded strings for memory efficiency
- **Set Types**:
  - **IndexSet**: For indexes with a shift value
  - **ExplicitSet**: For explicit enumerations of token positions
  - **DenseSet**: For representing ranges of consecutive positions
  - **MatchSet**: Wraps other set types with a complement flag

### Search Algorithms

The project implements multiple search strategies:
- Linear intersection for similarly sized sets
- Binary search-based intersection when one set is much smaller
- Specialized algorithms for dense sets
- Automatic selection of the optimal algorithm based on set characteristics

### Optimization Techniques

- **String Deduplication**: Stores unique attribute values as integers to save memory.
- **Precomputed Indexing**: Maps attribute values to token positions for fast lookups.
- **Efficient Query Execution**: Translates queries into set operations for quick processing.
- **Binary Search Accelerations**: Speeds up intersection and difference operations when one set is significantly smaller.

### Query Processing

Queries are processed by:
1. Parsing the query string into a structured representation
2. Converting each clause and literal into appropriate set operations
3. Applying optimized intersection and difference operations
4. Converting the results back into concrete token matches

## Future Work

- Further optimizations for large-scale corpora
- Enhanced query language with more complex filters
- Parallelized query execution for better performance

## Usage

```cpp
// Load corpus from file
Corpus corpus = load_corpus("path/to/corpus.txt");

// Execute a query
std::vector<Match> results = match(corpus, "[pos=\"ART\"] [lemma=\"house\"]");

// Process results
for (const Match& m : results) {
    // Access match information
    int sentence_index = m.sentence;
    int position = m.position;
    int length = m.len;
}
```
