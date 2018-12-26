# Heterogeneous Word Embedding

# Information

This library is a C implementation of the Heterogeneous Word Embedding (HWE), which is a general and flexible framework to incorporate each type (e.g. word-sense, part-of-speech, topic) of contextual feature for learning feature-specific word embeddings in an explicit fashion.

## Git
* <https://github.com/emfomy/hwe>

## Author
* Fann Jhih-Sheng <<fann1993814@gmail.com>>
* Mu Yang <<emfomy@gmail.com>>

# Usage

## Compile

```
make hwe
```

## Example

```
wget http://cs.fit.edu/~mmahoney/compression/enwik8.zip
unzip demo/enwik8

./hwe -train enwik8 -output enwik8.emb -size 100 -window 5 -sample 1e-4 -negative 5 -binary 0 -fmode 2 -knfile demo/wordnetlower.tree -iter 2 -threads 32
```

# Reference

* [Jhih-Sheng Fan, Mu Yang, Peng-Hsuan Li and Wei-Yun Ma, “HWE: Word Embedding with Heterogeneous Features”, ICSC2019](https://muyang.pro/file/paper/icsc_2019_hwe.pdf)

# License
[![License: CC BY-NC-SA 4.0](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](https://creativecommons.org/licenses/by-nc-sa/4.0/) Copyright (c) 2017-2018 Mu Yang & Fann Jhih-Sheng under the [CC-BY-NC-SA 4.0 License](https://creativecommons.org/licenses/by-nc-sa/4.0/). All rights reserved.

\include LICENSE.md
