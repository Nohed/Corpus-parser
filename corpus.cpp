#include "corpus.h"

//-----------------------------  PARSING FUNCTIONS BELOW  ----------------------------------------------------------

/**
 * @param text A query in string format
 * @param corpus A corpus
 * @brief Parses a string into a Query object
 *
 * @attention Throws a exception if the query is empty,
 *			  the query is not in the correct format or if
 *			  the query contains tokens that are not in the corpus
 *
 * @return A query object that can be used to search the corpus.
 */
Query parse_query(const std::string& text, const Corpus& corpus)
{
	Query query;
	std::vector<std::string> clauses = split_clauses(text);
	for (const std::string& clause : clauses)
	{
		std::vector<Literal> literals = parse_clause(clause, corpus);
		Clause current_clause;
		for (const Literal& lit : literals)
		{
			current_clause.push_back(lit);
		}
		query.push_back(current_clause);
	}
	if(query.empty())
		throw std::invalid_argument("Error: Empty Query");

	return query;
}

/**
 * @param text
 * @brief Splits a string of multiple clauses into a vector of strings,
 *		  where each string is a clause.
 * @return A vector of strings each representing a clause
 */
std::vector<std::string> split_clauses(const std::string& text)
{
	std::vector<std::string> clauses;
	std::string sub_clause;
	bool in_clause = false;

	for (char ch : text)
	{
		if (ch == '[')
		{
			// Start of a new clause, reset the clause string
			if (in_clause)
			{
				// Already inside a clause, a new  '[' =  invalid input
				throw std::invalid_argument("Nested or misplaced opening bracket '[' detected");
			}
			in_clause = true;
			sub_clause.clear();
		}
		else if (ch == ']')
		{
			// End of a clause, add the clause to the vector if it's not empty
			if (in_clause)
			{
				clauses.push_back(sub_clause);
				in_clause = false;
			}
			else
			{
				throw std::invalid_argument("Mismatched or empty clause: ']' without matching '['");
			}
		}
		else if (in_clause)
		{
			sub_clause += ch;
		}
	}

	// If in_clause is still true, it means there's an unclosed clause
	if (in_clause)
	{
		throw std::invalid_argument("Missing closing bracket for a clause");
	}

	return clauses;
}

/**
 *
 * @param text A string of literals
 * @param corpus The corpus
 * @brief Takes a string of literals( A clause), splits them one by one,
 *		  Ensures correct format, and then returns a vector containing all literals
 *
 * @attention Throws an exception if the literal is not in the correct format or
 *			  if the literal dosnt exist in the corpus
 * @return A vector of Literal objects
 */
std::vector<Literal> parse_clause(const std::string& text, const Corpus& corpus) {
    std::vector<Literal> literals;
    std::istringstream stream(text);
    std::string literal;

	auto trim_and_validate_lit = [](const std::string& value) -> std::string {
		if (value.length() > 2 && value.front() == '"' && value.back() == '"')
			return value.substr(1, value.length() - 2); // Trim quotes
		else
			throw std::invalid_argument("The value is missing one or more quotes: '" + value + "'");
	};

    // Helper lambda to process and set the literal values
    auto process_literal = [&](Literal& literal_obj, size_t pos, const std::string& op) {
        literal_obj.is_equality = (op == "=");
        literal_obj.attribute = literal.substr(0, pos);
        std::string value = literal.substr(pos + op.size());
        value = trim_and_validate_lit(value);

    	auto index = corpus.string2index.find(value);
    	if (index != corpus.string2index.end())
    		literal_obj.value = index->second;
    	else
    		throw std::logic_error("Error: "+value+" does not exist in corpus");
    };

    while (getline(stream, literal, ' ')) { // Split by space to get each literal
        Literal lit;
        size_t pos = literal.find("!=");

        if (pos != std::string::npos) {
            process_literal(lit, pos, "!=");
        } else if ((pos = literal.find('=')) != std::string::npos) {
            process_literal(lit, pos, "=");
        } else {
            throw std::invalid_argument("Cannot parse literal");
        }

        // Validate the attribute
    	if (lit.attribute != "word" && lit.attribute != "c5" && lit.attribute != "lemma" && lit.attribute != "pos")
    	{
    		throw std::invalid_argument("attribute not recognized");
    	}

        literals.push_back(lit);
    }

    return literals;
}

