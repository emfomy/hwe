# Heterogeneous Word Embedding

# Information

This library is a C implementation of the Heterogeneous Word Embedding (HWE), which is a general and flexible framework to incorporate each type (e.g. word-sense, part-of-speech, topic) of contextual feature for learning feature-specific word embeddings in an explicit fashion.

# Data format

## Training for HWE-POS or HWE-Topic
- Parameter setting: ```-fmode 1```
- Corpus file: Each word is appended by a corresponded feature.
	- Format: `<WORD>(<FEATURE>)`
	- Example:
		- Original sentence: ```my dog also likes eating sausage.```
		- Modified sentence: ```my(PRP$) dog(NN) also(RB) likes(VBZ) eating(VBG) sausage(NN)```


## Training for HWE-Sense

- Parameter setting: ```-fomde 2 -knfile <knowledge file>```
- Knowledge file: Each row contains a sense and corresponding words.
	- Fomat: `<Sense> <Word List>`
	- Example:
		- Line1: ```SENSE_FRUIT apple banana grape```
		- Line2: ```SENSE_ANIMAL tiger monkey```

## Attention
- The words/features are represented in lower/upper-cases respectively.

# Usage

## Compile

```
make hwe
```

## Setting
```
-train <file>
    Use text data from <file> to train the model
-output <file>
    Use <file> to save the resulting word vectors / word clusters
-size <int>
    Set size of word vectors; default is 100
-window <int>
    Set max skip length between words; default is 5
-sample <float>
    Set threshold for occurrence of words. Those that appear with higher frequency in the training data
    will be randomly down-sampled; default is 1e-3, useful range is (0, 1e-5)
-negative <int>
    Number of negative examples; default is 5, common values are 3 - 10 (0 = not used)
-threads <int>
    Use <int> threads (default 12)
-iter <int>
    Run more training iterations (default 5)
-min-count <int>
    This will discard words that appear less than <int> times; default is 5
-alpha <float>
    Set the starting learning rate; default is 0.025
-debug <int>
    Set the debug mode (default = 2 = more info during training)
-binary <int>
    Save the resulting vectors in binary moded; default is 0 (off)
-save-vocab <file>
    The vocabulary will be saved to <file>
-read-vocab <file>
    The vocabulary will be read from <file>, not constructed from the training data
-fmode <int>
    Enable the Feature mode (default = 0)
        0 = only using skip-gram
        1 = predicting self-feature of sequential feature tag
        2 = predicting self-feature of global feature table
-knfile <file>
    The sense-words file will be read from <file>
```

## Example

```
wget http://cs.fit.edu/~mmahoney/compression/enwik8.zip
unzip demo/enwik8

./hwe -train enwik8 -output enwik8.emb -size 100 -window 5 -sample 1e-4 -negative 5 -binary 0 -fmode 2 -knfile demo/wordnetlower.tree -iter 2 -threads 32
```

## Author
* Fan Jhih-Sheng <<fann1993814@gmail.com>>
* Mu Yang <<emfomy@gmail.com>>

# Reference

* [Jhih-Sheng Fan, Mu Yang, Peng-Hsuan Li and Wei-Yun Ma, “HWE: Word Embedding with Heterogeneous Features”, ICSC2019](https://muyang.pro/file/paper/icsc_2019_hwe.pdf)

# License
[![License: CC BY-NC-SA 4.0](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](https://creativecommons.org/licenses/by-nc-sa/4.0/) Copyright (c) 2017-2018 Fann Jhih-Sheng & Mu Yang under the [CC-BY-NC-SA 4.0 License](https://creativecommons.org/licenses/by-nc-sa/4.0/). All rights reserved.
