# CMake generated Testfile for 
# Source directory: /home/sword/swordProject/RCompiler
# Build directory: /home/sword/swordProject/RCompiler
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[parser_test]=] "parser_test")
set_tests_properties([=[parser_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/sword/swordProject/RCompiler/CMakeLists.txt;97;add_test;/home/sword/swordProject/RCompiler/CMakeLists.txt;0;")
add_test([=[semantic_test]=] "semantic_test")
set_tests_properties([=[semantic_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/sword/swordProject/RCompiler/CMakeLists.txt;98;add_test;/home/sword/swordProject/RCompiler/CMakeLists.txt;0;")
add_test([=[IR_test]=] "IR_test")
set_tests_properties([=[IR_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/sword/swordProject/RCompiler/CMakeLists.txt;99;add_test;/home/sword/swordProject/RCompiler/CMakeLists.txt;0;")
