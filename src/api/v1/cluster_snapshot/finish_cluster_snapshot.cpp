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

#include <shiori_root.h>
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

    registerInputField("user_id",
                       SAKURA_STRING_TYPE,
                       true,
                       "ID of the user, who belongs to the snapshot.");
    assert(addFieldBorder("user_id", 4, 256));
    assert(addFieldRegex("user_id", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("project_id",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    // TODO: issue Hanami-Meta#17
    //assert(addFieldRegex("project_id", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
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
                                 const Kitsunemimi::DataMap &,
                                 BlossomStatus &status,
                                 Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string inputUuid = blossomLeaf.input.get("uuid_input_file").getString();
    const std::string userId = blossomLeaf.input.get("id").getString();
    const std::string projectId = blossomLeaf.input.get("project_id").getString();

    // snapshots are created by another internal process, which gives the id's not in the context
    // object, but as normal values
    Kitsunemimi::Hanami::UserContext userContext;
    userContext.userId = userId;
    userContext.projectId = projectId;

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(ShioriRoot::clusterSnapshotTable->getClusterSnapshot(result,
                                                            uuid,
                                                            userContext,
                                                            error,
                                                            true) == false)
    {
        status.errorMessage = "Snapshot with uuid '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // read input-data from temp-file
    Kitsunemimi::DataBuffer inputBuffer;
    if(ShioriRoot::tempFileHandler->getData(inputBuffer, inputUuid) == false)
    {
        status.errorMessage = "Input-data with uuid '" + inputUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // move temp-file to target-location
    const std::string targetLocation = result.get("location").getString();
    if(ShioriRoot::tempFileHandler->moveData(inputUuid, targetLocation, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create output
    blossomLeaf.output.insert("uuid", uuid);

    return true;
}
