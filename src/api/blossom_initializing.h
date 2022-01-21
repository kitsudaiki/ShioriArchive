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

#ifndef SAGIRIARCHIVE_BLOSSOM_INITIALIZING_H
#define SAGIRIARCHIVE_BLOSSOM_INITIALIZING_H

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiCommon/logger.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <api/v1/data_files/list_train_data.h>
#include <api/v1/data_files/get_train_data.h>
#include <api/v1/data_files/mnist/add_train_data.h>
#include <api/v1/data_files/mnist/create_train_data.h>
#include <api/v1/data_files/mnist/finalize_train_data.h>
#include <api/v1/data_files/delete_train_data.h>

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

    assert(interface->addBlossom(group, "create", new CreateTrainData()));
    endpoints->addEndpoint("v1/train_data",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create");

    assert(interface->addBlossom(group, "add", new AddTrainData()));
    endpoints->addEndpoint("v1/train_data",
                           Kitsunemimi::Hanami::PUT_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "add");

    assert(interface->addBlossom(group, "finalize", new FinalizeTrainData()));
    endpoints->addEndpoint("v1/train_data",
                           Kitsunemimi::Hanami::PUT_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "finalize");

    assert(interface->addBlossom(group, "get", new GetTrainData()));
    endpoints->addEndpoint("v1/train_data",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "get");

    assert(interface->addBlossom(group, "delete", new DeleteTrainData()));
    endpoints->addEndpoint("v1/train_data",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");

    assert(interface->addBlossom(group, "list", new ListTrainData()));
    endpoints->addEndpoint("v1/train_data/all",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "list");

}

void
initBlossoms()
{
    trainDataBlossomes();
}

#endif // SAGIRIARCHIVE_BLOSSOM_INITIALIZING_H
