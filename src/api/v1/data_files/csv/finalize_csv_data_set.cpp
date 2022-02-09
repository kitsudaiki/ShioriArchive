/**
 * @file        finalize_csv_data_set.cpp
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

#include "finalize_csv_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/temp_file_handler.h>
#include <core/data_set_file.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_methods/vector_methods.h>

using namespace Kitsunemimi::Sakura;

FinalizeCsvDataSet::FinalizeCsvDataSet()
    : Kitsunemimi::Sakura::Blossom("Finalize uploaded train-data by checking completeness of the "
                                   "uploaded and convert into generic format.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));
    registerInputField("uuid_input_file",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identify the file for date upload of input-data.");
    assert(addFieldRegex("uuid_input_file", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                            "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
FinalizeCsvDataSet::runTask(BlossomLeaf &blossomLeaf,
                            const Kitsunemimi::DataMap &context,
                            BlossomStatus &status,
                            Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string inputUuid = blossomLeaf.input.get("uuid_input_file").getString();

    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::dataSetTable->getDataSet(result,
                                            uuid,
                                            userUuid,
                                            projectUuid,
                                            isAdmin,
                                            error,
                                            true) == false)
    {
        status.errorMessage = "Data with uuid '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // read input-data from temp-file
    Kitsunemimi::DataBuffer inputBuffer;
    if(SagiriRoot::tempFileHandler->getData(inputBuffer, inputUuid) == false)
    {
        status.errorMessage = "Input-data with uuid '" + inputUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // write data to file
    if(convertCsvData(result.get("location").getString(),
                      result.get("name").getString().c_str(),
                      inputBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to convert mnist-data");
        return false;
    }

    // delete temp-files
    SagiriRoot::tempFileHandler->removeData(inputUuid);

    // create output
    blossomLeaf.output.insert("uuid", uuid);

    return true;
}

/**
 * @brief convert mnist-data into generic format
 *
 * @param filePath
 * @param name
 * @param inputBuffer buffer with input-data
 *
 * @return true, if successfull, else false
 */
bool
FinalizeCsvDataSet::convertCsvData(const std::string &filePath,
                                   const std::string &name,
                                   const Kitsunemimi::DataBuffer &inputBuffer)
{
    DataSetFile file(filePath);
    file.type = TABLE_TYPE;
    file.name = name;

    // prepare content-processing
    const std::string stringContent(static_cast<char*>(inputBuffer.data),
                                    inputBuffer.usedBufferSize);

    // buffer for values to reduce write-access to file
    const uint32_t segmentSize = 10000000;
    std::vector<float> segment(segmentSize, 0.0f);
    uint64_t segmentPos = 0;
    uint64_t segmentCounter = 0;

    // prepare regex for value-identification
    const std::regex intVal("^-?([0-9]+)$");
    const std::regex floatVal("^-?([0-9]+)\\.([0-9]+)$");

    // split content
    std::vector<std::string> lines;
    Kitsunemimi::splitStringByDelimiter(lines, stringContent, '\n');

    bool isHeader = true;
    for(const std::string &line : lines)
    {
        // check if the line is relevant to ignore broken lines
        const uint64_t numberOfColumns = std::count(line.begin(), line.end(), ',') + 1;
        if(numberOfColumns == 1) {
            continue;
        }

        // split line
        std::vector<std::string> lineContent;
        Kitsunemimi::splitStringByDelimiter(lineContent, line, ',');

        if(isHeader)
        {
            file.tableHeader.numberOfColumns = numberOfColumns;

            for(const std::string &col : lineContent)
            {
                // create and add header-entry
                TableHeaderEntry entry;
                entry.setName(col);
                file.tableColumns.push_back(entry);
            }
            isHeader = false;

            if(file.initNewFile() == false) {
                return false;
            }
        }
        else
        {
            for(const std::string &col : lineContent)
            {
                // true
                if(col == "True"
                        || col == "true"
                        || col == "TRUE")
                {
                    segment[segmentPos] = 1.0f;
                }
                // false
                else if(col == "False"
                        || col == "false"
                        || col == "FALSE")
                {
                    segment[segmentPos] = 0.0f;
                }
                // int/long
                else if(regex_match(col, intVal))
                {
                    segment[segmentPos] = static_cast<float>(std::stoi(col.c_str()));
                }
                // float/double
                else if(regex_match(col, floatVal))
                {
                    segment[segmentPos] = std::strtof(col.c_str(), NULL);
                }
                else
                {
                    // ignore other lines
                    segment[segmentPos] = 0.0f;
                }

                // write next segment to file
                segmentPos++;
                if(segmentPos == segmentSize)
                {
                    file.addBlock(segmentCounter, &segment[0], segmentSize);
                    segmentPos = 0;
                    segmentCounter++;
                }
            }

            file.tableHeader.numberOfLines++;
        }
    }

    // write last incomplete segment to file
    if(segmentPos != 0) {
        file.addBlock(segmentCounter, &segment[0], segmentPos);
    }

    // update header in file for the final number of lines for the case,
    // that there were invalid lines
    if(file.updateHeader() == false) {
        return false;
    }

    return true;
}

