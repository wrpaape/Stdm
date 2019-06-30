#ifndef STDM_SOURCE_HPP
#define STDM_SOURCE_HPP

#include <string>
#include <vector>
#include <istream>

class StdmSource
{
public:
    StdmSource(const std::string &inputLine);

    bool
    empty() const { return blocks.empty(); }

    const std::string &
    getName() const { return name; }

    unsigned long
    getDataDuration() const { return dataDuration; }

    std::size_t
    getDataSize() const { return dataSize; }

    unsigned long
    getStartTime() const;

    unsigned long
    getEndTime() const;

    double
    averageTransmissionRate() const;

    std::string
    select(unsigned long startTime,
           unsigned long endTime);

private:
    struct DataBlock
    {
        unsigned long startTime;
        unsigned long endTime;
        std::string   data;
    }; // struct DataBlock

    static DataBlock
    makeDataBlock(const std::string &spec);

    template<typename FieldType> static void
    getDataBlockField(std::istream      &input,
                      const std::string &fieldName,
                      const std::string &spec,
                      FieldType         *field);

    static void
    throwInvalidDataBlockField(const std::string &fieldName,
                               const std::string &spec);

    std::string                            name;
    std::vector<DataBlock>::const_iterator cursor;
    std::vector<DataBlock>                 blocks;
    unsigned long                          dataDuration;
    std::size_t                            dataSize;
}; // class StdmSource

#endif // ifndef STDM_SOURCE_HPP
