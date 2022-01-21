/**
 * @file        sagiri_root.cpp
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

#include "sagiri_root.h"

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiSakuraDatabase/sql_database.h>
#include <database/data_set_table.h>

#include <api/blossom_initializing.h>

DataSetTable* SagiriRoot::dataSetTable = nullptr;
Kitsunemimi::Sakura::SqlDatabase* SagiriRoot::database = nullptr;

SagiriRoot::SagiriRoot() {}

/**
 * @brief SagiriRoot::init
 * @return
 */
bool
SagiriRoot::init()
{
    Kitsunemimi::ErrorContainer error;
    bool success = false;

    // read database-path from config
    database = new Kitsunemimi::Sakura::SqlDatabase();
    const std::string databasePath = GET_STRING_CONFIG("DEFAULT", "database", success);
    if(success == false)
    {
        error.addMeesage("No database-path defined in config.");
        LOG_ERROR(error);
        return false;
    }

    // initalize database
    if(database->initDatabase(databasePath, error) == false)
    {
        error.addMeesage("Failed to initialize sql-database.");
        LOG_ERROR(error);
        return false;
    }

    // initialize users-table
    dataSetTable = new DataSetTable(database);
    if(dataSetTable->initTable(error) == false)
    {
        error.addMeesage("Failed to initialize train-data-table in database.");
        LOG_ERROR(error);
        return false;
    }

    initBlossoms();

    return true;
}
