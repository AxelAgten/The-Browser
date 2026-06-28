#include <iostream>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main() {
    Lexer lexer("test.js");
    std::vector<Token> tokens = lexer.Tokenize();

    Parser parser(tokens);
    std::vector<Node*> nodes = parser.ParseToAst();

    Interpreter interpreter(nodes);
    for (size_t i = 0; i < nodes.size(); i++) {
        std::visit([](auto&& value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::string>) {
                std::cout << value;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                std::cout << (value ? "true" : "false");
            }
            else if constexpr (std::is_same_v<T, undefined>) {
                std::cout << "undefined";
            }
            else if constexpr (std::is_same_v<T, null>) {
                std::cout << "null";
            }
            else {
                std::cout << value;
            }
        }, interpreter.EvaluateExpression(nodes[i]));
        std::cout << std::endl;
    }
    return 0;
}
