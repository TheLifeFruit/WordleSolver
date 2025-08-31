#include "../include/WordleSolver.h"
#include <algorithm>
#include <cmath>
#include <iostream>
// #include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <array>

#include "WordleGame.h"
#include "FeedbackStrategy.h"

WordleSolver::WordleSolver(std::unique_ptr<WordleGame> m_game) {
    if (m_game->wordList.empty()) {
        throw WordListEmptyException();
    }
    m_feedbackStrategy = std::make_unique<FeedbackStrategy>();
    allWords = m_game->wordList;
    for (auto& word : allWords) {
      std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    }
    possibleWords = allWords;
    game = std::move(m_game);
    // -1: no information, maxLetters[2] == 2 -> letter c can not have more then 2 letters
    maxLetters.fill(-1);
}

std::vector<Feedback> WordleSolver::getStoredFeedback(int attempt) const {
  if (attempt < 0 || attempt >= storedFeedback.size()) {
    throw std::out_of_range("[ERROR] Invalid attempt number when accessing storedFeedback!");
  }
  return storedFeedback[attempt];
}

/**
 * @brief pushes the feedback
 * @param feedback
 */
void WordleSolver::updateFeedback(const std::vector<Feedback>& feedback) {
  storedFeedback.push_back(feedback);
  //std::cout << "[DEBUG] Tries inside: " << tries << ", Feedback("<< tries <<"): " << feedbackToString(storedFeedback[tries]) << '\n';
}


/**
 * @brief Calculates the Shannon entropy for a given guess word based on the current possible solutions.
 * The formula used is: H(X) = -Σ p(x) * log2(p(x)), where p(x) is the probability of each pattern.
 * The probability p(x) is calculated as the count of each pattern divided by the total number of possible words.
 * @param guess The guess word.
 * @param possibleWords The current set of possible solution words.
 * @return The expected entropy value.
 */
double WordleSolver::calculateEntropy(const std::string& guess, const std::vector<std::string>& possibleWords) const {
  // Should never happen without getting flagged before
  if (possibleWords.empty()) return 0.0;

  double entropy = 0;
  const double total = static_cast<double>(possibleWords.size());
  std::unordered_map<std::string, std::size_t> patternCount;

  for (const auto &word : possibleWords) {
    std::string pat = feedbackToString(feedbackPattern(guess, word));
    ++patternCount[pat];
  }

  // 2:
  for (const auto& [pat, count] : patternCount)
  {
    double p = count / total;
    entropy -= p * std::log2(p);
  }

  return entropy;
}


// Get LetterFrequency of a word
/**
 * @brief Get LetterFrequency of a word. Only adds letters that match the given Feedback
 * @param fdbk The feedback that needs to be given for the letter for it to count into its frequency: so if you only want to count Present letter Frequency
 */
std::array<int, 26> WordleSolver::getLetterFrequency(const std::string& word, const std::vector<Feedback>& feedback, Feedback fdbk) {
  std::array<int, 26> letterFrequency;
  letterFrequency.fill(0);

  for (int i = 0; i < 5; ++i) {
    char g = std::tolower(word[i]);
    if (feedback[i] == fdbk) {
      if (g >= 'a' && g <= 'z') {
        // Works because lowercase asci to index is 97 to 128 so difference between lowest 'a' is all we need
        letterFrequency[g - 'a']++;
      }
    }
  }
  return letterFrequency;
}


/**
 * @brief Determines the next best guess word based on maximum entropy.
 *
 * This function evaluates all remaining possible words, calculates the Shannon entropy
 * for each as a guess, and selects the word with the highest expected information gain.
 * It prints detailed entropy information for each candidate and returns the best guess.
 *
 * @throws NoValidGuessesLeftException if there are no possible words left to guess.
 * @return The next guess word with the highest entropy, or an empty string if none found.
 */
