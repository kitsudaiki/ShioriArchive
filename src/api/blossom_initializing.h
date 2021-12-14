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

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <api/data_files/list_train_data.h>
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
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "train_data";

    assert(interface->addBlossom(group, "add", new AddTrainData()));
    assert(endpoints->addEndpoint("train_data",
                                  Kitsunemimi::Hanami::POST_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "add"));

    assert(interface->addBlossom(group, "get", new GetTrainData()));
    assert(endpoints->addEndpoint("train_data",
                                  Kitsunemimi::Hanami::GET_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "get"));

    assert(interface->addBlossom(group, "delete", new DeleteTrainData()));
    assert(endpoints->addEndpoint("train_data",
                                  Kitsunemimi::Hanami::DELETE_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "delete"));

    assert(interface->addBlossom(group, "list", new ListTrainData()));
    assert(endpoints->addEndpoint("train_datas",
                                  Kitsunemimi::Hanami::GET_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "list"));

}

void
initBlossoms()
{
    trainDataBlossomes();
}

#endif // BLOSSOM_INITIALIZING_H
