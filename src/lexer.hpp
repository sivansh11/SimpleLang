#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <sstream>
#include <optional>
#include <vector>

namespace sl {

enum class token_type_t : uint32_t {
    e_undefined = 0,

    e_int,          // int
    e_identifier,   // name
    e_if,           // if

    e_number,       // 8  

    e_assign,       // =
    e_plus,         // +
    e_minus,        // -
    e_equal,        // ==
    e_lbrace,       // {
    e_rbrace,       // }
    e_lbracket,     // (
    e_rbracket,     // )

    e_semicolon,    // ;

    e_end = std::numeric_limits<uint32_t>::max(),
};

struct token_t {
    token_type_t type;
    std::string text;  // TODO(optimise): make this a string view to save space
};

struct lexer_t {
public:
    lexer_t(const std::string src) 
      : _src(src) {}

    std::optional<token_t> next() {
        token_t token{};
        while (auto c = try_get()) {
            if (isspace(*c)) {
                _index++;
                continue;
            }
            if (*c == '\n') {
                _index++;
                continue;
            }
            if (isalpha(*c)) {
                // get word
                std::string word{ *c };
                _index++;
                while (auto next = try_get()) {
                    if (isspace(*next)) {
                        break;
                    }
                    if (isalnum(*next)) {
                        word += *next;
                        _index++;
                    } else {
                        break;
                    }

                }
                if (word == "if") {
                    // return if token
                    token.type = token_type_t::e_if;
                    token.text = word;
                    return token;
                }
                if (word == "int") {
                    // return int token
                    token.type = token_type_t::e_int;
                    token.text = word;
                    return token;
                }
                // return identifier
                token.type = token_type_t::e_identifier;
                token.text = word;
                return token;
            }
            if (isdigit(*c)) {
                // get number
                std::string number{ *c };
                _index++;
                while (auto next = try_get()) {
                    if (!isdigit(*next)) break;
                    _index++;
                    number += *next;    
                }
                // return number
                token.type = token_type_t::e_number;
                token.text = number;
                return token;
            }
            // special char

            // =
            if (*c == '=') {
                // ==
                auto next = try_get(1);
                if (*next == '=') {
                    token.type = token_type_t::e_equal;
                    token.text = "==";
                    _index += 2;
                    return token;
                }
                token.type = token_type_t::e_assign;
                token.text = *c;
                _index++;
                return token;
            }
            // +
            if (*c == '+') {
                token.type = token_type_t::e_plus;
                token.text = *c;
                _index++;
                return token;
            }
            // -
            if (*c == '-') {
                token.type = token_type_t::e_minus;
                token.text = *c;
                _index++;
                return token;
            }
            // {
            if (*c == '{') {
                token.type = token_type_t::e_lbrace;
                token.text = *c;
                _index++;
                return token;
            }
            // }
            if (*c == '}') {
                token.type = token_type_t::e_rbrace;
                token.text = *c;
                _index++;
                return token;
            }
             // (
            if (*c == '(') {
                token.type = token_type_t::e_lbracket;
                token.text = *c;
                _index++;
                return token;
            }
            // )
            if (*c == ')') {
                token.type = token_type_t::e_rbracket;
                token.text = *c;
                _index++;
                return token;
            }
            // ;
            if (*c == ';') {
                token.type = token_type_t::e_semicolon;
                token.text = *c;
                _index++;
                return token;
            }
            // // comments
            if (*c == '/') {
                _index++;
                while (auto next = try_get()) {
                    if (*next == '\n') break;
                    _index++;
                }
                continue;
            }
            token.type = token_type_t::e_undefined;
            return token;
        }
        return { {token_type_t::e_end} };
    }

    std::vector<token_t> tokens() {
        std::vector<token_t> tokens;
        while (auto token = next()) {
            if (token->type == sl::token_type_t::e_end) break;
            if (token->type == sl::token_type_t::e_undefined) {
                throw std::runtime_error("Failed to lex token");
            }
            tokens.push_back(*token);
        }
        return tokens;
    }

private:
    std::optional<char> try_get(uint32_t offset = 0) {
        if (_index + offset >= _src.size()) return std::nullopt;
        return _src[_index + offset];
    }

private:
    std::string _src;
    uint32_t _index{ 0 };
};

} // namespace sl

namespace std {

std::string to_string(const sl::token_t& token) {
    std::stringstream s;
    s << "token type: ";
    switch (token.type) {
        case sl::token_type_t::e_undefined:
            s << "undefined";
            break;
        case sl::token_type_t::e_int:
            s << "int";
            break;
        case sl::token_type_t::e_identifier:
            s << "identifier";
            break;
        case sl::token_type_t::e_number:
            s << "number";
            break;
        case sl::token_type_t::e_assign:
            s << "assign";
            break;
        case sl::token_type_t::e_plus:
            s << "plus";
            break;
        case sl::token_type_t::e_minus:
            s << "minus";
            break;
        case sl::token_type_t::e_if:
            s << "if";
            break;
        case sl::token_type_t::e_equal:
            s << "equal";
            break;
        case sl::token_type_t::e_lbrace:
            s << "lbrace";
            break;
        case sl::token_type_t::e_rbrace:
            s << "rbrace";
            break;
        case sl::token_type_t::e_lbracket:
            s << "lbracket";
            break;
        case sl::token_type_t::e_rbracket:
            s << "rbracket";
            break;
        case sl::token_type_t::e_semicolon:
            s << "semicolon";
            break;
    }

    s << "\t\tvalue: " << token.text;
    return s.str();
}

} // namespace std


#endif