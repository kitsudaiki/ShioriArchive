/**
 *  @file    data_set_file.h
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 *
 *  @brief simple class for easier handling of sqlite-database
 *
 *  @detail This class provides only three abilities: open and close sqlite databases and
 *          execute sql-commands. The results of the database request are converted into
 *          table-itmes of libKitsunemimiCommon.
 *
 *          This class was created with the help of:
 *              https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
 */

#ifndef SAGIRIARCHIVE_DATASETFILE_H
#define SAGIRIARCHIVE_DATASETFILE_H

#include "data_set_header.h"

#include <libKitsunemimiCommon/logger.h>

#include <string>
#include <vector>

namespace Kitsunemimi {
struct DataBuffer;
class BinaryFile;
}

class DataSetFile
{
public:
    DataSetFile(const std::string &filePath);
    ~DataSetFile();

    bool initNewFile();
    bool readFromFile();

    bool updateHeader();
    bool addBlock(const uint64_t pos,
                  const float* data,
                  const u_int64_t numberOfValues);
    float* getPayload(uint64_t &payloadSize);

    DataSetType type = UNDEFINED_TYPE;
    std::string name = "";

    ImageTypeHeader imageHeader;

    TableTypeHeader tableHeader;
    std::vector<TableHeaderEntry> tableColumns;

private:
    Kitsunemimi::BinaryFile* m_targetFile = nullptr;

    uint64_t m_headerSize = 0;
    uint64_t m_totalFileSize = 0;
};

#endif // SAGIRIARCHIVE_DATASETFILE_H
