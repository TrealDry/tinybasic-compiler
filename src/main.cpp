#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "./lexer.hpp"
#include "./parser.hpp"
#include "generator.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is...\n";
        std::cerr << "tinyb <file.bas>\n";
        return EXIT_FAILURE;
    }

    std::fstream input(argv[1], std::ios::in);
    std::stringstream code_stream;
    code_stream << input.rdbuf();
    input.close();

    std::string code = code_stream.str();

    Lexer l{code};
    auto tokens = l.gen_tokens();

    Parser p{tokens};
    auto node_prog = p.gen_prog();

    std::fstream output("out.asm", std::ios::out);

    Generator g{node_prog, p.get_unique_let()};
    output << g.gen_asm();
    output.close();

    int result;
    result = system("nasm -felf64 out.asm");
    result = system("ld -o out out.o -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2");

    return EXIT_SUCCESS;
}