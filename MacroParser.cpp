#include <iostream>
#include <fstream>
#include <streambuf>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <stack>

class MacroParser {
public:
	MacroParser() {}
	MacroParser(std::string context) : context(context) {}
	void setContext(std::string context);
	std::string getContext();

	void load(std::string filename);
	void preDefine(std::string identifiers);
	void dumpDict();
	void dump(std::string filename);

private:
	std::string context;
	std::unordered_map<std::string, std::string> identifierToValue;
	std::unordered_set<std::string> preDefined;

	void mapTrigraphsAndDigraphs();
	void concateMultipleLines();
	void eliminateComments();
	void preprocessDirectives();
	void concateStrings();

	void parse();
	void copyPreDefined();
};

void MacroParser::setContext(std::string _context) {
	context = _context;
}

std::string MacroParser::getContext() {
	return context;
}

void MacroParser::parse() {
	copyPreDefined();
	preprocessDirectives();
	concateStrings();
}

void MacroParser::copyPreDefined() {
	for (auto &s : preDefined) {
		identifierToValue[s] = "";
	}
}

void MacroParser::load(std::string filename) {
	std::ifstream fin(filename);
	fin.seekg(0, std::ios::end);
	context.reserve(fin.tellg());
	fin.seekg(0, std::ios::beg);
	context.assign(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
	fin.close();

	if (context.back() != '\n') {
		context.push_back('\n');
	}

	mapTrigraphsAndDigraphs();
	concateMultipleLines();
	eliminateComments();
}

void MacroParser::preDefine(std::string identifiers) {
	preDefined.clear();
	int startPos = 0, endPos = 0;
	while ((endPos = identifiers.find_first_of(';', startPos)) != std::string::npos) {
		preDefined.insert(identifiers.substr(startPos, endPos-startPos));
		startPos = endPos+1;
	}
	preDefined.insert(identifiers.substr(startPos, endPos));
}

void MacroParser::dumpDict() {

}

void MacroParser::dump(std::string filename) {
	parse();
	std::ofstream fout(filename);
	for (auto &p : identifierToValue) {
		fout << "#define " << p.first << " " << p.second << std::endl;
	}
	fout.close();
	identifierToValue.clear();
}

void MacroParser::mapTrigraphsAndDigraphs() {
	
}

void MacroParser::concateMultipleLines() {
	
}

void MacroParser::eliminateComments() {
	enum class CommentStates {idle, startBackslash, blockComment, lineComment, blockEndAster, inString, inChar};
	CommentStates cs = CommentStates::idle;

	std::string result;
	for (int i = 0; i < context.size(); ++i) {
		char c = context[i];
		switch (cs) {
			case CommentStates::idle:
				if (c == '/') {
					cs = CommentStates::startBackslash;
				} else if (c == '"' && (!i || context[i-1] != '\\')) {
					cs = CommentStates::inString;
					result.push_back(c);
				} else if (c == '\'' && (!i || context[i-1] != '\\')) {
					cs = CommentStates::inChar;
					result.push_back(c);
				} else {
					result.push_back(c);
				}
				break;
			case CommentStates::startBackslash:
				if (c == '/') {
					cs = CommentStates::lineComment;
				} else if (c == '*') {
					cs = CommentStates::blockComment;
				} else {
					throw "Unexpected /";
				}
				break;
			case CommentStates::blockComment:
				if (c == '*') {
					cs = CommentStates::blockEndAster;
				}
				break;
			case CommentStates::lineComment:
				if (c == '\n') {
					cs = CommentStates::idle;
					result.push_back(' ');
					result.push_back('\n');
				}
				break;
			case CommentStates::blockEndAster:
				if (c == '/') {
					cs = CommentStates::idle;
					result.push_back(' ');
				} else {
					cs = CommentStates::blockComment;
				}
				break;
			case CommentStates::inString:
				if (c == '"' && (!i || context[i-1] != '\\')) {
					cs = CommentStates::idle;
				}
				result.push_back(c);
				break;
			case CommentStates::inChar:
				if (c == '\'' && (!i || context[i-1] != '\\')) {
					cs = CommentStates::idle;
				}
				result.push_back(c);
				break;
		}
	}
	switch (cs) {
		case CommentStates::idle:
			break;
		case CommentStates::startBackslash:
			throw "Unexpected /";
			break;
		case CommentStates::blockComment:
		case CommentStates::blockEndAster:
			throw "Unterminated block comment";
			break;
		case CommentStates::lineComment:
			throw "Unexpected ending of file";
			break;
		case CommentStates::inString:
			throw "Unterminated \" character";
			break;
		case CommentStates::inChar:
			throw "Unterminated ' character";
			break;
	}
	swap(result, context);
}

void MacroParser::preprocessDirectives() {
	enum class MacroStage {idle, directive, identifier, expression};
	enum class DirectiveType {_unknown, _ifdef, _ifndef, _else, _endif, _define, _undef};
	MacroStage ms = MacroStage::idle;
	DirectiveType dt = DirectiveType::_unknown;

	std::stack<signed char> isNestedTestTrue;
	std::stack<bool> isNestedLevelElseDefined;

	std::string identifier, expression;
	for (int i = 0; i < context.size(); ++i) {
		char c = context[i];
		switch (ms) {
			case MacroStage::idle:
				if (c == '#') {
					ms = MacroStage::directive;
					dt = DirectiveType::_unknown;
				}
				break;
			case MacroStage::directive:
				while (isspace(context[i]) && context[i] != '\n') {
					++i;
				}
				if (context[i] == '\n') {
					ms = MacroStage::idle;
				} else {
					int pos = i;
					while (!isspace(context[i])) {
						++i;
					}
					std::string directive = context.substr(pos, i-pos);
					if (directive == "ifdef") {
						dt = DirectiveType::_ifdef;
						ms = MacroStage::identifier;
					} else if (directive == "ifndef") {
						dt = DirectiveType::_ifndef;
						ms = MacroStage::identifier;
					} else if (directive == "else") {
						dt = DirectiveType::_else;
						if (isNestedTestTrue.empty()) {
							throw "#else without #if";
						}
						if (isNestedLevelElseDefined.top()) {
							throw "#else after #else";
						}
						while (isspace(context[i]) && context[i] != '\n') {
							++i;
						}
						if (context[i] != '\n') {
							throw "Extra token after #else";
						}
						if (isNestedTestTrue.top() != -1) {
							isNestedTestTrue.top() = 1-isNestedTestTrue.top();
						}
						isNestedLevelElseDefined.top() = true;
						ms = MacroStage::idle;
					} else if (directive == "endif") {
						dt = DirectiveType::_endif;
						if (isNestedTestTrue.empty()) {
							throw "#endif without #if";
						}
						while (isspace(context[i]) && context[i] != '\n') {
							++i;
						}
						if (context[i] != '\n') {
							throw "Extra token after #endif";
						}
						isNestedTestTrue.pop();
						isNestedLevelElseDefined.pop();
						ms = MacroStage::idle;
					} else if (directive == "define") {
						dt = DirectiveType::_define;
						if (isNestedTestTrue.empty() || isNestedTestTrue.top() == 1) {
							ms = MacroStage::identifier;
						} else {
							while (context[i] != '\n') {
								++i;
							}
							ms = MacroStage::idle;
						}
					} else if (directive == "undef") {
						dt = DirectiveType::_undef;
						if (isNestedTestTrue.empty() || isNestedTestTrue.top() == 1) {
							ms = MacroStage::identifier;
						} else {
							while (context[i] != '\n') {
								++i;
							}
							ms = MacroStage::idle;
						}
					} else {
						throw "Undefined directive "+directive;
					}
					--i;
					// std::cout << directive << std::endl;
				}
				break;
			case MacroStage::identifier:
				while (isspace(context[i]) && context[i] != '\n') {
					++i;
				}
				if (!islower(context[i]) && !isupper(context[i]) && context[i] != '_') {
					throw "Identifier cannot begin with "+context[i];
				} else {
					int pos = i;
					while (islower(context[i]) || isupper(context[i]) || isdigit(context[i]) || context[i] == '_') {
						++i;
					}
					identifier = context.substr(pos, i-pos);
					if (!isspace(context[i])) {
						throw identifier+" is not an identifier";
					}
					if (dt == DirectiveType::_define) {
						if (identifierToValue.find(identifier) != identifierToValue.end()) {
							throw "Identifier "+identifier+" defined multiple times";
						}
						ms = MacroStage::expression;
					} else if (dt == DirectiveType::_ifdef) {
						if (isNestedTestTrue.empty() || isNestedTestTrue.top() == 1) {
							isNestedTestTrue.push(identifierToValue.find(identifier) != identifierToValue.end());
						} else {
							isNestedTestTrue.push(-1);
						}
						isNestedLevelElseDefined.push(false);
						ms = MacroStage::idle;
					} else if (dt == DirectiveType::_ifndef) {
						if (isNestedTestTrue.empty() || isNestedTestTrue.top() == 1) {
							isNestedTestTrue.push(identifierToValue.find(identifier) == identifierToValue.end());
						} else {
							isNestedTestTrue.push(-1);
						}
						isNestedLevelElseDefined.push(false);
						ms = MacroStage::idle;
					} else if (dt == DirectiveType::_undef) {
						identifierToValue.erase(identifier);
						ms = MacroStage::idle;
					}
					--i;
					// std::cout << identifier << std::endl;
				}
				break;
			case MacroStage::expression:
				while (isspace(context[i]) && context[i] != '\n') {
					++i;
				}
				int pos = i;
				while (context[i] != '\n') {
					++i;
				}
				expression = context.substr(pos, i-pos);
				identifierToValue[identifier] = expression;
				ms = MacroStage::idle;
				// std::cout << expression << std::endl;
				break;
		}
	}
}

void MacroParser::concateStrings() {
	
}

int main(int argc, char const *argv[])
{
	try {
		MacroParser mp1;
		MacroParser mp2;
		mp1.load("test2.cpp");
		mp1.dump("test3.cpp");
		mp2.load("test3.cpp");
		mp2.dump("mp2.cpp");
		mp1.preDefine("MC1;MC2");
		mp1.dump("mp1.cpp");
	} catch (const char *msg) {
		std::cout << msg << std::endl;
	} catch (const std::string &msg) {
		std::cout << msg << std::endl;
	}
	return 0;
}