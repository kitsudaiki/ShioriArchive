/**
 * @file        blossom_initializing.h
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

#ifndef BLOSSOM_INITIALIZING_H
#define BLOSSOM_INITIALIZING_H

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiCommon/logger.h>

#include <api/data_files/get_train_data.h>
#include <api/data_files/add_train_data.h>
#include <api/data_files/delete_train_data.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief init special blossoms
 */
void
trainDataBlossomes()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "special";

    assert(interface->addBlossom(group, "add_train_data", new AddTrainData()));
    assert(interface->addBlossom(group, "get_train_data", new GetTrainData()));
    assert(interface->addBlossom(group, "delete_train_data", new DeleteTrainData()));
}

void
initBlossoms()
{
    trainDataBlossomes();
}

#endif // BLOSSOM_INITIALIZING_H
