#include "StdmSource.hpp"

#include <sstream>
#include <limits>
#include <stdexcept>
#include <utility>


StdmSource::StdmSource(const std::string &inputLine)
    : name()
    , cursor()
    , blocks()
    , dataDuration()
{
    std::istringstream input(inputLine);

    if (!((input >> name) && (name.back() == ':'))) {
        throw std::invalid_argument("channel name not found");
    }
    name.pop_back(); // remove ':' from name

    unsigned long prevEnd = 0;
    std::string dataBlockSpec;
    while (std::getline(input, dataBlockSpec, ',')) {
        DataBlock block = makeDataBlock(dataBlockSpec);
        if (block.startTime < prevEnd) {
            throw std::invalid_argument(
                 "data blocks provided with overlapping or out of order time"
                 "stamps"
            );
        }
        unsigned long duration = block.endTime - block.startTime;
        if ((dataDuration != 0) && (duration != dataDuration)) {
            throw std::invalid_argument(
                 "data blocks provided with overlapping or out of order time"
                 "stamps"
            );
        } else {
            dataDuration = duration; // initial pass
        }
        prevEnd = block.endTime;
        blocks.emplace_back(std::move(block));
    }

    cursor = blocks.cbegin(); // start cursor at first block
}

unsigned long
StdmSource::getStartTime() const
{
    return blocks.empty() ? 0 : blocks.front().startTime;
}

unsigned long
StdmSource::getEndTime() const
{
    return blocks.empty() ? 0 : blocks.back().endTime;
}

double
StdmSource::averageTransmissionRate() const
{
    if (blocks.empty()) {
        return 0.0;
    }

    unsigned long totalTime   = 0;
    std::size_t   totalLength = 0;
    for (const DataBlock &block : blocks) {
        totalTime   += (block.endTime - block.startTime);
        totalLength += block.data.length();
    }
    std::size_t totalBits = totalLength
                          * std::numeric_limits<std::string::value_type>::digits;

    return static_cast<double>(totalBits) / static_cast<double>(totalTime);
}

std::string
StdmSource::select(unsigned long startTime,
                   unsigned long endTime)
{
    std::string data;

    if (cursor != blocks.cend()) {
        // while data remains...
        const DataBlock& block = *cursor;
        if (startTime > block.startTime) {
            // if not idle
            if (endTime < block.endTime) {
                throw std::runtime_error("time slice is too small");
            }
            data = block.data;
            ++cursor; // advance cursor to next block
        }
    }

    return data;
}

StdmSource::DataBlock
StdmSource::makeDataBlock(const std::string &spec)
{
    std::istringstream input(spec);
    DataBlock block;
    getDataBlockField(input, "start time", spec, &block.startTime);
    getDataBlockField(input, "end time",   spec, &block.endTime);
    if (block.endTime <= block.startTime) {
        throwInvalidDataBlockField("start and/or end time", spec);
    }
    getDataBlockField(input, "data", spec, &block.data);
    if (input) {
        throwInvalidDataBlockField("extra", spec);
    }
    return block;
}

template<typename FieldType> StdmSource::DataBlock
StdmSource::getDataBlockField(std::istream      &input,
                              const std::string &fieldName,
                              const std::string &spec,
                              FieldType         *field)
{
    if (!(input >> *field)) {
        throwInvalidDataBlockField(fieldName, spec);
    }
}

void
StdmSource::throwInvalidDataBlockField(const std::string &fieldName,
                                       const std::string &spec)
{
    throw std::invalid_argument(
         "invalid \"" + fieldName + "\" "
         "field for data block spec \"" + spec + "\""
    );
}
