# The CMake setting of 'src'

add_executable(skipgram skipgram.cxx vocab.cxx)
hwe_set_target(skipgram)
hwe_set_target_omp(skipgram CXX)

add_custom_target(run
	COMMAND skipgram
	DEPENDS skipgram
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

add_custom_target(run2
	COMMAND word2vec -train enwik8.txt -output enwik8.emb -save-vocab enwik8.vocab -cbow 0 -threads 32
	DEPENDS word2vec
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
