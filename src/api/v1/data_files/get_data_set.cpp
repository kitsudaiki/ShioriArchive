/**
 * @file        get_data_set.cpp
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

#include "get_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/data_set_header.h>

#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiJson/json_item.h>

using namespace Kitsunemimi::Sakura;

GetDataSet::GetDataSet()
    : Blossom("Get information of a specific set of train-data.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the train-data set to delete.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the train-data-set.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the train-data-set.");
    registerOutputField("type",
                        SAKURA_STRING_TYPE,
                        "Type of the new set (For example: CSV)");
    registerOutputField("user_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the user who uploaded the data.");
    registerOutputField("location",
                        SAKURA_STRING_TYPE,
                        "File path on local storage.");
    registerOutputField("inputs",
                        SAKURA_INT_TYPE,
                        "Number of inputs.");
    registerOutputField("outputs",
                        SAKURA_INT_TYPE,
                        "Number of outputs.");
    registerOutputField("lines",
                        SAKURA_INT_TYPE,
                        "Number of lines.");
    registerOutputField("average_value",
                        SAKURA_FLOAT_TYPE,
                        "Average value within the data-set.");
    registerOutputField("max_value",
                        SAKURA_FLOAT_TYPE,
                        "Maximum value within the data-set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
GetDataSet::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      BlossomStatus &status,
                      Kitsunemimi::ErrorContainer &error)
{
    const std::string dataUuid = blossomLeaf.input.get("uuid").getString();
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

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
    if(getHeaderInformation(blossomLeaf.output, location, error) == false)
    {
        error.addMeesage("Failed the read information from file '" + location + "'");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}

/**
 * @brief get information from header of file
 *
 * @param result reference for result-output
 * @param location location of the file to read
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
GetDataSet::getHeaderInformation(Kitsunemimi::Json::JsonItem &result,
                                 const std::string &location,
                                 Kitsunemimi::ErrorContainer &error)
{
    DataSetHeader dataSetHeader;
    Kitsunemimi::BinaryFile file(location);

    // read data-set-header
    if(file.readDataFromFile(&dataSetHeader, 0, sizeof(DataSetHeader)) == false)
    {
        error.addMeesage("Failed to read data-set-header from file '" + location + "'");
        return false;
    }

    if(dataSetHeader.type == IMAGE_TYPE)
    {
        // read image-type-header
        ImageTypeHeader imageTypeHeader;
        if(file.readDataFromFile(&imageTypeHeader,
                                 sizeof(DataSetHeader),
                                 sizeof(ImageTypeHeader)) == false)
        {
            error.addMeesage("Failed to read image-type-header from file '" + location + "'");
            return false;
        }

        // write information to result
        const uint64_t size = imageTypeHeader.numberOfInputsX * imageTypeHeader.numberOfInputsY;
        result.insert("inputs", static_cast<long>(size));
        result.insert("outputs", static_cast<long>(imageTypeHeader.numberOfOutputs));
        result.insert("lines", static_cast<long>(imageTypeHeader.numberOfImages));
        result.insert("average_value", static_cast<float>(imageTypeHeader.avgValue));
        result.insert("max_value", static_cast<float>(imageTypeHeader.maxValue));

        return true;
    }

    // TODO: handle other types

    return false;
}
