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

#include <core/temp_file_handler.h>
#include <core/data_set_files/data_set_file.h>
#include <database/data_set_table.h>
#include <database/cluster_snapshot_table.h>
#include <sagiri_root.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCrypto/common.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <libSagiriArchive/sagiri_messages.h>


void streamDataCallback(void*,
                        Kitsunemimi::Sakura::Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    if(dataSize <= 40) {
        return;
    }

    const uint8_t* ptr = static_cast<const uint8_t*>(data);

    if(ptr[36] != static_cast<uint8_t>(','))
    {
        const std::string datasetId(reinterpret_cast<const char*>(ptr), 36);
        ptr += 36;

        const std::string fileId(reinterpret_cast<const char*>(ptr), 36);
        ptr += 36;

        const uint64_t pos = *reinterpret_cast<const uint64_t*>(ptr);
        ptr += 8;

        SagiriRoot::tempFileHandler->addDataToPos(fileId, pos, ptr, dataSize - 80);

        // TODO: better and more reliable condition to detect finish
        if(dataSize - 80 < 96 * 1024)
        {
            Kitsunemimi::ErrorContainer error;
            SagiriRoot::dataSetTable->setUploadFinish(datasetId, fileId, error);
            SagiriRoot::clusterSnapshotTable->setUploadFinish(datasetId, fileId, error);
        }
    }
    else
    {
        char messageBuffer[1024*1024];
        memcpy(messageBuffer, ptr, dataSize);
        std::string message(messageBuffer, dataSize);

        std::vector<std::string> splitMessage;
        Kitsunemimi::splitStringByDelimiter(splitMessage, message, ',');

        if(splitMessage.size() != 4) {
            return;
        }

        if(Kitsunemimi::Crypto::base64UrlToBase64(splitMessage[3]) == false) {
            return;
        }

        Kitsunemimi::DataBuffer payload;
        if(Kitsunemimi::Crypto::decodeBase64(payload, splitMessage[3]) == false) {
            return;
        }

        SagiriRoot::tempFileHandler->addDataToPos(splitMessage.at(1),
                                                  std::stoi(splitMessage.at(2)),
                                                  payload.data,
                                                  payload.usedBufferSize);

        // TODO: better and more reliable condition to detect finish
        if(payload.usedBufferSize < 96 * 1024)
        {
            const std::string datasetId = splitMessage.at(0);
            const std::string fileId = splitMessage.at(1);
            Kitsunemimi::ErrorContainer error;
            if(SagiriRoot::dataSetTable->setUploadFinish(datasetId, fileId, error) == false)
            {
                LOG_ERROR(error);
                return;
            }
        }
    }
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

/**
 * @brief handle cluster-snapshot-message
 *
 * @param msg message to process
 * @param session pointer to the session, which received the message
 * @param blockerId blocker-id for the response
 */
inline void
handleClusterSnapshotRequest(const Sagiri::ClusterSnapshotPull_Message &msg,
                             Kitsunemimi::Sakura::Session* session,
                             const uint64_t blockerId)
{
    // init file
    Kitsunemimi::BinaryFile* targetFile = new Kitsunemimi::BinaryFile(msg.location);
    DataSetFile::DataSetHeader header;
    DataBuffer content;
    targetFile->readCompleteFile(content);

    // send data
    Kitsunemimi::ErrorContainer error;
    if(session->sendResponse(content.data, content.usedBufferSize, blockerId, error) == false) {
        LOG_ERROR(error);
    }

    return;
}

/**
 * @brief handle dataset-request-message
 *
 * @param msg message to process
 * @param session pointer to the session, which received the message
 * @param blockerId blocker-id for the response
 */
inline void
handleDataSetRequest(const Sagiri::DatasetRequest_Message &msg,
                     Kitsunemimi::Sakura::Session* session,
                     const uint64_t blockerId)
{
    // init file
    DataSetFile* file = readDataSetFile(msg.location);
    if(file == nullptr) {
        return;
    }

    // get payload
    uint64_t payloadSize = 0;
    float* payload = file->getPayload(payloadSize, msg.columnName);
    if(payload == nullptr) {
        // TODO: error
        delete file;
        delete payload;

        return;
    }

    // send data
    Kitsunemimi::ErrorContainer error;
    if(session->sendResponse(payload, payloadSize, blockerId, error) == false) {
        LOG_ERROR(error);
    }

    delete file;
    delete payload;

    return;
}

/**
 * @brief handle result-push-message
 *
 * @param msg message to process
 * @param session pointer to the session, which received the message
 * @param blockerId blocker-id for the response
 */
inline void
handleResultPush(const Sagiri::ResultPush_Message &msg,
                 Kitsunemimi::Sakura::Session* session,
                 const uint64_t blockerId)
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;

    // TODO: handle result
    const std::string resultLocation = GET_STRING_CONFIG("sagiri", "result_location", success);
    if(writeFile(resultLocation + "/" + msg.uuid, msg.results, error) == false)
    {
        LOG_ERROR(error);

        const std::string ret = "fail";
        session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
        return;
    }

    const std::string ret = "success";
    session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
}

