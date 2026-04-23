#pragma once
#include <vector>
#include <string>

/**
 * @brief Enum representing feedback for each letter in a Wordle guess.
 */
enum class Feedback { Correct, Present, Absent };

/**
 * @brief The FeedbackStrategy class provides the logic to calculate feedback for a guess.
 */
class FeedbackStrategy {
public:
    /**
     * @brief Calculates the feedback for a guess compared to the solution.
     * @param guess The guessed word.
     * @param solution The solution word.
     * @return A vector of Feedback enums for each letter.
     */
    static std::vector<Feedback> calculateFeedback(const std::string& guess, const std::string& solution) ;
};
