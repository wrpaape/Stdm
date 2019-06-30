#include "StdmMux.hpp"

#include <cmath>
#include <string>
#include <utility>
#include <iterator>
#include <algorithm>
#include <sstream>

StdmMux::StdmMux(std::istream &input,
                 std::ostream &debug)
    : currentTime()
    , endTime()
    , frame(1)
    , totalFrames()
    , timeStep()
    , frameSize()
    , dataSize()
    , addressBits()
    , backlog()
    , sources()
{
    double totalAverageTransmissionRate = readSources(input, debug);
    if (sources.empty()) {
        debug << "no data transmitted from configured sources" << std::endl;
    }
    double averageTransmissionRate = totalAverageTransmissionRate
                                   / static_cast<double>(sources.size());
    double averageStepBlocks = averageTransmissionRate
                             * static_cast<double>(timeStep);
    frameSize = static_cast<std::size_t>(std::round(averageStepBlocks));

    addressBits = static_cast<std::size_t>(std::log2(frameSize)) + 1;

    double exactTotalFrames = static_cast<double>(endTime - currentTime)
                            / static_cast<double>(timeStep);

    totalFrames = static_cast<std::size_t>(std::ceil(exactTotalFrames));

    debug << "average transmission rate: " << averageTransmissionRate
          << " data blocks per second"
             "\ndata size: " << dataSize << " bits per data block"
             "\nframe size: " << frameSize << " data blocks per frame"
             "\naddress bits: " << addressBits << " bits per subframe"
             "\ntime step: " << timeStep << " second(s)"
             "\ntotal frames: " << totalFrames
          << std::endl;
}

double
StdmMux::readSources(std::istream &input,
                     std::ostream &debug)
{
    double totalAverageTransmissionRate = 0.0;
    std::string sourceLine;
    while (std::getline(input, sourceLine)) {
        sources.emplace_back(sourceLine);
        const StdmSource &source = sources.back();

        debug << "read source \"" << source.getName() << "\": ";

        if (source.empty()) {
            debug << "[NO DATA]" << std::endl;
            sources.pop_back();
            continue;
        }

        unsigned long duration = source.getDataDuration();
        std::size_t   size     = source.getDataSize();
        if (timeStep != 0) {
            if (duration != timeStep) {
                throw std::invalid_argument(
                    "sources have inconsistent data rates"
                );
            }
            if (size != dataSize) {
                throw std::invalid_argument(
                    "sources have inconsistent data block sizes"
                );
            }
        } else {
            // initial pass
            timeStep = duration;
            dataSize = size;
        }

        unsigned long sourceStartTime        = source.getStartTime();
        unsigned long sourceEndTime          = source.getEndTime();
        double sourceAverageTransmissionRate = source.averageTransmissionRate();

        debug << "startTime="                 << sourceStartTime
              << ", endTime="                 << sourceEndTime
              << ", dataBlocks="              << source.size()
              << ", averageTransmissionRate=" << sourceAverageTransmissionRate
              << std::endl;

        currentTime = std::min(currentTime, sourceStartTime);
        endTime     = std::max(endTime,     sourceEndTime);
        totalAverageTransmissionRate += sourceAverageTransmissionRate;
    }

    return totalAverageTransmissionRate;
}

bool
StdmMux::writeFrame(std::ostream &output,
                    std::ostream &debug)
{
    if ((currentTime >= endTime) && backlog.empty()) {
        debug << "DONE!" << std::endl;
        checkForDataInSources(debug);
        return false;
    }

    writeStartOfFrame("frame", '=', frame, totalFrames, currentTime, debug);
    debug << currentTime << ": "
          << backlog.size() << " data blocks in the backlog"
          << std::endl;

    updateBacklog();

    writeStartOfFrame("frame", '=', frame, totalFrames, currentTime, output);
    output << "SF (1 bit)" << std::endl;

    for (unsigned long subframe = 1; subframe <= frameSize; ++subframe) {
        writeStartOfFrame("subframe", '-', subframe, frameSize, currentTime, output);

        unsigned long sourceAddress = 0;
        std::string source = "NONE";
        std::string data;

        if (!backlog.empty()) {
            const BacklogItem& item = backlog.top();
            sourceAddress = item.source;
            source        = sources[sourceAddress - 1].getName();
            data          = item.data;
            backlog.pop();
        }

        writeSubframe(sourceAddress, source, data, output);
    }

    output << "EF (1 bit)" << std::endl;

    ++frame;
    return true;
}

void
StdmMux::updateBacklog()
{
    unsigned long nextTime      = currentTime + timeStep;
    unsigned long sourceAddress = 1;
    for (StdmSource& source : sources) {
        std::string data = source.select(currentTime, nextTime);
        if (!data.empty()) {
            BacklogItem item;
            item.source    = sourceAddress;
            item.timestamp = currentTime;
            item.data      = std::move(data);
            backlog.emplace(std::move(item));
        }
        ++sourceAddress;
    }

    currentTime = nextTime;
}

void
StdmMux::writeSubframe(unsigned long      sourceAddress,
                       const std::string &source,
                       const std::string &data,
                       std::ostream      &output)
{
    output << "address: " << sourceAddress << '/' << source
                          << " (" << addressBits << " bits)"
              "\ndata: \"" << data << "\" (" << dataSize << " bits)"
           << std::endl;
}

void
StdmMux::writeStartOfFrame(const std::string &frameType,
                           char               lineChar,
                           unsigned long      number,
                           unsigned long      total,
                           unsigned long      currentTime,
                           std::ostream      &output)
{
    static const std::size_t LINE_WIDTH = 80;

    std::ostringstream textStream;
    textStream << "start of " << frameType << ' ' << number << '/' << total
               << " (time=" << currentTime << ')';
    std::string text = textStream.str();

    std::size_t lengthLine       = 4;
    std::size_t lengthSpacedText = text.length() + 1;

    if ((lengthLine + lengthSpacedText) < LINE_WIDTH) {
        lengthLine = (LINE_WIDTH - lengthSpacedText);
    }

    std::fill_n(std::ostream_iterator<char>(output), lengthLine, lineChar);
    output << ' ' << text << std::endl;
}

void
StdmMux::checkForDataInSources(std::ostream &debug)
{
    bool sourcesHaveData = false;
    for (const StdmSource &source : sources) {
        if (!source.empty()) {
            debug << "ERROR: " << source.size()
                  << " entries remain in source " << source.getName()
                  << std::endl;
            sourcesHaveData = true;
        }
    }

    if (sourcesHaveData) {
        throw std::runtime_error("data remains in sources");
    }
}
