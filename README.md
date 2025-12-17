# Destructeur2Mots
An OCR (Optical Character Recognition) system that reads an image of a word-search puzzle and solves the hidden-words grid.  

=> Switch to branch [submit-version](https://github.com/pixilie/destructeur-2-mots/tree/submit-version) to test the project.

## Project structure
`destructeur-2-mots`  
- `assets`: Assets used in the project (images, datasets, models...)
- `build`: Compiled objects & executables  
- `docs`: Documentation (reports, documents...)
- `helpers`: Complementary programs we used
- `include`: Header files
- `src`: Source code
- `tests`: Unit tests
  
## Build
- `make`          → builds everything (except the tests) in build/, then start the main program.
- `make test`     → builds build/test from src/*.c + tests/*.c, then runs it. Results can be found in tests/result.
- `make clean`    → cleans everything.

## Documents
- `docs/ocr_word_search_solver_fr.pdf` : Specifications
- `docs/destructeur_2_mots_rapport_n1` : First presentation report
- `docs/destructeur_2_mots_rapport_n2` : Second presentation report
