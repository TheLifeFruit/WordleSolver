//
// Created by Marcel Auer on 01.06.2025.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../include/WordleGame.h"
#include "../include/WordleSolver.h"
#include <iostream>


struct WordleGameFixture {
  /**
   * @brief Constructor for WordleGameFixture.
   *
   * Initializes the Wordle and Solver instances for testing.
   */
  WordleGameFixture() {
    game = std::make_unique<WordleGame>("../data/word-bank.csv");
  }
  std::unique_ptr<WordleGame> game;            ///< Pointer to the WordleGame instance.
};

TEST_CASE_METHOD(WordleGameFixture, "DefaultFeedbackStrategy_FeedbackCalculation") {

    SECTION("AllLettersCorrect") {
        auto feedback = game->feedbackStrategy->calculateFeedback("abcde", "abcde");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Correct, Feedback::Correct, Feedback::Correct, Feedback::Correct, Feedback::Correct});
    }
    SECTION("AllLettersAbsent") {
        auto feedback = game->feedbackStrategy->calculateFeedback("abcde", "fghij");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Absent, Feedback::Absent, Feedback::Absent, Feedback::Absent, Feedback::Absent});
    }
    SECTION("SomeLettersPresentWrongPlace") {
        auto feedback = game->feedbackStrategy->calculateFeedback("abcde", "eabcd");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Present, Feedback::Present, Feedback::Present, Feedback::Present, Feedback::Present});
    }
    SECTION("MixedFeedback") {
        auto feedback = game->feedbackStrategy->calculateFeedback("aabbb", "ababa");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Correct, Feedback::Present, Feedback::Present, Feedback::Correct, Feedback::Absent});
    }
    SECTION("MultipleOccurrences_AbideVsSpeed") {
        auto feedback = game->feedbackStrategy->calculateFeedback("speed", "abide");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Absent, Feedback::Absent, Feedback::Present, Feedback::Absent, Feedback::Present});
    }
    SECTION("MultipleOccurrences_EraseVsSpeed") {
        // Solution: "erase", Guess: "speed" -> {1, 0, 1, 1, 0}
        auto feedback = game->feedbackStrategy->calculateFeedback("speed", "erase");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Present, Feedback::Absent, Feedback::Present, Feedback::Present, Feedback::Absent});
    }
    SECTION("MultipleOccurrences_StealVsSpeed") {
        // Solution: "steal", Guess: "speed" -> {2, 0, 2, 0, 0}
        auto feedback = game->feedbackStrategy->calculateFeedback("speed", "steal");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Correct, Feedback::Absent, Feedback::Correct, Feedback::Absent, Feedback::Absent});
    }
    SECTION("MultipleOccurrences_CrepeVsSpeed") {
        // Solution: "crepe", Guess: "speed" -> {0, 1, 2, 1, 0}
        auto feedback = game->feedbackStrategy->calculateFeedback("speed", "crepe");
        REQUIRE(feedback == std::vector<Feedback>{Feedback::Absent, Feedback::Present, Feedback::Correct, Feedback::Present, Feedback::Absent});
    }
}

TEST_CASE_METHOD(WordleGameFixture, "WordleGame_BasicFunctionality") {

    SECTION("CorrectWordLength") {
        REQUIRE(game->getWordLength() == 5);
    }
    SECTION("MaxTries") {
        REQUIRE(game->getMaxTries() == 6);
    }
    SECTION("FeedbackAndWin") {
        std::vector<Feedback> test_feedback = game->guess(game->getSecret());
        REQUIRE(game->isWon(test_feedback));
        REQUIRE(game->getTries() == 1);
    }
    SECTION("InvalidWordLengthThrows") {
        REQUIRE_THROWS_AS(game->guess("app"), NotAFiveLetterWordException);
    }
}