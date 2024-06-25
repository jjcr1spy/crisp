#include "../parse/astNodes.h"
#include "../parse/parse.h"
#include "../parse/symbols.h"
#include "emitter.h"

using namespace llvm;

CodeContext::CodeContext(StringTable& strings) noexcept
: mGlobalContext {std::make_unique<LLVMContext>()}
, mBuilder {std::make_unique<IRBuilder<>>(*mGlobalContext)}
, mModule {std::make_unique<Module>("crisp compiler", *mGlobalContext)}
, mStrings {strings}
, mFunc {nullptr}
, mBlock {nullptr}
, mPrintfIdent {nullptr} { }

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