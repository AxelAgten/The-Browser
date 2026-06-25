#ifndef JS_INTERPRETER_H
#define JS_INTERPRETER_H

#include "parser.h"
#include <unordered_map>
#include <variant>
#include <ostream>

struct null;

struct undefined {
	bool operator==(const undefined& rhs) const {return true;}
	bool operator!=(const undefined& rhs) const {return false;}
};

struct null {
	bool operator==(const null& rhs) const {return true;}
	bool operator!=(const null& rhs) const {return false;}
};

std::ostream& operator<<(std::ostream& os, const undefined&);
std::ostream& operator<<(std::ostream& os, const null&);

using Value = std::variant<double, std::string, bool, undefined, null>;

class Interpreter {
public:
	explicit Interpreter(const std::vector<Node*> &nodeList);

	Value EvaluateExpression(Node* node);
private:
	bool IsDouble(const std::string& s);
	std::string DoubleToStr(Value v);
	double StrToNumber(Value v);
	std::string BoolToStr(Value v);
	double BoolToDouble(Value v);
	double ToNumber(Value v);
	std::string ToString(Value v);
	bool ToBool(Value v);
	bool LooseEquals(Value left, Value right);

	std::vector<Node*> nodes;
	std::unordered_map<std::string, Value> variables;
};


#endif //JS_INTERPRETER_H