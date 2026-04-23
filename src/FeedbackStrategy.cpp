#include "../include/FeedbackStrategy.h"
#include <vector>
#include <string>

/**
 * @brief Calculates the feedback for a guess compared to the solution.
 * @param guess The guessed word.
 * @param solution The solution word.
 * @return A vector of Feedback enums for each letter.
 */
std::vector<Feedback> FeedbackStrategy::calculateFeedback(const std::string& guess, const std::string& solution) {
    std::vector<Feedback> feedback(solution.size(), Feedback::Absent);
    std::vector<bool> used(solution.size(), false);

    for (size_t i = 0; i < solution.size(); ++i) {
        if (guess[i] == solution[i]) {
            feedback[i] = Feedback::Correct;
            used[i] = true;
        }
    }
    for (size_t i = 0; i < solution.size(); ++i) {
        if (feedback[i] == Feedback::Correct) continue;
        for (size_t j = 0; j < solution.size(); ++j) {
            if (!used[j] && guess[i] == solution[j]) {
                feedback[i] = Feedback::Present;
                used[j] = true;
                break;
            }
        }
    }
    return feedback;
}
