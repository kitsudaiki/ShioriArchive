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

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

DeleteTrainData::DeleteTrainData()
    : Kitsunemimi::Sakura::Blossom()
{
    registerInputField("uuid", true);
}

/**
 * @brief runTask
 */
bool
DeleteTrainData::runTask(BlossomLeaf &blossomLeaf,
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

    if(SagiriRoot::trainDataTable->deleteTrainData(dataUuid, userUuid, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}
