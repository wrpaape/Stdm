#ifndef STDM_MUX_HPP
#define STDM_MUX_HPP

#include <istream>
#include "StdmSource.hpp"

class StdmMux
{
public:
    StdmMux(std::istream& input);

private:
    std::vector<StdmSource> channels;
}; // class StdmMux

#endif // ifndef STDM_MUX_HPP
