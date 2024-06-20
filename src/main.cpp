#include <iostream>
#include <unistd.h>  

#include "../parse/parse.h"
#include "../error/parseExcept.h"

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "crisp: error: Command line requires 1 argument to start the compilation process\n";
        return 1;
    }

    if (access(argv[1], F_OK | R_OK) == -1) {
        std::cout << "crisp: error: Input filename is either non-existent or non-readable\n";
        return 1;
    }

    /*
    
    input format: ./crisp [options] <examplefile.crisp>

    options:

    -a: emit AST to stdout. no LLVM IR will be generated unless -b | -c specified 

    -b: emit LLVM bitcode to stdout. 

    -o: specify target output file, the default output file is the input file stripped of .crisp

    -c: compile input file and produce exe. if -o not specified target output file will be input file minus the .crisp extension.

    -O: enables optimization passes

    -h: prints usage instructions

    */ 

    try {
        Scanner scanner {argv[1]};
        scanner.scanTokens();

        std::ostream * astStream = &std::cout, * errStream = &std::cerr;

        // continue w parsing 
        // print to stdout if specified but only if no parsing errors
        Parser parser {scanner, argv[1], errStream, astStream};

        // check parsing error count
        if (!parser.isValid()) {
            std::cerr << parser.getNumErrors() << " Error(s)" << std::endl;
			return 1;
        }

        // continue w llvm bitcode generation and print to stdout if needed
        // TODO (dont forget SSA)

        // continue w opt if specified 
        // TODO

        // continue w compiling to target architecture
        // TODO
    } catch (ParseExcept& e) {
		std::cerr << "crisp: error: Critical error. Compilation halted." << std::endl;
		return 1;
	}

    return 0;
}
