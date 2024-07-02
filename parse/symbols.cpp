#include <algorithm>
#include <iostream>
#include "../emitIR/emitter.h"
#include "symbols.h"

// note all llvm headers are in emitter.h

/*
------------------------------------------------------
Identifier methods
*/ 

llvm::Type * Identifier::llvmType(llvm::LLVMContext& ctx, bool treatArrayAsPtr) noexcept {
    llvm::Type * type = nullptr;

    switch (mType) {
        case Type::Void:
            type = llvm::Type::getVoidTy(ctx);

            break;
        case Type::Char:
            type = llvm::Type::getInt8Ty(ctx);

            break;
        case Type::Int:
            type = llvm::Type::getInt32Ty(ctx);

            break;
        case Type::Double:
            type = llvm::Type::getDoubleTy(ctx);

            break;
        case Type::CharArray:
            // treating array as pointer?
            if (treatArrayAsPtr) {
                type = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
            } else {
                type = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), mElemCount);
            }

            break;
        case Type::IntArray:
            // treating array as pointer?
            if (treatArrayAsPtr) {
                type = llvm::PointerType::get(llvm::Type::getInt32Ty(ctx), 0);
            } else {
                type = llvm::ArrayType::get(llvm::Type::getInt32Ty(ctx), mElemCount);
            }

            break;
        case Type::DoubleArray:
            // treating array as pointer?
            if (treatArrayAsPtr) {
                type = llvm::PointerType::get(llvm::Type::getDoubleTy(ctx), 0);
            } else {
                type = llvm::ArrayType::get(llvm::Type::getDoubleTy(ctx), mElemCount);
            }

            break;
        case Type::Function:
            break;
    }

    return type;
}

llvm::Value * Identifier::readFrom(CodeContext& ctx) noexcept {	
    // FOR SSA DOWN THE LINE
    // return ctx.mSSA.readVariable(this, ctx.mBlock);
}
	
void Identifier::writeTo(CodeContext& ctx, llvm::Value * value) noexcept {
    // FOR SSA DOWN THE LINE
    // use the custom SSA class to generate proper virtual register
    // ctx.mSSA.writeVariable(this, ctx.mBlock, value);
}

/*
------------------------------------------------------
SymbolTable methods
*/ 

SymbolTable::SymbolTable() noexcept
: mCurrentScope {nullptr} {
	mCurrentScope = enterScope();
    
	auto id = createIdentifier("@@function");
	id->setType(Type::Function);

	id = createIdentifier("@@variable");
	id->setType(Type::Int);

	id = createIdentifier("printf");
	id->setType(Type::Function);
}

SymbolTable::~SymbolTable() noexcept {
	delete mCurrentScope;
}

bool SymbolTable::isDeclaredInScope(const std::string& s) const noexcept {
    return mCurrentScope->searchInScope(s) != nullptr;
}

Identifier * SymbolTable::createIdentifier(const std::string& s) noexcept {
    if (isDeclaredInScope(s)) return nullptr;

    Identifier * ident = new Identifier(s);
    mCurrentScope->addIdentifier(ident);

    return ident;
}

Identifier * SymbolTable::getIdentifier(const std::string& s) const noexcept {
    return mCurrentScope->search(s);
}

ScopeTable * SymbolTable::enterScope() noexcept {
    return mCurrentScope = new ScopeTable(mCurrentScope);
}

void SymbolTable::exitScope() noexcept {
    mCurrentScope = mCurrentScope->getParent();
}

void SymbolTable::print(std::ostream& output) const noexcept {
    output << "Symbols:\n";

	if (mCurrentScope) mCurrentScope->print(output);
}

/*
------------------------------------------------------
ScopeTable methods
*/ 

ScopeTable::ScopeTable(ScopeTable * parent) noexcept
: mParent(parent) {
    if (parent) parent->mChildren.emplace_back(this);
}

ScopeTable::~ScopeTable() noexcept {
	for (auto t : mChildren) delete t;
}

Identifier * ScopeTable::searchInScope(const std::string& name) noexcept {
    std::unordered_map<std::string, Identifier *>::iterator iter = mSymbols.find(name);

    return iter != mSymbols.end() ? iter->second : nullptr;
}

Identifier * ScopeTable::search(const std::string& name) noexcept {
	Identifier * find = searchInScope(name);

	if (find) return find;
	if (mParent) return mParent->search(name);
	
    return nullptr;
}

