/**
 *  @file    data_set_file.cpp
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

#include "data_set_file.h"

#include <libKitsunemimiCommon/files/binary_file.h>

/**
 * @brief DataSetFile::DataSetFile
 * @param filePath
 */
DataSetFile::DataSetFile(const std::string &filePath)
{
    m_targetFile = new Kitsunemimi::BinaryFile(filePath);
}

/**
 * @brief DataSetFile::~DataSetFile
 */
DataSetFile::~DataSetFile()
{
    delete m_targetFile;
}

/**
 * @brief DataSetFile::persistHeaderData
 */
bool
DataSetFile::initNewFile()
{
    if(type == IMAGE_TYPE)
    {
        m_headerSize = sizeof(DataSetHeader) + sizeof(ImageTypeHeader);

        uint64_t lineSize = (imageHeader.numberOfInputsX * imageHeader.numberOfInputsY)
                            + imageHeader.numberOfOutputs;
        m_totalFileSize = m_headerSize + (lineSize * imageHeader.numberOfImages * sizeof(float));
    }
    else if(type == TABLE_TYPE)
    {
        m_headerSize = sizeof(DataSetHeader) + sizeof(TableTypeHeader);
        m_headerSize += tableColumns.size() * sizeof(TableHeaderEntry);

        tableHeader.numberOfColumns = tableColumns.size();
        m_totalFileSize = m_headerSize;
        m_totalFileSize += tableHeader.numberOfColumns * sizeof(float) * tableHeader.numberOfLines;
    }
    else
    {
        // not supported value
        return false;
    }

    // allocate storage
    if(m_targetFile->allocateStorage(m_totalFileSize, 1) == false) {
        return false;
    }

    // prepare dataset-header
    DataSetHeader dataSetHeader;
    dataSetHeader.type = type;
    uint32_t nameSize = name.size();
    if(nameSize > 255) {
        nameSize = 255;
    }
    memcpy(dataSetHeader.name, name.c_str(), nameSize);
    dataSetHeader.name[nameSize] = '\0';

    // write dataset-header to file
    if(m_targetFile->writeDataIntoFile(&dataSetHeader, 0, sizeof(DataSetHeader)) == false) {
        return false;
    }

    // write data to file
    return updateHeader();
}

/**
 * @brief DataSetFile::readFromFile
 * @return
 */
bool
DataSetFile::readFromFile()
{
    Kitsunemimi::DataBuffer buffer;
    if(m_targetFile->readCompleteFile(buffer) == false) {
        return false;
    }

    // prepare
    Kitsunemimi::ErrorContainer error;
    const uint8_t* u8buffer = static_cast<const uint8_t*>(buffer.data);

    // read data-set-header
    DataSetHeader dataSetHeader;
    memcpy(&dataSetHeader, u8buffer, sizeof(DataSetHeader));
    type = static_cast<DataSetType>(dataSetHeader.type);
    name = dataSetHeader.name;

    if(type == IMAGE_TYPE)
    {
        // read image-header
        m_headerSize = sizeof(DataSetHeader) + sizeof(ImageTypeHeader);
        memcpy(&imageHeader, &u8buffer[sizeof(DataSetHeader)], sizeof(ImageTypeHeader));

        // get sizes
        uint64_t lineSize = (imageHeader.numberOfInputsX * imageHeader.numberOfInputsY)
                            + imageHeader.numberOfOutputs;
        m_totalFileSize = m_headerSize + (lineSize * imageHeader.numberOfImages * sizeof(float));
    }
    else if(type == TABLE_TYPE)
    {
        // read table-header
        m_headerSize = sizeof(DataSetHeader) + sizeof(TableTypeHeader);
        memcpy(&tableHeader, &u8buffer[sizeof(DataSetHeader)], sizeof(TableTypeHeader));

        // header header
        for(uint64_t i = 0; i < tableHeader.numberOfColumns; i++)
        {
            TableHeaderEntry entry;
            memcpy(&entry,
                   &u8buffer[m_headerSize + (i * sizeof(TableHeaderEntry))],
                   sizeof(TableHeaderEntry));
            tableColumns.push_back(entry);
        }

        // get sizes
        m_headerSize += tableHeader.numberOfColumns * sizeof(TableHeaderEntry);
        m_totalFileSize = m_headerSize;
        m_totalFileSize += tableHeader.numberOfColumns * sizeof(float) * tableHeader.numberOfLines;
    }
    else
    {
        return false;
    }

    return true;
}

/**
 * @brief DataSetFile::updateHeader
 * @return
 */
bool
DataSetFile::updateHeader()
{
    if(type == IMAGE_TYPE)
    {
        // write image-header to file
        if(m_targetFile->writeDataIntoFile(&imageHeader,
                                           sizeof(DataSetHeader),
                                           sizeof(ImageTypeHeader)) == false)
        {
            return false;
        }

    }
    else if(type == TABLE_TYPE)
    {
        // write table-header to file
        if(m_targetFile->writeDataIntoFile(&tableHeader,
                                           sizeof(DataSetHeader),
                                           sizeof(TableTypeHeader)) == false)
        {
            return false;
        }

        // write table-header-entries to file
        const uint64_t offset = sizeof(DataSetHeader) + sizeof(TableTypeHeader);
        for(uint64_t i = 0; i < tableColumns.size(); i++)
        {
            if(m_targetFile->writeDataIntoFile(&tableHeader,
                                               offset + (i * sizeof(TableHeaderEntry)),
                                               sizeof(TableHeaderEntry)) == false)
            {
                return false;
            }
        }
    }
    else
    {
        // not supported value
        return false;
    }

    return true;
}

/**
 * @brief DataSetFile::addLine
 * @param pos
 * @param data
 * @param numberOfValues
 */
bool
DataSetFile::addBlock(const uint64_t pos,
                      const float* data,
                      const u_int64_t numberOfValues)
{
    if(m_headerSize + ((pos + numberOfValues) * sizeof(float)) > m_totalFileSize) {
        return false;
    }

    if(m_targetFile->writeDataIntoFile(data,
                                       m_headerSize + pos * sizeof(float),
                                       numberOfValues * sizeof(float)) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief getPayload
 * @param result
 * @return
 */
float*
DataSetFile::getPayload(uint64_t &payloadSize)
{
    payloadSize = m_totalFileSize - m_headerSize;
    float* payload = new float[payloadSize];
    m_targetFile->readDataFromFile(payload, m_headerSize, payloadSize);
    return payload;
}

