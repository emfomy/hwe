# The CMake setting of 'src'

add_executable(demo demo.cxx)
target_link_libraries(demo hwe)
target_include_directories(demo PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_custom_target(run
	COMMAND demo
	DEPENDS demo
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

add_custom_target(run2
	COMMAND word2vec -train enwik8.txt -output enwik8.emb -save-vocab enwik8.vocab -cbow 0 -threads 32
	DEPENDS word2vec
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
