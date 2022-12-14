/**
 * @file        finalize_csv_data_set.h
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

#ifndef SHIORIARCHIVE_CSV_FINALIZE_DATA_SET_H
#define SHIORIARCHIVE_CSV_FINALIZE_DATA_SET_H

#include <libKitsunemimiHanamiNetwork/blossom.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

class FinalizeCsvDataSet
        : public Kitsunemimi::Hanami::Blossom
{
public:
    FinalizeCsvDataSet();

protected:
    bool runTask(Kitsunemimi::Hanami::BlossomIO &blossomIO,
                 const Kitsunemimi::DataMap &context,
                 Kitsunemimi::Hanami::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);

private:
    bool convertCsvData(const std::string &filePath,
                        const std::string &name,
                        const Kitsunemimi::DataBuffer &inputBuffer);
    void convertField(float* segmentPos,
                      const std::string &cell,
                      const float lastVal);
};

#endif // SHIORIARCHIVE_CSV_FINALIZE_DATA_SET_H
