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

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi;

AddTrainData::AddTrainData()
    : Kitsunemimi::Sakura::Blossom()
{
    registerInputField("name", true);
    registerInputField("type", true);
    registerInputField("data", true);

    registerOutputField("uuid");
    registerOutputField("name");
    registerOutputField("user_uuid");
    registerOutputField("type");
}

/**
 * @brief runTask
 */
bool
AddTrainData::runTask(Sakura::BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      Sakura::BlossomStatus &status,
                      ErrorContainer &error)
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
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write train-data to file \"" + targetFilePath + "\"");
        return false;
    }
    targetFile.closeFile();

    // register in database
    TrainDataTable::TrainDataData newDbData;
    newDbData.name = name;
    newDbData.type = type;
    newDbData.userUuid = userUuid;
    newDbData.location = targetFilePath;
    const std::string uuid = SagiriRoot::trainDataTable->addTrainData(newDbData, error);
    if(uuid == "")
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create output
    blossomLeaf.output.insert("uuid", new Kitsunemimi::DataValue(uuid));
    blossomLeaf.output.insert("name", new Kitsunemimi::DataValue(name));
    blossomLeaf.output.insert("user_uuid", new Kitsunemimi::DataValue(userUuid));
    blossomLeaf.output.insert("type", new Kitsunemimi::DataValue(type));

    return true;
}
