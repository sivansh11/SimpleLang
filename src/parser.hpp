#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "pool.hpp"

#include "result.hpp"

#include <variant>
#include <cstring>
#include <string>
#include <algorithm>

using namespace std::literals::string_literals;

namespace sl {

struct identifier_t {
    uint32_t id;  // index into identifier table, TODO add scope ?
};

struct number_t {
    uint32_t number;
};

enum declaration_type_t {
    e_simple, // int a;
    e_complex, // int a = expr;
};

struct expression_t;

struct declaration_t {
    declaration_type_t type;
    union as_t {
        struct simple_t {
            identifier_t *identifier;
        } simple;
        struct complex_t {
            identifier_t *identifier;
            expression_t *expression;
        } complex;
    } as;
};

enum expression_type_t {
    e_unary,  // a;
    e_binary, // a + expression;
};

enum unary_type_t {
    e_identifier,
    e_number,
};

struct expression_t {
    expression_type_t type;
    union as_t {
        struct unary_t {
            unary_type_t type;
            union as_t {
                identifier_t *identifier;
                number_t *number;
            } as;
        } unary;
        struct binary_t {
            expression_t *left_expression;
            token_t *op;
            expression_t *right_expression;
        } binary;
    } as;
};

struct statement_t;

struct if_t {
    expression_t *expression;
    std::vector<statement_t *>truthy;
};

enum statement_type_t {
    e_declaration,
    e_expression,
    e_if,
};

struct statement_t {
    statement_type_t type;
    union as_t {
        declaration_t *declaration;
        expression_t *expression;
        if_t *_if;
    } as;
};

struct ast_t {
    std::vector<statement_t *> statements;
};

class parser_t {
public:
    parser_t(const std::vector<token_t>& tokens) : _tokens(tokens) {}

    Result<std::pair<ast_t, std::vector<std::string>>, std::string> parse() {
        ast_t ast{};
        while (_index < _tokens.size()) {
            {
                auto result = parse_statement();
                if (result) {
                    auto [advance, statement] = result.unwrap();
                    _index += advance;
                    ast.statements.push_back(statement);
                    continue;
                } else {
                    return Err(result.unwrapErr());
                }
            }
        }
        return Ok(std::pair{ast, identifer_table});
    }

private:
    std::optional<token_t> try_get(uint32_t offset = 0) {
        if (_index + offset >= _tokens.size()) return std::nullopt;
        return _tokens[_index + offset];
    }

    Result<std::pair<uint32_t, declaration_t *>, std::string> parse_declaration() {
        auto token_0 = try_get(0);
        auto token_1 = try_get(1);
        auto token_2 = try_get(2);

        if (token_1 && token_1->type != token_type_t::e_identifier) return Err("expected an identifier"s);

        if (token_2 && token_2->type == token_type_t::e_semicolon) {
            identifier_t *identifier = pool.alloc<identifier_t>();
            auto itr = std::find(identifer_table.begin(), identifer_table.end(), token_1->text);
            if (itr != identifer_table.end()) return Err("Redeclaraing variable"s);
            identifier->id = std::distance(identifer_table.begin(), itr);
            identifer_table.push_back(token_1->text);

            declaration_t *declaration = pool.alloc<declaration_t>();
            declaration->type = declaration_type_t::e_simple;
            declaration->as.simple.identifier = identifier;

            return Ok(std::pair{ 3u, declaration });

        } else if (token_2 && token_2->type == token_type_t::e_assign) {
            identifier_t *identifier = pool.alloc<identifier_t>();
            auto itr = std::find(identifer_table.begin(), identifer_table.end(), token_1->text);
            if (itr != identifer_table.end()) return Err("Redeclaraing variable"s);
            identifier->id = std::distance(identifer_table.begin(), itr);
            identifer_table.push_back(token_1->text);

            declaration_t *declaration = pool.alloc<declaration_t>();
            declaration->type = declaration_type_t::e_complex;
            declaration->as.complex.identifier = identifier;

            _index += 3;

            auto result = parse_expression();
            if (result) {
                auto [advance, expression] = result.unwrap();
                declaration->as.complex.expression = expression;
                return Ok(std::pair{advance, declaration});
            } else {
                return Err(result.unwrapErr());
            }
        }

        return Err("unexpected error"s);

        // } else {
        //     return Err("complex declaration not implemented"s);
        // }
    }

