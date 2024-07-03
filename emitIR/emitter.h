/*
defines the LLVM IR Emitter class which is used for codegen aswell as a helper struct CodeContext to store all LLVM info for program
*/

#ifndef EMITTER_H
#define EMITTER_H

#include <memory>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

// in ../parse/symbols.h
class StringTable; class Identifier;

// in ../parse/parser.h
class Parser;

struct CodeContext {
    CodeContext(StringTable& strings, const char * fileName) noexcept;
    ~CodeContext() noexcept = default;

    // an opaque object that owns a lot of core 
    // LLVM data structures such as the type and constant value tables
    std::unique_ptr<llvm::LLVMContext> mGlobalContext;

    // a LLVM construct that contains functions and global variables
    std::unique_ptr<llvm::Module> mModule;

    // constant global strings
    StringTable& mStrings;

    // current Function
    llvm::Function * mFunc;

    // current BasicBlock
    llvm::BasicBlock * mBlock;

    // non-null if we need extern printf
    Identifier * mPrintfIdent;
};

class Emitter {
public:
	Emitter(Parser& parser) noexcept;
    ~Emitter() noexcept = default;

    // print bitcode to stdout
    void print() noexcept;

    // check for erros in bitcode 
    bool verify() noexcept;

    // run opt passes on llvm ssa ir
    void optimize() noexcept;
private:
    // store all LLVM IR info
	CodeContext mCodeContext;
};

#endif