#include <iostream>
#include <fstream>
#include "StdmMux.h"

namespace {

void
usage(std::ostream &output,
      const char   *program)
{
    output << "\nusage:"
              "\n\t" << program << " <input file>"
           << std::endl;
}

} // namespace

int
main(int   argc,
     char *argv[])
{
    if (argc <= 1) {
        const char *program = (argc > 0) ? argv[0] : "./STDM";
        usage(std::cerr, program);
        exit(1);
    }
    
    const char *inputFile = argv[1];
    std::ifstream input(inputFile);

    StdmMux mux(input);

    return 0;
}
