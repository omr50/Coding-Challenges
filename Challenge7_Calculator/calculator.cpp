#include <vector>
#include <string>
#include <unordered_set>
#include <stdexcept>

std::unordered_set operations;
enum opTypes { PLUS = 1, MINUS, MULT, DIV, NUM, OPEN_PAREN, CLOSING_PAREN };

bool inOperations(char operation){
	return operations.find(c) == operations.end();
}

bool inOperations(opTypes op) {
	return (op == MINUS || op == MULT || op == PLUS || op == DIV);
}

bool isDigit(char c) {
	return (c >= '0' && c <= '9');
}

float makeNum(int &i, std::string expression) {
	string num = "";
	bool decimalPlace = false;
	while (i < expression.size()) {
		char c = expression[i];
		if (isDigit(c)) {
			num += c;
		}
		else if (c == '.') {
			if (!decimalPlace) {
				decimalPlace = true;
				num += c;
			} else {
				throw std::invalid_argument("There is 2 decimal places\n");
				return 0;
			}
		} else if (c == ' ') {

			// end here
			// don't increment i
			break;
		}	
		else {
			throw std::invalid_argument("The character: " + c + " cannot be part of a number!\n");
			return 0;
		}
		i++;
	}

	return std::stoi(num);
}

int main() {

	// make all of the numbers doubles. Better than
	// dealing with int division accidentally
	
	// use a vector to hold the series of operation 
	// formulate them into tokens and valid or invalid
	// operations.
	
	std::vector<string> stack;
	std::vector<string> opStack;

	operations.insert("+");
	operations.insert("-");
	operations.insert("/");
	operations.insert("*");

	std::string expression;
	printf("Enter expression:\n);
	scanf("%s", expression);

	// also handle characters that are beyond the scope of this project


	for (int i = 0; i < expression.size(); i++) {
		char c = expression[i];
		bool isOpenParen = (true ? c == '(' else false);
		bool digit = isDigit(c);
		int vars = 0;
		float number = 0;
		if (digit) {
			try {
				number = makeNum(i, expression);
			} catch (const std::invalid_argument& e) {
				printf(e);
			}

		}
		opTypes prev = NULL;
		if (c == ' ') {
			continue;
		}

		if (stack.empty()) {
			// if empty and not operation then we can continue 
			if (isOpenParen) {
				stack.push_back(c);
				prev = OPEN_PAREN;
			} else if inOperations(c)  {
				printf("Error! Operation was added before a number\n");
				return 0;
			} else if (digit) {
				stack.push_back(number);		
				prev = NUM;
				vars++;
			else {
				printf("Character " + c + " is invalid character!\n");
				return 0;
			}
		} 
		// stack has something inside
		else {
			char upper = stack.back();		
			// previous char is an operation
			if (inOperations(prev)) {
				if (inOperations(c)) {
					// may add the exact point in the string where the error occured for clarity.
					printf("Error! Operation was added directly after another operation!\n");
					return 0;
				}
				else if (isOpenParen) {
					stack.push_back(c);
					continue;
				} else if (digit) {
					stack.push_back(number);
					continue;
				}
			} else if (isOpenParen) {
				if (inOperations(c)) {
					printf("Error! Cannot have operation directly after opening parenthesis!\n);
					return 0;
				} else if (digit) {
					stack.push_back(number);
				}
				else if (c == ')') {
					// i guess its technically valid but just do nothing
					stack.pop();
				}
			} else if (prev == NUM) {
				if (isDigit(c)) {
					// cant have digit after digit
					printf("Error! Cannot have a number after another number!\n);
					return 0;
				}	

			}
		}

	}
}
