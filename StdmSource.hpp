#ifndef STDM_SOURCE_HPP
#define STDM_SOURCE_HPP

#include <string>
#include <vector>
#include <istream>

class StdmSource
{
public:
    StdmSource(const std::string &inputLine);

    unsigned long
    startTime();

    unsigned long
    endTime();

    double
    averageTransmissionRate();

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

    template<typename FieldType> static DataBlock
    getDataBlockField(std::istream      &input,
                      const std::string &fieldName,
                      const std::string &spec,
                      FieldType         *field);

    static void
    throwInvalidDataBlockField(const std::string &fieldName,
                               const std::string &spec);

    std::string                            name;
    std::vector<DataBlock>                 blocks;
    std::vector<DataBlock>::const_iterator cursor;
}; // class StdmSource

#endif // ifndef STDM_SOURCE_HPP
