#ifndef STDM_MUX_HPP
#define STDM_MUX_HPP

#include <istream>
#include <ostream>
#include <queue>
#include "StdmSource.hpp"

class StdmMux
{
public:
    StdmMux(std::istream &input,
            std::ostream &debug);

    bool
    writeFrame(std::ostream &output);

private:
    struct BacklogItem
    {
        unsigned long source;
        unsigned long timestamp;
        std::string   data;

    };
    friend bool
    operator<(const BacklogItem& lhs,
              const BacklogItem& rhs);

    double
    readSources(std::istream &input,
                std::ostream &debug);

    void
    writeSubframe(std::ostream      &output,
                  unsigned long      source,
                  const std::string &data);

    unsigned long                    currentTime;
    unsigned long                    endTime;
    unsigned long                    timeStep;
    std::size_t                      frameSize;
    std::priority_queue<BacklogItem> backlog;
    std::vector<StdmSource>          sources;
}; // class StdmMux

inline bool
operator<(const StdmMux::BacklogItem& lhs,
          const StdmMux::BacklogItem& rhs)
{
    if (lhs.source != rhs.source) {
        // lower source number is higher priority
        return lhs.source < rhs.source;
    }

    // prioritize items that arrived earlier
    return lhs.timestamp < rhs.timestamp;
}

#endif // ifndef STDM_MUX_HPP
