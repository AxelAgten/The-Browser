#ifndef JS_INTERPRETER_H
#define JS_INTERPRETER_H

#include "parser.h"
#include <unordered_map>
#include <variant>

using Value = std::variant<double, std::string, bool>;

class Interpreter {
public:
	explicit Interpreter(const std::vector<Node*> &nodeList);

	Value EvaluateExpression(Node* node);
private:
	bool IsDouble(const std::string& s);

	std::vector<Node*> nodes;
	std::unordered_map<std::string, Value> variables;
};


#endif //JS_INTERPRETER_H