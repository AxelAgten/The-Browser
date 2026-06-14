#include "interpreter.h"

#include <stdexcept>

Interpreter::Interpreter(const std::vector<Node*> &nodeList) : nodes(nodeList) {

}

Value Interpreter::EvaluateExpression(Node* node) {
	if (!(node->left || node->right)) {
		if (node->type == TokenType::Number) {
			return std::stod(node->op);
		}
		if (node->type == TokenType::True) {
			return true;
		}
		if (node->type == TokenType::False) {
			return false;
		}
		if (node->type == TokenType::String) {
			return node->op;
		}
		return variables[node->op];
	}

	if (node->op == "let") {
		variables.emplace(node->left->op, EvaluateExpression(node->right));
		return 0.0;
	}

	if (node->op == "=") {
		variables.at(node->left->op) = EvaluateExpression(node->right);
		return 0.0;
	}

	Value left = EvaluateExpression(node->left);
	Value right = EvaluateExpression(node->right);

	return std::visit([&](auto&& a) -> Value {
		return std::visit([&](auto&& b) -> Value {
			using A = std::decay_t<decltype(a)>;
			using B = std::decay_t<decltype(b)>;

			if constexpr (std::is_same_v<A, double> && std::is_same_v<B, double>) {
				if (node->op == "+") return a + b;
				if (node->op == "-") return a - b;
				if (node->op == "*") return a * b;
				if (node->op == "/") return a / b;
				if (node->op == "%") {
					int ia = static_cast<int>(a);
					int ib = static_cast<int>(b);

					if (ib == 0)
						throw std::runtime_error("Modulo by zero");

					return ia % ib;
				}
				if (node->op == ">") return a > b;
				if (node->op == ">=") return a >= b;
				if (node->op == "<") return a < b;
				if (node->op == "<=") return a <= b;
				if (node->op == "==") return a == b;
				if (node->op == "!=") return a != b;
			}
			else if constexpr (std::is_same_v<A, bool> && std::is_same_v<B, bool>) {
				if (node->op == "&&") return a && b;
				if (node->op == "||") return a || b;
				if (node->op == "==") return a == b;
				if (node->op == "!=") return a != b;
			}
			else if constexpr (std::is_same_v<A, std::string> && std::is_same_v<B, std::string>) {
				if (node->op == "+") return a + b;
				if (node->op == ">") return a > b;
				if (node->op == ">=") return a >= b;
				if (node->op == "<") return a < b;
				if (node->op == "<=") return a <= b;
				if (node->op == "==") return a == b;
				if (node->op == "!=") return a != b;
			}

			throw std::runtime_error("Unknown operator: " + node->op);

		}, right);
	}, left);
}

bool Interpreter::IsDouble(const std::string& s) {
	try {
		size_t pos;
		std::stod(s, &pos);

		// Ensure the whole string was consumed (no junk like "12abc")
		return pos == s.size();
	} catch (const std::invalid_argument&) {
		return false;
	} catch (const std::out_of_range&) {
		return false;
	}
}
