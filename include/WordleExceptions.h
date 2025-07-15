//
// Created by Marcel Auer on 18.06.2025.
//

#ifndef WORDLEEXCEPTIONS_H
#define WORDLEEXCEPTIONS_H
#include <stdexcept>

class NotAFiveLetterWordException : public std::runtime_error {
  public:
    explicit NotAFiveLetterWordException(const std::string& word)
        : std::runtime_error("[ERROR] The entered word '" + word + "' is not a five-letter word.") {}
};

class GuessLimitReachedException : public std::runtime_error {
  public:
    GuessLimitReachedException()
        : std::runtime_error("[ERROR] You have reached the maximum number of guesses allowed.") {}
};

class NotAValidWordException : public std::runtime_error {
  public:
    explicit NotAValidWordException(const std::string& word)
        : std::runtime_error("[ERROR] The entered word '" + word + "' is not a valid guess.") {}
};

class WordListEmptyException : public std::runtime_error {
  public:
        WordListEmptyException()
                : std::runtime_error("[ERROR] The word list is empty. Please check the word list file.") {}
};

class NoValidGuessesLeftException : public std::runtime_error {
  public:
        NoValidGuessesLeftException()
                : std::runtime_error("[ERROR] There are no valid guesses left. Please check the word list.") {}
};

class DifferentLengthOfGuessAndFeedbackException : public std::runtime_error {
  public:
        explicit DifferentLengthOfGuessAndFeedbackException(const std::string& guess)
                : std::runtime_error("[ERROR] The length of the guess '" + guess + "' does not match the length of the feedback vector.") {}
};

class GuessEmptyException : public std::runtime_error {
  public:
    GuessEmptyException()
      : std::runtime_error("[ERROR] The guess cannot be empty. Please enter a valid word.") {}
};

class FeedbackEmptyException : public std::runtime_error {
  public:
        FeedbackEmptyException()
          : std::runtime_error("[ERROR] The feedback vector cannot be empty.") {}
};
#endif //WORDLEEXCEPTIONS_H
