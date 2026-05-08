#include "../include/WordleSolver.h"
#include <algorithm>

#include <iostream>

#include <stdexcept>
#include <unordered_map>
#include <array>
#include <random>
#include <fstream>

#include "WordleExceptions.h"
#include "FeedbackStrategy.h"

static const std::array<std::array<std::string, 5>, 21> startingGuesses = {{
  {{ "", "", "", "", "" }}, // 0
  {{ "", "", "", "", "" }}, // 1
  {{ "", "", "", "", "" }}, // 2
  {{ "", "", "", "", "" }}, // 3
  {{ "aloe",  "tear",  "rate",  "sale",  "roam"  }}, // 4
  {{ "salet", "crane", "slate", "trace", "raise" }}, // 5
  {{ "radios", "senior", "altern", "retail", "orient" }}, // 6
  {{ "toadies", "staring", "relates", "retails", "oration" }}, // 7
  {{ "calories", "relation", "creation", "reaction", "tailored" }}, // 8
  {{ "relations", "striation", "alternate", "retailers", "assertion" }}, // 9
  {{ "cigarettes", "alternates", "restrained", "trailblaze", "threadings" }}, // 10
  {{ "clandestine", "orientation", "alternation", "stereotyped", "neutralized" }}, // 11
  {{ "relationship", "derelictions", "constructions", "intermediate", "organized" }}, // 12
  {{ "congratulates", "consideration", "determination", "revolutionary", "international" }}, // 13
  {{ "congratulatory", "identification", "representation", "administration", "rehabilitation" }}, // 14
  {{ "bacteriologists", "synchronization", "standardization", "proportionality", "professionalism" }}, // 15
  {{ "interpenetration", "responsibilities", "characterization", "overintellectual", "sentimentalizing" }}, // 16
  {{ "comprehensibility", "institutionalized", "misinterpretation", "telecommunication", "overenthusiastic" }}, // 17
  {{ "anticonstitutional", "incomprehensible", "characteristically", "compartmentalizing", "institutionalizing" }}, // 18
  {{ "chlorofluorocarbons", "unconstitutionality", "hypercharacteristic", "interchangeability", "disproportionality" }}, // 19
  {{ "buckminsterfullerene", "comprehensibilities", "institutionalization", "counterrevolutionary", "uncharacteristically" }}  // 20
}};



WordleSolver::WordleSolver(const std::string& wordListFile) {
  wordList = readWordList(wordListFile);

  if (wordList.empty()) {
    throw WordListEmptyException();
  }

  secret = chooseRandomSecret(wordList);
  tries = 0;

  for (auto& word : wordList) {
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
  }
  possibleWords = wordList;

  // -1: no information, maxLetters[2] == 2 -> letter c can not have more then 2 letters
  maxLetters.fill(-1);
}







std::vector<Feedback> WordleSolver::getStoredFeedback(int attempt) const {
  if (attempt < 0 || attempt >= storedAttempts.size()) {
    throw std::out_of_range("[ERROR] Invalid attempt number when accessing storedFeedback!");
  }
  return storedAttempts[attempt].second;;
}




/**
 * @brief pushes the attempt and feedback
 * @param feedback
 */
