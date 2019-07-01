#ifndef STDM_MUX_HPP
#define STDM_MUX_HPP

#include <istream>
#include <ostream>
#include <queue>
#include "StdmSource.hpp"

/**
 * This class represents a Statical Time Division Multiplexer (STDM).
 * Every frame interval (StdmMux::timeStep):
 *     1. Sources are polled every for incoming data blocks.
 *     2. These blocks are queued in an input buffer (StdmMux::backlog) and
 *        prioritized according to:
 *         1. earliest time received
 *         2. earliest-listed source (first line to last line)
 *             - sources are addressed with an integer [1,N] where N is the
 *               number of lines in <input file>
 *             - an address of 0 indicates an empty subframe
 *         - see BacklogItem::operator<()
 *     3. A fixed number of subframes are fetched from the input buffer
 *        (StdmMux::frameSize).
 *     4. These subframes are packed into a frame.
 *         - If there are not enough data blocks to fill a frame, pad the
 *           remainder of the frame with zeros.
 *     5. These frames are written to standard output.
 */
class StdmMux
{
public:
    /**
     * @brief construct a StdmMux
     * @param[in] input the input specification of sources
     * @param[in] debug the output log stream
     */
    StdmMux(std::istream &input,
            std::ostream &debug);

    /**
     * @brief Advance the Multiplexer by a single time step.  If there is data
     *     available, write it to @p output in a frame.
     * @param[in] output the output frame stream
     * @param[in] debug the output log stream
     */
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
