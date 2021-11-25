/**
 * @file        users_database.cpp
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

#include <database/train_data_table.h>

#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiSakuraDatabase/sql_database.h>

TrainDataTable::TrainDataTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : SqlTable(db)
{
    m_tableName = "train_data";

    DbHeaderEntry name;
    name.name = "name";
    name.maxLength = 256;
    m_tableHeader.push_back(name);

    DbHeaderEntry user;
    user.name = "user_uuid";
    user.maxLength = 256;
    m_tableHeader.push_back(user);

    DbHeaderEntry type;
    type.name = "type";
    type.maxLength = 64;
    m_tableHeader.push_back(type);

    DbHeaderEntry location;
    location.name = "location";
    m_tableHeader.push_back(location);
}

TrainDataTable::~TrainDataTable() {}

/**
 * @brief Users::addUser
 * @param data
 * @param errorMessage
 * @return
 */
const std::string
TrainDataTable::addTrainData(const TrainDataData &data,
                             Kitsunemimi::ErrorContainer &error)
{
    const std::vector<std::string> values = { data.name,
                                              data.userUuid,
                                              data.type,
                                              data.location };
    return insertToDb(values, error);
}

/**
 * @brief UsersDatabase::getUser
 * @param userID
 * @param error
 * @return
 */
bool
TrainDataTable::getTrainData(Kitsunemimi::Json::JsonItem &result,
                             const std::string &uuid,
                             const std::string &userUuid,
                             Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::TableItem tableContent;

    // get user from db
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", uuid);
    conditions.emplace_back("user_uuid", userUuid);

    // get user from db
    if(getFromDb(&tableContent, conditions, error) == false)
    {
        LOG_ERROR(error);
        return false;
    }

    if(tableContent.getNumberOfRows() == 0)
    {
        error.addMeesage("Train-Data with ID '"
                         + uuid
                         + "' for user '"
                         + userUuid
                         + "'not found;");
        LOG_ERROR(error);
        return false;
    }

    processGetResult(result, tableContent);

    return true;
}

/**
 * @brief TrainDataTable::getAllTrainData
 * @param result
 * @param error
 * @return
 */
bool
TrainDataTable::getAllTrainData(Kitsunemimi::TableItem &result,
                                Kitsunemimi::ErrorContainer &error)
{
    return getAllFromDb(&result, error);
}

/**
 * @brief UsersDatabase::deleteUser
 * @param userID
 * @param error
 * @return
 */
bool
TrainDataTable::deleteTrainData(const std::string &uuid,
                                const std::string &userUuid,
                                Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", uuid);
    conditions.emplace_back("user_uuid", userUuid);
    return deleteFromDb(conditions, error);
}

/**
 * @brief TrainDataTable::processGetResult
 * @param result
 * @param tableContent
 */
void
TrainDataTable::processGetResult(Kitsunemimi::Json::JsonItem &result,
                                 Kitsunemimi::TableItem &tableContent)
{
    const Kitsunemimi::DataItem* firstRow = tableContent.getBody()->get(0);

    for(uint32_t i = 0; i < m_tableHeader.size(); i++) {
        result.insert(m_tableHeader.at(i).name, firstRow->get(i));
    }
}
