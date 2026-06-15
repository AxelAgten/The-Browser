#include "interpreter.h"

#include <stdexcept>
#include <cmath>
#include <sstream>
#include <limits>
#include <cctype>

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

	auto doubleToStr = []<typename T>(T&& v) -> std::string {
		if constexpr (std::is_same_v<std::decay_t<T>, double>) {
			if (std::isnan(v)) return "NaN";
			if (std::isinf(v)) return v > 0 ? "Infinity" : "-Infinity";

			std::ostringstream oss;
			if (v == static_cast<long long>(v))
				oss << static_cast<long long>(v);
			else
				oss << v;
			return oss.str();
		}
		return "";
	};

	auto strToDouble = []<typename T>(T&& v) -> double {
		if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
			if (v == "") return 0.0;
			bool isWhiteSpace = true;
			for (const char &c : v) {
				if (!isspace(c)) isWhiteSpace = false;
			}
			if (isWhiteSpace) return 0.0;

			try {
				return std::stod(v);
			}
			catch (const std::invalid_argument& e) {
				return std::numeric_limits<double>::quiet_NaN();
			}
			catch (const std::out_of_range& e) {
				return std::numeric_limits<double>::quiet_NaN();
			}
		}
		return std::numeric_limits<double>::quiet_NaN();
	};

	auto isStrDouble = []<typename T>(T&& v) -> bool {
		if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
			try {
				double d = std::stod(v);
				return true;
			}
			catch (const std::invalid_argument& e) {
				return false;
			}
			catch (const std::out_of_range& e) {
				return false;
			}
		}
		return false;
	};

	auto boolToStr = []<typename T>(T&& v) -> std::string {
		if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
			if (v) {
				return "true";
			}
			return "false";
		}
		return "NaN";
	};

	auto boolToDouble = []<typename T>(T&& v) -> double {
		if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
			if (v) {
				return 1;
			}
			return 0;
		}
		return std::numeric_limits<double>::quiet_NaN();
	};

	auto isNanSafe = []<typename T>(T&& v) -> bool {
		if constexpr (std::is_same_v<std::decay_t<T>, double>) {
			return std::isnan(v);
		}
		return false;
	};

	if (node->left && !node->right) {
		Value left = EvaluateExpression(node->left);
		return std::visit([&](auto&& a) -> Value {
			using A = std::decay_t<decltype(a)>;
			if constexpr (std::is_same_v<A, double>) {
				switch (node->type) {
					case TokenType::Plus: return +a;
					case TokenType::Minus: return -a;
					case TokenType::ExclamationMark: return a == 0;
				}
			}
			else if constexpr (std::is_same_v<A, bool>) {
				switch (node->type) {
					case TokenType::Plus: return +boolToDouble(a);
					case TokenType::Minus: return -boolToDouble(a);
					case TokenType::ExclamationMark: return !a;
				}
			}
			else if constexpr (std::is_same_v<A, std::string>) {
				switch (node->type) {
					case TokenType::Plus: return +strToDouble(a);
					case TokenType::Minus: return -strToDouble(a);
					case TokenType::ExclamationMark: return a != "";
				}
			}
			throw std::runtime_error("Unknown operator: " + node->op);
		}, left);
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

				switch (node->type) {
					case TokenType::Plus: return a + b;
					case TokenType::Minus: return a - b;
					case TokenType::Star: return a * b;
					case TokenType::Slash: return a / b;
					case TokenType::Percent: return std::fmod(a, b);
					case TokenType::StarStar: return pow(a, b);
					case TokenType::LargerThen:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a > b;
					case TokenType::LargerEquals:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a >= b;
					case TokenType::SmallerThen:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a < b;
					case TokenType::SmallerEquals:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a <= b;
					case TokenType::Equals:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a == b;
					case TokenType::NotEquals:
						if (isNanSafe(a) || isNanSafe(b)) return false;
						return a != b;
				}
			}
	    	else if constexpr (std::is_same_v<A, bool> && std::is_same_v<B, bool>) {

				switch (node->type) {
					case TokenType::And: return a && b;
					case TokenType::Or: return a || b;
					case TokenType::Equals: return a == b;
					case TokenType::NotEquals: return a != b;
					case TokenType::Plus: return boolToDouble(a) + boolToDouble(b);
					case TokenType::Minus: return boolToDouble(a) - boolToDouble(b);
					case TokenType::Star: return boolToDouble(a) * boolToDouble(b);
					case TokenType::Slash: return boolToDouble(a) / boolToDouble(b);
					case TokenType::Percent: return std::fmod(boolToDouble(a), boolToDouble(b));
					case TokenType::StarStar: return pow(boolToDouble(a), boolToDouble(b));
					case TokenType::LargerThen: return boolToDouble(a) > boolToDouble(b);
					case TokenType::LargerEquals: return boolToDouble(a) >= boolToDouble(b);
					case TokenType::SmallerThen: return boolToDouble(a) < boolToDouble(b);
					case TokenType::SmallerEquals: return boolToDouble(a) <= boolToDouble(b);
				}
			}
	    	else if constexpr (std::is_same_v<A, std::string> && std::is_same_v<B, std::string>) {
				switch (node->type) {
					case TokenType::Plus: return a + b;
					case TokenType::Minus: return strToDouble(a) - strToDouble(b);
					case TokenType::Star: return strToDouble(a) * strToDouble(b);
					case TokenType::Slash: return strToDouble(a) / strToDouble(b);
					case TokenType::Percent: return std::fmod(strToDouble(a), strToDouble(b));
					case TokenType::StarStar: return pow(strToDouble(a), strToDouble(b));
					case TokenType::LargerThen: return a > b;
					case TokenType::LargerEquals: return a >= b;
					case TokenType::SmallerThen: return a < b;
					case TokenType::SmallerEquals: return a <= b;
					case TokenType::Equals: return a == b;
					case TokenType::NotEquals: return a != b;
				}
			}
	    	else if constexpr (std::is_same_v<A, double> && std::is_same_v<B, std::string>) {

				switch (node->type) {
					case TokenType::Plus: return doubleToStr(a) + b;
					case TokenType::Minus: return a - strToDouble(b);
					case TokenType::Star: return a * strToDouble(b);
					case TokenType::Slash: return a / strToDouble(b);
					case TokenType::Percent: return std::fmod(a, strToDouble(b));
					case TokenType::StarStar: return pow(a, strToDouble(b));
					case TokenType::Equals:
						if (isStrDouble(b)) return a == strToDouble(b);
						return false;
					case TokenType::LargerThen:
						if (isStrDouble(b)) return a > strToDouble(b);
						return false;
					case TokenType::LargerEquals:
						if (isStrDouble(b)) return a >= strToDouble(b);
						return false;
					case TokenType::SmallerThen:
						if (isStrDouble(b)) return a < strToDouble(b);
						return false;
					case TokenType::SmallerEquals:
						if (isStrDouble(b)) return a <= strToDouble(b);
						return false;
					case TokenType::NotEquals:
						if (isStrDouble(b)) return a != strToDouble(b);
						return false;
				}
			}
	    	else if constexpr (std::is_same_v<A, std::string> && std::is_same_v<B, double>) {
				switch (node->type) {
					case TokenType::Plus: return a + doubleToStr(b);
					case TokenType::Minus: return strToDouble(a) - b;
					case TokenType::Star: return strToDouble(a) * b;
					case TokenType::Slash: return strToDouble(a) / b;
					case TokenType::Percent: return std::fmod(strToDouble(a), b);
					case TokenType::StarStar: return pow(strToDouble(a), b);
					case TokenType::Equals:
						if (isStrDouble(a)) return strToDouble(a) == b;
						return false;
					case TokenType::LargerThen:
						if (isStrDouble(a)) return strToDouble(a) > b;
						return false;
					case TokenType::LargerEquals:
						if (isStrDouble(a)) return strToDouble(a) >= b;
						return false;
					case TokenType::SmallerThen:
						if (isStrDouble(a)) return strToDouble(a) < b;
						return false;
					case TokenType::SmallerEquals:
						if (isStrDouble(a)) return strToDouble(a) <= b;
						return false;
					case TokenType::NotEquals:
						if (isStrDouble(a)) return strToDouble(a) != b;
						return false;
				}
			}
	    	else if constexpr (std::is_same_v<A, bool> && std::is_same_v<B, std::string>) {

				switch (node->type) {
					case TokenType::Plus: return boolToStr(a) + b;
					case TokenType::Star: return boolToDouble(a) * strToDouble(b);
					case TokenType::Minus: return boolToDouble(a) - strToDouble(b);
					case TokenType::Slash: return boolToDouble(a) / strToDouble(b);
					case TokenType::Percent: return std::fmod(boolToDouble(a), strToDouble(b));
					case TokenType::StarStar: return pow(boolToDouble(a), strToDouble(b));
					case TokenType::LargerThen: return boolToDouble(a) > strToDouble(b);
					case TokenType::LargerEquals: return boolToDouble(a) >= strToDouble(b);
					case TokenType::SmallerThen: return boolToDouble(a) < strToDouble(b);
					case TokenType::SmallerEquals: return boolToDouble(a) <= strToDouble(b);
					case TokenType::Equals: return boolToDouble(a) == strToDouble(b);
					case TokenType::NotEquals: return boolToDouble(a) != strToDouble(b);
				}
			}
	    	else if constexpr (std::is_same_v<A, std::string> && std::is_same_v<B, bool>) {
				switch (node->type) {
					case TokenType::Plus: return a + boolToStr(b);
					case TokenType::Star: return strToDouble(a) * boolToDouble(b);
					case TokenType::Minus: return strToDouble(a) - boolToDouble(a);
					case TokenType::Slash: return strToDouble(a) / boolToDouble(a);
					case TokenType::Percent: return std::fmod(strToDouble(a), boolToDouble(a));
					case TokenType::StarStar: return pow(strToDouble(a), boolToDouble(a));
					case TokenType::LargerThen: return strToDouble(a) > boolToDouble(a);
					case TokenType::LargerEquals: return strToDouble(a) >= boolToDouble(a);
					case TokenType::SmallerThen: return strToDouble(a) < boolToDouble(a);
					case TokenType::SmallerEquals: return strToDouble(a) <= boolToDouble(a);
					case TokenType::Equals: return strToDouble(a) == boolToDouble(a);
					case TokenType::NotEquals: return strToDouble(a) != boolToDouble(a);
				}
			}
	    	else if constexpr (std::is_same_v<A, double> && std::is_same_v<B, bool>) {
				switch (node->type) {
					case TokenType::Plus: return a + b;
					case TokenType::Minus: return a - b;
					case TokenType::Star: return a * b;
					case TokenType::Slash: return a / b;
					case TokenType::Percent: return std::fmod(a, b);
					case TokenType::StarStar: return pow(a, boolToDouble(b));
					case TokenType::LargerThen: return a > b;
					case TokenType::LargerEquals: return a >= b;
					case TokenType::SmallerThen: return a < b;
					case TokenType::SmallerEquals: return a <= b;
					case TokenType::Equals: return a == b;
					case TokenType::NotEquals: return a != b;
				}
			}
	    	else if constexpr (std::is_same_v<A, bool> && std::is_same_v<B, double>) {
				switch (node->type) {
					case TokenType::Plus: return a + b;
					case TokenType::Minus: return a - b;
					case TokenType::Star: return a * b;
					case TokenType::Slash: return a / b;
					case TokenType::Percent: return std::fmod(a, b);
					case TokenType::StarStar: return pow(boolToDouble(a), b);
					case TokenType::LargerThen: return a > b;
					case TokenType::LargerEquals: return a >= b;
					case TokenType::SmallerThen: return a < b;
					case TokenType::SmallerEquals: return a <= b;
					case TokenType::Equals: return a == b;
					case TokenType::NotEquals: return a != b;
				}
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