    Result<std::pair<uint32_t, expression_t *>, std::string> parse_expression() {
        auto token_0 = try_get(0);
        auto token_1 = try_get(1);

        if (token_1 && (token_1->type == token_type_t::e_semicolon || token_1->type == token_type_t::e_rbrace || token_1->type == token_type_t::e_rbracket)) {
            // unary expression

            if (token_0->type == token_type_t::e_identifier) {
                identifier_t *identifier = pool.alloc<identifier_t>();
                auto itr = std::find(identifer_table.begin(), identifer_table.end(), token_0->text);
                if (itr == identifer_table.end()) {
                    return Err("identifier not found"s);
                }
                identifier->id = std::distance(identifer_table.begin(), itr);

                expression_t *expression = pool.alloc<expression_t>();
                expression->type = expression_type_t::e_unary;
                expression->as.unary.type = unary_type_t::e_identifier;
                expression->as.unary.as.identifier = identifier;

                return Ok(std::pair{2u, expression});
            } else if (token_0->type == token_type_t::e_number) {
                number_t *number = pool.alloc<number_t>();
                number->number = std::stoi(token_0->text);

                expression_t *expression = pool.alloc<expression_t>();
                expression->type = expression_type_t::e_unary;
                expression->as.unary.type = unary_type_t::e_number;
                expression->as.unary.as.number = number;

                return Ok(std::pair{2u, expression});
            }
        } else {
            // binary expression
            if (token_1 && (token_1->type == token_type_t::e_assign || token_1->type == token_type_t::e_plus || token_1->type == token_type_t::e_minus || token_1->type == token_type_t::e_equal)) {
                expression_t *expression = pool.alloc<expression_t>();
                expression->type = expression_type_t::e_binary;
                
                if (token_0 && token_0->type == token_type_t::e_identifier) {
                    identifier_t *identifier = pool.alloc<identifier_t>();
                    auto itr = std::find(identifer_table.begin(), identifer_table.end(), token_0->text);
                    if (itr == identifer_table.end()) {
                        return Err("identifier not found"s);
                    }
                    identifier->id = std::distance(identifer_table.begin(), itr);
                    expression->as.binary.left_expression = pool.alloc<expression_t>();
                    expression->as.binary.left_expression->type = expression_type_t::e_unary;
                    expression->as.binary.left_expression->as.unary.type = unary_type_t::e_identifier;
                    expression->as.binary.left_expression->as.unary.as.identifier = identifier;
                } else if (token_0 && token_0->type == token_type_t::e_number) {
                    number_t *number = pool.alloc<number_t>();
                    number->number = std::stoi(token_0->text);
                    expression->as.binary.left_expression = pool.alloc<expression_t>();
                    expression->as.binary.left_expression->type = expression_type_t::e_unary;
                    expression->as.binary.left_expression->as.unary.type = unary_type_t::e_number;
                    expression->as.binary.left_expression->as.unary.as.number = number;
                } else {
                    return Err("unexpected token"s);
                }
                
                expression->as.binary.op = pool.alloc<token_t>(*token_1);
                
                _index += 2;  

                auto result = parse_expression();
                if (result) {
                    auto [advance, sub_expression] = result.unwrap();
                    expression->as.binary.right_expression = sub_expression;
                    return Ok(std::pair{advance, expression});
                } else {
                    return Err(result.unwrapErr());
                }
            }

            // + - == 
            return Err("not recognised operator"s);
        }

        return Err("expcted ; or an operator"s);
    }

    Result<std::pair<uint32_t, if_t *>, std::string> parse_if() {
        auto token_0 = try_get(0);
        auto token_1 = try_get(1);

        if (token_1 && token_1->type != token_type_t::e_lbracket) return Err("Expected ("s);

        _index += 2;

        if_t *_if = pool.alloc<if_t>();

        auto result_expression = parse_expression();
        if (result_expression) {
            auto [advance, expression] = result_expression.unwrap();
            
            auto last_token = try_get(advance - 1);
            if (last_token && last_token->type != token_type_t::e_rbracket) return Err("Expected )"s);

            _if->expression = expression;

            _index += advance;

            auto current_token = try_get(0);
            if (current_token && current_token->type != token_type_t::e_lbrace) return Err("Expected {"s);

            _index += 1;

            while (_index < _tokens.size()) {
                {
                    auto result = parse_statement();
                    if (result) {
                        auto [advance, statement] = result.unwrap();
                        _index += advance;
                        if (statement) {
                            _if->truthy.push_back(statement);
                            continue;
                        } else {
                            return Ok(std::pair{0u, _if});
                        }
                    } else {
                        return Err(result.unwrapErr());
                    }
                }
            }
            
        }
        return Err("unparsable if"s);
    }

    Result<std::pair<uint32_t, statement_t *>, std::string> parse_statement() {
        auto token_0 = try_get(0);
        if (token_0 && token_0->type == token_type_t::e_int) {
            auto result = parse_declaration();
            if (result) {
                auto [advance, declaration] = result.unwrap();
                statement_t *statement = pool.alloc<statement_t>();
                statement->type = statement_type_t::e_declaration;
                statement->as.declaration = declaration;
                return Ok(std::pair{advance, statement});
            } else {
                return Err(result.unwrapErr());
            }
        } 

        if (token_0 && token_0->type == token_type_t::e_identifier) {
            auto result = parse_expression();
            if (result) {
                auto [advance, expression] = result.unwrap();
                statement_t *statement = pool.alloc<statement_t>();
                statement->type = statement_type_t::e_expression;
                statement->as.expression = expression;
                return Ok(std::pair{advance, statement});
            } else {
                return Err(result.unwrapErr());
            }
        }

        if (token_0 && token_0->type == token_type_t::e_if) {
            auto result = parse_if();
            if (result) {
                auto [advance, _if] = result.unwrap();
                statement_t *statement = pool.alloc<statement_t>();
                statement->type = statement_type_t::e_if;
                statement->as._if = _if;
                return Ok(std::pair{advance, statement});
            } else {
                return Err(result.unwrapErr());
            }
        }

        if (token_0 && token_0->type == token_type_t::e_rbrace) {
            return Ok(std::pair{1u, (statement_t *)nullptr});
        }
        return Err("unparsable statement"s);
    }
    
private:
    std::vector<token_t> _tokens;
    uint32_t _index{ 0 };
    pool_t pool;

    std::vector<std::string> identifer_table;  // TODO: add 2 maps, name to id, and id to name
};

} // namespace sl

#endif