//
// Created by Noel Hedlund on 2024-10-11.
//
#include <string>
#include <chrono>
#include "corpus.h"

// Display functions
std::string get_input();
void handle_input(const Corpus& corpus, const std::string& query_string);
void display_matches(const Corpus& corpus, const std::vector<Match>& matches);

const std::string COLOR_RED = "\033[1;31m";
const std::string COLOR_GREEN = "\033[1;32m";
const std::string COLOR_RESET = "\033[0m";
const std::string BOLD_UNDERLINE = "\033[1;4m";


//---------------------------------  DISPLAY FUNCTIONS BELOW  ------------------------------------------------------
void benchmark(const Corpus &corpus, const Query &query, const std::string &query_string, int runs = 1) {
	if (runs <= 0) {
		throw std::invalid_argument("Number of runs must be positive.");
	}

	double time = 0.0;

	for (int i = 0; i < runs; ++i) {
		auto start = std::chrono::high_resolution_clock::now();

		std::vector<Match> results = match2(corpus, query);

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;

		time += elapsed.count();
	}

	double average_time = time / runs;
	std::cout << query_string << " Time taken (average over " << runs << " runs): "
			  << average_time << " ms" << std::endl;
}

void run_benchmark() {
	const std::string corpus_filename = "bnc-05M.csv";

	// Load the corpus with error handling
	Corpus corpus;
	try {
		corpus = load_corpus(corpus_filename);
	} catch (const std::exception &e) {
		std::cerr << "Error loading corpus: " << e.what() << std::endl;
		return;
	}

	// Parse queries
	const Query q1 = parse_query("[lemma=\"house\" pos!=\"VERB\"]", corpus);
	const Query q2 = parse_query("[word=\"the\"] [] []", corpus);
	const Query q3 = parse_query("[lemma=\"poop\"] [lemma=\"scoop\"] [lemma=\"and\"]", corpus);

	// Run benchmarks with a specified number of runs for more accurate results
	const int runs = 25;
	benchmark(corpus, q1, "[lemma=\"house\" pos!=\"VERB\"]", runs);
	benchmark(corpus, q2, "[word=\"the\"] [] []", runs);
	benchmark(corpus, q3, "[lemma=\"poop\"] [lemma=\"scoop\"] [lemma=\"and\"]", runs);
}

std::string get_input() {
	std::string query_string;
	std::cout << "\nEnter a query (or leave empty to exit): ";
	std::getline(std::cin, query_string);
	return query_string;
}

void handle_input(const Corpus& corpus, const std::string& query_string) {
	try {
		std::vector<Match> matches;
		try
		{
			matches = match2(corpus, parse_query(query_string,corpus));
		}catch(const std::logic_error& e){
			std::cout << COLOR_RED << "No matches found." << COLOR_RESET << std::endl;
		}

		if (!matches.empty())
			display_matches(corpus, matches);

	} catch (const std::invalid_argument& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void display_matches(const Corpus& corpus, const std::vector<Match>& matches) {
	size_t displayed_matches = std::min(matches.size(), static_cast<size_t>(10));
	std::cout << "Found " << matches.size() << " matches. Showing first " << displayed_matches << std::endl;

	for (int i = 0; i < displayed_matches; ++i) {
		const Match& match = matches[i];
		int sentence_start = corpus.sentences[match.sentence];
		int sentence_end = (match.sentence + 1 < corpus.sentences.size()) ? corpus.sentences[match.sentence + 1] : corpus.tokens.size();

		std::cout << BOLD_UNDERLINE << "Match " << (i + 1) << COLOR_RESET <<" in sentence " << match.sentence + 1 << ": ";

		for (int j = sentence_start; j < sentence_end; ++j) {
			std::string word = corpus.index2string[corpus.tokens[j].word];

			if (j >= match.pos && j < match.pos + match.len) {
				std::cout << COLOR_GREEN << word << COLOR_RESET << " ";
			} else {
				std::cout << word << " ";
			}
		}
		std::cout << std::endl;
	}
}


int main()
{
	const std::string corpus_filename = "bnc-05M.csv";
	Corpus corpus;

	try {
		corpus = load_corpus(corpus_filename);
		std::cout << "Corpus loaded successfully from " << corpus_filename << std::endl;

	} catch (const std::invalid_argument& e) {
		std::cerr << "Error loading corpus: " << e.what() << std::endl;
		exit(1);
	}

	while (true)
	{
		std::string query_string = get_input();
		if (query_string.empty()) {
			std::cout << COLOR_GREEN << "Exiting program." << COLOR_RESET << std::endl;
			break;
		}

		handle_input(corpus, query_string);
	}

	return 0;
}