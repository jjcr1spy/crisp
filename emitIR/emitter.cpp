#include "../parse/astNodes.h"
#include "../parse/parse.h"
#include "../parse/symbols.h"
#include "emitter.h"

// note all llvm headers are in emitter.h

CodeContext::CodeContext(StringTable& strings) noexcept
: mGlobalContext {std::make_unique<llvm::LLVMContext>()}
, mBuilder {std::make_unique<llvm::IRBuilder<>>(*mGlobalContext)}
, mModule {std::make_unique<llvm::Module>("crisp compiler", *mGlobalContext)}
, mStrings {strings}
, mFunc {nullptr}
, mBlock {nullptr}
, mPrintfIdent {nullptr} 
, mSSA {*mGlobalContext} { }

Emitter::Emitter(Parser& parser) noexcept
: mCodeContext {parser.mStringTable} {
	if (parser.mNeedPrintf) {
		mCodeContext.mPrintfIdent = parser.mSymbolTable.getIdentifier("printf");
	}
		
	// this is what kicks off the generation of the LLVM IR from the AST
	parser.mRoot->codegen(mCodeContext);
}

void Emitter::print() noexcept {
	mCodeContext.mModule->print(llvm::outs(), nullptr);
}