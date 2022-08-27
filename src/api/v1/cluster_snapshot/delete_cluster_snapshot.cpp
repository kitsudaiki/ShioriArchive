/**
 * @file        delete_cluster_snapshot.cpp
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

#include "delete_cluster_snapshot.h"

#include <sagiri_root.h>
#include <database/cluster_snapshot_table.h>

#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/methods/file_methods.h>

#include <libKitsunemimiHanamiCommon/enums.h>


using namespace Kitsunemimi;

DeleteClusterSnapshot::DeleteClusterSnapshot()
    : Kitsunemimi::Sakura::Blossom("Delete a result-set from sagiri.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster-snapshot to delete.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
DeleteClusterSnapshot::runTask(Sakura::BlossomLeaf &blossomLeaf,
                               const Kitsunemimi::DataMap &context,
                               Sakura::BlossomStatus &status,
                               ErrorContainer &error)
{
    const std::string dataUuid = blossomLeaf.input.get("uuid").getString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::clusterSnapshotTable->getClusterSnapshot(result,
                                                            dataUuid,
                                                            userContext,
                                                            error,
                                                            true) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get location from response
    const std::string location = result.get("location").getString();

    // delete entry from db
    if(SagiriRoot::clusterSnapshotTable->deleteClusterSnapshot(dataUuid,
                                                               userContext,
                                                               error) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // delete local files
    if(Kitsunemimi::deleteFileOrDir(location, error) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}
