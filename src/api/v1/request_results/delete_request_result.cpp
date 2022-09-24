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

#include <shiori_root.h>
#include <database/request_result_table.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi;

DeleteRequestResult::DeleteRequestResult()
    : Kitsunemimi::Sakura::Blossom("Delete a result-set from shiori.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the original request-task, which placed the result in shiori.");
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
                             const Kitsunemimi::DataMap &context,
                             Sakura::BlossomStatus &status,
                             ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // check if request-result exist within the table
    Kitsunemimi::Json::JsonItem result;
    if(ShioriRoot::requestResultTable->getRequestResult(result,
                                                        uuid,
                                                        userContext,
                                                        error,
                                                        false) == false)
    {
        status.errorMessage = "Request-result with UUID '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // delete entry from db
    if(ShioriRoot::requestResultTable->deleteRequestResult(uuid, userContext, error) == false)
    {
        status.statusCode = Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}
