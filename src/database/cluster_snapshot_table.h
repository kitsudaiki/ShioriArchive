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

#ifndef SAGIRIARCHIVE_CLUSTER_SNAPSHOT_TABLE_H
#define SAGIRIARCHIVE_CLUSTER_SNAPSHOT_TABLE_H

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiHanamiDatabase/hanami_sql_table.h>

namespace Kitsunemimi {
namespace Json {
class JsonItem;
}
}

class ClusterSnapshotTable
        : public Kitsunemimi::Hanami::HanamiSqlTable
{
public:
    ClusterSnapshotTable(Kitsunemimi::Sakura::SqlDatabase* db);
    ~ClusterSnapshotTable();

    bool addClusterSnapshot(Kitsunemimi::Json::JsonItem &data,
                            const Kitsunemimi::Hanami::UserContext &userContext,
                            Kitsunemimi::ErrorContainer &error);
    bool getClusterSnapshot(Kitsunemimi::Json::JsonItem &result,
                            const std::string &snapshotUuid,
                            const Kitsunemimi::Hanami::UserContext &userContext,
                            Kitsunemimi::ErrorContainer &error,
                            const bool showHiddenValues);
    bool getAllClusterSnapshot(Kitsunemimi::TableItem &result,
                               const Kitsunemimi::Hanami::UserContext &userContext,
                               Kitsunemimi::ErrorContainer &error);
    bool deleteClusterSnapshot(const std::string &snapshotUuid,
                               const Kitsunemimi::Hanami::UserContext &userContext,
                               Kitsunemimi::ErrorContainer &error);
    bool setUploadFinish(const std::string &uuid,
                         const std::string &fileUuid,
                         Kitsunemimi::ErrorContainer &error);
};

#endif // SAGIRIARCHIVE_CLUSTER_SNAPSHOT_TABLE_H
