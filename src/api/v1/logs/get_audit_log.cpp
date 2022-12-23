/**
 * @file        get_audit_log.cpp
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

#include "get_audit_log.h"

#include <shiori_root.h>
#include <database/audit_log_table.h>

#include <libKitsunemimiHanamiCommon/defines.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>

using namespace Kitsunemimi::Hanami;

GetAuditLog::GetAuditLog()
    : Blossom("Get audit-log of a user.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------
    registerInputField("user_id",
                       SAKURA_STRING_TYPE,
                       false,
                       "ID of the user, whos entries are requested. Only an admin is allowed to "
                       "set this values. Any other user get only its own log output based on the "
                       "token-context.");
    assert(addFieldBorder("user_id", 4, 256));
    assert(addFieldRegex("user_id", ID_EXT_REGEX));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("header",
                        SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");
    assert(addFieldMatch("header", new Kitsunemimi::DataValue("[\"timestamp\","
                                                              "\"user_id\","
                                                              "\"component\","
                                                              "\"endpoint\","
                                                              "\"request_type\"]")));
    registerOutputField("body",
                        SAKURA_ARRAY_TYPE,
                        "Array with all rows of the table, which array arrays too.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
GetAuditLog::runTask(BlossomIO &blossomIO,
                     const Kitsunemimi::DataMap &context,
                     BlossomStatus &status,
                     Kitsunemimi::ErrorContainer &error)
{
    const Kitsunemimi::Hanami::UserContext userContext(context);
    std::string userId = blossomIO.input.get("user_id").getString();

    // check that if user-id is set, that the user is also an admin
    if(userContext.isAdmin == false
            && userId.length() != 0)
    {
        status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
        status.errorMessage = "'user_id' can only be set by an admin";
        return false;
    }

    // if no user-id was defined, use the id of the context
    if(userId.length() == 0) {
        userId = userContext.userId;
    }

    // get data from table
    Kitsunemimi::TableItem table;
    if(ShioriRoot::auditLogTable->getAllAuditLogEntries(table, userId, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    blossomIO.output.insert("header", table.getInnerHeader());
    blossomIO.output.insert("body", table.getBody());

    return true;
}
