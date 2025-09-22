# Destructeur2Mots

## Project structure
destructeur-2-mots/  
├── src/           # Source code (.c files)  
│   ├── main.c  
│   └── ...  
├── include/       # Header files (.h)  
│   ├── mylib.h  
│   └── ...  
├── build/         # Compiled objects & executables (ignored by git)  
├── tests/         # Unit tests  
│   └── test_main.c  
├── docs/          # Documentation (reports, diagrams, doxygen, etc.)  
├── .gitignore  
├── Makefile  
├── README.md  
└── LICENSE  

## Build
- make → builds your main program (build/main).
- make test → builds build/test from src/*.c + tests/*.c, then runs it.
- make clean → cleans everything.
