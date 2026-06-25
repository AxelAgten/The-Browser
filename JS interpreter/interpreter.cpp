#include "interpreter.h"

#include <stdexcept>
#include <cmath>
#include <sstream>
#include <limits>
#include <cctype>

Interpreter::Interpreter(const std::vector<Node*> &nodeList) : nodes(nodeList) {

}

std::ostream& operator<<(std::ostream& os, const undefined&) {return os << "undefined";}
std::ostream& operator<<(std::ostream& os, const null&) {return os << "null";}

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
		if (variables.find(node->op) == variables.end()) {
			throw std::runtime_error("ReferenceError: '" +node->op + "' is not defined");
		}
		return variables[node->op];
	}

	auto tokenTypeFromBool = []<typename  T>(T&& v) -> TokenType {
		if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
			if (v) {
				return TokenType::True;
			}
			return TokenType::False;
		}
		throw std::runtime_error("This variable is no boolean");
	};

	if (node->left && !node->right && node->left->type == TokenType::Identifier && (node->type == TokenType::PlusPlus || node->type == TokenType::MinMin)) {
		if (variables.find(node->left->op) == variables.end()) throw std::runtime_error(node->op + " not used on a variable");

		Node *parent, *right, *left;

		std::visit([&](auto&& a) {
			using A = std::decay_t<decltype(a)>;
			if constexpr (std::is_same_v<A, double>) {
				left = new Node(DoubleToStr(a), nullptr, nullptr, TokenType::Number);
			} else if constexpr (std::is_same_v<A, std::string>) {
				left = new Node(a, nullptr, nullptr, TokenType::Number);
			} else if constexpr (std::is_same_v<A, bool>) {
				left = new Node(DoubleToStr(BoolToDouble(a)), nullptr, nullptr, TokenType::Number);
			}
		}, variables[node->left->op]);

		right = new Node("1", nullptr, nullptr, TokenType::Number);

		switch (node->type) {
		case TokenType::PlusPlus: parent = new Node("+", left, right, TokenType::Plus); break;
		case TokenType::MinMin: parent = new Node("-", left, right, TokenType::Minus); break;
		}

		Value value = EvaluateExpression(parent);
		delete left; delete right; delete parent;
		variables[node->left->op] = value;
		return value;
	}
	if (!node->left && node->right && node->right->type == TokenType::Identifier  && (node->type == TokenType::PlusPlus || node->type == TokenType::MinMin)) {
		if (variables.find(node->right->op) == variables.end()) throw std::runtime_error(node->op + " not used on a variable");

		Node *parent, *right, *left;
		Value value = variables[node->right->op];

		std::visit([&](auto&& a) {
			using A = std::decay_t<decltype(a)>;
			if constexpr (std::is_same_v<A, double>) {
				left = new Node(DoubleToStr(a), nullptr, nullptr, TokenType::Number);
			} else if constexpr (std::is_same_v<A, std::string>) {
				left = new Node(a, nullptr, nullptr, TokenType::Number);
			} else if constexpr (std::is_same_v<A, bool>) {
				left = new Node(DoubleToStr(BoolToDouble(a)), nullptr, nullptr, TokenType::Number);
			}
		}, variables[node->right->op]);


		right = new Node("1", nullptr, nullptr, TokenType::Number);

		switch (node->type) {
		case TokenType::PlusPlus: parent = new Node("+", left, right, TokenType::Plus); break;
		case TokenType::MinMin: parent = new Node("-", left, right, TokenType::Minus); break;
		}

		Value newValue = EvaluateExpression(parent);
		delete left; delete right; delete parent;
		variables[node->right->op] = newValue;
		return value;
	}

	if (node->left && !node->right) {
		Value left = EvaluateExpression(node->left);
		switch (node->type) {
		case TokenType::Plus: return ToNumber(left);
		case TokenType::Minus: return -ToNumber(left);
		case TokenType::ExclamationMark: return !ToBool(left);
		}
	}

	if (node->type == TokenType::Let) {
		if (variables.find(node->left->op) != variables.end()) {
			throw std::runtime_error("SyntaxError: Identifier '" +node->op + "' has already been declared");
		}
		variables.emplace(node->left->op, EvaluateExpression(node->right));
		return variables[node->left->op];
	}

	if (node->type == TokenType::Equals) {
		if (variables.find(node->left->op) == variables.end()) {
			throw std::runtime_error("variable " + node->left->op + " does not exist");

		}
		variables.at(node->left->op) = EvaluateExpression(node->right);
		return variables[node->left->op];
	}

	if (node->type == TokenType::StrictEquals) {
		Value left = EvaluateExpression(node->left);
		Value right = EvaluateExpression(node->right);
		return left.index() == right.index() && left == right;
	}
	if (node->type == TokenType::StrictNotEquals) {
		Value left = EvaluateExpression(node->left);
		Value right = EvaluateExpression(node->right);
		return left.index() != right.index() || left != right;
	}

	if (node->type == TokenType::PlusEquals || node->type == TokenType::MinEquals || node->type == TokenType::StarEquals || node->type == TokenType::SlashEquals || node->type == TokenType::PercentEquals) {
		if (node->left->type != TokenType::Identifier) {
			throw std::runtime_error("This operator can only be used on variables");
		}
		if (variables.find(node->left->op) == variables.end()) {
			throw std::runtime_error("variable " + node->left->op + " does not exist");
		}

		Node *left, *parent;

		std::visit([&](auto&& a) {
			using A = std::decay_t<decltype(a)>;
			if constexpr (std::is_same_v<A, double>) {
				left = new Node(DoubleToStr(a), nullptr, nullptr, TokenType::Number);
			} else if constexpr (std::is_same_v<A, std::string>) {
				left = new Node(a, nullptr, nullptr, TokenType::String);
			} else if constexpr (std::is_same_v<A, bool>) {
				left = new Node(BoolToStr(a), nullptr, nullptr, tokenTypeFromBool(a));
			}
		}, variables[node->left->op]);

		switch (node->type) {
		case TokenType::PlusEquals: parent = new Node("+", left, node->right, TokenType::Plus); break;
		case TokenType::MinEquals: parent = new Node("-", left, node->right, TokenType::Minus); break;
		case TokenType::StarEquals: parent = new Node("*", left, node->right, TokenType::Star); break;
		case TokenType::SlashEquals: parent = new Node("/", left, node->right, TokenType::Slash); break;
		case TokenType::PercentEquals: parent = new Node("%", left, node->right, TokenType::Percent); break;
		}

		Value value = EvaluateExpression(parent);
		delete left;
		delete parent;
		variables[node->left->op] = value;
		return value;
	}

	Value left = EvaluateExpression(node->left);
	Value right = EvaluateExpression(node->right);

	switch (node->type) {
	case TokenType::Plus: {

		if (std::holds_alternative<std::string>(left) ||
			std::holds_alternative<std::string>(right))
		{
			return ToString(left) + ToString(right);
		}

		return ToNumber(left) + ToNumber(right);
	}
	case TokenType::Minus: return ToNumber(left) - ToNumber(right);
	case TokenType::Star: return ToNumber(left) * ToNumber(right);
	case TokenType::Slash: return ToNumber(left) / ToNumber(right);
	case TokenType::Percent: return std::fmod(ToNumber(left), ToNumber(right));
	case TokenType::StarStar: return pow(ToNumber(left), ToNumber(right));
	case TokenType::LargerThen: {
		if (std::holds_alternative<std::string>(left) &&
			std::holds_alternative<std::string>(right))
		{
			return ToString(left) > ToString(right);
		}
		return ToNumber(left) > ToNumber(right);
	}
	case TokenType::LargerEquals: {
		if (std::holds_alternative<std::string>(left) &&
			std::holds_alternative<std::string>(right))
		{
			return ToString(left) >= ToString(right);
		}
		return ToNumber(left) >= ToNumber(right);
	}
	case TokenType::SmallerThen: {
		if (std::holds_alternative<std::string>(left) &&
			std::holds_alternative<std::string>(right))
		{
			return ToString(left) < ToString(right);
		}
		return ToNumber(left) < ToNumber(right);
	}
	case TokenType::SmallerEquals: {
		if (std::holds_alternative<std::string>(left) &&
			std::holds_alternative<std::string>(right))
		{
			return ToString(left) <= ToString(right);
		}
		return ToNumber(left) <= ToNumber(right);
	}
	case TokenType::Equals: return LooseEquals(left, right);
	case TokenType::NotEquals: return !LooseEquals(left, right);
	case TokenType::And:
	{
		if (!ToBool(left))
			return left;

		return right;
	}

	case TokenType::Or:
	{
		if (ToBool(left))
			return left;

		return right;
	}
	}
	return 0.0;
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

