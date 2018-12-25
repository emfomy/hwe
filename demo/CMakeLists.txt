# The CMake setting of 'src'

add_executable(demo demo.cxx)
hwe_set_target(demo)
hwe_set_target_omp(demo CXX)
target_link_libraries(demo hwe)
target_include_directories(demo PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_custom_target(run
	COMMAND demo
	DEPENDS demo
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

add_executable(demo2 demo2.c)
hwe_set_target(demo2)

add_custom_target(run2
	COMMAND demo2 -train alice.txt -output alice.emb -save-vocab alice.vocab -cbow 0 -threads 32
	DEPENDS demo2
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
