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

#include <shiori_root.h>
#include <database/data_set_table.h>
#include <core/data_set_files/data_set_file.h>
#include <core/data_set_files/image_data_set_file.h>
#include <core/data_set_files/table_data_set_file.h>

#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiJson/json_item.h>

using namespace Kitsunemimi::Sakura;

GetDataSet::GetDataSet()
    : Blossom("Get information of a specific set of dataset.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the dataset set to delete.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the data-set.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the data-set.");
    registerOutputField("type",
                        SAKURA_STRING_TYPE,
                        "Type of the new set (For example: CSV)");
    registerOutputField("user_id",
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
    const Kitsunemimi::Hanami::UserContext userContext(context);

    if(ShioriRoot::dataSetTable->getDataSet(blossomLeaf.output,
                                            dataUuid,
                                            userContext,
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
    blossomLeaf.output.remove("owner_id");
    blossomLeaf.output.remove("project_id");
    blossomLeaf.output.remove("visibility");
    blossomLeaf.output.remove("temp_files");

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
    DataSetFile* file = readDataSetFile(location);
    if(file == nullptr) {
        return false;
    }

    // read data-set-header
    if(file->readFromFile() == false)
    {
        error.addMeesage("Failed to read header from file '" + location + "'");
        return false;
    }

    if(file->type == DataSetFile::IMAGE_TYPE)
    {
        ImageDataSetFile* imgF = dynamic_cast<ImageDataSetFile*>(file);
        if(imgF == nullptr) {
            return false;
        }

        // write information to result
        const uint64_t size = imgF->imageHeader.numberOfInputsX * imgF->imageHeader.numberOfInputsY;
        result.insert("inputs", static_cast<long>(size));
        result.insert("outputs", static_cast<long>(imgF->imageHeader.numberOfOutputs));
        result.insert("lines", static_cast<long>(imgF->imageHeader.numberOfImages));
        result.insert("average_value", static_cast<float>(imgF->imageHeader.avgValue));
        result.insert("max_value", static_cast<float>(imgF->imageHeader.maxValue));

        return true;
    }
    else if(file->type == DataSetFile::TABLE_TYPE)
    {
        TableDataSetFile* imgT = dynamic_cast<TableDataSetFile*>(file);
        if(imgT == nullptr) {
            return false;
        }

        long inputs = 0;
        long outputs = 0;

        // get number of inputs and outputs
        for(const DataSetFile::TableHeaderEntry &entry : imgT->tableColumns)
        {
            if(entry.isInput) {
                inputs++;
            }
            if(entry.isOutput) {
                outputs++;
            }
        }

        result.insert("inputs", inputs);
        result.insert("outputs", outputs);
        result.insert("lines", static_cast<long>(imgT->tableHeader.numberOfLines));
        result.insert("average_value", 0.0f);
        result.insert("max_value", 0.0f);

        return true;
    }

    // TODO: handle other types

    return false;
}
