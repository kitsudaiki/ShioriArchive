﻿/**
 * @file        create_cluster_snapshot.cpp
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

#include "create_cluster_snapshot.h"

#include <shiori_root.h>
#include <database/cluster_snapshot_table.h>
#include <core/temp_file_handler.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/defines.h>
#include <libKitsunemimiHanamiNetwork/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi::Sakura;

CreateClusterSnapshot::CreateClusterSnapshot()
    : Blossom("Init new snapshot of a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    // HINT(kitsudaiki): Snapshots are created internally asynchrous by the cluster and get the same
    //                   uuid as the task, where the snapshot was created. Because of this the uuid
    //                   has to be predefined.
    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the new snapshot.");
    assert(addFieldRegex("uuid", UUID_REGEX));

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new snapshot.");
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", NAME_REGEX));

    registerInputField("user_id",
                       SAKURA_STRING_TYPE,
                       true,
                       "ID of the user, who owns the snapshot.");
    assert(addFieldBorder("user_id", 4, 256));
    assert(addFieldRegex("user_id", ID_EXT_REGEX));

    registerInputField("project_id",
                       SAKURA_STRING_TYPE,
                       true,
                       "ID of the project, where the snapshot belongs to.");
    assert(addFieldBorder("project_id", 4, 256));
    assert(addFieldRegex("project_id", ID_REGEX));

    registerInputField("header",
                       SAKURA_MAP_TYPE,
                       true,
                       "Header of the file with information of the splits.");

    registerInputField("input_data_size",
                       SAKURA_INT_TYPE,
                       true,
                       "Total size of the snapshot in number of bytes.");
    assert(addFieldBorder("input_data_size", 1, 10000000000));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new snapshot.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new snapshot.");
    registerOutputField("uuid_input_file",
                        SAKURA_STRING_TYPE,
                        "UUID to identify the file for data upload of the snapshot.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateClusterSnapshot::runTask(BlossomLeaf &blossomLeaf,
                               const Kitsunemimi::DataMap &,
                               BlossomStatus &status,
                               Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string name = blossomLeaf.input.get("name").getString();
    const std::string userId = blossomLeaf.input.get("id").getString();
    const std::string projectId = blossomLeaf.input.get("project_id").getString();
    const long inputDataSize = blossomLeaf.input.get("input_data_size").getLong();
    const std::string header = blossomLeaf.input.get("header").toString();

    // snapshots are created by another internal process, which gives the id's not in the context
    // object, but as normal values
    Kitsunemimi::Hanami::UserContext userContext;
    userContext.userId = userId;
    userContext.projectId = projectId;

    // get directory to store data from config
    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("shiori", "cluster_snapshot_location", success);
    if(success == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("snapshot-location to store cluster-snapshot is missing in the config");
        return false;
    }

    // init temp-file for input-data
    const std::string tempFileUuid = Kitsunemimi::Hanami::generateUuid().toString();
    if(ShioriRoot::tempFileHandler->initNewFile(tempFileUuid, inputDataSize) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to initialize temporary file for new input-data.");
        return false;
    }    

    // build absolut file-path to store the file
    if(targetFilePath.at(targetFilePath.size() - 1) != '/') {
        targetFilePath.append("/");
    }
    targetFilePath.append(uuid + "_snapshot_" + userId);

    // register in database
    blossomLeaf.output.insert("uuid", uuid);
    blossomLeaf.output.insert("name", name);
    blossomLeaf.output.insert("location", targetFilePath);
    blossomLeaf.output.insert("header", header);
    blossomLeaf.output.insert("project_id", projectId);
    blossomLeaf.output.insert("owner_id", userId);
    blossomLeaf.output.insert("visibility", "private");

    // init placeholder for temp-file progress to database
    Kitsunemimi::Json::JsonItem tempFiles;
    tempFiles.insert(tempFileUuid, Kitsunemimi::Json::JsonItem(0.0f));
    blossomLeaf.output.insert("temp_files", tempFiles);

    // add to database
    if(ShioriRoot::clusterSnapshotTable->addClusterSnapshot(blossomLeaf.output,
                                                            userContext,
                                                            error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // add values to output
    blossomLeaf.output.insert("uuid_input_file", tempFileUuid);

    // remove blocked values from output
    blossomLeaf.output.remove("location");
    blossomLeaf.output.remove("header");
    blossomLeaf.output.remove("project_id");
    blossomLeaf.output.remove("owner_id");
    blossomLeaf.output.remove("visibility");
    blossomLeaf.output.remove("temp_files");

    return true;
}
