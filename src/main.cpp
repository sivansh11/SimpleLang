#include <iostream>

#include "utility.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

static const char *test_file = "../example_simpleLang_program.sl";

std::ostream& operator<<(std::ostream& o, const sl::token_t& token) {
    o << std::to_string(token);
    return o;
}

int main() {
    std::string src = sl::read_file(test_file);

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

    sl::interpreter_t internpreter{ ast, identifier_table };
    internpreter.run();

    return 0;
}