#include "StdmMux.hpp"

#include <cmath>
#include <string>
#include <utility>

StdmMux::StdmMux(std::istream &input,
                 std::ostream &debug)
    : currentTime()
    , endTime()
    , timeStep()
    , frameSize()
    , backlog()
    , sources()
{
    double totalAverageTransmissionRate = readSources(input, debug);
    if (!sources.empty()) {
        double averageTransmissionRate = totalAverageTransmissionRate
                                       / static_cast<double>(sources.size());
        double averageStepBlocks = averageTransmissionRate
                                 * static_cast<double>(timeStep);
        frameSize = static_cast<std::size_t>(std::round(averageStepBlocks));
    }
}

double
StdmMux::readSources(std::istream &input,
                     std::ostream &debug)
{
    unsigned long dataDuration = 0;
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
        if ((timeStep != 0) && (timeStep != dataDuration)) {
            throw std::invalid_argument("sources have inconsistent data rates");
        } else {
            timeStep = duration; // initial pass
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
