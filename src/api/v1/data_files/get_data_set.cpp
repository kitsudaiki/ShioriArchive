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

#include <libKitsunemimiHanamiCommon/enums.h>
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

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
