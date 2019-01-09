#include <iostream>
#include <unordered_map>

using namespace std;

#define/* asd  asd */dat/* asd  asd */0x20

#define logger cerr
#define errorLogger logger << "[Error] "
#define warningLogger logger << "[Warning] "

enum class MacroState {idle, keyword, identifier, expression};
enum class MacroKeyword {idle, define, undef, ifdef, ifudef};
enum class CommentState {idle, paraComment, lineComment};
enum class CommentIndicatorState {idle, startSlash, paraEndAster};
enum class StringState {idle, inString};

/\
*
*/ # /*
*/ defi\
ne FO\
O 10\
20

#define asd "foo\\""bar"

int main(int argc, char const *argv[])
{
	cout << dat/* asd  asd */ << FOO << asd << endl;

	FILE *fin = fopen("test.cpp", "r");

	MacroState macroState = MacroState::idle;
	MacroKeyword macroKeyword = MacroKeyword::idle;
	CommentState commentState = CommentState::idle;
	CommentIndicatorState commentIndicatorState = CommentIndicatorState::idle;
	StringState stringState = StringState::idle;

	unordered_map<string, string> macroHash;

	char c;
	while ((c = getc(fin)) != EOF) {
		switch (commentState) {
			case CommentState::idle:
				switch (commentIndicatorState) {
					case CommentIndicatorState::idle:
						if (c == '/') {
							commentIndicatorState = CommentIndicatorState::startSlash;
						}
						break;
					case CommentIndicatorState::startSlash:
						commentIndicatorState = CommentIndicatorState::idle;
						if (c == '*') {
							commentState = CommentState::paraComment;
						} else if (c == '/') {
							commentState = CommentState::lineComment;
						}
						break;
					default:
						errorLogger << "Impossible CommentIndicatorState when CommentState is idle" << endl;
				}
				break;
			case CommentState::paraComment:
				switch (commentIndicatorState) {
					case CommentIndicatorState::idle:
						if (c == '*') {
							commentIndicatorState = CommentIndicatorState::paraEndAster;
						}
						continue;
					case CommentIndicatorState::paraEndAster:
						commentIndicatorState = CommentIndicatorState::idle;
						if (c == '/') {
							commentState = CommentState::idle;
						}
						continue;
					default:
						errorLogger << "Impossible CommentIndicatorState when CommentState is paraComment" << endl;
				}
				break;
			case CommentState::lineComment:
				if (c == '\n') {
					commentState = CommentState::idle;
				}
				break;
		}

		switch (commentIndicatorState) {
			case CommentIndicatorState::idle:
				if (c == '/') {
					commentIndicatorState = CommentIndicatorState::startSlash;
				} else if (c == '*')
				break;
			case CommentIndicatorState::startSlash:
				if (c == '*') {
					commentState = CommentState::paraComment;
				} else if (c == '/') {
					commentState = CommentState::lineComment;
				}
				break;
			case CommentIndicatorState::paraEndAster:
				break;
			default:
				errorLogger << " " << endl;
		}

		switch (macroState) {
			case MacroState::idle:
				if (c == '#') {
					macroState = MacroState::keyword;
				}
				break;
			case MacroState::keyword:
				// if (c == 'd')
				break;
			case MacroState::identifier:
				break;
			case MacroState::expression:
				break;
			default:
				errorLogger << "Undefined Macro State" << endl;
		}
	}

	fclose(fin);
	return 0;
}