/**
 * @brief handle error-log-message
 *
 * @param msg message to process
 */
inline void
handleErrorLog(const Kitsunemimi::Hanami::ErrorLog_Message &msg)
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;

    // TODO: handle result
    const std::string resultLocation = GET_STRING_CONFIG("sagiri", "error_location", success);
    std::string filePath = resultLocation + "/";
    if(msg.userUuid == "") {
        filePath += "generic";
    } else {
        filePath +=msg. userUuid;
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
    tableOutput.addRow(std::vector<std::string>{"component", msg.component});
    if(msg.context != "") {
        tableOutput.addRow(std::vector<std::string>{"context", msg.context});
    }
    if(msg.values != "") {
        tableOutput.addRow(std::vector<std::string>{"values", msg.values});
    }
    tableOutput.addRow(std::vector<std::string>{"error", msg.errorMsg});

    // write to file
    const std::string finalMessage = tableOutput.toString(200, true) + "\n\n\n";
    if(appendText(filePath, finalMessage, error) == false) {
        LOG_ERROR(error);
    }
}

/**
 * @brief handle audit-log-message
 *
 * @param msg message to process
 */
inline void
handleAuditLog(const Sagiri::AuditLog_Message &msg)
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;

    // TODO: handle result
    const std::string resultLocation = GET_STRING_CONFIG("sagiri", "audit_location", success);
    std::string filePath = resultLocation + "/";
    if(msg.userUuid == "") {
        filePath += "generic";
    } else {
        filePath += msg.userUuid;
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
    tableOutput.addRow(std::vector<std::string>{"component", msg.component});
    tableOutput.addRow(std::vector<std::string>{"endpoint", msg.endpoint});
    tableOutput.addRow(std::vector<std::string>{"type", msg.type});

    // write to file
    const std::string finalMessage = tableOutput.toString(200, true) + "\n\n\n";
    if(appendText(filePath, finalMessage, error) == false) {
        LOG_ERROR(error);
    }
}

/**
 * @brief handle errors of message which to requires a response
 *
 * @param msg error-message
 */
inline void
handleFail(const std::string &msg,
           Kitsunemimi::Sakura::Session* session,
           const uint64_t blockerId)
{
    Kitsunemimi::ErrorContainer error;
    error.addMeesage(msg);
    LOG_ERROR(error);

    const std::string ret = "-";
    session->sendResponse(ret.c_str(), ret.size(), blockerId, error);
    return;
}

/**
 * @brief handle generic message-content
 *
 * @param session pointer to the session, which received the message
 * @param data received bytes
 * @param dataSize number of bytes in the received message
 * @param blockerId blocker-id for the response
 */
void
genericMessageCallback(Kitsunemimi::Sakura::Session* session,
                       void* data,
                       const uint64_t dataSize,
                       const uint64_t blockerId)
{
    u_int8_t* u8Data = static_cast<uint8_t*>(data);

    switch(u8Data[0])
    {
        case Sagiri::CLUSTER_SNAPSHOT_PULL_MESSAGE_TYPE:
            {
                Sagiri::ClusterSnapshotPull_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken cluster-snapshot-message", session, blockerId);
                    return;
                }

                handleClusterSnapshotRequest(msg, session, blockerId);
            }
            break;
        case Sagiri::DATASET_REQUEST_MESSAGE_TYPE:
            {
                Sagiri::DatasetRequest_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken dataset-requests-message", session, blockerId);
                    return;
                }

                handleDataSetRequest(msg, session, blockerId);
            }
            break;
        case Sagiri::RESULT_PUSH_MESSAGE_TYPE:
            {
                Sagiri::ResultPush_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken result-push-message", session, blockerId);
                    return;
                }

                handleResultPush(msg, session, blockerId);
            }
            break;
        case Sagiri::AUDIT_LOG_MESSAGE_TYPE:
            {
                Sagiri::AuditLog_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    Kitsunemimi::ErrorContainer error;
                    error.addMeesage("Receive broken audit-log-message");
                    LOG_ERROR(error);
                    return;
                }

                handleAuditLog(msg);
            }
            break;
        case 255:
            {
                Kitsunemimi::Hanami::ErrorLog_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    Kitsunemimi::ErrorContainer error;
                    error.addMeesage("Receive broken error-log-message");
                    LOG_ERROR(error);
                    return;
                }

                handleErrorLog(msg);
            }
        break;
        default:
            handleFail("Received unknown generic message", session, blockerId);
            break;
    }
}

#endif // SAGIRIARCHIVE_CALLBACKS_H