std::string Interpreter::DoubleToStr(Value v) {
	return std::visit([&](auto&& a) -> std::string {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, double>) {
			if (std::isnan(a)) return "NaN";
			if (std::isinf(a)) return a > 0 ? "Infinity" : "-Infinity";

			std::ostringstream oss;
			if (a == static_cast<long long>(a))
				oss << static_cast<long long>(a);
			else
				oss << a;
			return oss.str();
		}
		throw std::runtime_error("this is not a double");
	}, v);
}

double Interpreter::StrToNumber(Value v) {
	return std::visit([&](auto&& a) -> double {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, std::string>) {
			if (a == "") return 0.0;
			bool isWhiteSpace = true;
			for (const char &c : a) {
				if (!isspace(c)) isWhiteSpace = false;
			}
			if (isWhiteSpace) return 0.0;

			try {
				size_t pos;
				double d = std::stod(a,&pos);
				if(pos != a.size())
					return std::numeric_limits<double>::quiet_NaN();
				return d;
			}
			catch (const std::invalid_argument& e) {
				return std::numeric_limits<double>::quiet_NaN();
			}
			catch (const std::out_of_range& e) {
				return std::numeric_limits<double>::quiet_NaN();
			}
		}
		return std::numeric_limits<double>::quiet_NaN();
	}, v);
}

