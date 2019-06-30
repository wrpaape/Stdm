#include "StdmMux.hpp"

#include <cmath>
#include <string>
#include <utility>
#include <iterator>
#include <algorithm>

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

    totalFrames = (endTime - currentTime) / timeStep;

    debug << "average transmission rate: " << averageTransmissionRate
          << " data blocks per second"
             "\ndata size: " << dataSize << " bits per data block"
             "\nframe size: " << frameSize << " data blocks per frame"
             "\naddress bits: " << addressBits << " bits per subframe"
             "\ntime step: " << timeStep << " seconds"
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
        unsigned long sourceEndTime          = source.getStartTime();
        double sourceAverageTransmissionRate = source.averageTransmissionRate();

        debug << "startTime="                 << sourceStartTime
              << ", endTime="                 << sourceEndTime
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

    if (currentTime >= endTime) {
        debug << "DONE!" << std::endl;
        return false;
    }

    writeStartOfFrame("frame", '=', frame, totalFrames, debug);
    debug << currentTime << ": "
          << backlog.size() << " data blocks in the backlog"
          << std::endl;

    updateBacklog();

    writeStartOfFrame("frame", '=', frame, totalFrames, output);
    output << "time: " << currentTime << std::endl;
    output << "SF (1 bit)" << std::endl;

    for (unsigned long subframe = 1; subframe <= frameSize; ++subframe) {
        writeStartOfFrame("subframe", '-', subframe, frameSize, output);

        unsigned long source = 0;
        std::string data;

        if (!backlog.empty()) {
            const BacklogItem& item = backlog.top();
            source = item.source;
            data   = item.data;
            backlog.pop();
        }

        writeSubframe(source, data, output);

        writeEndOfFrame(  "subframe", '-', subframe, frameSize, output);
    }

    output << "EF (1 bit)" << std::endl;
    writeEndOfFrame(  "frame", '=', frame, totalFrames, output);
    writeEndOfFrame(  "frame", '=', frame, totalFrames, debug);

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
StdmMux::writeSubframe(unsigned long      source,
                       const std::string &data,
                       std::ostream      &output)
{
    output << "address: " << source << '(' << addressBits << " bits)"
              "\ndata: \"" << data << "\" ( " << dataSize << " bits)"
           << std::endl;
}

void
StdmMux::writeStartOfFrame(const std::string &frameType,
                           char               delimiter,
                           unsigned long      number,
                           unsigned long      total,
                           std::ostream      &output)
{
    writeDelimiter(delimiter, output);
    output << "start of " << frameType << ' ' << number << '/' << total
           << std::endl;
}

void
StdmMux::writeEndOfFrame(const std::string &frameType,
                         char               delimiter,
                         unsigned long      number,
                         unsigned long      total,
                         std::ostream      &output)
{
    output << "end of " << frameType << ' ' << number << '/' << total
           << std::endl;
    writeDelimiter(delimiter, output);
}

void
StdmMux::writeDelimiter(char delimiter, std::ostream &output)
{
    std::fill_n(std::ostream_iterator<char>(output), 80, delimiter);
    output << std::endl;
}
