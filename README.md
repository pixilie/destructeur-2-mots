# Destructeur2Mots
An OCR (Optical Character Recognition) system that reads an image of a word-search puzzle and solves the hidden-words grid.  

## Project structure
`destructeur-2-mots`  
- `src`: Source code  
  - `solver.c`
  - `neural_network.c`  
- `include`: Header files  
  - `neural_network.h`  
- `build`: Compiled objects & executables  
- `tests`: Unit tests  
	- `neural_network_tests.c`  
	- `solver_grid_sample.txt`  
- `docs`: Documentation (reports, documents...)
  - `ocr_word_search_solver_fr.pdf`

## Build
- make → builds the main program.
- make test → builds build/test from src/*.c + tests/*.c, then runs it.
- make solver → builds src/solver.c
- make clean → cleans everything.

## Commit convention
- git commit -m "feat: new features"  
- git commit -m "fix: bug fix"  
- git commit -m "chore: other"  

## Links
- [Moodle](https://moodle.epita.fr/course/view.php?id=5071)

