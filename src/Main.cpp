
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

int mode = 0;
bool reset = true;



static std::string feedbackToString(const std::vector<Feedback>& feedback) {
  std::string s;
  for (auto f : feedback) {
    if (f == Feedback::Correct) s += '2';
    else if (f == Feedback::Present) s += '1';
    else s += '0';
  }
  return s;
}

static std::vector<Feedback> StringToFeedback(std::string fdbkString) {
  std::vector<Feedback> fdbk;
  for (const auto fdbk_char : fdbkString) {
    if (fdbk_char == '2')  {
      fdbk.push_back(Feedback::Correct);
    }else if (fdbk_char == '1')  {
      fdbk.push_back(Feedback::Present);
    } else {
      fdbk.push_back(Feedback::Absent);
    }
  }

  return fdbk;
}


void printBestGuesses(std::vector<std::string> bestGuesses) {
  for (auto guess : bestGuesses) {
    std::cout << "Try: " << guess << '\n';
  }
}

std::string get_params(std::string userInput) {
  std::string param = "";
  bool hitSlash = false;
  bool hitParam = false;

  for (auto c : userInput) {
    if (not hitSlash and c == '/') {
      hitSlash = true;
    }
    if (hitParam and c != ' ') {
      param.push_back(c);
    }
    if (not hitParam and hitSlash and c == ' ') {
      hitParam = true;
    }
  }

  return param;
}


bool isValidFedback(const std::string& s) {
  for (char c : s) {
    if (c < '0' || c > '2') {
      return false;
    }
  }
  return !s.empty();
}



/*
 * brief
 */
bool isValidWord(const std::string& s) {
  for (char c : s) {
    if (c > 123 || c < 96) {
      return false;
    }
  }
  return !s.empty();
}



