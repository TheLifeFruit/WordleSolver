[GUESSES]
> START
I have tested the best entry words as it allows for experimantation whit added performance gains:
Lowest avg orderd: salet, slate, trace, crate, raise
While salet is the best starting word I tested I do not use salet as it is not in the word-bank

--------------------------------------------------------------------------------------------------------

salet:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.49725
[INFO] Game Fails: 0
[INFO] 1-Tries: 0, 2-Tries: 287, 3-Tries: 1765, 4-Tries: 1648, 5-Tries: 272, 6-Tries: 28
[INFO] 1-Tries: 0%, 2-Tries: 7.175%, 3-Tries: 44.125%, 4-Tries: 41.2%, 5-Tries: 6.8%, 6-Tries: 0.7%

--------------------------------------------------------------------------------------------------------

crate:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.552
[INFO] Game Fails: 0
[INFO] 1-Tries: 3, 2-Tries: 246, 3-Tries: 1747, 4-Tries: 1586, 5-Tries: 380, 6-Tries: 38
[INFO] 1-Tries: 0.075%, 2-Tries: 6.15%, 3-Tries: 43.675%, 4-Tries: 39.65%, 5-Tries: 9.5%, 6-Tries: 0.95%

--------------------------------------------------------------------------------------------------------

trace:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.5405
[INFO] Game Fails: 0
[INFO] 1-Tries: 5, 2-Tries: 240, 3-Tries: 1736, 4-Tries: 1662, 5-Tries: 321, 6-Tries: 36
[INFO] 1-Tries: 0.125%, 2-Tries: 6%, 3-Tries: 43.4%, 4-Tries: 41.55%, 5-Tries: 8.025%, 6-Tries: 0.9%

--------------------------------------------------------------------------------------------------------

slate:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.538
[INFO] Game Fails: 0
[INFO] 1-Tries: 3, 2-Tries: 248, 3-Tries: 1740, 4-Tries: 1642, 5-Tries: 337, 6-Tries: 30
[INFO] 1-Tries: 0.075%, 2-Tries: 6.2%, 3-Tries: 43.5%, 4-Tries: 41.05%, 5-Tries: 8.425%, 6-Tries: 0.75%

--------------------------------------------------------------------------------------------------------

raise:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.5995
[INFO] Game Fails: 0
[INFO] 1-Tries: 2, 2-Tries: 190, 3-Tries: 1748, 4-Tries: 1598, 5-Tries: 392, 6-Tries: 70
[INFO] 1-Tries: 0.05%, 2-Tries: 4.75%, 3-Tries: 43.7%, 4-Tries: 39.95%, 5-Tries: 9.8%, 6-Tries: 1.75%

--------------------------------------------------------------------------------------------------------

cramp:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.64875
[INFO] Game Fails: 0
[INFO] 1-Tries: 3, 2-Tries: 171, 3-Tries: 1535, 4-Tries: 1858, 5-Tries: 385, 6-Tries: 48
[INFO] 1-Tries: 0.075%, 2-Tries: 4.275%, 3-Tries: 38.375%, 4-Tries: 46.45%, 5-Tries: 9.625%, 6-Tries: 1.2%

--------------------------------------------------------------------------------------------------------

aeiou:
[INFO] Games: 4000
[INFO] Game Average Tries: 3.82175
[INFO] Game Fails: 13
[INFO] 1-Tries: 0, 2-Tries: 111, 3-Tries: 1404, 4-Tries: 1698, 5-Tries: 674, 6-Tries: 100
[INFO] 1-Tries: 0%, 2-Tries: 2.775%, 3-Tries: 35.1%, 4-Tries: 42.45%, 5-Tries: 16.85%, 6-Tries: 2.5%

--------------------------------------------------------------------------------------------------------

[SOLVER]
> METHODS
 > a: Information Theory -> sorting by Entropy
 > b: Make a probe Word if Pattern is restricting


> ISSUES
 > Max entropy word is close/identical to the other top words
  // Loop over map and update max each iteration where your previous max was beaten.
  // THIS MEANS THAT MAX COULD BE SHARED BY MULTIPLE WORDS WITH THE SAME ENTROPY!!
  // FIX: If equal/close: most infrequent letters, smallest prior occurrence count

 > Word Families
  // Your solver will get stuck on word families gaining almost no new insight
  // I did implement a method that throws in the probeWord, so the nextWord is not a blind guess
  // FIX: ??? There are probably some good improvements
  // https://www.youtube.com/watch?v=fVMlnSfGq0c has uploaded a python project that solves on < 3 tries
  // So there is lots of improvements with better coding skills
