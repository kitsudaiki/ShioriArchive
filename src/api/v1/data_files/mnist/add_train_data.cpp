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
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));

    registerInputField("position",
                       SAKURA_INT_TYPE,
                       true,
                       "Segment-position, where the new data have to be written into the file.");
    assert(addFieldBorder("position", 0, 10000000000));

    registerInputField("data_type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the new data (options: input or label)");
    assert(addFieldRegex("data_type", "(input|label)"));

    registerInputField("data",
                       SAKURA_STRING_TYPE,
                       true,
                       "New data as base64-string.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
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
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string base64Data = blossomLeaf.input.get("data").getString();
    const long position = blossomLeaf.input.get("position").getLong();
    const std::string dataType = blossomLeaf.input.get("data_type").getString();
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::trainDataTable->getTrainData(result,
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

    const std::string targetFilePath = result.get("location").getString()
                                       + "_"
                                       + dataType
                                       + "_temp";

    // check data-input
    if(base64Data.size() == 0)
    {
        status.statusCode = Kitsunemimi::Hanami::BAD_REQUEST_RTYPE;
        status.errorMessage = "Data are missing in request";
        error.addMeesage(status.errorMessage);
        return false;
    }

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
    if(targetFile.writeSegment(data, position, data.usedBufferSize) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write train-data to file \"" + targetFilePath + "\"");
        return false;
    }
    targetFile.closeFile();

    return true;
}
