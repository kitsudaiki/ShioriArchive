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
#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCrypto/common.h>

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

/**
 * @brief get the current datetime of the system
 *
 * @return datetime as string
 */
const std::string
getDatetime()
{
    const time_t now = time(nullptr);
    tm *ltm = localtime(&now);

    const std::string datatime =
            std::to_string(1900 + ltm->tm_year)
            + "-"
            + std::to_string(1 + ltm->tm_mon)
            + "-"
            + std::to_string(ltm->tm_mday)
            + " "
            + std::to_string(ltm->tm_hour)
            + ":"
            + std::to_string(ltm->tm_min)
            + ":"
            + std::to_string(ltm->tm_sec);

    return datatime;
}

void
genericMessageCallback(Kitsunemimi::Sakura::Session* session,
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
    else if(messageType == "audit_log")
    {
        bool success = false;
        Kitsunemimi::ErrorContainer error;

        const std::string userUuid = message.get("user_uuid").getString();
        const std::string component = message.get("component").getString();
        const std::string endpoint = message.get("endpoint").getString();
        const std::string type = message.get("type").getString();

        // TODO: handle result
        const std::string resultLocation = GET_STRING_CONFIG("sagiri", "audit_location", success);
        std::string filePath = resultLocation + "/";
        if(userUuid == "") {
            filePath += "generic";
        } else {
            filePath += userUuid;
        }

        // create an empty file, if no exist
        if(std::filesystem::exists(filePath) == false)
        {
            // create new file and write content
            std::ofstream outputFile;
            outputFile.open(filePath);
            outputFile.close();
        }

        // init table
        Kitsunemimi::TableItem tableOutput;
        tableOutput.addColumn("key");
        tableOutput.addColumn("value");

        // fill table
        tableOutput.addRow(std::vector<std::string>{"timestamp", getDatetime()});
        tableOutput.addRow(std::vector<std::string>{"component", component});
        tableOutput.addRow(std::vector<std::string>{"endpoint", endpoint});
        tableOutput.addRow(std::vector<std::string>{"type", type});

        // write to file
        const std::string finalMessage = tableOutput.toString(200, true) + "\n\n\n";
        if(appendText(filePath, finalMessage, error) == false) {
            LOG_ERROR(error);
        }

        return;
    }
    else if(messageType == "error_log")
    {
        bool success = false;
        Kitsunemimi::ErrorContainer error;

        const std::string userUuid = message.get("user_uuid").getString();
        const std::string component = message.get("component").getString();
        const std::string base64Error = message.get("message").getString();
        const std::string context = message.get("context").toString(true);
        const std::string values = message.get("values").toString(true);

        // decode message
        std::string errorMessage;
        if(Kitsunemimi::Crypto::decodeBase64(errorMessage, base64Error) == false)
        {
            error.addMeesage("failed to decode error-message");
            LOG_ERROR(error);
            return;
        }

        // TODO: handle result
        const std::string resultLocation = GET_STRING_CONFIG("sagiri", "error_location", success);
        std::string filePath = resultLocation + "/";
        if(userUuid == "") {
            filePath += "generic";
        } else {
            filePath += userUuid;
        }

        // create an empty file, if no exist
        if(std::filesystem::exists(filePath) == false)
        {
            // create new file and write content
            std::ofstream outputFile;
            outputFile.open(filePath);
            outputFile.close();
        }

        // init table
        Kitsunemimi::TableItem tableOutput;
        tableOutput.addColumn("key");
        tableOutput.addColumn("value");

        // fill table
        tableOutput.addRow(std::vector<std::string>{"timestamp", getDatetime()});
        tableOutput.addRow(std::vector<std::string>{"component", component});
        if(context != "") {
            tableOutput.addRow(std::vector<std::string>{"context", context});
        }
        if(values != "") {
            tableOutput.addRow(std::vector<std::string>{"values", values});
        }
        tableOutput.addRow(std::vector<std::string>{"error", errorMessage});

        // write to file
        const std::string finalMessage = tableOutput.toString(200, true) + "\n\n\n";
        if(appendText(filePath, finalMessage, error) == false) {
            LOG_ERROR(error);
        }

        return;
    }
    else
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("unknown message-type '" + messageType + "'");
        LOG_ERROR(error);

        const std::string ret = "-";
        session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
        return;
    }
}

#endif // SAGIRIARCHIVE_CALLBACKS_H
