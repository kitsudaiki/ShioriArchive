/**
 * @file        users_database.h
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

#ifndef TRAIN_DATA_TABLE_H
#define TEST_FILES_TABLE_H

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiSakuraDatabase/sql_table.h>

namespace Kitsunemimi {
namespace Json {
class JsonItem;
}
}

class TrainDataTable
        : public Kitsunemimi::Sakura::SqlTable
{
public:
    struct TrainDataData
    {
        std::string uuid = "";
        std::string name = "";
        std::string project = "";
        std::string userUuid = "";
        std::string type = "";
        std::string location = "";
    };

    TrainDataTable(Kitsunemimi::Sakura::SqlDatabase* db);
    ~TrainDataTable();

    const std::string addTrainData(const TrainDataData &data,
                                   Kitsunemimi::ErrorContainer &error);
    bool getTrainData(Kitsunemimi::Json::JsonItem &result,
                      const std::string &uuid,
                      const std::string &userUuid,
                      Kitsunemimi::ErrorContainer &error);
    bool getAllTrainData(Kitsunemimi::TableItem &result,
                         Kitsunemimi::ErrorContainer &error);
    bool deleteTrainData(const std::string &uuid,
                         const std::string &userUuid,
                         Kitsunemimi::ErrorContainer &error);

private:
    void processGetResult(Kitsunemimi::Json::JsonItem &result,
                          Kitsunemimi::TableItem &tableContent);
};

#endif // TEST_FILES_TABLE_H
