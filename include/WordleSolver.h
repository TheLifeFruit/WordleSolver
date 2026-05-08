#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <array>


#include "FeedbackStrategy.h"

class WordleSolver {



  public:
    int maxTries = 6;
    int tries = 0;
    std::vector<std::pair<std::string, std::vector<Feedback>>> storedAttempts = {};
    std::vector<std::vector<Feedback>> storedFeedback = {};
    std::vector<std::string> possibleWords = {};
    std::vector<std::string> allWords = {};
    std::unordered_set<char> absentLetters = {};
    std::array<int, 26> maxLetters;
    std::array<int, 26> oldPresentLetters = {};
    std::unique_ptr<FeedbackStrategy> m_feedbackStrategy;
    std::vector<std::string> wordList;


    explicit WordleSolver(const std::string& wordListFile);



    std::string chooseRandomSecret(const std::vector<std::string>& wordList);
    int getTries() const;
    int getMaxTries() const;
    int getWordLength() const;
    const std::string& getSecret() const { return secret; }
    static std::vector<std::string> readWordList(const std::string& filename);


    void storeAttempt(const std::string& attempt, const std::vector<Feedback>& feedback);
    void updateFeedback(const std::vector<Feedback>& feedback);

    std::string nextGuess();
    std::vector<std::string> nextBestGuesses(int amount);

    bool matchesFeedback(const std::string& word,
                                 const std::string& guess,
                                 const std::vector<Feedback>& feedback) const;
    void updatePossibleWords(const std::string& guess,
                             const std::vector<Feedback>& feedback);
    std::vector<std::string> getPossibleWords();
    void addAbsentLetters(const std::string& guess,
                          const std::vector<Feedback>& feedback);
    void updateMaxLetters(const std::string& guess,
                      const std::vector<Feedback>& feedback);
    std::array<int, 26> getLetterFrequency(const std::string& word,
                     const std::vector<Feedback>& feedback, Feedback fdbk);
    struct ProbeInfo {
      std::string word; // the probe word to guess
      int coverage = 0; // how many unknown letters it checks
    };
    ProbeInfo findProbeWord(const std::array<int, 26>& probeChars, const int& correctAmount);
    std::vector<std::pair<std::string, int>> findBestProbeWords(const std::array<int, 26>& probeChars, const int& correctAmount, const int& amount);


    int scoreProbeEntropy(const std::string& word, const std::array<int, 26>& probeChars);
    int scoreProbeCoverage(const std::string& word, const std::array<int, 26>& probeChars);
    std::array<int, 26> getProbeLetterFrequency(const std::vector<Feedback>& lastFeedback);


    int scoreProbe4Word(const std::string& word, const std::array<int, 26>& probeChars);
    int scoreProbe3Word(const std::string& word, const std::array<int, 26>& probeChars);
    const std::unordered_set<char>& getAbsentLetters() const { return absentLetters; }
    std::vector<Feedback> feedbackPattern(const std::string& guess, const std::string& solution) const;
    double calculateEntropy(const std::string& guess, const std::vector<std::string>& possibleWords) const;


    const double   ENTROPY_THRESHOLD  = 1.0;


  private:
    std::vector<Feedback> getStoredFeedback(int attempt) const;
    static std::string feedbackToString(const std::vector<Feedback>& feedback);
    void printGuessingInfo() const;
    static void printEntropyResults(
    const std::vector<std::pair<std::string, double>>& entropyResults);
    static void printBestGuess(const std::string& guess);
    static void printErrorNoMaximumEntropy();
    std::string secret;

};
