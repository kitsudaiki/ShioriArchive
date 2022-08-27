/**
 * @file        get_cluster_snapshot.cpp
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

#include "get_cluster_snapshot.h"

#include <sagiri_root.h>
#include <database/cluster_snapshot_table.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

GetClusterSnapshot::GetClusterSnapshot()
    : Kitsunemimi::Sakura::Blossom("Get snapshot of a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the original request-task, which placed the result in sagiri.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        Sakura::SAKURA_STRING_TYPE,
                        "UUID of the data-set.");
    registerOutputField("name",
                        Sakura::SAKURA_STRING_TYPE,
                        "Name of the data-set.");
    registerOutputField("location",
                        Sakura::SAKURA_STRING_TYPE,
                        "File path on local storage.");
    registerOutputField("header",
                        Sakura::SAKURA_MAP_TYPE,
                        "Header-information of the snapshot-file.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
GetClusterSnapshot::runTask(Sakura::BlossomLeaf &blossomLeaf,
                            const Kitsunemimi::DataMap &context,
                            Sakura::BlossomStatus &status,
                            ErrorContainer &error)
{
    const std::string dataUuid = blossomLeaf.input.get("uuid").getString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    if(SagiriRoot::clusterSnapshotTable->getClusterSnapshot(blossomLeaf.output,
                                                            dataUuid,
                                                            userContext,
                                                            error,
                                                            true) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // prepare header-information for output
    Kitsunemimi::Json::JsonItem parsedHeader;
    if(parsedHeader.parse(blossomLeaf.output.get("header").getString(), error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }
    blossomLeaf.output.insert("header", parsedHeader.getItemContent()->copy(), true);

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_id");
    blossomLeaf.output.remove("project_id");
    blossomLeaf.output.remove("visibility");
    blossomLeaf.output.remove("temp_files");

    return true;
}
