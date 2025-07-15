//
// Created by Marcel Auer on 01.06.2025.
//


#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>
#include "../include/WordleGame.h"
#include "../include/WordleSolver.h"
#include <iostream>

struct WordleSolverFixture {
  /**
   * @brief Constructor for WordleGameFixture.
   *
   * Initializes the Wordle and Solver instances for testing.
   */
  WordleSolverFixture() {
    strategy = std::make_unique<FeedbackStrategy>();
    game = std::make_unique<WordleGame>("../data/word-bank.csv");
    solver = std::make_unique<WordleSolver>(std::move(game));
  }
  std::unique_ptr<FeedbackStrategy> strategy;  ///< Pointer to the FeedbackStrategy instance.
  std::unique_ptr<WordleGame> game;            ///< Pointer to the WordleGame instance.
  std::unique_ptr<WordleSolver> solver;        ///< Pointer to the WordleSolver instance.
};

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_FilteringAndEntropy") {
    std::vector<std::string> words = {"apple", "apply", "angle", "amble"};

    SECTION("AbsentLettersDetected") {
        solver->addAbsentLetters("apple", {Feedback::Absent, Feedback::Absent, Feedback::Absent, Feedback::Absent, Feedback::Present});
        auto absent = solver->getAbsentLetters();
        REQUIRE(absent.count('a') == 1);
        REQUIRE(absent.count('p') == 1);
        REQUIRE(absent.count('l') == 1);
        REQUIRE(absent.count('e') == 0);
    }
    SECTION("NextGuessReturnsWord") {
        std::string guess = solver->nextGuess();
        REQUIRE_FALSE(guess.empty());
    }
    SECTION("FeedbackPatternMatchesStrategy") {
        auto fb = solver->feedbackPattern("apple", "apply");
        auto fb2 = strategy->calculateFeedback("apple", "apply");
        REQUIRE(fb == fb2);
    }
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_EntropyCalculation") {
    std::vector<std::string> words = {"apple", "apply", "angle", "amble", "ample", "place", "plate", "table"};

    SECTION("EntropyForGuess") {
        double entropy = solver->calculateEntropy("apple", words);
        REQUIRE(entropy > 0.0);
    }

    SECTION("EntropyForDifferentGuesses") {
        double entropy1 = solver->calculateEntropy("aaaaa", words);
        double entropy2 = solver->calculateEntropy("apply", words);
        REQUIRE(entropy1 < entropy2);
    }
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_EntropyFirstGuess") {
    std::vector<std::pair<std::string, double>> expected = {
{"RAISE", 5.877910},
{"SLATE", 5.855775},
{"CRATE", 5.834874},
{"IRATE", 5.831397},
{"TRACE", 5.830549},
{"ARISE", 5.820940},
{"STARE", 5.807280},
{"SNARE", 5.770089},
{"AROSE", 5.767797},
{"LEAST", 5.751646},
{"ALERT", 5.745837},
{"CRANE", 5.742782},
{"STALE", 5.738573},
{"SANER", 5.733713},
{"ALTER", 5.713171},
{"LATER", 5.706089},
{"REACT", 5.696354},
{"LEANT", 5.684574},
{"TRADE", 5.681561},
{"LEARN", 5.656074},
{"CATER", 5.646715},
{"ROAST", 5.645193},
{"AISLE", 5.636828}
};

    SECTION("EntropyOfFirstGuessMatchesReference") {
        for (const auto& [word, ref_entropy] : expected) {
            double entropy = solver->calculateEntropy(word, solver->possibleWords);
            REQUIRE(entropy == Catch::Approx(ref_entropy).epsilon(0.0006));
        }
    }
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_FeedbackPattern") {
    SECTION("FeedbackPattern_Correct") {
        auto feedback = solver->feedbackPattern("APPLE", "APPLE");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Correct, Feedback::Correct, Feedback::Correct, Feedback::Correct, Feedback::Correct});
    }
    SECTION("FeedbackPattern_PresentAndAbsent") {
        auto feedback = solver->feedbackPattern("APPLE", "LEMON");
        REQUIRE(feedback[0] == Feedback::Absent);
        REQUIRE(feedback[1] == Feedback::Absent);
        REQUIRE(feedback[2] == Feedback::Absent);
        REQUIRE(feedback[3] == Feedback::Present);
        REQUIRE(feedback[4] == Feedback::Present);
    }
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_UpdatePossibleWords") {
    SECTION("UpdatePossibleWords_FiltersCorrectly") {
        std::vector<std::string> before = solver->possibleWords;
        std::string guess = before.front();
        auto feedback = solver->feedbackPattern(guess, before[1]);
        solver->updatePossibleWords(guess, feedback);
        // At least one word should be removed
        REQUIRE(solver->possibleWords.size() < before.size());
        // Check that the guess is not in the possible words anymore
        REQUIRE(std::find(solver->possibleWords.begin(), solver->possibleWords.end(), guess) == solver->possibleWords.end());
    }
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_NextGuess") {
    SECTION("NextGuess_ReturnsValidWord") {
        std::string guess = solver->nextGuess();
        // Das Wort muss in der möglichen Wortliste sein
        REQUIRE(std::find(solver->possibleWords.begin(), solver->possibleWords.end(), guess) != solver->possibleWords.end());
    }
}

TEST_CASE("WordleSolver_ThrowsOnEmptyWordList") {
    auto emptyGame = std::make_unique<WordleGame>("../data/word-bank.csv");
    emptyGame->wordList.clear();
    REQUIRE_THROWS_AS(WordleSolver(std::move(emptyGame)), WordListEmptyException);
}

TEST_CASE_METHOD(WordleSolverFixture, "WordleSolver_AddAbsentLetters_DoesNotAddIfPresentElsewhere") {
  // Example: guess = "apple", feedback = [Absent, Absent, Correct, Absent, Absent]
  // The 'p' at position 0 is Absent, but at position 1 it is Correct -> should NOT be added to absentLetters!
  solver->addAbsentLetters("apple", {Feedback::Absent, Feedback::Correct, Feedback::Absent, Feedback::Absent, Feedback::Absent});
  auto absent = solver->getAbsentLetters();
  // Check that 'p' is not in absentLetters
  REQUIRE(absent.count('p') == 0);
  REQUIRE(absent.count('a') == 1);
  REQUIRE(absent.count('l') == 1);
  REQUIRE(absent.count('e') == 1);
}
