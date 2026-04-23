#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <array>
#include "FeedbackStrategy.h"
#include "WordleGame.h"

class WordleSolver {

public:
  int tries = 0;
  std::vector<std::vector<Feedback>> storedFeedback = {};
  std::vector<std::string> possibleWords = {};
  std::vector<std::string> allWords = {};
  std::unordered_set<char> absentLetters = {};
  std::array<int, 26> maxLetters;
  std::array<int, 26> oldPresentLetters = {};
  std::unique_ptr<FeedbackStrategy> m_feedbackStrategy;
  void updateFeedback(const std::vector<Feedback>& feedback);
  explicit WordleSolver(std::unique_ptr<WordleGame> m_game);
  std::string nextGuess();
  bool matchesFeedback(const std::string& word,
                               const std::string& guess,
                               const std::vector<Feedback>& feedback) const;
  void updatePossibleWords(const std::string& guess,
                           const std::vector<Feedback>& feedback);
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
  int scoreProbe4Word(const std::string& word, const std::array<int, 26>& probeChars);
  int scoreProbe3Word(const std::string& word, const std::array<int, 26>& probeChars);
  const std::unordered_set<char>& getAbsentLetters() const { return absentLetters; }
  std::vector<Feedback> feedbackPattern(const std::string& guess, const std::string& solution) const;
  double calculateEntropy(const std::string& guess, const std::vector<std::string>& possibleWords) const;

private:
  std::vector<Feedback> getStoredFeedback(int attempt) const;
  static std::string feedbackToString(const std::vector<Feedback>& feedback);
  void printGuessingInfo() const;
  static void printEntropyResults(
  const std::vector<std::pair<std::string, double>>& entropyResults);
  static void printBestGuess(const std::string& guess);
  static void printErrorNoMaximumEntropy();

// save game as its object inside the class
std::unique_ptr<WordleGame> game;

const double   ENTROPY_THRESHOLD  = 1.0;

};