std::string WordleSolver::nextGuess() {
  // std::cout << "[DEBUG] Remaining possible Words: " << possibleWords.size() << std::endl;
  // std::cout << "[DEBUG] Tries: " << tries << '\n' ;

  if(game->getTries() >= game->getMaxTries()) {
    // Won't work if WordleGame::guess is never used (currently feedbackStrategy handles guesses)
    throw NoValidGuessesLeftException();
  }

  // precomputed: https://www.youtube.com/watch?v=fRed0Xmc2Wg
  // Check out README.txt for more info
  if (tries == 0) {
    tries++;
    return "slate";
  }

  std::string nextGuess;
  std::vector<std::string> topGuesses;
  std::unordered_map<std::string, double> entropyMap;
  double entropy;
  double maxEntropy = -1.0;

  for (const std::string& word : possibleWords) {
    entropy = calculateEntropy(word, possibleWords);
    entropyMap[word] = entropy;
  }


  for (const auto& [word, entropy] : entropyMap) {
    if(entropy > maxEntropy) {
      maxEntropy = entropy;
      nextGuess = word;
    }
  }

  int CorrectCount = 0;
  bool repeating = false;

  for (Feedback fbk : getStoredFeedback(tries - 1) ) {
    if (fbk == Feedback::Correct) {
      CorrectCount++;
    }
  }


  // Checks if Solver gets stuck on same Feedback twice
  if (tries > 1) {
    if (getStoredFeedback(tries - 2) == getStoredFeedback(tries - 1)) {
      repeating = true;
      // std::cout << "[DEBUG] Repeating feedback: " << feedbackToString(storedFeedback[tries]) << '\n';
    }
  }



  // CASE 1: 4 Correct slots
  if (CorrectCount == 4 && tries - 1 < 4 && possibleWords.size() > 2 || repeating && CorrectCount == 3 && tries - 1 < 4 && possibleWords.size() > 2) {

    std::array<int, 26> probeLetterFrequency{};
    for (int i = 0; i < possibleWords.size(); i++) {
      std::array<int, 26> tempFreq = getLetterFrequency(possibleWords[i], getStoredFeedback(tries -1), Feedback::Absent);
      for (int j = 0; j < 26; j++) {
        probeLetterFrequency[j] = probeLetterFrequency[j] + tempFreq[j];
      }
    }
    int counter = 0;
    for (int i = 0; i < 26; ++i) {
      if (probeLetterFrequency[i] > 1) {
        probeLetterFrequency[i] = 0;
      }else {
        counter++;
      }
    }
    // std::cout << "Unique lettersFrequencies: " << counter <<'\n';


    ProbeInfo probe = findProbeWord(probeLetterFrequency, CorrectCount);
    if (CorrectCount == 4 && probe.coverage >= 2 || CorrectCount == 3 && probe.coverage >= 3) {
      nextGuess = probe.word;
    }
  }

  // Possible Improvements:
  /* Letter frequency top entropy sorting (WIP)
  if (possibleWords.size() <= 20 && possibleWords.size() > 2 && tries < 4 && maxEntropy <= ENTROPY_THRESHOLD) {
    // std::cout << "[DEBUG] maxEntropy: "<< maxEntropy << '\n';
    for (const auto& [word, entropy] : entropyMap) {
      if (entropy  >= maxEntropy-(maxEntropy * 0.80 )) {
        topGuesses.push_back(word);
        // std::cout << "[DEBUG] top Word added: "<< word << '\n';
      }
    }
  }
  */


  tries++;
  return nextGuess;
}

int WordleSolver::scoreProbe3Word(const std::string& word, const std::array<int, 26>& probeChars){
  std::unordered_set<std::string> patterns;
  std::string fdbk = feedbackToString(getStoredFeedback(tries -1));

  for (const std::string& possible_word : possibleWords) {
    for (int i = 0; i < 5; ++i) {
      fdbk[i] = (word[i] == possible_word[i]) ? '2' : '0';
    }

    for (int i = 0; i < 5; ++i) {
      if (fdbk[i] == '0') {
        for (int j = 0; j < 5; ++j)
          if (fdbk[j] != '2' && word[i] == possible_word[j])
          { fdbk[i] = '1'; break; }
      }
    }
    patterns.insert(fdbk);
  }
  return static_cast<int>(patterns.size());
}

