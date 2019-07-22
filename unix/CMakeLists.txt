project(hid_parser)



add_library(
    parser_lib STATIC 
    ../hid-parser/util/inc/util.h    
    ../hid-parser/parser/inc/ast.h 
    ../hid-parser/parser/inc/parser.h 
    ../hid-parser/parser/inc/tokenizer.h
    ../hid-parser/precision/inc/precision.h
    ../hid-parser/util/src/util.c    
    ../hid-parser/parser/src/ast.c 
    ../hid-parser/parser/src/parser.c 
    ../hid-parser/parser/src/tokenizer.c
    ../hid-parser/precision/src/precision.c
    port/fileio.c
    port/fileio.h
)

target_include_directories(parser_lib PUBLIC
    ../hid-parser/util/inc
    ../hid-parser/parser/inc 
    ../hid-parser/precision/inc
    port
)


add_executable(test main.c)
target_link_libraries(test parser_lib)
