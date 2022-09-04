/**
 * @file        create_mnist_data_set.cpp
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

#include "create_mnist_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/temp_file_handler.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiNetwork/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi::Sakura;

CreateMnistDataSet::CreateMnistDataSet()
    : Kitsunemimi::Sakura::Blossom("Init new set of dataset.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("input_data_size",
                       SAKURA_INT_TYPE,
                       true,
                       "Total size of the input-data.");
    assert(addFieldBorder("input_data_size", 1, 10000000000));

    registerInputField("label_data_size",
                       SAKURA_INT_TYPE,
                       true,
                       "Total size of the label-data.");
    assert(addFieldBorder("label_data_size", 1, 10000000000));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new set.");
    registerOutputField("user_id",
                        SAKURA_STRING_TYPE,
                        "UUID of the user who uploaded the data.");
    registerOutputField("type",
                        SAKURA_STRING_TYPE,
                        "Type of the new set (For example: csv)");
    registerOutputField("uuid_input_file",
                        SAKURA_STRING_TYPE,
                        "UUID to identify the file for date upload of input-data.");
    registerOutputField("uuid_label_file",
                        SAKURA_STRING_TYPE,
                        "UUID to identify the file for date upload of label-data.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

bool
CreateMnistDataSet::runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                            const Kitsunemimi::DataMap &context,
                            Kitsunemimi::Sakura::BlossomStatus &status,
                            Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const long inputDataSize = blossomLeaf.input.get("input_data_size").getLong();
    const long labelDataSize = blossomLeaf.input.get("label_data_size").getLong();
    const std::string uuid = Kitsunemimi::Hanami::generateUuid().toString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // get directory to store data from config
    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "data_set_location", success);
    if(success == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("file-location to store dataset is missing in the config");
        return false;
    }

    // init temp-file for input-data
    const std::string inputUuid = Kitsunemimi::Hanami::generateUuid().toString();
    if(SagiriRoot::tempFileHandler->initNewFile(inputUuid, inputDataSize) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to initialize temporary file for new input-data.");
        return false;
    }

    // init temp-file for label-data
    const std::string labelUuid = Kitsunemimi::Hanami::generateUuid().toString();
    if(SagiriRoot::tempFileHandler->initNewFile(labelUuid, labelDataSize) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to initialize temporary file for new label-data.");
        return false;
    }

    // build absolut file-path to store the file
    if(targetFilePath.at(targetFilePath.size() - 1) != '/') {
        targetFilePath.append("/");
    }
    targetFilePath.append(uuid + "_mnist_" + userContext.userId);

    // register in database
    blossomLeaf.output.insert("uuid", uuid);
    blossomLeaf.output.insert("name", name);
    blossomLeaf.output.insert("type", "mnist");
    blossomLeaf.output.insert("location", targetFilePath);
    blossomLeaf.output.insert("project_id", userContext.projectId);
    blossomLeaf.output.insert("owner_id", userContext.userId);
    blossomLeaf.output.insert("visibility", "private");

    // init placeholder for temp-file progress to database
    Kitsunemimi::Json::JsonItem tempFiles;
    tempFiles.insert(inputUuid, Kitsunemimi::Json::JsonItem(0.0f));
    tempFiles.insert(labelUuid, Kitsunemimi::Json::JsonItem(0.0f));
    blossomLeaf.output.insert("temp_files", tempFiles);

    // add to database
    if(SagiriRoot::dataSetTable->addDataSet(blossomLeaf.output, userContext, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // add values to output
    blossomLeaf.output.insert("uuid_input_file", inputUuid);
    blossomLeaf.output.insert("uuid_label_file", labelUuid);

    // remove blocked values from output
    blossomLeaf.output.remove("location");
    blossomLeaf.output.remove("project_id");
    blossomLeaf.output.remove("owner_id");
    blossomLeaf.output.remove("visibility");
    blossomLeaf.output.remove("temp_files");

    return true;
}
