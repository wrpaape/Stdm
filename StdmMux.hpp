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
    writeFrame(std::ostream &output,
               std::ostream &debug);

private:
    struct BacklogItem
    {
        unsigned long sourceAddress;
        unsigned long timestamp;
        std::string   data;

    };
    friend bool
    operator<(const BacklogItem& lhs,
              const BacklogItem& rhs);

    std::size_t
    readSources(std::istream &input,
                std::ostream &debug);

    std::size_t
    updateBacklog();

    std::size_t
    writeSubframes(std::ostream &output);

    void
    writeSubframe(unsigned long      sourceAddress,
                  const std::string &source,
                  const std::string &data,
                  std::ostream      &output);

    void
    writeStartOfBlock(const std::string &leader,
                      unsigned long      number,
                      char               lineChar,
                      std::ostream      &output);

    void
    checkForDataInSources(std::ostream &debug);

    unsigned long                    currentTime;
    unsigned long                    endTime;
    unsigned long                    frame;
    unsigned long                    timeStep;
    std::size_t                      frameSize;
    std::size_t                      dataSize;
    std::size_t                      dataBits;
    std::size_t                      addressBits;
    std::priority_queue<BacklogItem> backlog;
    std::vector<StdmSource>          sources;
}; // class StdmMux

inline bool
operator<(const StdmMux::BacklogItem& lhs,
          const StdmMux::BacklogItem& rhs)
{
    if (lhs.timestamp != rhs.timestamp) {
        // prioritize items that arrived earlier
        return lhs.timestamp > rhs.timestamp;
    }

    // lower source number is higher priority
    return lhs.sourceAddress > rhs.sourceAddress;
}

#endif // ifndef STDM_MUX_HPP
