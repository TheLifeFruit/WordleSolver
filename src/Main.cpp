#include "../include/WordleGame.h"
#include "../include/WordleSolver.h"
#include "../include/FeedbackStrategy.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <random>
#include <memory>

static std::string feedbackToString(const std::vector<Feedback>& feedback) {
  std::string s;
  for (auto f : feedback) {
    if (f == Feedback::Correct) s += '2';
    else if (f == Feedback::Present) s += '1';
    else s += '0';
  }
  return s;
}

int main() {
  double average = 0.0;
  int runs = 4000;
  int fails = 0;
  std::array<int, 6> tries = {0, 0, 0, 0, 0, 0};
  for (int g=0; g < runs; g++) {
    try {
      auto feedbackStrategy = std::make_unique<FeedbackStrategy>();

      std::unique_ptr<WordleGame> game = std::make_unique<WordleGame>("../data/word-bank.csv");
      int maxTries = game->getMaxTries();
      std::string secret = game->getSecret();
      const auto solver = std::make_unique<WordleSolver>(std::move(game));

      // std::cout << "[DEBUG] Secret word is: " <<secret << std::endl;
      std::cout << "[INFO] Start Wordle-Solver..." << std::endl;

      for (int i = 0; i < maxTries; ++i) {
        std::string guess = solver->nextGuess();
        std::cout << "Attempt " + std::to_string(i + 1) + ": " + guess << std::endl;

        std::vector<Feedback> feedback;
        try {
            feedback = solver->feedbackPattern(guess, secret);
        } catch (const std::invalid_argument& e) {
            std::cout << "Invalid Argument - Abort!" + std::string(e.what()) << std::endl;
            break;
        } catch (const std::exception& e) {
            std::cout << "Exception - Abort! " + std::string(e.what()) << std::endl;
            break;
        }
        // std::cout << "Feedback: " << feedbackToString(feedback) << '\n';
        solver->updateFeedback(feedback);

        // Update wrong letters:
        solver->addAbsentLetters(guess, feedback);
        solver->updateMaxLetters(guess, feedback);


        std::string fb;
        for (auto f : feedback) {
            if (f == Feedback::Correct) fb += "=";
            else if (f == Feedback::Present) fb += "-";
            else if (f == Feedback::Absent) fb += ".";
            else fb += "?";
        }
        std::cout << "Feedback:  " + fb << std::endl;

        // Game Flags: Win/Loss
        if (std::all_of(feedback.begin(), feedback.end(), [](Feedback f) { return f == Feedback::Correct; })) {
          if (i+1 != 0) {
            std::cout << "[INFO] Won in " + std::to_string(i + 1) + " attempts! The solution was: " + secret << std::endl;
          } else {
            std::cout << "[INFO] Won in 1 attempt! The solution was: " + secret << std::endl;
          }

          average += i + 1;
          tries[i]++;
          break;
        }

        if (i == maxTries - 1 || guess.empty()) {
          std:: cout << "[INFO] You lost the game! The solution was: " + secret << std::endl;
          average += 7;
          fails++;
          break;
        }

        // only update AFTER the game has checked if the loops is done
        solver->updatePossibleWords(guess, feedback);

      }
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 2;
    }

  } // End of Game Loop
  average = average / runs;
  std::cout << "[INFO] Games: " << runs << std::endl;
  std::cout << "[INFO] Game Average Tries: " << average << std::endl;
  std::cout << "[INFO] Game Fails: " << fails << std::endl;
  std::cout << "[INFO] 1-Tries: " << tries[0] << ", 2-Tries: " << tries[1] << ", 3-Tries: " << tries[2] << ", 4-Tries: " << tries[3] << ", 5-Tries: " << tries[4] << ", 6-Tries: " << tries[5] << std::endl;
  std::cout << "[INFO] 1-Tries: " << (static_cast<double> (tries[0])/runs) *100 << "%, 2-Tries: " << static_cast<double>(tries[1])/runs*100 << "%, 3-Tries: " << static_cast<double>(tries[2])/runs*100 << "%, 4-Tries: " << static_cast<double>(tries[3])/runs*100 << "%, 5-Tries: " << static_cast<double>(tries[4])/runs*100 << "%, 6-Tries: " << static_cast<double>(tries[5])/runs*100 << "%" << std::endl;

  return 0;
}