int WordleSolver::scoreProbe4Word(const std::string& word, const std::array<int, 26>& probeChars) {
  std::array<int,26> used{};
  std::string fdbk = feedbackToString(getStoredFeedback(tries -1));

  for (const char &ch : word) {
    ++used[std::tolower(ch) - 'a'];
  }

  double score = 0;
  for (int i = 0; i < 26; ++i){
    score += std::min(used[i], probeChars[i]);
  }
  // std::cout << "[DEBUG] Score("<< word << "): " << score << '\n';
  return score;
}


/**
 * @brief Looks for a probe word with as many uncertain letters as possible
 *
 * @return Probe Word
 */
WordleSolver::ProbeInfo WordleSolver::findProbeWord(const std::array<int, 26>& probeChars, const int& correctAmount) {
  WordleSolver::ProbeInfo bestProbe;
  int cover;

  for (const std::string w : allWords) {
    if (correctAmount == 3) {
      cover = scoreProbe3Word(w, probeChars);
    }else {
      cover = scoreProbe4Word(w, probeChars);
    }

    if (cover > bestProbe.coverage) {
      bestProbe.coverage = cover;
      bestProbe.word = w;
      if (cover == 5) break;
    }
  }
  // std::cout << "[DEBUG] best Probe Score("<< bestProbe.word << "): " << bestProbe.coverage << '\n';
  return bestProbe;
}


/**
 * @brief Checks if a given word matches the feedback pattern.
 * @param word The word to check.
 * @param guess The old guess to compare letters
 * @param feedback The feedback pattern to match.
 * @return True if the word matches the feedback pattern, false otherwise.
 */
bool WordleSolver::matchesFeedback(const std::string& word, const std::string& guess, const std::vector<Feedback>& feedback) const {
  // 1) Filters out words which didn't match absent letters and the '=' feedback
  for (int i = 0; i < 5; ++i) {
    if ( (feedback[i] == Feedback::Correct  && word[i] != guess[i])   ||
         (absentLetters.count(word[i]))                              ||
         (feedback[i] == Feedback::Present && word[i] == guess[i]) ) {
      return false;
    }
  }

  // checks if all present letters are reused
  for (int i= 0; i < 26; i++) {
    int need = oldPresentLetters[i];
    if (need == 0) continue;
    for (int j = 0; j < 5; j++) {
      if (feedback[j] != Feedback::Correct && std::tolower(word[j]) == ('a' + i)) {
        need--;
      }
    }

    // if > 0 that means present letters are not reused, == 0 means all present reused, < 0 means tried with at least more of that letter
    if (need > 0) {
      return false;
    }
  }

  // Removes all words that have more then the maxLetters
  // Attempt 2: salsa
  // Feedback:  .=.=.
  // FIXES: [DEBUG] Word added: sassy -> now not possible because max(s) == 1
  for (int i = 0; i < 26; ++i) {
    int limit = maxLetters[i];       // -1 means “unlimited”
    int count = 0;
    for (const char &ch : word) {
      if (ch - 'a' == i) {
        count++;
      }
    }
    if (limit != -1 && count > limit) {
      return false;
    }
  }

  // All 3 checks must be correct so it can return true
  return true;
}

/**
 * @brief Updates the list of possible words based on the feedback from a guess.
 *        Filters out words that contain absent letters and match the feedback pattern.
 * @param guess The guessed word.
 * @param feedback The feedback vector for the guess.
 * @throws GuessEmptyException if the guess is empty.
 * @throws FeedbackEmptyException if the feedback vector is empty.
 * @throws DifferentLengthOfGuessAndFeedbackException if the guess and feedback lengths do not match.
 */
