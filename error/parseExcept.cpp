#include "parseExcept.h"

void ParseExcept::printException(std::ostream& output) const noexcept {
	output << what();
}

void ParseExceptMsg::printException(std::ostream& output) const noexcept {
    output << mMsg;
}

void UnknownToken::printException(std::ostream& output) const noexcept {
	output << "Invalid symbol: " << mStr;
}

void TokenMismatch::printException(std::ostream& output) const noexcept {
    output << "Expected: " << Token::mToString[mExpected];
    output << " but saw: ";

    if (mActual != TokenType::Identifier && mActual != TokenType::StringLit &&
        mActual != TokenType::CharLit && mActual != TokenType::IntLit && 
        mActual != TokenType::DoubleLit) {

        output << Token::mToString[mActual];
    } else {
        output << mStr;
    }
}

void OperandMissing::printException(std::ostream& output) const noexcept {
	output << "Binary operation " << Token::mToString[mOp];
	output << " requires two operands.";
}