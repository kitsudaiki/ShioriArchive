/**
 * @file        add_train_data.cpp
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

#include "add_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi::Sakura;

AddTrainData::AddTrainData()
    : Kitsunemimi::Sakura::Blossom("Add new set of train-data.")
{
    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    registerInputField("type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the new set (For example: CSV)");
    registerInputField("data",
                       SAKURA_STRING_TYPE,
                       true,
                       "New data as base64-string.");

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new set.");
    registerOutputField("user_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the user who uploaded the data.");
    registerOutputField("type",
                        SAKURA_STRING_TYPE,
                        "Type of the new set (For example: CSV)");
}

/**
 * @brief runTask
 */
bool
AddTrainData::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      BlossomStatus &status,
                      Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.getStringByKey("name");
    const std::string type = blossomLeaf.input.getStringByKey("type");
    const std::string base64Data = blossomLeaf.input.getStringByKey("data");
    const std::string userUuid = context.getStringByKey("uuid");

    // get directory to store data from config
    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "train_data_location", success);
    if(success == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("file-location to store train-data is missing in the config");
        return false;
    }

    // build absolut file-path to store the file
    if(targetFilePath.at(targetFilePath.size() - 1) != '/') {
        targetFilePath.append("/");
    }
    targetFilePath.append(name + "_" + type + "_" + userUuid);

    // decode base64-data
    Kitsunemimi::DataBuffer data;
    if(Kitsunemimi::Crypto::decodeBase64(data, base64Data) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::BAD_REQUEST_RTYPE;
        status.errorMessage = "Data are no valid base64.";
        error.addMeesage(status.errorMessage);
        return false;
    }

    // write data to file
    Kitsunemimi::BinaryFile targetFile(targetFilePath, false);
    if(targetFile.writeCompleteFile(data) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write train-data to file \"" + targetFilePath + "\"");
        return false;
    }
    targetFile.closeFile();

    // register in database
    Kitsunemimi::Json::JsonItem newDbData;
    newDbData.insert("name", name);
    newDbData.insert("user_uuid", userUuid);
    newDbData.insert("type", type);
    newDbData.insert("location", targetFilePath);

    newDbData.insert("project_uuid", "-");
    newDbData.insert("owner_uuid", "-");
    newDbData.insert("visibility", 0);

    if(SagiriRoot::trainDataTable->addTrainData(newDbData, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create output
    blossomLeaf.output = *newDbData.getItemContent()->toMap();

    return true;
}
