#include <iostream>

#include "utility.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "code_gen.hpp"

std::ostream& operator<<(std::ostream& o, const sl::token_t& token) {
    o << std::to_string(token);
    return o;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 2) {
        std::cout << "./simpleLang {path to .sl src}\n"; 
        exit(EXIT_FAILURE);
    }
    std::string src = sl::read_file(argv[1]);

    sl::lexer_t lexer{ src };
    std::vector<sl::token_t> tokens = lexer.tokens();

    // for (auto& token : tokens) {
    //     std::cout << token << '\n';
    // }

    sl::parser_t parser{ tokens };

    auto result = parser.parse();

    if (!result) {
        throw std::runtime_error(result.unwrapErr());
    }

    auto [ast, identifier_table] = result.unwrap();

    sl::interpreter_t interpreter{ ast, identifier_table };
    // interpreter.run();
    while (interpreter.can_run()) {
        interpreter.run_statement();
    }
    // std::cout << interpreter.get_state() << '\n';

    sl::code_gen_t code_gen{ ast };

    std::cout << code_gen.gen() << '\n';

    return 0;
}