#include <sstream>
#include "../parse/astNodes.h"
#include "../parse/parse.h"
#include "../parse/symbols.h"
#include "emitter.h"

// note all llvm headers are in emitter.h

CodeContext::CodeContext(StringTable& strings, const char * file) noexcept
: mGlobalContext {std::make_unique<llvm::LLVMContext>()}
, mModule {std::make_unique<llvm::Module>(file, *mGlobalContext)}
, mStrings {strings}
, mFunc {nullptr}
, mBlock {nullptr}
, mPrintfIdent {nullptr} { }

Emitter::Emitter(Parser& parser) noexcept
: mCodeContext {parser.mStringTable, parser.mFileName} {
	if (parser.mNeedPrintf) {
		mCodeContext.mPrintfIdent = parser.mSymbolTable.getIdentifier("printf");
	}
		
	// this is what kicks off the generation of the LLVM IR from the AST
	parser.mRoot->codegen(mCodeContext);
}

void Emitter::print() noexcept {
	mCodeContext.mModule->print(llvm::outs(), nullptr);
}

bool Emitter::verify() noexcept {
	std::string errMsg;
    llvm::raw_string_ostream errorStream(errMsg);

	// returns true if failed
    bool result = llvm::verifyModule(*mCodeContext.mModule, &errorStream);

    if (result) {
		llvm::errs() << "\nLLVM module verification failed:\n" << errMsg;
		llvm::errs() << "\ncrisp: error: Emitted bad IR. Compilation halted.\n";
	}

	return !result;
}