//-----------------------------  CORPUS FUNCTIONS BELOW  ----------------------------------------------------------
/**
 *
 * @param filename Name of input file¨
 * @brief Takes a file containing a corpus of sentences and parses it into a Corpus object
 * @return Corpus object
 */
Corpus load_corpus(const std::string& filename)
{
	std::ifstream corpus_file(filename);
	if (!corpus_file) // Exit if file didnt open
		throw std::invalid_argument("Could not open file " + filename);

	Corpus corpus;
	std::string row;
	size_t token_index = 0;
	bool in_sentence = false;

	getline(corpus_file, row); // Skip header
	while (getline(corpus_file, row)) // Parse file row by row
	{
		if (row.empty() || row[0] == '#')
		{
			// If row is empty, sentence complete, set flag to later save sentence index
			if (row.empty())
			{
				in_sentence = false;
			}
			// If row is comment, skip
			continue;
		}

		std::istringstream row_content(row);
		std::string word, c5, lemma, pos;

		std::istringstream token_stream(row);
		// If full token is found, add to Corpus
		if (row_content >> word >> c5 >> lemma >> pos)
		{
			Token row_token{};
			row_token.word = insert_and_get_index(corpus, word);
			row_token.c5 = insert_and_get_index(corpus, c5);
			row_token.lemma = insert_and_get_index(corpus, lemma);
			row_token.pos = insert_and_get_index(corpus, pos);

			if (!in_sentence)
			{
				in_sentence = true;
				corpus.sentences.push_back(static_cast<int>(token_index));
			}

			corpus.tokens.push_back(row_token);
			token_index++;
		}
		// Files should include 4 string per row, if not stringstream probably failed, or file is incomplete ->EXIT
		else
		{
			throw std::invalid_argument(std::string("Error: could not parse line " + row + " of file " + filename));
		}
	}
	// Last sentence of file
	if (!row.empty())
	{
		corpus.sentences.push_back(static_cast<int>(token_index));
	}

	build_indices(corpus);
	return corpus;
}

//--------------------------------- OLD MATCHING FUNCTIONS BELOW  ------------------------------------------------------
/**
 * @brief Helper function for "match(const Corpus &corpus, const Query &query)"
 *		  This function simply creates a query object from a string and calls the main Match()  function.
 *
 * @param corpus The corpus to search.
 * @param query_string The query to match. As a string
 * @return std::vector<Match> The matches found.
 */
std::vector<Match> match(const Corpus& corpus, const std::string& query_string)
{
	return match(corpus, parse_query(query_string, corpus));
}

/**
 * @brief Matches a query against a corpus and returns the matches.
 *
 * @param corpus The corpus to search.
 * @param query The query to match.
 * @return std::vector<Match> The matches found.
 */
std::vector<Match> match(const Corpus& corpus, const Query& query)
{
	std::vector<Match> matches;

	for (size_t sentence_index = 0; sentence_index < corpus.sentences.size(); ++sentence_index)
	{
		size_t start = corpus.sentences[sentence_index];
		size_t end = (sentence_index + 1 < corpus.sentences.size()) ? corpus.sentences[sentence_index + 1] : corpus.tokens.size();

		for (size_t i = start; i < end; ++i)
		{
			bool all_clauses_match = true;
			for (size_t j = 0; j < query.size(); ++j)
			{
				if (i + j >= end || !compare_token_clause(corpus.tokens[i + j], query[j], corpus))
				{
					all_clauses_match = false;
					break;
				}
			}

			if (all_clauses_match)
			{
				matches.push_back({static_cast<int>(sentence_index),
					static_cast<int>(i), static_cast<int>(query.size())});
			}
		}
	}

	return matches;
}

/**
 *
 * @param corpus A corpus
 * @param attr A attribute of a literal in string format
 * @param value A value of a literal in string format.
 * @brief Matches a singe literal against a corpus and returns the matches.
 *
 * @return A vector of match objects, see headerfile for struture.
 */
