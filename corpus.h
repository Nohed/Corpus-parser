//
// Created by Noel Hedlund on 2024-10-11.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <span>
#include <variant>
#include <ranges>
#include <algorithm>
#include <cctype>
#include <utility>
#include <fstream>
#include <numeric>

#ifndef CORPUS_H
#define CORPUS_H
/*********************************************************
 * @author
 *			Noel Hedlund (C22nhd)
 * @brief
 *			This program matches queries against a corpus of sentences.
 * @details
 *			The program loads a corpus from a file, parses a query,
 *          and finds sentences in the corpus based on a query.
 *          Sentences can be searched not only based on words but on
 *          words, c5, lemma, and pos.
 * @version
 *			1.0 - Initial implementation
 *			2.0 - Improved main function with color output
 *				- Optimization
 *				- Altered structs, for more compact and optimized code.
 *			3.0	- Query proccessor is now integer-based
 *				- Corpus is now indexed
 *				- Recreated everything related to matching, match2 is now
 *				  the primary function for matching.
 *
 *
 *
 * @date
 *			v1. 2024-09-13
 *			v2. 2024-10-04
 *			v3. 2024-10-16
 */
//*********************************************************

struct Literal;
// ----------------- ALIASES -----------------
using Clause = std::vector<Literal>;
using Query = std::vector<Clause>;

using Index = std::vector<int>;

// ----------------- STRUCTS -----------------
struct Token
{
	uint32_t word;
	uint32_t c5;
	uint32_t lemma;
	uint32_t pos;
};

struct Literal
{
	std::string attribute;
	uint32_t value;
	bool is_equality;
};

struct Corpus
{
	std::vector<Token> tokens;
	std::vector<int> sentences;
	std::vector<std::string> index2string;
	std::map<std::string, uint32_t> string2index;
	Index word_index;
	Index c5_index;
	Index lemma_index;
	Index pos_index;
};

struct Match
{
	int sentence;
	int pos;
	int len;
};

struct IndexSet
{
	std::span<const int> elems;
	int shift;
};

struct DenseSet // Empty clause
{
	int first;
	int last;
};

struct ExplicitSet
{
	std::vector<int> elems;
};

struct MatchSet // variant of sets
{
	std::variant<DenseSet, IndexSet, ExplicitSet> set;
	bool complement;
};

// ----------------- FUNCTION DECLARATIONS -----------------

// Corpus functions
Corpus load_corpus(const std::string& filename);

// Indexing
uint32_t insert_and_get_index(Corpus& corpus, const std::string& str);
Index build_index(const std::vector<Token> &tokens, uint32_t Token::* attribute);
void build_indices(Corpus &corpus);
IndexSet index_lookup(const Corpus &corpus, const std::string &attribute, uint32_t value);

// Parsing functions
Query parse_query(const std::string& text, const Corpus& corpus);
std::vector<std::string> split_clauses(const std::string& text);
std::vector<Literal> parse_clause(const std::string& text, const Corpus& corpus);

// Matching
std::vector<Match> match(const Corpus& corpus, const Query& query);
std::vector<Match> match(const Corpus& corpus, const std::string& query_string);
bool compare_token_clause(const Token& token, const Clause& clause, const Corpus& corpus);
bool compare_literal_token(const Token& token, const Literal& literal, const Corpus& corpus);

// NEW matching
std::vector<Match> match_single(const Corpus &corpus, const std::string &attr, const std::string &value);
MatchSet intersection(const MatchSet &A, const MatchSet &B);
MatchSet intersect_with_plan(std::vector<MatchSet> &sets);

MatchSet match_set(const Corpus &corpus, const Query &query);
std::vector<Match> match2(const Corpus &corpus, const Query &query);



#endif //CORPUS_H
