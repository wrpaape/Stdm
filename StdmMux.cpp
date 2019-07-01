#include "StdmMux.hpp"

#include <cmath>
#include <string>
#include <limits>
#include <utility>
#include <iterator>
#include <algorithm>
#include <sstream>

StdmMux::StdmMux(std::istream &input,
                 std::ostream &debug)
    : currentTime()
    , endTime()
    , frame(1)
    , timeStep()
    , frameSize()
    , dataSize()
    , dataBits()
    , addressBits()
    , backlog()
    , sources()
{
    std::size_t totalDataBlocks = readSources(input, debug);
    if (totalDataBlocks == 0) {
        debug << "\nno data transmitted from configured sources" << std::endl;
        return;
    }
    double totalTime = static_cast<double>(endTime - currentTime);

    double averageTransmissionRate = static_cast<double>(totalDataBlocks)
                                   / totalTime;
    double averageStepBlocks = averageTransmissionRate
                             * static_cast<double>(timeStep);
    frameSize = static_cast<std::size_t>(std::ceil(averageStepBlocks));

    addressBits = static_cast<std::size_t>(std::log2(frameSize)) + 1;

    dataBits = dataSize * std::numeric_limits<std::string::value_type>::digits;

    std::size_t subframeBits = dataBits + addressBits;
    std::size_t frameBits    = (frameSize * subframeBits)
                             + 2; // SF + EF

    debug << "\nstats:"
             "\n\taverage transmission rate: " << averageTransmissionRate
                  << " data blocks per second"
             "\n\tdata bits: " << dataBits << " bits per data block"
                  << " (" << dataSize << " characters * "
                  << std::numeric_limits<std::string::value_type>::digits
                  << " bits/character)"
             "\n\taddress bits: " << addressBits << " bits per subframe"
             "\n\tsubframe bits: " << subframeBits << " bits"
             "\n\tframe size: " << frameSize << " subframes per frame"
             "\n\tframe bits: " << frameBits << " bits"
                 " (" << frameSize << " subframes/frame * "
                      << subframeBits << " bits/subframe + SF + EF)"
             "\n\tframe time duration: " << timeStep << " second(s)"
          << '\n' << std::endl;
}

std::size_t
StdmMux::readSources(std::istream &input,
                     std::ostream &debug)
{
    debug << "reading sources..." << std::endl;

    std::size_t totalDataBlocks = 0;
    std::string sourceLine;
    while (std::getline(input, sourceLine)) {
        sources.emplace_back(sourceLine);
        const StdmSource &source = sources.back();

        debug << "\tread source \"" << source.getName() << "\": ";

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

        unsigned long sourceStartTime = source.getStartTime();
        unsigned long sourceEndTime   = source.getEndTime();

        debug << "startTime="                 << sourceStartTime
              << ", endTime="                 << sourceEndTime
              << ", dataBlocks="              << source.size()
              << std::endl;

        totalDataBlocks += source.size();

        currentTime = std::min(currentTime, sourceStartTime);
        endTime     = std::max(endTime,     sourceEndTime);
    }

    debug << "read sources!" << std::endl;

    return totalDataBlocks;
}

bool
StdmMux::writeFrame(std::ostream &output,
                    std::ostream &debug)
{
    if ((currentTime >= endTime) && backlog.empty()) {
        debug << "\nDONE!" << std::endl;
        checkForDataInSources(debug);
        return false;
    }

    writeStartOfBlock("reading subframes for frame", frame, '=', debug);
    std::size_t initialBacklogSize = backlog.size();
    debug << initialBacklogSize << " data block(s) in the backlog"
          << std::endl;

    std::size_t incomingDataBlocks = updateBacklog();

    debug << incomingDataBlocks << " data block(s) read from sources"
          << std::endl;

    writeStartOfBlock("start of frame", frame, '=', output);
    output << "SF (1 bit)" << std::endl;

    std::size_t busySubframes = writeSubframes(output);

    debug << busySubframes << '/' << frameSize
          << " subframes utilized"
          << std::endl;

    std::size_t backloggedDataBlocks = 0;
    if (backlog.size() > initialBacklogSize) {
        backloggedDataBlocks = backlog.size() - initialBacklogSize;
    }
    debug << backloggedDataBlocks
          << " data block(s) backlogged"
          << std::endl;

    output << "EF (1 bit)" << std::endl;

    ++frame;
    return true;
}

std::size_t
StdmMux::writeSubframes(std::ostream &output)
{
    std::size_t busySubframes = 0;
    for (unsigned long subframe = 1; subframe <= frameSize; ++subframe) {
        writeStartOfBlock("start of subframe", subframe, '-', output);

        unsigned long sourceAddress = 0;
        std::string source          = "<NONE>";
        std::string data            = "<ZERO-FILLED>";

        if (!backlog.empty()) {
            const BacklogItem& item = backlog.top();
            sourceAddress = item.sourceAddress;
            source        = sources[sourceAddress - 1].getName();
            data          = item.data;
            backlog.pop();
            ++busySubframes;
        }

        writeSubframe(sourceAddress, source, data, output);
    }

    return busySubframes;
}

std::size_t
StdmMux::updateBacklog()
{
    std::size_t incomingDataBlocks = 0;
    unsigned long nextTime         = currentTime + timeStep;
    unsigned long sourceAddress    = 1;
    for (StdmSource& source : sources) {
        std::string data = source.select(currentTime, nextTime);
        if (!data.empty()) {
            BacklogItem item;
            item.sourceAddress = sourceAddress;
            item.timestamp     = currentTime;
            item.data          = std::move(data);
            backlog.emplace(std::move(item));
            ++incomingDataBlocks;
        }
        ++sourceAddress;
    }
    currentTime = nextTime; // timeStep has elapsed - update the time
    return incomingDataBlocks;
}

void
StdmMux::writeSubframe(unsigned long      sourceAddress,
                       const std::string &source,
                       const std::string &data,
                       std::ostream      &output)
{
    output << "address: " << sourceAddress << '/' << source
                          << " (" << addressBits << " bits)"
              "\ndata: \"" << data << "\" (" << dataBits << " bits)"
           << std::endl;
}

void
StdmMux::writeStartOfBlock(const std::string &leader,
                           unsigned long      number,
                           char               lineChar,
                           std::ostream      &output)
{
    static const std::size_t LINE_WIDTH = 80;

    unsigned long endOfFrameTime = currentTime + timeStep;
    std::ostringstream textStream;
    textStream << leader << ' ' << number
               << " (time=" << currentTime << '-' << endOfFrameTime << ')';
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