std::vector<Match> match_single(const Corpus &corpus, const std::string &attr, const std::string &value)
{
	std::vector<Match> matches;

	auto value_element = corpus.string2index.find(value);
	if (value_element == corpus.string2index.end()) {
		// If word doesnt exist, return no matches
		return matches;
	}

	// Create a indexSet
	uint32_t value_index = value_element->second;
	IndexSet matching_tokens = index_lookup(corpus, attr, value_index);

	// Binary search the corpus sentences vector for the span in the IndexSet,
	// Then create a Match object and add to the vector to be returned
	for (const int index : matching_tokens.elems) {
		auto sentence = std::upper_bound(corpus.sentences.begin(),
			corpus.sentences.end(), index);
		int sentence_index = std::distance(corpus.sentences.begin(), sentence) - 1;

		matches.push_back({sentence_index, index, 1});
	}
	return matches;
}

/**
 * @brief Checks if a token matches any literal in a clause.
 *
 * @param token The token to check.
 * @param clause The clause to match against.
 * @param corpus The corpus
 * @return bool True if the token matches any literal in the clause, false otherwise.
 */
bool compare_token_clause(const Token& token, const Clause& clause, const Corpus& corpus)
{
	if (clause.empty())
		return true;

	size_t matching_literals = 0;
	for (const Literal& literal : clause)
	{
		if (compare_literal_token(token, literal, corpus))
		{
			matching_literals++;
		}
	}
	return matching_literals == clause.size();
}

/**
 * @brief Checks if a token matches a literal.
 *
 * @param token The token to check.
 * @param literal The literal to match against.
 * @param corpus The corpus
 * @return bool True if the token matches the literal, false otherwise.
 */
bool compare_literal_token(const Token& token, const Literal& literal, const Corpus& corpus)
{
	const std::string& attr = literal.attribute;
	const uint32_t& val = literal.value;
	bool is_equal = literal.is_equality;

	if (attr == "word") return is_equal ? token.word == val : token.word != val;
	if (attr == "c5") return is_equal ? token.c5 == val : token.c5 != val;
	if (attr == "lemma") return is_equal ? token.lemma == val : token.lemma != val;
	if (attr == "pos") return is_equal ? token.pos == val : token.pos != val;

	throw std::invalid_argument("Attribute not recognized");
}

// -----------------------------  Intersections  ----------------------------------------------------------

/**
 * Computes the intersection of two sets.
 * @param A First input set.
 * @param B Second input set
 *			- Sets can be of type IndexSet or ExplicitSet.
 * @brief  Uses binary search to find the intersection of two sets.
 *		  - Is suitable for use if A is smaller than B.
 * @return A Explicitset containing the intersection of A and B.
 */
template<typename T1, typename T2>
ExplicitSet binary_intersect_two_sets(const T1& A, const T2& B, int A_shift = 0, int B_shift = 0)
{
	ExplicitSet C;
	// Binary search B for each element in A
	for (const int x : A)
	{
		// Check if A exists in B
		if (std::binary_search(B.begin(), B.end(), x - A_shift + B_shift))
		{
			C.elems.push_back(x);
		}
	}
	return C;
}

/**
 * Computes the difference of two sets.
 * @param A First input set.
 * @param B Second input set
 *			- Sets can be of type IndexSet or ExplicitSet.
 * @brief  Uses binary search to find the difference of two sets.
 *		  - Is suitable for use if A is smaller than B.
 * @return A Explicitset containing (A difference B)
 */
template<typename T1, typename T2>
ExplicitSet binary_diff_two_sets(const T1& A, const T2& B, int A_shift = 0, int B_shift = 0) {
	ExplicitSet C;
	std::cout<<"bin diff"<<std::endl;
	// Binary search B for each element in A
	for (const int x : A) {
		// Check if x exists in B (after applying the shifts)
		if (!std::binary_search(B.begin(), B.end(), x - A_shift + B_shift)) {
			C.elems.push_back(x);
		}
	}

	return C;
}
/**
 * Computes the intersection of two sets.
 * @param A First input set.
 * @param B Second input set
 *			- Sets can be of type IndexSet or ExplicitSet.
 * @return A vector containing the intersection of A and B.
 */
template<typename T1, typename T2>
ExplicitSet intersect_two_sets(const T1& A, const T2& B, int A_shift = 0, int B_shift = 0) {
	ExplicitSet C;
	size_t p = 0, q = 0;

	while (p < A.size() && q < B.size()) {
		if (A[p] - A_shift < B[q] - B_shift) {
			++p;
		} else if (B[q] - B_shift < A[p] - A_shift) {
			++q;
		} else {
			C.elems.push_back(A[p] - A_shift); // Found an intersection
			++p;
			++q;
		}
	}
	return C;
}