void WordleSolver::storeAttempt(const std::string& attempt , const std::vector<Feedback>& feedback) {
  storedAttempts.push_back({attempt, feedback});
  tries++;
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

  for (int i = 0; i < word.length(); ++i) {
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

  if(tries >= maxTries) {
    // Won't work if WordleGame::guess is never used (currently feedbackStrategy handles guesses)
    throw NoValidGuessesLeftException();
  }

  size_t currentWordLength = possibleWords[0].length();

  // precomputed: https://www.youtube.com/watch?v=fRed0Xmc2Wg
  // Check out README.txt for more info
  if (tries == 0) {
    return startingGuesses[currentWordLength][0];
  }


  std::string nextGuess;
  std::vector<std::string> topGuesses;
  std::unordered_map<std::string, double> entropyMap;
  double entropy;
  double maxEntropy = -1.0;
  int CorrectCount = 0;
  bool repeating = false;


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



  std::vector<Feedback> lastFeedback = getStoredFeedback(tries - 1);
  for (Feedback fbk : lastFeedback) {
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


  bool shouldProbe = (CorrectCount >= static_cast<int>(currentWordLength) - 1 && possibleWords.size() > 2);

  if (shouldProbe || (repeating && CorrectCount >= static_cast<int>(currentWordLength) - 2)) {
    std::array<int, 26> probeLetterFrequency{};
    probeLetterFrequency.fill(0);

    for (const auto& pWord : possibleWords) {
      std::array<int, 26> tempFreq = getLetterFrequency(pWord, lastFeedback, Feedback::Absent);
      for (int j = 0; j < 26; j++) {
        probeLetterFrequency[j] += tempFreq[j];
      }
    }

    for (int i = 0; i < 26; ++i) {
      if (probeLetterFrequency[i] > 1) probeLetterFrequency[i] = 1;
    }

    ProbeInfo probe = findProbeWord(probeLetterFrequency, CorrectCount);

    int uniqueLettersInPlay = 0;
    for (int count : probeLetterFrequency) {
      if (count > 0) uniqueLettersInPlay++;
    }

    int threshold = (uniqueLettersInPlay > 4) ? 3 : 2;
    if (probe.coverage >= threshold) {
      nextGuess = probe.word;
    }
  }

 /*
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

  */


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


  return nextGuess;
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
std::vector<std::string> WordleSolver::nextBestGuesses(int amount) {
  // std::cout << "[DEBUG] Remaining possible Words: " << possibleWords.size() << std::endl;
  // std::cout << "[DEBUG] Tries: " << tries << '\n' ;

  std::vector<std::string> topGuesses;
  size_t currentWordLength = possibleWords[0].length();


  // precomputed: https://www.youtube.com/watch?v=fRed0Xmc2Wg
  // Check out README.txt for more info
  if (tries == 0) {
    for (int i = 0; i < amount; i++) {
      topGuesses.push_back(startingGuesses[currentWordLength][i]);
    }
    return topGuesses;
  }


  if (possibleWords.size() <= amount) {
    for (const std::string& word : possibleWords) {
      topGuesses.push_back(word);
    }
    return topGuesses;
  }


  std::unordered_map<std::string, double> entropyMap;
  std::vector<std::pair<std::string, double>> bestWords;
  double entropy;
  double minTopEntropy = -1.0;


  for (const std::string& word : possibleWords) {
    entropy = calculateEntropy(word, possibleWords);
    entropyMap[word] = entropy;
  }


  for (const auto& [word, entropy] : entropyMap) {
    if (entropy > minTopEntropy || bestWords.size() < amount) {
      bestWords.push_back({word, entropy});

      // Sort descending so the smallest value is at the back
      std::sort(bestWords.begin(), bestWords.end(), [](auto &a, auto &b) {
              return a.second > b.second;
      });

      if (bestWords.size() > amount) {
              bestWords.pop_back();
      }

      minTopEntropy = bestWords.back().second;
    }
  }

  std::sort(bestWords.begin(), bestWords.end(), [](auto &a, auto &b) {
    return a.second > b.second;
  });

  for (int i = 0; i < std::min(amount, (int)bestWords.size()); ++i) {
    topGuesses.push_back(bestWords[i].first);
  }



  // CHECK PROBES
  int CorrectCount = 0;
  bool repeating = false;

  std::vector<Feedback> lastFeedback = getStoredFeedback(tries - 1);
  for (Feedback fbk : lastFeedback) {
    if (fbk == Feedback::Correct) {
      CorrectCount++;
    }
  }


  // Checks if Solver gets stuck on same Feedback twice
  /*
  if (tries > 1) {
    if (getStoredFeedback(tries - 2) == getStoredFeedback(tries - 1)) {
      repeating = true;
      // std::cout << "[DEBUG] Repeating feedback: " << feedbackToString(storedFeedback[tries]) << '\n';
    }
  }
  */

  bool shouldProbe = (CorrectCount >= static_cast<int>(currentWordLength) - 1 && possibleWords.size() > 2);
  if (shouldProbe) {
    std::array<int, 26> probeFreq = getProbeLetterFrequency(lastFeedback);
    auto probes = findBestProbeWords(probeFreq, CorrectCount, amount);

    // Insert probes at the beginning of the list if they are highly effective
    for (const auto& p : probes) {
      // Threshold: Probe is useful if it tests at least 2 unknown letters
      if (p.second >= 2) {
        // Avoid duplicates if the probe word was already in topGuesses
        if (std::find(topGuesses.begin(), topGuesses.end(), p.first) == topGuesses.end()) {
          topGuesses.insert(topGuesses.begin(), p.first);
        }
      }
    }
  }


  /*
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


    std::vector<std::pair<std::string, int>> probeWords = findBestProbeWords(probeLetterFrequency, CorrectCount, amount);

    for (const auto& [word, count] : probeWords) {
     if (CorrectCount == 4 && count >= 2 || CorrectCount == 3 && count >= 3) {
       topGuesses.push_back(word);
    }
  }


  }

  // Possible Improvements:
   Letter frequency top entropy sorting (WIP)
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


  return topGuesses;
}




std::array<int, 26> WordleSolver::getProbeLetterFrequency(const std::vector<Feedback>& lastFeedback) {
  std::array<int, 26> freq;
  freq.fill(0);

  for (const std::string& word : possibleWords) {
    // Use your existing logic: only count letters at indices marked 'Absent'
    std::array<int, 26> temp = getLetterFrequency(word, lastFeedback, Feedback::Absent);
    for (int i = 0; i < 26; ++i) {
      // If the letter exists in this candidate at an unknown position, mark it
      if (temp[i] > 0) freq[i] = 1;
    }
  }
  return freq;
}



int WordleSolver::scoreProbe3Word(const std::string& word, const std::array<int, 26>& probeChars){
  std::unordered_set<std::string> patterns;
  std::string fdbk = feedbackToString(getStoredFeedback(tries -1));

  for (const std::string& possible_word : possibleWords) {
    for (int i = 0; i < word.length(); ++i) {
      fdbk[i] = (word[i] == possible_word[i]) ? '2' : '0';
    }

    for (int i = 0; i < word.length(); ++i) {
      if (fdbk[i] == '0') {
        for (int j = 0; j < word.length(); ++j)
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


int WordleSolver::scoreProbeEntropy(const std::string& word, const std::array<int, 26>& probeChars) {
	std::unordered_set<std::string> patterns;
	size_t L = word.length();

	for (const std::string& possible_word : possibleWords) {
		// Use the already dynamic feedbackPattern logic
		std::vector<Feedback> pattern = feedbackPattern(word, possible_word);
		patterns.insert(feedbackToString(pattern));
	}
	return static_cast<int>(patterns.size());
}

int WordleSolver::scoreProbeCoverage(const std::string& word, const std::array<int, 26>& probeChars) {
	std::array<int, 26> used{};
	used.fill(0);

	for (const char &ch : word) {
		int index = std::tolower(static_cast<unsigned char>(ch)) - 'a';
		if (index >= 0 && index < 26) {
			++used[index];
		}
	}

	int score = 0;
	for (int i = 0; i < 26; ++i) {
		// Only count the overlap between the probe word and the letters we need to test
		score += std::min(used[i], probeChars[i]);
	}
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
      if (cover == bestProbe.word.length()) break;
    }
  }
  // std::cout << "[DEBUG] best Probe Score("<< bestProbe.word << "): " << bestProbe.coverage << '\n';
  return bestProbe;
}




/**
 * @brief Looks for a probe word with as many uncertain letters as possible
 *
 * @return Probe Word
 */
std::vector<std::pair<std::string, int>> WordleSolver::findBestProbeWords(const std::array<int, 26>& probeChars, const int& correctAmount, const int& amount) {
  WordleSolver::ProbeInfo bestProbe;
  int cover;
  std::vector<std::pair<std::string, int>> bestProbeWords;

  for (const std::string w : allWords) {
    if (correctAmount == 3) {
      cover = scoreProbe3Word(w, probeChars);
    }else {
      cover = scoreProbe4Word(w, probeChars);
    }

    if (cover > bestProbe.coverage || bestProbeWords.size() < amount) {
      bestProbeWords.push_back({w, cover});
      std::sort(bestProbeWords.begin(), bestProbeWords.end(), [](auto &a, auto &b) {
	 return a.second > b.second;
      });

      if (bestProbeWords.size() > amount) {
	bestProbeWords.pop_back();
      }

      bestProbe.coverage = bestProbeWords.front().second;
      bestProbe.word = bestProbeWords.front().first;
      if (cover == bestProbe.word.length()) break;
    }
  }


  // std::cout << "[DEBUG] best Probe Score("<< bestProbe.word << "): " << bestProbe.coverage << '\n';
  return bestProbeWords;
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
  for (int i = 0; i < guess.length(); ++i) {
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
    for (int j = 0; j < guess.length(); j++) {
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


std::vector<std::string> WordleSolver::getPossibleWords() {
      return possibleWords;
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

  if (feedback.size() != guess.length()) {
    throw std::runtime_error("Invalid feedback/guess length!");
  }

  for (int i = 0; i < guess.length(); i++) {
    // could call WordleSolver::feedbackToString but who cares?
    if (feedback[i] == Feedback::Absent) {
      char c = std::tolower(guess[i]);
      bool seen_elsewhere = false;
      for (int j = 0; j < guess.length(); ++j) {
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
  for (int i = 0; i < guess.length(); i++) {
    if (feedback[i] != Feedback::Absent) continue;
    int count = 0;
    for (int j = 0; j < guess.length(); j++) {
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
  const size_t wordLength = solution.length();

  if (guess.size() != wordLength) {
    throw WordLengthMismatchException(guess);
  }


  std::vector<Feedback> feedback(wordLength, Feedback::Absent);

  std::vector<char> solutionChar(solution.begin(), solution.end());


  for (size_t i = 0; i < wordLength; ++i) {
    if (std::tolower(guess[i]) == std::tolower(solutionChar[i])) {
      feedback[i] = Feedback::Correct;
      solutionChar[i] = '_';
    }
  }

  // Pass 2: Check for misplaced letters (Yellow)
  for (size_t i = 0; i < wordLength; ++i) {
    // Skip if already marked Correct
    if (feedback[i] == Feedback::Correct) {
      continue;
    }

    for (size_t j = 0; j < wordLength; ++j) {
      if (std::tolower(guess[i]) == std::tolower(solutionChar[j])) {
        feedback[i] = Feedback::Present;
        // Mark this specific letter in solution as used
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


/**
 * @brief Selects a random secret word from a word list.
 * @param wordList The list of possible words.
 * @return A randomly chosen word.
 * @throws std::invalid_argument if the word list is empty.
 */
std::string WordleSolver::chooseRandomSecret(const std::vector<std::string>& wordList) {
  if (wordList.empty()) {
    throw WordListEmptyException();
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, wordList.size() - 1);
  return wordList[dis(gen)];
}


/**
 * @brief Reads a word list from a file.
 * @param filename The path to the word list file.
 * @return A vector of words.
 */
std::vector<std::string> WordleSolver::readWordList(const std::string& filename) {
  std::vector<std::string> words;
  std::ifstream file(filename);
  std::string word;
  while (file >> word) {
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    words.push_back(word);
  }
  return words;
}


/**
 * @brief Returns the number of tries used so far.
 * Useless in this implementation: If Main was not constructed by my teammates then this will naturally just give 0 tries when accessed by the solver
 */
int WordleSolver::getTries() const { return tries; }

/**
 * @brief Returns the maximum number of allowed tries.
 */
int WordleSolver::getMaxTries() const { return maxTries; }

/**
 * @brief Returns the length of the secret word.
 */
int WordleSolver::getWordLength() const { return static_cast<int>(secret.size()); }
