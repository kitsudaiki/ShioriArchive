/**
 * @file        finish_cluster_snapshot.cpp
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

#include "finish_cluster_snapshot.h"

#include <sagiri_root.h>
#include <database/cluster_snapshot_table.h>
#include <core/temp_file_handler.h>

#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi::Sakura;

FinalizeClusterSnapshot::FinalizeClusterSnapshot()
    : Kitsunemimi::Sakura::Blossom("Finish snapshot of a cluster.")
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

    registerInputField("user_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldRegex("user_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                      "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("project_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    // TODO: issue Hanami-Meta#17
    //assert(addFieldRegex("project_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
    //                                     "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("uuid_input_file",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identify the file for date upload of input-data.");
    assert(addFieldRegex("uuid_input_file", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                            "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
FinalizeClusterSnapshot::runTask(BlossomLeaf &blossomLeaf,
                               const Kitsunemimi::DataMap &context,
                               BlossomStatus &status,
                               Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string inputUuid = blossomLeaf.input.get("uuid_input_file").getString();
    const std::string userUuid = blossomLeaf.input.get("user_uuid").getString();
    const std::string projectUuid = blossomLeaf.input.get("project_uuid").getString();

    const bool isAdmin = context.getBoolByKey("is_admin");

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::clusterSnapshotTable->getClusterSnapshot(result,
                                                            uuid,
                                                            userUuid,
                                                            projectUuid,
                                                            isAdmin,
                                                            error,
                                                            true) == false)
    {
        status.errorMessage = "Snapshot with uuid '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // read input-data from temp-file
    Kitsunemimi::DataBuffer inputBuffer;
    if(SagiriRoot::tempFileHandler->getData(inputBuffer, inputUuid) == false)
    {
        status.errorMessage = "Input-data with uuid '" + inputUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // move temp-file to target-location
    const std::string targetLocation = result.get("location").getString();
    if(SagiriRoot::tempFileHandler->moveData(inputUuid, targetLocation) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create output
    blossomLeaf.output.insert("uuid", uuid);

    return true;
}