/**
 * Computes the difference A \ B.
 * @param A First input set.
 * @param B Second input set
 *			- Sets can be of type IndexSet or ExplicitSet.
 * @return A vector containing all elements in A that are not in B.
 */
template<typename T1, typename T2>
ExplicitSet diff_two_sets(const T1& A, const T2& B, int A_shift = 0, int B_shift = 0) {
	ExplicitSet C;
	size_t p = 0, q = 0;

	while (q < A.size() && p < B.size()) {
		if (A[q] - A_shift < B[p] - B_shift) {
			C.elems.push_back(A[q] - A_shift); // A[q] is not in B
			++q;
		} else if (B[p] - B_shift < A[q] - A_shift) {
			++p; // Skip B[p] as it's not in A
		} else {
			++q; // Found a common element, skip it
			++p;
		}
	}

	// Add remaining elements from A
	while (q < A.size()) {
		C.elems.push_back(A[q] - A_shift);
		++q;
	}

	return C;
}
/**
 * Computes the Difference between two sets
 * @param A A dense set
 * @param B A set of type IndexSet or ExplicitSet.
 * @return A ExplicitSet containing (A difference B)
 */
template<typename T2>
ExplicitSet diff_dense_x(const DenseSet& A, const T2& B, int B_shift = 0)
{
	ExplicitSet C;

	int p = A.first;

	int q = 0;
	while(p <= A.last && q < B.size()){
		if(p < B[q] - B_shift){
			C.elems.push_back(p);
			p++;
		} else if(p > B[q] - B_shift){
			q++;
		} else{
			p++;
			q++;
		}
	}
	while(p <= A.last)
	{
		C.elems.push_back(p);
		p++;
	}
	// ALl elements in A thats not in B
	// All first-last - B

	return C;
}

/**
 * Computes the Difference between two sets
 * @param A A set of type IndexSet or ExplicitSet.
 * @param B A dense set
 * @return A ExplicitSet containing (A difference B)
 */
template<typename T1>
ExplicitSet diff_x_denseset(const T1& A, const DenseSet B, int A_shift = 0)
{
	ExplicitSet C;

	for (const int elem : A) {
		auto elem_shifted = elem - A_shift;
		if (elem_shifted < B.first || elem_shifted > B.last) {
			C.elems.push_back(elem_shifted);
		}
	}

	return C;
}

// ----- SAME SET TYPE OPERATIONS ----------------------------------------------------
ExplicitSet intersection(const ExplicitSet &A, const ExplicitSet &B){
	if(A.elems.size()*10<=B.elems.size())
		return binary_intersect_two_sets(A.elems, B.elems);
	else if(A.elems.size()>B.elems.size()*10)
		return binary_intersect_two_sets(B.elems, A.elems);
	else
		return intersect_two_sets(A.elems, B.elems);
}

// times 10 because of ....
ExplicitSet difference(const ExplicitSet &A, const ExplicitSet &B){
	if(A.elems.size()>=B.elems.size()*10)
		return binary_diff_two_sets(A.elems, B.elems);
	else
		return diff_two_sets(A.elems, B.elems);
}

ExplicitSet intersection(const IndexSet& A, const IndexSet& B) {

	if(A.elems.size()*10<=B.elems.size())
		return binary_intersect_two_sets(A.elems, B.elems, A.shift, B.shift);
	else if(A.elems.size()>B.elems.size()*10)
		return binary_intersect_two_sets(B.elems, A.elems, B.shift, A.shift);
	else
		return intersect_two_sets(A.elems, B.elems, A.shift, B.shift);
}

ExplicitSet difference(const IndexSet &A, const IndexSet &B){
	if(A.elems.size()>=B.elems.size()*10)
		return binary_diff_two_sets(A.elems, B.elems, A.shift, B.shift);
	else
		return diff_two_sets(A.elems, B.elems, A.shift, B.shift);
}

DenseSet intersection(const DenseSet& A, const DenseSet& B) {
	DenseSet D{};
	D.first = std::max(A.first, B.first);
	D.last = std::min(A.last, B.last);
	return D;
}

DenseSet difference(const DenseSet &A, const DenseSet &B){

	if (A.last <= B.first || A.first >= B.last)
		return A;

	if (A.first < B.first)
		return {A.first, B.first};

	return {0,0};
}

