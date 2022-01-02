/**
 * @file        delete_train_data.cpp
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

#include "delete_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

DeleteTrainData::DeleteTrainData()
    : Kitsunemimi::Sakura::Blossom("Delete a speific set of train-data.")
{
    registerInputField("uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the train-data set to delete.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));
}

/**
 * @brief runTask
 */
bool
DeleteTrainData::runTask(Sakura::BlossomLeaf &blossomLeaf,
                         const Kitsunemimi::DataMap &context,
                         Sakura::BlossomStatus &status,
                         ErrorContainer &error)
{
    const std::string dataUuid = blossomLeaf.input.get("uuid").getString();
    const std::string userUuid = context.getStringByKey("uuid");

    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::trainDataTable->getTrainData(result, dataUuid, userUuid, error, false) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    if(SagiriRoot::trainDataTable->deleteTrainData(dataUuid, userUuid, error) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}
