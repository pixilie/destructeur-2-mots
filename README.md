# Destructeur2Mots
An OCR (Optical Character Recognition) system that reads an image of a word-search puzzle and solves the hidden-words grid.  

## Project structure
`destructeur-2-mots`  
- `src`: Source code
  - `image`
    - `image_helpers.c`
    - `image_rotation.c`
    - `image_slice.c`
    - `image_treatment.c`
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
  - `image_treatment.h`
- `build`: Compiled objects & executables  
- `tests`: Unit tests
  - `results`
	- `poc_neural_tests.c`  
  - `image_rotation_tests.c`
  - `image_slice_tests.c`
  - `image_treatment_tests.c`
	- `solver_grid_sample.txt`
- `docs`: Documentation (reports, documents...)
  - `ocr_word_search_solver_fr.pdf`

## Build
- `make`        → builds everything except the tests.
- `make test`   → builds build/test from src/*.c + tests/*.c, then runs it.
- `make solver` → builds src/solver.c
- `make ui`     → builds src/ui.c
- `make image`  → builds src/image/*.c
- `make clean`  → cleans everything.

## Usage
- `./image <function_name> <image_path> <function_params> <optional:output>`
  - Available functions:
    - `./image convert_to_grayscale <image_path> <optional:output>`
    - `./image binarize_image <image_path> <threshold> <optional:output>`
    - `./image rotate_image <image_path> <angle_degrees> <optional:output>`
    - `./image slice_from <image_path> <x> <y> <direction> <optional:output1> <optional:output2>`
    - `./image slice_in_n <image_path> <n_slice> <optional:output_prefix>`
    - `./image crop <image_path> <x1> <y1> <x2> <y2> <optional:output>`
    - `./image crop <image_path> <x1> <y1> <x2> <y2> <optional:output>`

- `./solver <grid_path> <word>`

- `./ui`

## Commit convention
- git commit -m "feat: new features"  
- git commit -m "fix: bug fix"  
- git commit -m "chore: other"  

## Links
- [Moodle](https://moodle.epita.fr/course/view.php?id=5071)

