/**
 * @file        split_data_set.cpp
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

#include "split_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/data_set_files/data_set_file.h>

#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>

using namespace Kitsunemimi;

SplitDataSet::SplitDataSet()
    : Kitsunemimi::Sakura::Blossom("Split a data-set into two files.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       Sakura::SAKURA_STRING_TYPE,
                       true,
                       "UUID of the data-set to split.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("ratio",
                       Sakura::SAKURA_FLOAT_TYPE,
                       true,
                       "Ratio to identify where to split to.");
    assert(addFieldBorder("ratio", 0, 1));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("correctness",
                        Sakura::SAKURA_FLOAT_TYPE,
                        "Correctness of the values compared to the data-set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
SplitDataSet::runTask(Sakura::BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      Sakura::BlossomStatus &status,
                      ErrorContainer &error)
{

}
