#include "../include/WordleGame.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <random>

WordleGame::WordleGame(const std::string& wordListFile) {
  wordList = readWordList(wordListFile);
  if (wordList.empty()) {
    throw WordListEmptyException();
  }
  secret = chooseRandomSecret(wordList);
  tries = 0;
  feedbackStrategy = std::make_unique<FeedbackStrategy>();
}


/**
 * @brief Evaluates a guess and returns the feedback.
 * @param word The guessed word.
 * @return A vector of Feedback enums for each letter.
 * @throws std::invalid_argument if the guess has the wrong length.
 */
std::vector<Feedback> WordleGame::guess(const std::string& word) {
    if (word.length() != secret.length()) {
        throw NotAFiveLetterWordException(word);
    }
    tries++;
    return feedbackStrategy->calculateFeedback(word, secret);
}

/**
 * @brief Checks if the feedback indicates a win (all letters correct).
 * @param feedback The feedback vector.
 * @return True if all letters are correct, false otherwise.
 */
bool WordleGame::isWon(const std::vector<Feedback>& feedback) {
    return std::all_of(feedback.begin(), feedback.end(),
        [](Feedback f) { return f == Feedback::Correct; });
}

/**
 * @brief Returns the number of tries used so far.
 * Useless in this implementation: If Main was not constructed by my teammates then this will naturally just give 0 tries when accessed by the solver
 */
int WordleGame::getTries() const { return tries; }

/**
 * @brief Returns the maximum number of allowed tries.
 */
int WordleGame::getMaxTries() const { return maxTries; }

/**
 * @brief Returns the length of the secret word.
 */
int WordleGame::getWordLength() const { return static_cast<int>(secret.size()); }

/**
 * @brief Reads a word list from a file.
 * @param filename The path to the word list file.
 * @return A vector of words.
 */
std::vector<std::string> WordleGame::readWordList(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    while (file >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::toupper);
        words.push_back(word);
    }
    return words;
}

/**
 * @brief Selects a random secret word from a word list.
 * @param wordList The list of possible words.
 * @return A randomly chosen word.
 * @throws std::invalid_argument if the word list is empty.
 */
std::string WordleGame::chooseRandomSecret(const std::vector<std::string>& wordList) {
    if (wordList.empty()) {
        throw WordListEmptyException();
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, wordList.size() - 1);
    return wordList[dis(gen)];
}