std::string Interpreter::BoolToStr(Value v) {
	return std::visit([&](auto&& a) -> std::string {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, bool>) {
			if (a) {
				return "true";
			}
			return "false";
		}
		return "NaN";
	}, v);
}

double Interpreter::BoolToDouble(Value v) {
	return std::visit([&](auto&& a) -> double{
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, bool>) {
			if (a) {
				return 1;
			}
			return 0;
		}
		return std::numeric_limits<double>::quiet_NaN();
	}, v);
}

double Interpreter::ToNumber(Value v) {
	return std::visit([&](auto&& a) -> double {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, double>) {
			return a;
		}
		if constexpr (std::is_same_v<A, bool>) {
			return BoolToDouble(a);
		}
		if constexpr (std::is_same_v<A, std::string>) {
			return StrToNumber(a);
		}
		if constexpr (std::is_same_v<A, null>) {
			return  0.0;
		}
		if constexpr (std::is_same_v<A, undefined>) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		throw std::runtime_error("can not convert this item to a number");
	}, v);
}

std::string Interpreter::ToString(Value v) {
	return std::visit([&](auto&& a) -> std::string {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, double>) {
		return DoubleToStr(a);
	}
	if constexpr (std::is_same_v<A, bool>) {
		return BoolToStr(a);
	}
	if constexpr (std::is_same_v<A, std::string>) {
		return a;
	}
	if constexpr (std::is_same_v<A, undefined>) {
		return "undefined";
	}
	if constexpr (std::is_same_v<A, null>) {
		return "null";
	}
	throw std::runtime_error("can not convert this item to a string");
	}, v);
}

bool Interpreter::ToBool(Value v) {
	return std::visit([&](auto&& a) -> bool {
		using A = std::decay_t<decltype(a)>;
		if constexpr (std::is_same_v<A, bool>) {
			return a;
		}
		if constexpr (std::is_same_v<A, double>) {
			return a != 0 && !std::isnan(a);
		}
		if constexpr (std::is_same_v<A, std::string>) {
			return !a.empty();
		}
		if constexpr (std::is_same_v<A, undefined>) {
			return false;
		}
		if constexpr (std::is_same_v<A, null>) {
			return false;
		}
		throw std::runtime_error("can not convert this item to a bool");
	}, v);
}

bool Interpreter::LooseEquals(Value left, Value right)
{
	// same type
	if (left.index() == right.index())
		return left == right;


	// null == undefined
	if ((std::holds_alternative<null>(left) &&
		 std::holds_alternative<undefined>(right)) ||
		(std::holds_alternative<undefined>(left) &&
		 std::holds_alternative<null>(right)))
	{
		return true;
	}


	// boolean gets converted to number
	if (std::holds_alternative<bool>(left))
		left = ToNumber(left);

	if (std::holds_alternative<bool>(right))
		right = ToNumber(right);


	// number/string conversion
	if ((std::holds_alternative<double>(left) &&
		 std::holds_alternative<std::string>(right)) ||
		(std::holds_alternative<std::string>(left) &&
		 std::holds_alternative<double>(right)))
	{
		return ToNumber(left) == ToNumber(right);
	}


	return false;
}