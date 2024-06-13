#include "parser.h"
#include "../error/parseExcept.h"

Parser::Parser(Scanner& scanner, std::ostream * errStream, std::ostream * astStream) 
: mScanner {scanner}
, mErrStream {errStream}
, mAstStream {astStream}
, mCurrReturnType {Type::Void}
, mCurrToken {scanner.mTokens[0]}
, mTokenIndex {0}
, mLine {1}
, mCol {1}
, mErrors {0}
, mNeedPrintf {false} {
    try {

    } catch (ParseExcept& e) {
        reportError(e);
    }

    if (!isValid()) {
        displayErrors();
    }
}

void Parser::reportError(const ParseExcept& except) noexcept {
	std::stringstream errStrm;
	except.printException(errStrm);
	mErrors.push_back(std::make_shared<Error>(errStrm.str(), mLine, mCol));
}

void Parser::reportError(const std::string& msg) noexcept {
	mErrors.push_back(std::make_shared<Error>(msg, mLine, mCol));
}

// Helper function to report a semantic error
void Parser::reportSemantError(const std::string& msg, int colOverride = -1, int lineOverride = -1) noexcept {
    int col;

    if (colOverride == -1) col = mCol;
    else col = colOverride;
    
    int line;
    if (lineOverride == -1) line = mLine;
    else line = lineOverride;
    
    mErrors.push_back(std::make_shared<Error>(msg, line, col));
}

// Write an error message to the error stream
void Parser::displayErrorMsg(const std::string& line, std::shared_ptr<Error> error) noexcept {
    (*mErrStream) << mFileName << ":" << error->mLine << ":" << error->mCol;
	(*mErrStream) << ": error: ";
	(*mErrStream) << error->mMsg << std::endl;
	
	(*mErrStream) << line << std::endl;
	
    // now add the caret
	for (int i = 0; i < error->mCol - 1; ++i) {
		if (line[i] == '\t') (*mErrStream) << '\t';
		else (*mErrStream) << ' ';
	}

	(*mErrStream) << '^' << std::endl;
}

// Writes out all the error messages
void Parser::displayErrors() noexcept {
	int lineNum = 0;
	std::string lineTxt;
	std::ifstream fileStream(mFileName);
	
	for (auto i = mErrors.begin(); i != mErrors.end(); ++i) {
		while (lineNum < (*i)->mLine) {
			std::getline(fileStream, lineTxt);
			lineNum++;
		}
		
		displayErrorMsg(lineTxt, *i);
	}
}





