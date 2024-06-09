# Welcome to Crisp

This is the beginning of my solo compiler project, code-named "crisp". I'll be documenting all my progress for anyone interested in following along. This project is a great learning experience for me as I'm fascinated by compilers and the abstraction they provide to programmers.

## Characteristics of the PL

1. **Statically-typed:** Type is bound to variables at compile time.
2. **Imperative:** Statements change the program’s state and explicitly define step-by-step instructions to do so.

The first step in making a compiler is to implement the front end. Regardless of whether you are compiling or interpreting your language, the front end is normally very similar.

## Front End Components

### 1. Lexical Analysis (Scanning)

Responsible for breaking the text file full of characters into tokens that are the unit of "words" for our language. E.g. keywords, identifiers (variable/function names), constants, and so on.

Tools such as Flex are designed to let you throw a handful of regexes at them, and they give you a complete scanner back. 

We will do this by ourselves.

There are a couple of ways to implement this after looking online:

- **Deterministic Finite Automaton (DFA):** Each state represents a specific lexical pattern, and transitions between states occur based on the input characters.
- **Trie:** Insert each token into a trie data structure and then use that to tokenize your input.
- **Scanning:** Scan the next identifier and then compare it to a list of keywords sorted alphabetically with the longest matches before the shortest matches. If nothing matches, it is an identifier not a keyword.

We will use a simple version of DFA with switches.

### 2. Syntactical Analysis (Parsing)

Once we have the tokens, we must somehow analyze the structure of the tokens and see if it fits a valid definition of what we define as a program in this language. We must model the tokens in a way that we can catch any errors in the code. We must define a grammar which dictates the syntactic rules of the language i.e., how legal sentences could be formed. This is where we use trees.

Tools such as Bison take a formal grammar specification and generate C code for a parser that recognizes the structure of the input based on that grammar.

We will do this by ourselves.

### 3. Semantic Analysis

This part is responsible for catching errors not possible to be caught by simply parsing and generating a tree representation of our code. If we have:

```crisp
int x = "sadfsf"

This matches our context-free grammar format, but semantically it doesn't make sense. This is where semantic analysis comes in to play. We will do variable binding and 
type checking here as it requires context.

## TODO