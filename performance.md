# High-Performance Corpus Search Benchmark

## Overview

This repository contains benchmarking results for our high-performance corpus search implementation. The benchmark tests were conducted on a corpus of varying sizes to evaluate search performance and scalability.

## Performance Metrics

The corpus size used for the primary benchmark was 4,568,905 tokens. throughput is measured in tokens per second using:

```
Average throughput = Number of Tokens / Average Time
```

### Query Performance

Performance for specific query patterns:

| Query | throughput (tokens/second) |
|-------|------------------------|
| `[lemma="house" pos!="VERB"]` | 2.08 × 10¹⁰ |
| `[word="the"] [] []` | 5.7 × 10⁸ |
| `[lemma="poop"] [lemma="scoop"] [lemma="and"]` | 2.28 × 10¹² |

## Scalability Analysis

Assuming query match speed doesn't scale with corpus size, a corpus with approximately 2.08 × 10¹⁰ tokens would result in a 1-second latency for the query `[lemma="house" pos!="VERB"]`.

Most searches utilize binary search algorithms. Theoretically, as corpus size grows, the throughput (tokens/second) should not decrease.

## Benchmark Results with Multiple Corpus Sizes

The benchmark was run with corpus sizes of [1,142,226, 2,284,452, 4,568,905, 9,137,810, 18,275,620] tokens.

| Number of Tokens | Time (ms) | Throughput (tokens/second) |
|------------------|-----------|----------------------------|
| 1,142,226 | 0.0402384 | 2.839 × 10¹⁰ |
| 2,284,452 | 0.0913834 | 2.5 × 10¹⁰ |
| 4,568,905 | 0.22 | 2.0768 × 10¹⁰ |
| 9,137,810 | 0.454898 | 2.00876 × 10¹⁰ |

## Key Findings

- The throughput doesn't decrease as corpus size increases
- Counterintuitively, query search speed increases as the corpus doubles in size, up to approximately 9,137,810 tokens
- At larger corpus sizes, the throughput stabilizes around 2 × 10¹⁰ tokens/second
- For the limited data points analyzed, search time appears to increase somewhat linearly with corpus size
- The throughput does not decrease as the corpus grows

Based on these findings, the corpus size required for a 1-second latency for the query `[lemma="house" pos!="VERB"]` should be close to 2 × 10¹⁰ tokens.
