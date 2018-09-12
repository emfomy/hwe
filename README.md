# Heterogeneous Word Embedding

# Information

This library is a C implementation of the Heterogeneous Word Embedding (HWE), which is a general and flexible framework to incorporate each type (e.g. word-sense, part-of-speech, topic) of contextual feature for learning feature-specific word embeddings in an explicit fashion.

## Git
* https://github.com/emfomy/hwe

## Documentation
* Download **hwe-#.#.#-docs.zip** from https://github.com/emfomy/hwe/releases/latest
* May also build it using **make doc** (see below) on your own computer.

## Author
* Mu Yang <<emfomy@gmail.com>>
* Fann Jhih-Sheng <<fann1993814@gmail.com>>

# Requirements

* [CMake](https://cmake.org) 3.0+.
* C/C++ compiler with C11/C++11 standard support ([GCC](https://gcc.gnu.org) 5+ recommended).

## Optional
* [Doxygen](http://www.stack.nl/~dimitri/doxygen/) (Used for documentation).

# Installation

Please use the following commands to create Makefiles

```
cd <hwe-source-folder>
mkdir build
cd build
cmake ..
```

# Reference

* Heterogeneous Word Embedding

# License {#Readme_License}
\include LICENSE.md
