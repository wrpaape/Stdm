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
    updateBacklog();

    void
    writeSubframe(unsigned long      sourceAddress,
                  const std::string &source,
                  const std::string &data,
                  std::ostream      &output);

    static void
    writeStartOfFrame(const std::string &frameType,
                      char               lineChar,
                      unsigned long      number,
                      unsigned long      total,
                      unsigned long      currentTime,
                      std::ostream      &output);

    void
    checkForDataInSources(std::ostream &debug);

    unsigned long                    currentTime;
    unsigned long                    endTime;
    unsigned long                    frame;
    unsigned long                    totalFrames;
    unsigned long                    timeStep;
    std::size_t                      frameSize;
    std::size_t                      dataSize;
    std::size_t                      addressBits;
    std::priority_queue<BacklogItem> backlog;
    std::vector<StdmSource>          sources;
}; // class StdmMux

inline bool
operator<(const StdmMux::BacklogItem& lhs,
          const StdmMux::BacklogItem& rhs)
{
    if (lhs.source != rhs.source) {
        // lower source number is higher priority
        return lhs.source > rhs.source;
    }

    // prioritize items that arrived earlier
    return lhs.timestamp > rhs.timestamp;
}

#endif // ifndef STDM_MUX_HPP