// ----- Index and explicit ----------------------------------------------------
ExplicitSet intersection(const IndexSet& A, const ExplicitSet& B) {
	if(A.elems.size()*10<=B.elems.size())
		return binary_intersect_two_sets(A.elems, B.elems, A.shift);

	else if(A.elems.size()>B.elems.size()*10)
		return binary_intersect_two_sets(B.elems, A.elems, 0, A.shift);
	else
		return intersect_two_sets(A.elems, B.elems, A.shift);
}
ExplicitSet intersection(const ExplicitSet& A, const IndexSet& B) {
	return intersection(B, A);
}

ExplicitSet difference(const IndexSet &A, const ExplicitSet &B){
	if(A.elems.size()>=B.elems.size()*10)
		return binary_diff_two_sets(A.elems, B.elems, A.shift);
	else
		return diff_two_sets(A.elems, B.elems, A.shift);
}
ExplicitSet difference(const ExplicitSet &A, const IndexSet &B)
{
	if(A.elems.size()>=B.elems.size()*10)
		return binary_diff_two_sets(A.elems, B.elems, 0,B.shift);
	else
		return diff_two_sets(A.elems, B.elems, 0, B.shift);
}

// ----- Dense and explicit ----------------------------------------------------
ExplicitSet intersection(const DenseSet& A, const ExplicitSet& B) {
	std::vector<int> c;
	for (const int i : B.elems){
		if(i > A.last){
			return ExplicitSet{c};
		}
		c.push_back(i);
	}
	return ExplicitSet{c};
}

ExplicitSet intersection(const ExplicitSet& A, const DenseSet& B) {
	return intersection(B, A);
}

ExplicitSet difference(const DenseSet &A, const ExplicitSet &B){

	return diff_dense_x(A,B.elems);

}

ExplicitSet difference(const ExplicitSet &A, const DenseSet &B)
{
	return diff_x_denseset(A.elems,B);
}

// ----- Index and dense ------------------------------------------------
ExplicitSet intersection(const IndexSet& A, const DenseSet& B) {
	std::vector<int> c;
	for(const int i: A.elems){
		if(i - A.shift <= B.last && i - A.shift >= B.first){
			c.push_back(i-A.shift);
		}
	}
	return ExplicitSet{c};
}

ExplicitSet intersection(const DenseSet& A, const IndexSet& B) {
	return intersection(B, A);
}

ExplicitSet difference(const DenseSet &A, const IndexSet &B)
{
	return diff_dense_x(A,B.elems, B.shift);
}
ExplicitSet difference(const IndexSet &A, const DenseSet &B)
{
	return diff_x_denseset(A.elems,B, A.shift);
}


//------------------------------------------------------------------------
/**
 *
 * @param A A MatchSet
 * @param B A MatchSet
 *
 * @brief Computes the intersection of two MatchSets
 * @return A new MatchSet containing the intersection.
 */
MatchSet intersection(const MatchSet &A, const MatchSet &B)
{
	MatchSet matchset;
	if(A.complement && B.complement) // return the compliment of (A intersect B)
	{
		matchset.complement = true;
		matchset.set = std::visit([](auto &&a, auto &&b) -> std::variant<DenseSet, IndexSet, ExplicitSet>
								{ return intersection(a, b); }, A.set, B.set);
	}
	else if(A.complement) // Return (B diff A)
	{
		matchset.complement = false;
		matchset.set = std::visit([](auto &&a, auto &&b) -> std::variant<DenseSet, IndexSet, ExplicitSet>
								{ return difference(a, b); }, B.set, A.set);
	}
	else if(B.complement) // return ( B diff A)
	{
		matchset.complement = false;
		matchset.set = std::visit([](auto &&a, auto &&b) -> std::variant<DenseSet, IndexSet, ExplicitSet>
								{ return difference(a, b); }, A.set, B.set);
	}
	else // Return (A intersect B)
	{
		matchset.complement = false;
		matchset.set = std::visit([](auto &&a, auto &&b) -> std::variant<DenseSet, IndexSet, ExplicitSet>{
		return intersection(a, b); }, A.set, B.set);
	}
	return matchset;
}


//-----------------------------  Indexing  ----------------------------------------------------------


/**
 * @brief Retrieves the index of the string from the corpus mappings.
 *		  If the string is new, its added to the mapping.
 *
 * @param corpus the corpus object
 * @param str the string to look up or add
 * @return the index corresponding to the string
 */
