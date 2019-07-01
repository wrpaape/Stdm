#include "StdmSource.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>


StdmSource::StdmSource(const std::string &inputLine)
    : name()
    , cursor()
    , blocks()
    , dataDuration()
    , dataSize()
{
    std::istringstream input(inputLine);

    if (!std::getline(input, name, ':') && (name.back() != ':')) {
        throw std::invalid_argument("channel name not found");
    }

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
        std::size_t size = block.data.length();
        if (dataDuration != 0) {
            if (duration != dataDuration) {
                throw std::invalid_argument(
                     "data blocks provided with overlapping or out of order"
                     "time stamps"
                );
            }
            if (size != dataSize) {
                throw std::invalid_argument(
                     "data blocks provided with multiple data sizes"
                );
            }
        } else {
            // initial pass - set the data properties
            dataDuration = duration;
            dataSize     = size;
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

std::string
StdmSource::select(unsigned long startTime,
                   unsigned long endTime)
{
    std::string data;

    if (cursor != blocks.cend()) {
        // while data remains...
        const DataBlock& block = *cursor;
        if (startTime >= block.startTime) {
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
    if (!input.eof()) {
        throwInvalidDataBlockField("extra", spec);
    }
    return block;
}

template<typename FieldType> void
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
