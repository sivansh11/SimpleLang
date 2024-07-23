#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

// just for testing ast

#include "parser.hpp"

#include <map>
#include <stack>

namespace sl {

class interpreter_t {
public:
    interpreter_t(const ast_t& ast, const std::vector<std::string>& identifier_table) : _ast(ast), _identifier_table(identifier_table) {}

    void run() {
        for (int32_t i = _ast.statements.size() - 1; i >= 0; i--) {
            statement_t *statement = _ast.statements[i];
            stack.push(statement);
        }

        while (stack.size()) {
            statement_t *statement = stack.top(); stack.pop();
            switch (statement->type) {
                case statement_type_t::e_declaration:
                    run_declaration(statement->as.declaration);
                    break;
                case statement_type_t::e_expression:
                    run_expression(statement->as.expression);
                    break;
                case statement_type_t::e_if:
                    run_if(statement->as._if);
                    break;
            }
        }
    }

private:
    void run_declaration(declaration_t *declaration) {
        switch(declaration->type) {
            case declaration_type_t::e_simple:
                _variables[_identifier_table[declaration->as.simple.identifier->id]] = 0;
                break;
            case declaration_type_t::e_complex:
                _variables[_identifier_table[declaration->as.complex.identifier->id]] = run_expression(declaration->as.complex.expression);
                break;
        }
    }

    uint8_t run_expression(expression_t *expression) {
        if (expression->type == expression_type_t::e_unary) {
            if (expression->as.unary.type == unary_type_t::e_number) {
                return expression->as.unary.as.number->number;
            } else {
                return _variables[_identifier_table[expression->as.unary.as.identifier->id]];
            }
        } 

        uint8_t acc = run_expression(expression->as.binary.right_expression);

        switch (expression->as.binary.op->type) {
            case token_type_t::e_assign:
                if (expression->as.binary.left_expression->type != expression_type_t::e_unary) throw std::runtime_error("unexpected error");
                _variables[_identifier_table[expression->as.binary.left_expression->as.unary.as.identifier->id]] = acc;
                break;
            case token_type_t::e_plus:
                acc = run_expression(expression->as.binary.left_expression) + acc;
                break;
            case token_type_t::e_minus:
                acc = run_expression(expression->as.binary.left_expression) - acc;
                break;
            case token_type_t::e_equal:
                acc = run_expression(expression->as.binary.left_expression) == acc;
                break;
        }
        return acc;
    }

    void run_if(if_t *_if) {
        uint8_t acc = run_expression(_if->expression);

        if (acc) {
            for (int i = _if->truthy.size() - 1; i >= 0; i--) {
                statement_t *statement = _if->truthy[i];
                stack.push(statement);
            }
        }
    }

private:
    ast_t _ast;
    std::vector<std::string> _identifier_table;

    std::map<std::string, uint8_t> _variables; 

    std::stack<statement_t *> stack;
};

} // namespace sl

#endif