#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include "parser.hpp"

namespace sl {

class code_gen_t {
public:
    code_gen_t(const ast_t& ast) : _ast(ast) {}
    
    std::string gen() {
        s << ".text\n";

        s << "\nstart:\n";
        for (auto& statement : _ast.statements) {
            switch (statement->type) {
                case statement_type_t::e_declaration:
                    gen_declaration(statement->as.declaration);
                    break;
                case statement_type_t::e_expression:
                    gen_expression(statement->as.expression);
                    s << "\t\n"; 
                    break;
                case statement_type_t::e_if:
                    gen_if(statement->as._if);
                    s << "\t\n"; 
                    break;
            }
        }
        s << "\thlt\n";
        return s.str();
    }

private:
    void gen_declaration(declaration_t *declaration) {
        if (declaration->type == declaration_type_t::e_simple) {
            variable_offset[declaration->as.simple.identifier->id] = variable_offset.size();
        } else {
            // throw std::runtime_error("complex declaration not implemented");
            expression_t left_expression{};
            left_expression.type = expression_type_t::e_unary;
            left_expression.as.unary.type = unary_type_t::e_identifier;
            left_expression.as.unary.as.identifier = declaration->as.complex.identifier;
            token_t assign_op = token_t{ .type = token_type_t::e_assign };
            expression_t expression{};
            expression.type = expression_type_t::e_binary;
            expression.as.binary.left_expression = &left_expression;
            expression.as.binary.op = &assign_op;
            expression.as.binary.right_expression = declaration->as.complex.expression;
            gen_expression(&expression);
        }
    }

    std::string get_register() {
        if (register_counter == 1) return "A";
        if (register_counter == 2) return "B";
        throw std::runtime_error("unexpected error");
    }

    void gen_expression(expression_t *expression) {
        if (expression->type == expression_type_t::e_unary) {
            register_counter++;
            if (expression->as.unary.type == unary_type_t::e_number) {
                s << "\tldi " << get_register() << " " << expression->as.unary.as.number->number << '\n';
            } else {
                s << "\tmov " << get_register() << " " << "M " << variable_offset[expression->as.unary.as.identifier->id] << '\n';
            }
            return;
        } else {
            if (expression->as.binary.op->type == token_type_t::e_assign) {
                gen_expression(expression->as.binary.right_expression);
                if (expression->as.binary.left_expression->as.unary.type == unary_type_t::e_number) {
                    throw std::runtime_error("cannot assign a number to another number");
                } else {
                    s << "\tmov M " << "A" << " " << variable_offset[expression->as.binary.left_expression->as.unary.as.identifier->id] << '\n';
                }
            } else if (expression->as.binary.op->type == token_type_t::e_plus) {
                gen_expression(expression->as.binary.right_expression);
                register_counter++;
                if (expression->as.binary.left_expression->as.unary.type == unary_type_t::e_number) {
                    s << "\tldi " << get_register() << " " << expression->as.binary.left_expression->as.unary.as.number->number << '\n';
                } else {
                    s << "\tmov " << get_register() << " M " << variable_offset[expression->as.binary.left_expression->as.unary.as.identifier->id] << '\n';
                }
                s << "\tadd\n"; 
            } else if (expression->as.binary.op->type == token_type_t::e_minus) {
                gen_expression(expression->as.binary.left_expression);
                register_counter++;
                if (expression->as.binary.right_expression->as.unary.type == unary_type_t::e_number) {
                    s << "\tldi " << get_register() << " " << expression->as.binary.right_expression->as.unary.as.number->number << '\n';
                } else {
                    s << "\tmov " << get_register() << " M " << variable_offset[expression->as.binary.right_expression->as.unary.as.identifier->id] << '\n';
                }
                s << "\tsub\n"; 
            } else if (expression->as.binary.op->type == token_type_t::e_equal) {
                gen_expression(expression->as.binary.right_expression);
                register_counter++;
                if (expression->as.binary.left_expression->as.unary.type == unary_type_t::e_number) {
                    s << "\tldi " << get_register() << " " << expression->as.binary.left_expression->as.unary.as.number->number << '\n';
                } else {
                    s << "\tmov " << get_register() << " M " << variable_offset[expression->as.binary.left_expression->as.unary.as.identifier->id] << '\n';
                }
                s << "\tcmp\n"; 
            }
            
            register_counter--;
            return;
        }
    }


    void gen_if(if_t *_if) {
        gen_expression(_if->expression);
        s << "\t\n"; 
        s << "\tjne %section" << section_number << '\n';
        s << "\t\n"; 
        register_counter--; // ??
        for (auto& statement : _if->truthy) {
            switch (statement->type) {
                case statement_type_t::e_declaration:
                    gen_declaration(statement->as.declaration);
                    break;
                case statement_type_t::e_expression:
                    gen_expression(statement->as.expression);
                    s << "\t\n"; 
                    break;
                case statement_type_t::e_if:
                    gen_if(statement->as._if);
                    s << "\t\n"; 
                    break;
            }
        }
        s << "section" << section_number++ << ":" << '\n';
    }

private:
    ast_t _ast;
    std::map<uint32_t, uint32_t> variable_offset;
    std::stringstream s;

    uint32_t register_counter{ 0 };
    uint32_t section_number{ 0 };
};

} // namespace sl

#endif