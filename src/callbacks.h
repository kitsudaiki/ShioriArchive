/**
 * @file        callbacks.h
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

#ifndef SAGIRIARCHIVE_CALLBACKS_H
#define SAGIRIARCHIVE_CALLBACKS_H

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <core/temp_file_handler.h>
#include <core/data_set_header.h>
#include <core/data_set_file.h>

#include <sagiri_root.h>

void streamDataCallback(void*,
                        Kitsunemimi::Sakura::Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    const uint8_t* ptr = static_cast<const uint8_t*>(data);

    const std::string id(reinterpret_cast<const char*>(ptr), 36);
    ptr += 36;

    const uint64_t pos = *reinterpret_cast<const uint64_t*>(ptr);
    ptr += 8;

    SagiriRoot::tempFileHandler->addDataToPos(id, pos, ptr, dataSize - 44);
}


void genericMessageCallback(Kitsunemimi::Sakura::Session* session,
                            const Kitsunemimi::Json::JsonItem &message,
                            const uint64_t blockerId)
{
    const std::string messageType = message.get("message_type").getString();

    if(messageType == "data_set_request")
    {
        // get location from message
        const std::string location = message.get("location").getString();
        if(location == "")
        {
            Kitsunemimi::ErrorContainer error;
            session->sendResponse("-", 1, blockerId, error);
            LOG_ERROR(error);
            return;
        }

        // init file
        DataSetFile file(location);
        if(file.readFromFile() == false) {
            // TODO: error
            return;
        }

        // get payload
        uint64_t payloadSize = 0;
        float* payload = file.getPayload(payloadSize);
        if(payload == nullptr) {
            // TODO: error
            return;
        }

        // send data
        Kitsunemimi::ErrorContainer error;
        if(session->sendResponse(payload, payloadSize, blockerId, error) == false) {
            LOG_ERROR(error);
        }

        return;
    }
    else if(messageType == "result_push")
    {
        bool success = false;
        Kitsunemimi::ErrorContainer error;

        const std::string uuid = message.get("uuid").getString();
        const std::string result = message.get("result").toString();

        // TODO: handle result
        const std::string resultLocation = GET_STRING_CONFIG("sagiri", "result_location", success);
        if(writeFile(resultLocation + "/" + uuid, result, error) == false)
        {
            LOG_ERROR(error);

            const std::string ret = "fail";
            session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
            return;
        }

        const std::string ret = "success";
        session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
        return;
    }
}

#endif // SAGIRIARCHIVE_CALLBACKS_H
