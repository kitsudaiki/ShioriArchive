/**
 * @file        error_log_table.cpp
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

#include <database/error_log_table.h>

#include <libKitsunemimiCommon/items/table_item.h>
#include <libKitsunemimiCommon/methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiSakuraDatabase/sql_database.h>

/**
 * @brief constructor
 *
 * @param db pointer to database
 */
ErrorLogTable::ErrorLogTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : HanamiSqlLogTable(db)
{
    m_tableName = "error_log";

    DbHeaderEntry userid;
    userid.name = "user_id";
    userid.maxLength = 256;
    m_tableHeader.push_back(userid);

    DbHeaderEntry component;
    component.name = "component";
    component.maxLength = 128;
    m_tableHeader.push_back(component);

    DbHeaderEntry context;
    context.name = "context";
    m_tableHeader.push_back(context);

    DbHeaderEntry values;
    values.name = "input_values";
    m_tableHeader.push_back(values);

    DbHeaderEntry message;
    message.name = "message";
    m_tableHeader.push_back(message);
}

/**
 * @brief destructor
 */
ErrorLogTable::~ErrorLogTable() {}

/**
 * @brief add new error-log-entry into the database
 *
 * @param timestamp UTC-timestamp of the error
 * @param userid id of the user, who had the error
 * @param component component, where the error appeared
 * @param context
 * @param values
 * @param message
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
ErrorLogTable::addErrorLogEntry(const std::string &timestamp,
                                const std::string &userid,
                                const std::string &component,
                                const std::string &context,
                                const std::string &values,
                                const std::string &message,
                                Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::Json::JsonItem data;
    data.insert("timestamp", timestamp);
    data.insert("user_id", userid);
    data.insert("component", component);
    data.insert("context", context);
    data.insert("input_values", values);
    data.insert("message", message);

    if(insertToDb(data, error) == false)
    {
        error.addMeesage("Failed to add error-log-entry to database");
        return false;
    }

    return true;
}

/**
 * @brief get all error-log-entries from the database
 *
 * @param result reference for the result-output
 * @param userId id of the user, whos logs are requested
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
ErrorLogTable::getAllErrorLogEntries(Kitsunemimi::TableItem &result,
                                     const std::string &userId,
                                     Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.push_back(RequestCondition("user_id", userId));
    if(getFromDb(result, conditions, error, true) == false)
    {
        error.addMeesage("Failed to get all error-log-entries from database");
        return false;
    }

    return true;
}
