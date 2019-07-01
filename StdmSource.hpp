#ifndef STDM_SOURCE_HPP
#define STDM_SOURCE_HPP

#include <string>
#include <vector>
#include <istream>

/**
 * This class represents a non-constant stream of data described by a line of
 * the following format:
 *     <NAME>:<start1> <end1> <data1>,...,<startN> <endN> <dataN>
 */
class StdmSource
{
public:
    /**
     * @brief construct a StdmSource
     * @param[in] inputLine the line specifying this source's data stream
     */
    StdmSource(const std::string &inputLine);

    bool
    empty() const { return cursor == blocks.cend(); }

    std::size_t
    size() const { return blocks.cend() - cursor; }

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

    /**
     * @brief poll this source for a data block
     * @param[in] startTime the start of the poll
     * @param[in] endTime   the end of the poll
     * @return non-empty() data block if data is available during the requested
     *     time interval
     */
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
