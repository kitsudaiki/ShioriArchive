/**
 * @file        main.cpp
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

#include <iostream>

#include <config.h>
#include <args.h>
#include <thread>
#include <callbacks.h>
#include <sagiri_root.h>

#include <libKitsunemimiHanamiCommon/generic_main.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libAzukiHeart/azuki_input.h>
#include <libMisakiGuard/misaki_input.h>

#include <libAzukiHeart/azuki_input.h>
#include <libMisakiGuard/misaki_input.h>

using Kitsunemimi::Hanami::HanamiMessaging;
using Kitsunemimi::Hanami::initMain;

int main(int argc, char *argv[])
{
    Kitsunemimi::ErrorContainer error;
    if(initMain(argc, argv, "sagiri", &registerArguments, &registerConfigs, error) == false)
    {
        LOG_ERROR(error);
        return 1;
    }

    // init included components
    Azuki::initAzukiBlossoms();
    Misaki::initMisakiBlossoms();

    // initialize server and connections based on the config-file
    const std::vector<std::string> groupNames = {"misaki"};
    if(HanamiMessaging::getInstance()->initialize("sagiri",
                                                  groupNames,
                                                  nullptr,
                                                  &streamDataCallback,
                                                  &genericMessageCallback,
                                                  error,
                                                  true) == false)
    {
        LOG_ERROR(error);
        return 1;
    }

    // init included components
    Azuki::initAzukiBlossoms();
    Misaki::initMisakiBlossoms();

    SagiriRoot rootObj;
    if(rootObj.init() == false) {
        return 1;
    }

    // sleep forever
    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());

    return 0;
}
