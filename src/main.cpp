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

    try {
        Scanner scanner {argv[1]};
        scanner.scanTokens();

        std::ostream * astStream = &std::cout, * errStream = &std::cerr;
        
        Parser parser {scanner, argv[1], errStream, astStream};

        // cannot continue with optimizations if errors
        if (!parser.isValid()) {
            std::cerr << parser.getNumErrors() << " Error(s)" << std::endl;
			return 1;
        }

        // continue w llvm bitcode gen
    } catch (ParseExcept& e) {
		std::cerr << "crisp: error: Critical error. Compilation halted." << std::endl;
		return 1;
	}

    return 0;
}
