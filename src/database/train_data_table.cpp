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

/**
 * @brief constructor
 *
 * @param db pointer to database
 */
TrainDataTable::TrainDataTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : HanamiSqlTable(db)
{
    m_tableName = "train_data";

    DbHeaderEntry name;
    name.name = "name";
    name.maxLength = 256;
    m_tableHeader.push_back(name);

    DbHeaderEntry type;
    type.name = "type";
    type.maxLength = 64;
    m_tableHeader.push_back(type);

    DbHeaderEntry location;
    location.name = "location";
    location.hide = true;
    m_tableHeader.push_back(location);
}

/**
 * @brief destructor
 */
TrainDataTable::~TrainDataTable() {}

/**
 * @brief add new metadata of a train-data-set into the database
 *
 * @param userData json-item with all information of the data to add to database
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TrainDataTable::addTrainData(Kitsunemimi::Json::JsonItem &data,
                             const std::string &userUuid,
                             const std::string &projectUuid,
                             Kitsunemimi::ErrorContainer &error)
{
    return add(data, userUuid, projectUuid, error);
}

/**
 * @brief get a metadata-entry for a specific train-data-set from the database
 *
 * @param result reference for the result-output
 * @param uuid uuid of the data
 * @param userUuid uuid of the user
 * @param error reference for error-output
 * @param showHiddenValues set to true to also show as hidden marked fields
 *
 * @return true, if successful, else false
 */
bool
TrainDataTable::getTrainData(Kitsunemimi::Json::JsonItem &result,
                             const std::string &uuid,
                             const std::string &userUuid,
                             const std::string &projectUuid,
                             const bool isAdmin,
                             Kitsunemimi::ErrorContainer &error,
                             const bool showHiddenValues)
{
    // get user from db
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", uuid);

    // get user from db
    if(get(result, userUuid, projectUuid, isAdmin, conditions, error, showHiddenValues) == false)
    {
        LOG_ERROR(error);
        return false;
    }

    return true;
}

/**
 * @brief get metadata of all train-data-sets from the database
 *
 * @param result reference for the result-output
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TrainDataTable::getAllTrainData(Kitsunemimi::TableItem &result,
                                const std::string &userUuid,
                                const std::string &projectUuid,
                                const bool isAdmin,
                                Kitsunemimi::ErrorContainer &error)
{
    return getAll(result, userUuid, projectUuid, isAdmin, error);
}

/**
 * @brief delete metadata of a train-data-set from the database
 *
 * @param uuid uuid of the data
 * @param userUuid uuid of the user
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TrainDataTable::deleteTrainData(const std::string &uuid,
                                const std::string &userUuid,
                                const std::string &projectUuid,
                                const bool isAdmin,
                                Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", uuid);
    return del(conditions, userUuid, projectUuid, isAdmin, error);
}
