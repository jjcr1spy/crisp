#include <algorithm>
#include <iostream>
#include "symbols.h"

SymbolTable::~SymbolTable() noexcept {
	delete mCurrentScope;
}

ScopeTable::ScopeTable(ScopeTable * parent) noexcept
: mParent(parent) {
    if (parent) parent->mChildren.emplace_back(this);
}

ScopeTable::~ScopeTable() noexcept {
	for (auto t : mChildren) delete t;
}

void ScopeTable::addIdentifier(Identifier * ident) {
    mSymbols.emplace(ident->getName(), ident);
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

void ScopeTable::print(std::ostream& output, int depth = 0) const noexcept {
    std::vector<Identifier*> idents;
	
    for (const auto& sym : mSymbols) idents.push_back(sym.second);

	std::sort(idents.begin(), idents.end(), [](Identifier * a, Identifier * b) {
		return a->getName() < b->getName();
	});

	for (const auto& ident : idents) {
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

bool SymbolTable::isDeclaredInScope(const std::string& s) const noexcept {
    return mCurrentScope->searchInScope(s) != nullptr;
}

Identifier * SymbolTable::createIdentifier(const std::string& s) noexcept {
    if (isDeclaredInScope(s)) return nullptr;

    Identifier * ident = new Identifier(s);
    mCurrentScope->addIdentifier(ident);

    return ident;
}

Identifier * SymbolTable::getIdentifier(const std::string& s) noexcept {
    return mCurrentScope->search(s);
}

ScopeTable * SymbolTable::enterScope() noexcept {
    mCurrentScope = new ScopeTable(mCurrentScope);
}

void SymbolTable::exitScope() noexcept {
    mCurrentScope->getParent();
}

void SymbolTable::print(std::ostream& output) const noexcept {
    output << "Symbols:\n";

	if (mCurrentScope) mCurrentScope->print(output);
}