void to_lowercase(std::string &s) {
  for (char &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
}


bool starts_with(const std::string& str, const std::string& prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}


int is_valid_input(std::string &s, int expected_length) {
  if (s.empty()) {
    return 0;
  }

  if (s[0] == '/') {
    if (starts_with(s, "/n")) {
      return 10;
    }

    if (starts_with(s, "/s")) {
      return 11;
    }

    if (starts_with(s, "/f")) {
      return 12;
    }

    if (starts_with(s, "/r")) {
      return 13;
    }

    if (starts_with(s, "/e")) {
      return 14;
    }
    if (starts_with(s, "/h")) {
      return 20;
    }

    return 0;
  }

  if (s.length() != expected_length) {
    return 0;
  }

  if (isValidFedback(s)) {
    return 2;
  }

  if (isValidWord(s)) {
    return 3;
  }



  return 1;
}







void play_mode_0() {
  double average = 0.0;
  int runs = 4000;
  int fails = 0;
  std::array<int, 6> tries = {0, 0, 0, 0, 0, 0};

  std::string l;
  std::cout << "Select word length" << '\n';
  std::cin >> l;

  for (int g=0; g < runs; g++) {
    try {

      std::unique_ptr<WordleSolver> solver = std::make_unique<WordleSolver>("C:/Code GIT/praktikuminfauto25wordlepart2-gruppe105/data/en" + l + ".csv");
      int maxTries = solver->getMaxTries();
      std::string secret = solver->getSecret();


      // std::cout << "[DEBUG] Secret word is: " <<secret << std::endl;

      for (int i = 0; i < maxTries; ++i) {
        std::string guess = solver->nextGuess();
        std::cout << "Attempt " + std::to_string(i + 1) + ": " + guess << '\n';

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
        solver->storeAttempt(guess, feedback);

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
        return;
    }

  } // End of Game Loop
  average = average / runs;
  std::cout << "[INFO] Word Length: " << l << std::endl;
  std::cout << "[INFO] Games: " << runs << std::endl;
  std::cout << "[INFO] Game Average Tries: " << average << std::endl;
  std::cout << "[INFO] Game Fails: " << fails << std::endl;
  std::cout << "[INFO] 1-Tries: " << tries[0] << ", 2-Tries: " << tries[1] << ", 3-Tries: " << tries[2] << ", 4-Tries: " << tries[3] << ", 5-Tries: " << tries[4] << ", 6-Tries: " << tries[5] << std::endl;
  std::cout << "[INFO] 1-Tries: " << (static_cast<double> (tries[0])/runs) *100 << "%, 2-Tries: " << static_cast<double>(tries[1])/runs*100 << "%, 3-Tries: " << static_cast<double>(tries[2])/runs*100 << "%, 4-Tries: " << static_cast<double>(tries[3])/runs*100 << "%, 5-Tries: " << static_cast<double>(tries[4])/runs*100 << "%, 6-Tries: " << static_cast<double>(tries[5])/runs*100 << "%" << std::endl;

}




void play_mode_1 (){
  int i = 0;
  std::string dict;
  std::cout << "Select Special dict? like en6." << '\n';
  std::cin >> dict;



  const auto solver = std::make_unique<WordleSolver>("C:/Code GIT/praktikuminfauto25wordlepart2-gruppe105/data/" + dict + ".csv");
  std::string secret = solver->getSecret();
  int lenght = solver->getWordLength();

  while (true){
  try {
    int amount = 0;
    std::string userInput;
    std::vector<std::string> bestGuesses;

    bool valid_word = false;
    bool valid_feedback = false;
    std::vector<Feedback> feedback;
    std::string guess;


    if (i == 0) {
      bestGuesses = solver->nextBestGuesses(3);
      printBestGuesses(bestGuesses);
      std::cout << "RND Start Word: "<<  secret << '\n';
    }else{
      std::vector<std::string> possible_words = solver->getPossibleWords();
      if (possible_words.size() < 1) {
        std::cout << "[ERROR] No possible word in dictionary" << '\n';
        return;
      }
      if (possible_words.size() == 1) {
        std::cout << "Solution Based on English Dict: " << '\n';
        bestGuesses = solver->nextBestGuesses(1);
        printBestGuesses(bestGuesses);
      }else if (possible_words.size() <= 10) {
        bestGuesses = solver->nextBestGuesses(10);
        printBestGuesses(bestGuesses);
      }
      else {
        std::cout << possible_words.size() << " words remaining!" << " /show [x] to see top guesses: " << '\n';
      }
    }




    while (true) {
      std::cin >> userInput;
      to_lowercase(userInput);
      int userInputInt = is_valid_input(userInput, lenght);
      // std::cout << userInputInt << '\n';
      switch (userInputInt) {
        case 0:
          std::cout << "Invalid Input" << '\n';
        case 1:
          std::cout << "Unsupported Input" << '\n';
          break;
        case 2:
          valid_feedback = true;
          feedback = StringToFeedback(userInput);
          if (!valid_word) {
            std::cout << "Enter Word to feed the algorithm: " << userInput << '\n';
          }
          break;
        case 3:
          valid_word = true;
          guess = userInput;
          if (!valid_feedback) {
            std::cout << "[0: false] [1:correct Letter] [2: correct]\n";
          }
          break;

        case 10: //new
          reset = true;
          if (int a10 = 0; std::cin >> a10) {
            mode = a10;
          }

          break;

        case 11: { //show
          if (int a11 = 0; std::cin >> a11) {
            bestGuesses = solver->nextBestGuesses(a11);
            printBestGuesses(bestGuesses);
          }

          break;
        }

        case 12: //find
          return;
          break;
        case 13: //restart
          return;
          break;
        case 14: //end
          return;
          break;
        case 20: //help
          std::cout << "Functions:" << '\n';
          std::cout << "/new (1,2,3): Makes a new instance" << '\n';
          std::cout << "/show (x): Shows x top recommended guesses. Based on Feedback and Dictionary" << '\n';
          std::cout << "/find (a-z): Looks for words in your total dictionary conatining letters" << '\n';
          std::cout << "/rs : Restarts on the same Mode/Dictionary" << '\n';
          std::cout << "/end : Stops C++ Instance" << '\n';
          std::cout << "/l : Prints current length" << '\n';
          std::cout << "----------------------------------------" << '\n';
          std::cout << "Word: For example 'crane' will set the word" << '\n';
          std::cout << "Feedback (0,1,2): For example '01020' will set the feedback" << '\n';
          break;

        default:
          std::cout << "Unknown command" << '\n';
          break;
      }

      if (valid_feedback && valid_word) {
        solver->storeAttempt(guess, feedback);
        solver->addAbsentLetters(guess, feedback);
        solver->updateMaxLetters(guess, feedback);
        solver->updatePossibleWords(guess, feedback);
        break;
      }

    }


    ++i;

  } catch (const std::exception& e) {
    std::cerr << "[FATAL] " << e.what() << std::endl;
    return;
    }
  }
}


void play(int mode) {
  if (mode == 0) {
    play_mode_0();
  }
  if (mode == 1) {
    play_mode_1();
  }

}



int main() {
  int mode = 0;
  std::cout << "[INFO] Welcome to the Wordle Solver!" << '\n';
  std::cout << "[INFO] /help if youre stuck" << '\n';
  std::cout << "[INFO] Input mode" << '\n';
  std::cout << "[INFO] 0: Solution Simultaion" << '\n';
  std::cout << "[INFO] 1: NextGuess Helper" << std::endl;

  std::cin >> mode;

  while (reset) {
    reset = false;
    play(mode);
  }


 return 0;
}