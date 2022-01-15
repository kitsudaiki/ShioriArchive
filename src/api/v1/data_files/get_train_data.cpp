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
#include <libKitsunemimiJson/json_item.h>

using namespace Kitsunemimi::Sakura;

GetTrainData::GetTrainData()
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

    registerInputField("with_data",
                       SAKURA_BOOL_TYPE,
                       false,
                       "Have to be set to true to also return the data "
                       "and not only the meta-information");

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
    registerOutputField("data",
                        SAKURA_STRING_TYPE,
                        "If the flag 'with_data' was set the true, this field contains the data "
                        "of the stored train-data-set as base64 encoded string.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
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
    const std::string dataUuid = blossomLeaf.input.get("uuid").getString();
    const bool withData = blossomLeaf.input.get("with_data").getBool();
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    if(SagiriRoot::trainDataTable->getTrainData(blossomLeaf.output,
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

    std::cout<<blossomLeaf.input.toString(true)<<std::endl;
    // read data from file and add to output, if requested
    if(withData)
    {
        const std::string location = blossomLeaf.output.get("location").getString();

        Kitsunemimi::BinaryFile targetFile(location, false);
        Kitsunemimi::DataBuffer data;
        if(targetFile.readCompleteFile(data) == false)
        {
            status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
            error.addMeesage("Failed to read train-data from file \"" + location + "\"");
            return false;
        }
        targetFile.closeFile();

        std::string base64String;
        Kitsunemimi::Crypto::encodeBase64(base64String, data.data, data.usedBufferSize);

        for(int i = 0; i < 100; i++) {
            std::cout<<base64String[i];
        }
        std::cout<<std::endl;

        // create output
        blossomLeaf.output.insert("data", base64String);
    }

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");
    blossomLeaf.output.remove("location");

    return true;
}