void WordleSolver::updatePossibleWords(const std::string& guess, const std::vector<Feedback>& feedback) {
  // std::cout << "[INFO] Updating possible words..." << std::endl;
  // Checks:
  if (guess.empty()) {
    throw GuessEmptyException();
  }
  if (feedback.empty()) {
    throw FeedbackEmptyException();
  }
  if (guess.length() != feedback.size()) {
    throw DifferentLengthOfGuessAndFeedbackException(guess);
  }

  if (feedback.size() != 5) {
    throw std::runtime_error("Invalid feedback size!");
  }

  // std::cout << "[DEBUG] Stored feedback size: " << storedFeedback.size() << '\n';

  // Generate letterFrequency based on the old guess:
  // !!! DO NOT keep the present list for next guesses as it may get upgraded to = later so just redo every time you do a new guess
  oldPresentLetters.fill(0);
  oldPresentLetters = getLetterFrequency(guess, feedback, Feedback::Present);

  std::vector<std::string> filtered;
  for (const std::string& word : possibleWords) {
    if (word == guess) continue;
    if (matchesFeedback(word, guess, feedback)) {
      filtered.push_back(word);
     // std::cout << "[DEBUG] Added possible Word: " << word << '\n' ;
    }
  }

  if (filtered.empty()) {
    throw std::logic_error("No candidates remain: 0 !");
  }

  // Update possibleWords: Memory inefficient ? but in this case okay since it casts?
  possibleWords = std::move(filtered);
}


/**
 * @brief Adds letters to the absentLetters set based on the guess and feedback.
 *        Letters are added only if they are marked as Absent and not present elsewhere as Correct or Present.
 * @param guess The guessed word.
 * @param feedback The feedback vector for the guess.
 */
void WordleSolver::addAbsentLetters(const std::string& guess, const std::vector<Feedback>& feedback) {
  //std::string delta = feedbackToString(feedback);
  //std::cout << "[DEBUG] Updating absent letters..." << std::endl;
  // std::cout << "[DEBUG] Feedback: " << feedbackToString(feedback) << '\n';

  if (feedback.size() != 5) {
    throw std::runtime_error("Invalid feedback size!");
  }

  for (int i = 0; i < 5; i++) {
    // could call WordleSolver::feedbackToString but who cares?
    if (feedback[i] == Feedback::Absent) {
      char c = std::tolower(guess[i]);
      bool seen_elsewhere = false;
      for (int j = 0; j < 5; ++j) {
        if (j != i && std::tolower(guess[j]) == c && (feedback[j] == Feedback::Correct || feedback[j] == Feedback::Present)) {
          seen_elsewhere = true;
          break;
        }
      }
      // IF there is 1.) a unique absent letter 2.) not in the list -> add it
      if (!seen_elsewhere && !absentLetters.count(c)) {
        absentLetters.insert(c);
        // std::cout << "[DEBUG] New absent Letter: " << c << '\n';
      }
      // std::cout << "[DEBUG] Amount of absent Letter "<< guess[i] << " already inside: " << absentLetters.count(guess[i]) << '\n';
    }
  }
}



// Improved functionality of addAbsentLetters, because simply showing the if a letter is 100% missing or dont know is bad
void WordleSolver::updateMaxLetters(const std::string& guess, const std::vector<Feedback>& feedback) {
  std::string tempWord = guess;
  for (int i = 0; i < 5; i++) {
    if (feedback[i] != Feedback::Absent) continue;
    int count = 0;
    for (int j = 0; j < 5; j++) {
      if (tempWord[j] != guess[i]) continue;
      if (feedback[j] == Feedback::Absent) {
        tempWord[j] = '_';
      } else {
        count++;
      }
    }
    if (maxLetters[guess[i] - 'a'] == -1) {
      maxLetters[guess[i] - 'a'] = count;
    }
  }
  // std::cout << "Max letter(n): "<< maxLetters[13] << '\n';
}