uint32_t insert_and_get_index(Corpus& corpus, const std::string& str)
{
	auto index = corpus.string2index.find(str);
	if (index != corpus.string2index.end()) // If string exists, return index.
	{
		return index->second;
	}
	else // If string isnt indexed, add it and return new index.
	{
		auto new_index = corpus.index2string.size();
		corpus.index2string.push_back(str);
		corpus.string2index[str] = new_index;
		return new_index;
	}
}

/**
 *
 * @param tokens
 * @param attribute
 * @return
 */
Index build_index(const std::vector<Token> &tokens, uint32_t Token::* attribute)
{
	Index index(tokens.size());
	for(int i = 0; i < tokens.size(); i++)
		index[i] = i;

	std::stable_sort(index.begin(), index.end(), [&](int a, int b) {
		return tokens[a].*attribute < tokens[b].*attribute;
	});

	return index;
}

/**
 *
 * @param corpus
 */
void build_indices(Corpus &corpus)
{
	corpus.word_index = build_index(corpus.tokens, &Token::word);
	corpus.c5_index = build_index(corpus.tokens, &Token::c5);
	corpus.lemma_index = build_index(corpus.tokens, &Token::lemma);
	corpus.pos_index = build_index(corpus.tokens, &Token::pos);
}

/**
 *
 * @param corpus
 * @param attribute
 * @param value
 * @return
 */
IndexSet index_lookup(const Corpus &corpus, const std::string &attribute, uint32_t value)
{
	// Get correct index
	const Index* index = nullptr;
	uint32_t Token::*attribute_point;

	// Set pointers for attribute type, and index
	if (attribute == "word")
	{
		index = &corpus.word_index;
		attribute_point = &Token::word;
	}
	else if (attribute == "c5")
	{
		index = &corpus.c5_index;
		attribute_point = &Token::c5;
	}
	else if (attribute == "lemma")
	{
		index = &corpus.lemma_index;
		attribute_point = &Token::lemma;
	}
	else if (attribute == "pos")
	{
		index = &corpus.pos_index;
		attribute_point = &Token::pos;
	}
	else
	{
		throw std::invalid_argument("Unknown attribute: " + attribute);
	}

	auto first = std::lower_bound(index->begin(), index->end(),
		value , [&](int a, int b) {

		return corpus.tokens[a].*attribute_point < value;
	});
	auto last = std::upper_bound(first, index->end(),
		value, [&](int a, int b) {

		return value < corpus.tokens[b].*attribute_point;
	});
	IndexSet new_set = {std::span<const int>(&*first, std::distance(first, last)), 0};
	// Return braced initializer
	return new_set;
}
// -----------------------------  NEW MATCHING FUNCTIONS  ----------------------------------------------------------

/**
 *
 * @param set A matchset
 * @return THe size of the set
 */
int find_set_size(const MatchSet &set)
{
	if (std::holds_alternative<DenseSet>(set.set))
	{
		DenseSet denseSet = std::get<DenseSet>(set.set);
		return denseSet.last - denseSet.first + 1;
	}
	else if (std::holds_alternative<IndexSet>(set.set))
	{
		IndexSet indexSet = std::get<IndexSet>(set.set);
		return static_cast<int>(indexSet.elems.size());
	}
	else if (std::holds_alternative<ExplicitSet>(set.set))
	{
		ExplicitSet explicitSet = std::get<ExplicitSet>(set.set);
		return static_cast<int>(explicitSet.elems.size());
	}
	throw std::invalid_argument("Error: Unknown set type");
}

/**
 *
 * @param sets A vector of MatchSets to intersect
 * @brief Intersects a vector of MatchSets,
 *	     - All densesets are intersected together
 *	     - ALl other sets are sorted from smallest to largest,
 *	       And intersected in order Small->Large
 *	     - Then the Two resulting sets are intersected
 * @return
 */
