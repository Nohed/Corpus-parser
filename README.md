# Annotated Text Corpus Query Tool

This project implements a tool for querying an annotated text corpus efficiently. The corpus consists of sentences where each token (word, punctuation, etc.) is annotated with attributes such as:

- **word**: The actual token
- **c5**: A fine-grained word class tag
- **lemma**: The dictionary form of the word
- **pos**: A coarse-grained part-of-speech tag (e.g., noun, verb, adjective)

## Features

- **Basic Query Processor**: Allows queries based on token attributes using a simple query language.
- **Indexing for Optimization**: Reduces memory usage by storing unique strings as integers and pre-compiling attribute indexes.
- **Set-Based Query Matching**: Uses set operations (intersection, difference, complement) for efficient multi-clause queries.
- **Accelerated Query Execution**: Implements binary search-based optimizations for fast set intersections and differences.

## Query Language

A query is a sequence of clauses, each matching a token. Clauses contain attribute-based conditions:

- `[lemma="house"]` — Matches tokens with lemma "house".
- `[pos="ART"] [lemma="house"]` — Matches an article followed by "house".
- `[lemma="house" pos!="VERB"]` — Matches "house" but not as a verb.

## Optimization Techniques

- **String Deduplication**: Stores unique attribute values as integers to save memory.
- **Precomputed Indexing**: Maps attribute values to token positions for fast lookups.
- **Efficient Query Execution**: Translates queries into set operations for quick processing.
- **Binary Search Accelerations**: Speeds up intersection and difference operations when one set is significantly smaller.

## Example Corpus Format

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

## References

This project is inspired by the paper: *Efficient corpus search using unary and binary indexes* by Peter Ljunglöf and Nicholas Smallbone.

## Future Work

- Further optimizations for large-scale corpora
- Enhanced query language with more complex filters
- Parallelized query execution for better performance

