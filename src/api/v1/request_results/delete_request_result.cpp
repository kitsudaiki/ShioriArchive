/**
 * @file        delete_request_result.cpp
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

#include "delete_request_result.h"

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

DeleteRequestResult::DeleteRequestResult()
    : Kitsunemimi::Sakura::Blossom("Delete a result-set from sagiri.")
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
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
DeleteRequestResult::runTask(Sakura::BlossomLeaf &blossomLeaf,
                             const Kitsunemimi::DataMap &,
                             Sakura::BlossomStatus &status,
                             ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();

    bool success = false;
    const std::string resultLocation = GET_STRING_CONFIG("sagiri", "result_location", success);
    const std::string filePath = resultLocation + "/" + uuid;

    // TODO: precheck if file exist
    if(deleteFileOrDir(filePath, error) == false)
    {
        status.statusCode = Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    return true;
}