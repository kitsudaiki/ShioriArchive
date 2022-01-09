/**
 * @file        create_token.h
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

#include "list_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

ListTrainData::ListTrainData()
    : Kitsunemimi::Sakura::Blossom("Get information of all uploaded sets fo train-data as table.")
{
    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("header",
                        SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");

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
ListTrainData::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    // get data from table
    Kitsunemimi::TableItem table;
    if(SagiriRoot::trainDataTable->getAllTrainData(table, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    table.deleteColumn("pw_hash");
    blossomLeaf.output.insert("header", table.getInnerHeader());
    blossomLeaf.output.insert("body", table.getBody());

    return true;
}