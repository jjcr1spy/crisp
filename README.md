Welcome to the start of my solo compiler project aka crisp. I will be documenting all my progress for anyone to see down the line; it should be a great learning experience! Why may you ask? Simply put, I am fascinated by compilers and the abstraction they provide to the normal programmer. It is a beautiful interface that I have the urge to get a better understanding of. Let's begin!
----------------------------------------------------------------------------------------------------
Characteristics of the language: 
1. Statically typed: type is bound to variables at compile time.
2. Imperative: statements change the program’s state and explicitly define the step-by-step instructions to do so.  
----------------------------------------------------------------------------------------------------
The first step in making a compiler requires us to implement the front end. Regardless of whether you are compiling or interpreting your language, the front end is normally very similar or so I have heard.
----------------------------------------------------------------------------------------------------
1. Lexical Analysis (Scanning): responsible for breaking the text file full of characters into tokens that are the unit of "words" for our language. E.g. keywords, identifiers (variable/function names), constants, and so on.
Tools such as Flex are designed to let you throw a handful of regexes at them, and they give you a complete scanner back. They are typically called a compiler-compiler, because you automate the tedious and error-prone process of manually implementing the lexing and parsing components of your compiler. We will do this ourselves. There are a couple ways to implement this from what I have looked at online:
•	Deterministic Finite Automaton (DFA): Each state represents a specific lexical pattern, and transitions between states occur based on the input characters. Typically done with switch statements or Transition tables.
•	Trie: Insert each token into a trie data structure and then from there use that to tokenize your input
•	Scan the next identifier then compare it to a list of keywords sorted alphabetically with the longest matches before the shortest matches. If nothing matches, it is an identifier not a keyword.
We will use a simple version of DFA with switches.
----------------------------------------------------------------------------------------------------
2. Syntactical Analysis (Parsing): once we have the tokens, we must somehow analyze the structure of the tokens and see if it fits a valid definition of what we define as a program in this language. We must model the tokens in a way that we can catch any errors in the code. We must define a grammar which dictates the syntactic rules of the language i.e., how legal sentences could be formed. This is where we use trees.
Tools such as Bison take a formal grammar specification and generate C code for a parser that recognizes the structure of the input based on that grammar.
We will do this by ourselves.
----------------------------------------------------------------------------------------------------
3. Semantic Analysis: this part is responsible for catching errors not possible to be caught by simply parsing and generating a tree representation of our code. If we have 
int x = "sadfsf"
This is valid syntactically in our context-free grammar, but semantically it doesn’t make sense. We can’t have an int variable assigned to a string. This is why we will do type checking and variable binding as it requires context whereas the scanning and parsing portion is context free.
----------------------------------------------------------------------------------------------------




