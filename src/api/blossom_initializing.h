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

#include <api/v1/data_files/list_data_set.h>
#include <api/v1/data_files/get_data_set.h>
#include <api/v1/data_files/delete_data_set.h>
#include <api/v1/data_files/check_data_set.h>
#include <api/v1/data_files/get_progress_data_set.h>

#include <api/v1/data_files/mnist/create_mnist_data_set.h>
#include <api/v1/data_files/mnist/finalize_mnist_data_set.h>

#include <api/v1/data_files/csv/create_csv_data_set.h>
#include <api/v1/data_files/csv/finalize_csv_data_set.h>

#include <api/v1/request_results/delete_request_result.h>
#include <api/v1/request_results/get_request_result.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief init data_set blossoms
 */
void
dataSetBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "data_set";

    assert(interface->addBlossom(group, "create_mnist", new CreateMnistDataSet()));
    endpoints->addEndpoint("v1/mnist/data_set",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_mnist");

    assert(interface->addBlossom(group, "finalize_mnist", new FinalizeMnistDataSet()));
    endpoints->addEndpoint("v1/mnist/data_set",
                           Kitsunemimi::Hanami::PUT_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "finalize_mnist");

    assert(interface->addBlossom(group, "create_csv", new CreateCsvDataSet()));
    endpoints->addEndpoint("v1/csv/data_set",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_csv");

    assert(interface->addBlossom(group, "finalize_csv", new FinalizeCsvDataSet()));
    endpoints->addEndpoint("v1/csv/data_set",
                           Kitsunemimi::Hanami::PUT_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "finalize_csv");

    assert(interface->addBlossom(group, "check", new CheckDataSet()));
    endpoints->addEndpoint("v1/data_set/check",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "check");

    assert(interface->addBlossom(group, "progress", new GetProgressDataSet()));
    endpoints->addEndpoint("v1/data_set/progress",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "progress");

    assert(interface->addBlossom(group, "get", new GetDataSet()));
    endpoints->addEndpoint("v1/data_set",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "get");

    assert(interface->addBlossom(group, "delete", new DeleteDataSet()));
    endpoints->addEndpoint("v1/data_set",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");

    assert(interface->addBlossom(group, "list", new ListDataSet()));
    endpoints->addEndpoint("v1/data_set/all",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "list");
}

/**
 * @brief init request_result blossoms
 */
void
resultBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "request_result";

    assert(interface->addBlossom(group, "get", new GetRequestResult()));
    endpoints->addEndpoint("v1/request_result",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "get");

    assert(interface->addBlossom(group, "delete", new DeleteRequestResult()));
    endpoints->addEndpoint("v1/request_result",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");
}

void
initBlossoms()
{
    dataSetBlossoms();
    resultBlossoms();
}

#endif // SAGIRIARCHIVE_BLOSSOM_INITIALIZING_H
