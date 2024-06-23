#include <algorithm>
#include <iostream>
#include "symbols.h"

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