/**
 * @brief Generates the feedback pattern for a given guess and solution word.
 *
 * Uses the current feedback strategy to calculate the feedback vector,
 * indicating for each letter whether it is correct, present, or absent.
 *
 * @param guess The guessed word.
 * @param solution The actual solution word.
 * @return A vector of Feedback enums representing the feedback pattern.
 */
std::vector<Feedback> WordleSolver::feedbackPattern(const std::string& guess, const std::string& solution) const {
  // No neutral element so I choose Absent as it might cause less issues, if there are mistakes down the code
  std::vector<Feedback> feedback = {Feedback::Absent, Feedback::Absent, Feedback::Absent ,Feedback::Absent , Feedback::Absent};
  std::vector<char> solutionChar = {solution[0], solution[1], solution[2], solution[3], solution[4]};

  if (guess.size() != 5 || solution.size() != 5) {
    throw NotAFiveLetterWordException(guess);
  }

  // Check for correctly placed chars and remove those from the solution String
  for (int i = 0; i < 5; i++) {
    if (std::tolower(solution[i]) == std::tolower(guess[i]) ) {
      feedback[i] = Feedback::Correct;
      // Needs to replace letter so that it can't get flag it as "Present" later on
      solutionChar[i] = '_';
    }
  }

  // Check how many misspositioned letters there are
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      if (feedback[i] == Feedback::Correct) {
        break;
      }
      if (std::tolower(guess[i]) == std::tolower(solutionChar[j])) {
        feedback[i] = Feedback::Present;
        // remove this char (_) so that
        // Solution: "steal" -> Guess: "speed" -> will only set give one "1" for your e instead of "1" for each e
        solutionChar[j] = '_';
        break;
      }
    }
  }


  return feedback;
}

/**
 * @brief Converts a feedback vector to a string representation.
 * @param feedback The feedback vector.
 * @return A string encoding the feedback pattern.
 */
std::string WordleSolver::feedbackToString(const std::vector<Feedback>& feedback) {
    std::string s;
    for (auto f : feedback) {
        if (f == Feedback::Correct) s += '2';
        else if (f == Feedback::Present) s += '1';
        else s += '0';
    }
    return s;
}

/**
 * @brief Prints information about the current guessing state.
 *        Displays the number of possible solutions remaining.
 */
void WordleSolver::printGuessingInfo() const {
  std::cout << "[INFO] Determining the next guess..." << std::endl;
  std::cout << "[INFO] " << possibleWords.size() << " possible solutions remaining." << std::endl;
}

/**
 * @brief Prints the entropy results for the remaining possible words.
 * @param entropyResults A vector of pairs containing words and their corresponding entropy values.
 */
void WordleSolver::printEntropyResults(const std::vector<std::pair<std::string, double>>& entropyResults) {
  std::cout << "[INFO] Entropy results for remaining possible words (sorted):" << std::endl;
  for (const auto& [word, entropy] : entropyResults) {
    std::ostringstream oss;
    oss << "  Entropy for \"" << word << "\": ";
    if (entropy < 0.01 && entropy > 0.0) {
      oss << std::scientific << entropy << " bits";
    } else {
      oss << std::fixed << entropy << " bits";
    }
    std::cout << oss.str() << std::endl;
  }
}

/**
 * @brief Prints the best guess word based on maximum entropy.
 * @param guess The best guess word.
 */
// Useless ahh function
void WordleSolver::printBestGuess(const std::string& guess) {
    std::cout << "[INFO] Best guess: \"" << guess << "\"" << std::endl;
}

/**
 * @brief Prints an error message when no guess with maximum entropy could be found.
 */
void WordleSolver::printErrorNoMaximumEntropy() {
    std::cerr << "[ERROR] Could not find a guess word with maximum entropy!" << std::endl;
}