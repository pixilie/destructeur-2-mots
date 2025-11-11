# Destructeur2Mots
An OCR (Optical Character Recognition) system that reads an image of a word-search puzzle and solves the hidden-words grid.  

## Project structure
`destructeur-2-mots`  
- `src`: Source code
  - `image`
    - `image_helpers.c`
    - `image_rotation.c`
    - `image_slice.c`
    - `image_processing.c`
    - `main.c`  
  - `solver.c`
  - `neural_network.c`
  - `ui.c`
  - `main.c`
- `include`: Header files  
  - `neural_network.h`
  - `ui.h`
  - `image_helpers.h`
  - `image_rotation.h`
  - `image_slice.h`
  - `image_processing.h`
- `build`: Compiled objects & executables  
- `tests`: Unit tests
  - `results`
	- `poc_neural_tests.c`  
  - `image_rotation_tests.c`
  - `image_slice_tests.c`
  - `image_processing_tests.c`
	- `solver_grid_sample.txt`
- `docs`: Documentation (reports, documents...)
  - `ocr_word_search_solver_fr.pdf`
  - `destructeur_2_mots_rapport_n1.pdf`

## Build
- `make`          → builds everything except the tests.
- `make test`     → builds build/test from src/*.c + tests/*.c, then runs it.
- `make solver`   → builds src/solver.c
- `make ui`       → builds src/ui.c
- `make image`    → builds src/image/*.c
- `make pipeline` → builds src/line_detection.c
- `make clean`    → cleans everything.

## Usage
- `./image <function_name> <image_path> <function_params> <optional:output>` : Applies image processing transformations on a given image `<image_path>`
  - Available functions:
    - `./image convert_to_grayscale <image_path> <optional:output>`
    - `./image binarize_image <image_path> <threshold> <optional:output>`
    - `./image convert_to_black_and_white <image_path> <optional:output>`
    - `./image rotate_image <image_path> <angle_degrees> <optional:output>`
    - `./image slice_from <image_path> <x> <y> <direction> <optional:output1> <optional:output2>`
    - `./image slice_in_n <image_path> <n_slice> <optional:output_prefix>`
    - `./image crop <image_path> <x1> <y1> <x2> <y2> <optional:output>`

- `./solver <grid_path> <word>`: Attempts to solve the given word `<word>` in the grid `<grid_path>` by finding its coordinates in the grid

- `./ui <optional:image_path>`: Launches UI and loads `<image_path>` (default: level_1_image_1.png)

- `./pipeline <input_image> <output_grid_words_dir> <output_letters_dir>`: Cuts the given image `<input_image>` in sub-images with:
  - `<output_grid_words_dir>` containing the grid, the list of words and each word
  - `<output_letters_dir>` containing each letter of the grid and each letter of each word of the word list

## Documents
- `docs/destructeur_2_mots_rapport_n1` : First presentation report
- `docs/ocr_word_search_solver_fr.pdf` : Specifications

## Commit convention
- git commit -m "feat: new features"  
- git commit -m "fix: bug fix"  
- git commit -m "chore: other"  

## Links
- [Moodle](https://moodle.epita.fr/course/view.php?id=5071)