void ScopeTable::addIdentifier(Identifier * ident) {
    mSymbols.emplace(ident->getName(), ident);
}

void ScopeTable::print(std::ostream& output, int depth) const noexcept {
    std::vector<Identifier*> idents;
	
    for (const auto& sym : mSymbols) idents.push_back(sym.second);

	std::sort(idents.begin(), idents.end(), [](Identifier * a, Identifier * b) {
		return a->getName() < b->getName();
	});

	for (const auto& ident : idents) {
        if (ident->getName()[0] == '@') continue;

		for (int i = 0; i < depth; i++) {
			output << "---";
		}

		switch (ident->getType()) {
            case Type::Void:
                output << "void ";
                break;
            case Type::Int:
                output << "int ";
                break;
            case Type::Char:
                output << "char ";
                break;
            case Type::Double:
                output << "double ";
                break;
            case Type::IntArray:
                output << "int[] ";
                break;
            case Type::CharArray:
                output << "char[] ";
                break;
            case Type::DoubleArray:
                output << "double[] ";
                break;
            case Type::Function:
                output << "function ";
                break;
            default:
                output << "unknown ";
                break;
		}

		output << ident->getName();
		output << '\n';
	}

	for (const auto& child : mChildren) {
		child->print(output, depth + 1);
	}
}

void ScopeTable::codegen(CodeContext& ctx) noexcept {
    // the only thing we should alloca are arrays of a specified size
	// emit all the symbols in this scope
	for (auto sym : mSymbols) {
		Identifier * ident = sym.second;

        // build instructions for this function
		llvm::IRBuilder<> build(ctx.mBlock);

		llvm::Value * decl = nullptr;
		
		const std::string& name = ident->getName();
		
		// getArrayCount() is -1 if its an array thats passed into a function which we dont allocate it
		if (ident->isArray() && ident->getArrayCount() != -1) {
            // get type for the ident
			llvm::Type * type = ident->llvmType(*ctx.mGlobalContext, false);

			// note we pass in nullptr array size because size is set llvm::Type above 
			decl = build.CreateAlloca(type, nullptr, name);

            // pointer should be 8 byte aligned
			llvm::cast<llvm::AllocaInst>(decl)->setAlignment(llvm::Align(8));
			
			// make a GEP here so we can access it later on without issue
			std::vector<llvm::Value *> gepIdx;
			gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
			gepIdx.push_back(llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*ctx.mGlobalContext)));
			
            // the Get Element Pointer (GEP) instruction is an instruction to apply 
            // the pointer offset to the base pointer and return the resultant pointer
            decl = build.CreateInBoundsGEP(type, decl, gepIdx);
			
			// wow write this GEP and save it for this identifier
			ident->writeTo(ctx, decl);
		}
	}
	
	// emit all the variables in the child scopes
	for (auto table : mChildren) {
		table->codegen(ctx);
	}
}

/*
------------------------------------------------------
StringTable methods
*/ 

// delete all allocated ConstStr * using new
StringTable::~StringTable() noexcept {
	for (auto i : mStrings) {
		delete i.second;
	}
}

// looks up the requested string in the string table
// if it exists returns the corresponding ConstStr
// otherwise constructs a new ConstStr and returns that
ConstStr * StringTable::getString(const std::string& val) noexcept {
	std::unordered_map<std::string, ConstStr *>::iterator iter = mStrings.find(val);

	if (iter != mStrings.end()) {
		return iter->second;
	} else {
        // result is a pair where first is an iterator to the element and second is a boolean
        auto result = mStrings.emplace(val, new ConstStr(val));
        
        // iterator to the key value pair
        auto it = result.first;

        // pointer to value in pair
		return it->second;
	}
}

void StringTable::codegen(CodeContext& ctx) noexcept {
	for (const auto& s : mStrings) {
		ConstStr * str = s.second;
		
        // make the value 
		llvm::Constant * strVal = llvm::ConstantDataArray::getString(*ctx.mGlobalContext, str->mText);
		
        // make the type
		llvm::ArrayType * type = llvm::ArrayType::get(llvm::Type::getInt8Ty(*ctx.mGlobalContext), str->mText.size() + 1);

        // create global var using strVal and type
		llvm::GlobalValue * globVal = new llvm::GlobalVariable(*ctx.mModule, type, true, llvm::GlobalValue::LinkageTypes::PrivateLinkage, strVal, ".str");

		// this can be "unnamed" since the address location is not significant
		globVal->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
		
		str->mValue = globVal;
	}
}