MatchSet intersect_with_plan(std::vector<MatchSet> &sets)
{
	std::vector<MatchSet> otherSets;
	MatchSet dense_set;
	bool dense_found = false;
	for (const auto &set : sets)
	{
		if (std::holds_alternative<DenseSet>(set.set))
		{
			if (!dense_found)
			{
				dense_set = set;
			}
			else
			{
				dense_set = intersection(dense_set, set); // Collapse DenseSets
			}
			dense_found = true;
		}
		else
		{
			otherSets.push_back(set);
		}
	}
	std::sort(otherSets.begin(),otherSets.end(), [](const MatchSet &a, const MatchSet &b) {
		return find_set_size(a) < find_set_size(b);
	});


	MatchSet returnSet;
	if(!otherSets.empty())
	{
		returnSet = otherSets[0];
		for (size_t i = 1; i < otherSets.size(); ++i)
		{
			returnSet = intersection(returnSet, otherSets[i]);
		}
		if(dense_found)
		{
			returnSet = intersection(returnSet, dense_set);
		}
	}
	else
	{
		return dense_set;
	}

	return returnSet;
}

/**
 *
 * @param corpus A corpus
 * @param literal A literal
 * @param shift The shift of the literal
 * @return A matchSet containing the matches for the literal
 */
MatchSet match_set(const Corpus &corpus, const Literal &literal, int shift)
{

	IndexSet index_set = index_lookup(corpus, literal.attribute, literal.value);
	index_set.shift = shift;
	if (!literal.is_equality)
	{
		return MatchSet{index_set, true};
	}
	else
	{
		return MatchSet{index_set, false};
	}
}

/**
 * 
 * @param corpus A corpus
 * @param clause A clause
 * @param shift Shift of the clause
 * @return A matchSet containing the matches
 */
MatchSet match_set(const Corpus &corpus, const Clause &clause, int shift)
{
	if(clause.empty())
	{
		DenseSet entire_corp = {0, static_cast<int>(corpus.tokens.size()-1)};
		return MatchSet(entire_corp, false);
	}

	std::vector<MatchSet> sets;

	for (const auto &literal : clause) // Create all sets to later intersect
	{
		sets.push_back(match_set(corpus, literal, shift));
	}
	return intersect_with_plan(sets);

}

/**
 *
 * @param corpus A corpus
 * @param query A query
 * @return A MatchSet containing the matches
 */
MatchSet match_set(const Corpus &corpus, const Query &query)
{
	if(query.empty())
		return {};

	std::vector<MatchSet> sets;
	int shift = 0;

	for (const auto &clause : query)
	{
		sets.push_back(match_set(corpus, clause, shift++));
	}

	MatchSet set = intersect_with_plan(sets);

	// Handle complements
	if(set.complement)
	{
		DenseSet entire_corp = {0, static_cast<int>(corpus.tokens.size()-1)};
		MatchSet entire_corp_match = {entire_corp, false};
		set = intersection(entire_corp_match, set);
	}
	return set;
}

/**
 *
 * @param corpus A corpus
 * @param query A query
 * @return A vector of Match objects
 */
std::vector<Match> match2(const Corpus &corpus, const Query &query)
{
	MatchSet matchSet = match_set(corpus, query);
	std::vector<Match> matches;

	if(std::holds_alternative<DenseSet>(matchSet.set))
	{
		DenseSet denseSet = std::get<DenseSet>(matchSet.set);
		for(int i = denseSet.first; i <= denseSet.last; i++)
		{
			auto sentence = std::upper_bound(corpus.sentences.begin(),
				corpus.sentences.end(), i);
			int sentence_index = std::distance(corpus.sentences.begin(), sentence) - 1;
			matches.push_back({sentence_index, i, 1});
		}
	}
	else if(std::holds_alternative<IndexSet>(matchSet.set))
	{
		IndexSet indexSet = std::get<IndexSet>(matchSet.set);
		for(int i : indexSet.elems)
		{
			auto sentence = std::upper_bound(corpus.sentences.begin(),
				corpus.sentences.end(), i);
			int sentence_index = std::distance(corpus.sentences.begin(), sentence) - 1;
			matches.push_back({sentence_index, i, 1});
		}
	}
	else if(std::holds_alternative<ExplicitSet>(matchSet.set))
	{
		ExplicitSet explicitSet = std::get<ExplicitSet>(matchSet.set);
		for(int i : explicitSet.elems)
		{
			auto sentence = std::upper_bound(corpus.sentences.begin(),
				corpus.sentences.end(), i);
			int sentence_index = std::distance(corpus.sentences.begin(), sentence) - 1;
			Match newMatch = {sentence_index, i, 1};
			newMatch.len = query.size();
			matches.push_back(newMatch);
		}
	}
	return matches;
}
