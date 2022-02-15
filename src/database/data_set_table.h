/**
 * @file        data_set_table.h
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

#ifndef SAGIRIARCHIVE_DATA_SET_TABLE_H
#define SAGIRIARCHIVE_DATA_SET_TABLE_H

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiHanamiDatabase/hanami_sql_table.h>

namespace Kitsunemimi {
namespace Json {
class JsonItem;
}
}

class DataSetTable
        : public Kitsunemimi::Hanami::HanamiSqlTable
{
public:
    DataSetTable(Kitsunemimi::Sakura::SqlDatabase* db);
    ~DataSetTable();

    bool addDataSet(Kitsunemimi::Json::JsonItem &data,
                    const std::string &userUuid,
                    const std::string &projectUuid,
                    Kitsunemimi::ErrorContainer &error);
    bool getDataSet(Kitsunemimi::Json::JsonItem &result,
                    const std::string &uuid,
                    const std::string &userUuid,
                    const std::string &projectUuid,
                    const bool isAdmin,
                    Kitsunemimi::ErrorContainer &error,
                    const bool showHiddenValues);
    bool getDataSetByName(Kitsunemimi::Json::JsonItem &result,
                          const std::string &name,
                          const std::string &userUuid,
                          const std::string &projectUuid,
                          const bool isAdmin,
                          Kitsunemimi::ErrorContainer &error,
                          const bool showHiddenValues);
    bool getAllDataSet(Kitsunemimi::TableItem &result,
                       const std::string &userUuid,
                       const std::string &projectUuid,
                       const bool isAdmin,
                       Kitsunemimi::ErrorContainer &error);
    bool deleteDataSet(const std::string &uuid,
                       const std::string &userUuid,
                       const std::string &projectUuid,
                       const bool isAdmin,
                       Kitsunemimi::ErrorContainer &error);
};

#endif // SAGIRIARCHIVE_DATA_SET_TABLE_H
