#pragma once
#include <memory>
#include <string>
#include <vector>
#include "FeedbackStrategy.h"
#include "WordleExceptions.h"

/**
 * @brief The WordleGame class encapsulates the logic for a Wordle game session.
 * It manages the secret word, number of tries, feedback calculation, and provides
 * utility methods for reading word lists and starting new games.
 */
class WordleGame {
private:
    std::string secret;
    int tries;
public:
    std::vector<std::string> wordList;
    int maxTries = 6;
    explicit WordleGame(const std::string& wordListFile);
    std::vector<Feedback> guess(const std::string& word);
    static bool isWon(const std::vector<Feedback>& feedback) ;
    [[nodiscard]] int getTries() const; // why do you insist on calling Feedback strategy ???? I seriously do NOT get it. This will never result in 0 if guess isn't called in main. Am I supposed to change that architecture?
    [[nodiscard]] int getMaxTries() const;
    [[nodiscard]] int getWordLength() const;
    [[nodiscard]] const std::string& getSecret() const { return secret; }
    static std::vector<std::string> readWordList(const std::string& filename);
    static std::string chooseRandomSecret(const std::vector<std::string>& wordList);
    std::unique_ptr<FeedbackStrategy> feedbackStrategy;
};
