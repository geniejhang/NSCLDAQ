#include "CParameters.h"
#include <iostream>

int main(int argc, char** argv)
{
    CParameters params(argc, argv);
    std::cout << "--service: " << params.service() << std::endl;
    std::cout << "--file: "    << params.filename() << std::endl;
}
