/**
 * @file        get_train_data.cpp
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

#include "get_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>

#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;

GetTrainData::GetTrainData()
    : Kitsunemimi::Sakura::Blossom()
{
    registerInputField("uuid", true);

    registerOutputField("uuid");
    registerOutputField("name");
    registerOutputField("type");
    registerOutputField("user_uuid");
    registerOutputField("data");
}

/**
 * @brief runTask
 */
bool
GetTrainData::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      BlossomStatus &status,
                      Kitsunemimi::ErrorContainer &error)
{
    const std::string dataUuid = blossomLeaf.input.getStringByKey("uuid");
    const std::string userUuid = context.getStringByKey("uuid");

    TrainDataTable::TrainDataData result;
    if(SagiriRoot::trainDataTable->getTrainData(result, dataUuid, userUuid, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // write data to file
    Kitsunemimi::BinaryFile targetFile(result.location, false);
    Kitsunemimi::DataBuffer data;
    if(targetFile.readCompleteFile(data) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to read train-data from file \"" + result.location + "\"");
        return false;
    }
    targetFile.closeFile();

    std::string base64String;
    Kitsunemimi::Crypto::encodeBase64(base64String, data.data, data.usedBufferSize);

    // create output
    blossomLeaf.output.insert("uuid", new Kitsunemimi::DataValue(result.userUuid));
    blossomLeaf.output.insert("name", new Kitsunemimi::DataValue(result.name));
    blossomLeaf.output.insert("user_uuid", new Kitsunemimi::DataValue(result.userUuid));
    blossomLeaf.output.insert("type", new Kitsunemimi::DataValue(result.type));
    blossomLeaf.output.insert("data", new Kitsunemimi::DataValue(base64String));

    return true;
}
