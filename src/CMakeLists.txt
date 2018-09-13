# The CMake setting of 'src'

add_executable(skipgram skipgram.cxx vocab.cxx def.cxx)
hwe_set_target(skipgram)

add_custom_target(run
	COMMAND skipgram
	DEPENDS skipgram
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
