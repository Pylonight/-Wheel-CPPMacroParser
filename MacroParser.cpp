#include <iostream>

using namespace std;

#define/* asd  asd */dat/* asd  asd */0x20

enum class MacroState {idle, keyword, identifier, expression};
enum class MacroKeyword {idle, define, undef, ifdef, ifudef};
enum class CommentState {idle, paraComment, lineComment};
enum class CommentIndicatorState {idle, startSlash, paraEndAster};
enum class StringState {idle, inString};

int main(int argc, char const *argv[])
{
	cout << dat/* asd  asd */ << endl;

	FILE *fin = fopen("test.cpp", "r");

	MacroState macroState = MacroState::idle;
	MacroKeyword macroKeyword = MacroKeyword::idle;
	CommentState commentState = CommentState::idle;
	CommentIndicatorState commentIndicatorState = CommentIndicatorState::idle;
	StringState stringState = StringState::idle;
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
						cout << "Error: Impossible CommentIndicatorState when CommentState is idle" << endl;
				}
				break;
			case CommentState::paraComment:
				switch (commentIndicatorState) {
					case CommentIndicatorState::idle:
						if (c == '*') {
							commentIndicatorState = CommentIndicatorState::paraEndAster;
						}
						break;
					case CommentIndicatorState::paraEndAster:
						commentIndicatorState = CommentIndicatorState::idle;
						if (c == '/') {
							commentState = CommentState::idle;
						}
						break;
					default:
						cout << "Error: Impossible CommentIndicatorState when CommentState is paraComment" << endl;
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
				cout << " " << endl;
		}

		switch (macroState) {
			case MacroState::idle:
				if (c == '#') {
					// indicates the beginning of a macro statement
					// marco keyword must be parsed
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
				cout << "Error: Undefined Macro State" << endl;
		}
	}

	fclose(fin);
	return 0;
}