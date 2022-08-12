/**
 * @file        check_data_set.cpp
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

#include "check_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/data_set_files/data_set_file.h>

#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/methods/file_methods.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

CheckDataSet::CheckDataSet()
    : Kitsunemimi::Sakura::Blossom("Compare a list of values with a data-set to check correctness.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("result_uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the data-set to compare to.");
    assert(addFieldRegex("result_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                        "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("data_set_uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the data-set to compare to.");
    assert(addFieldRegex("data_set_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("correctness",
                        Sakura::SAKURA_FLOAT_TYPE,
                        "Correctness of the values compared to the data-set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CheckDataSet::runTask(Sakura::BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      Sakura::BlossomStatus &status,
                      ErrorContainer &error)
{
    const std::string resultUuid = blossomLeaf.input.get("result_uuid").getString();
    const std::string dataUuid = blossomLeaf.input.get("data_set_uuid").getString();

    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get result
    bool success = false;
    const std::string resultLocation = GET_STRING_CONFIG("sagiri", "result_location", success);
    const std::string filePath = resultLocation + "/" + resultUuid;

    // TODO: precheck if file exist
    std::string resultText = "";
    if(readFile(resultText, filePath, error) == false)
    {
        status.statusCode = Hanami::NOT_FOUND_RTYPE;
        return false;
    }
    Json::JsonItem resultData;
    if(resultData.parse(resultText, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get data-info from database
    if(SagiriRoot::dataSetTable->getDataSet(blossomLeaf.output,
                                            dataUuid,
                                            userUuid,
                                            projectUuid,
                                            isAdmin,
                                            error,
                                            true) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get file information
    const std::string location = blossomLeaf.output.get("location").getString();

    Kitsunemimi::DataBuffer buffer;
    DataSetFile::DataSetHeader dataSetHeader;
    DataSetFile::ImageTypeHeader imageTypeHeader;
    Kitsunemimi::BinaryFile file(location);

    // read data-set-header
    if(file.readCompleteFile(buffer, error) == false)
    {
        error.addMeesage("Failed to read data-set-header from file '" + location + "'");
        return false;
    }

    // prepare values
    uint64_t correctValues = 0;
    uint64_t dataPos = sizeof(DataSetFile::DataSetHeader) + sizeof(DataSetFile::ImageTypeHeader);
    const uint8_t* u8Data = static_cast<const uint8_t*>(buffer.data);
    memcpy(&dataSetHeader, buffer.data, sizeof(DataSetFile::DataSetHeader));
    memcpy(&imageTypeHeader,
           &u8Data[sizeof(DataSetFile::DataSetHeader)],
           sizeof(DataSetFile::ImageTypeHeader));
    const uint64_t lineOffset = imageTypeHeader.numberOfInputsX * imageTypeHeader.numberOfInputsY;
    const uint64_t lineSize = (imageTypeHeader.numberOfInputsX * imageTypeHeader.numberOfInputsY)
                              + imageTypeHeader.numberOfOutputs;
    const float* content = reinterpret_cast<const float*>(&u8Data[dataPos]);

    // iterate over all values and check
    DataArray* compareData = resultData.getItemContent()->toArray();
    for(uint64_t i = 0; i < compareData->size(); i++)
    {
        const uint64_t actualPos = (i * lineSize) + lineOffset;
        const uint64_t checkVal = compareData->get(i)->toValue()->getInt();
        if(content[actualPos + checkVal] > 0.0f) {
            correctValues++;
        }
    }

    // add result to output
    const float correctness = (100.0f / static_cast<float>(compareData->size()))
                              * static_cast<float>(correctValues);
    blossomLeaf.output.deleteContent();
    blossomLeaf.output.insert("correctness", correctness);

    return true;
}
