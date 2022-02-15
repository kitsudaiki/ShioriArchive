/**
 * @file        table_data_set_file.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2021 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "table_data_set_file.h"

/**
 * @brief constructor
 *
 * @param filePath path to file
 */
TableDataSetFile::TableDataSetFile(const std::string &filePath)
    : DataSetFile(filePath) {}

/**
 * @brief constructor
 *
 * @param file pointer to binary-file object
 */
TableDataSetFile::TableDataSetFile(Kitsunemimi::BinaryFile* file)
    : DataSetFile(file) {}

/**
 * @brief destructor
 */
TableDataSetFile::~TableDataSetFile() {}

/**
 * @brief init header-sizes
 */
void
TableDataSetFile::initHeader()
{
    m_headerSize = sizeof(DataSetHeader) + sizeof(TableTypeHeader);
    m_headerSize += tableColumns.size() * sizeof(TableHeaderEntry);

    tableHeader.numberOfColumns = tableColumns.size();
    m_totalFileSize = m_headerSize;
    m_totalFileSize += tableHeader.numberOfColumns * sizeof(float) * tableHeader.numberOfLines;
}

/**
 * @brief read header from buffer
 *
 * @param u8buffer buffer to read
 */
void
TableDataSetFile::readHeader(const uint8_t* u8buffer)
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

/**
 * @brief update header in file
 *
 * @return true, if successful, else false
 */
bool
TableDataSetFile::updateHeader()
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

    return true;
}
