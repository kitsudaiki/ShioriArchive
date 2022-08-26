/**
 * @file        list_cluster_snapshot.cpp
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

#include "list_cluster_snapshot.h"

#include <sagiri_root.h>
#include <database/cluster_snapshot_table.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

ListClusterSnapshot::ListClusterSnapshot()
    : Kitsunemimi::Sakura::Blossom("List snapshots of all cluster.")
{
    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("header",
                        Sakura::SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");
    // TODO: regex for header
    registerOutputField("body",
                        Sakura::SAKURA_ARRAY_TYPE,
                        "Array with all rows of the table, which array arrays too.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
ListClusterSnapshot::runTask(Sakura::BlossomLeaf &blossomLeaf,
                             const Kitsunemimi::DataMap &context,
                             Sakura::BlossomStatus &status,
                             ErrorContainer &error)
{
    const std::string userId = context.getStringByKey("uuid");
    const std::string projectId = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get data from table
    Kitsunemimi::TableItem table;
    if(SagiriRoot::clusterSnapshotTable->getAllClusterSnapshot(table,
                                                               userId,
                                                               projectId,
                                                               isAdmin,
                                                               error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    table.deleteColumn("pw_hash");
    blossomLeaf.output.insert("header", table.getInnerHeader());
    blossomLeaf.output.insert("body", table.getBody());

    return true;
}
