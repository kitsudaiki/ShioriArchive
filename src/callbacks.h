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

#ifndef SHIORIARCHIVE_CALLBACKS_H
#define SHIORIARCHIVE_CALLBACKS_H

#include <core/temp_file_handler.h>
#include <core/data_set_files/data_set_file.h>
#include <database/data_set_table.h>
#include <database/cluster_snapshot_table.h>
#include <database/request_result_table.h>
#include <shiori_root.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiCommon/items/table_item.h>
#include <libKitsunemimiCommon/methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCrypto/common.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <libShioriArchive/shiori_messages.h>

#include <../libKitsunemimiHanamiMessages/protobuffers/shiori_messages.proto3.pb.h>
#include <../libKitsunemimiHanamiMessages/hanami_messages/shiori_messages.h>

/**
 * @brief handleProtobufFileUpload
 * @param data
 * @param dataSize
 * @return
 */
bool
handleProtobufFileUpload(const void* data,
                         const uint64_t dataSize)
{
    FileUpload_Message msg;
    if(msg.ParseFromArray(data, dataSize) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Got invalid FileUpload-Message");
        LOG_ERROR(error);
        return false;
    }

    if(ShioriRoot::tempFileHandler->addDataToPos(msg.fileuuid(),
                                                 msg.position(),
                                                 msg.data().c_str(),
                                                 msg.data().size()) == false)
    {
        // TODO: error-handling
        std::cout<<"failed to write data"<<std::endl;
        return false;
    }

    if(msg.islast() == false) {
        return false;
    }

    Kitsunemimi::ErrorContainer error;

    if(msg.type() == UploadDataType::DATASET_TYPE)
    {
        if(ShioriRoot::dataSetTable->setUploadFinish(msg.datasetuuid(),
                                                     msg.fileuuid(),
                                                     error) == false)
        {
            // TODO: error-handling
            return false;
        }
    }

    if(msg.type() == UploadDataType::CLUSTER_SNAPSHOT_TYPE)
    {
        if(ShioriRoot::clusterSnapshotTable->setUploadFinish(msg.datasetuuid(),
                                                             msg.fileuuid(),
                                                             error) == false)
        {
            // TODO: error-handling
            return false;
        }
    }

    return true;
}

/**
 * @brief handleHanamiFileUpload
 * @param data
 * @param dataSize
 * @return
 */
bool
handleHanamiFileUpload(const void* data,
                       const uint64_t dataSize)
{
    Kitsunemimi::Hanami::FileUpload_Message msg;
    if(msg.read(const_cast<void*>(data), dataSize) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Got invalid FileUpload-Message");
        LOG_ERROR(error);
        return false;
    }

    if(ShioriRoot::tempFileHandler->addDataToPos(msg.fileUuid,
                                                 msg.position,
                                                 msg.payload,
                                                 msg.numberOfBytes) == false)
    {
        // TODO: error-handling
        return false;
    }

    if(msg.isLast == false) {
        return false;
    }

    Kitsunemimi::ErrorContainer error;

    if(msg.type == UploadDataType::DATASET_TYPE)
    {
        if(ShioriRoot::dataSetTable->setUploadFinish(msg.datasetUuid,
                                                     msg.fileUuid,
                                                     error) == false)
        {
            // TODO: error-handling
            return false;
        }
    }

    if(msg.type == UploadDataType::CLUSTER_SNAPSHOT_TYPE)
    {
        if(ShioriRoot::clusterSnapshotTable->setUploadFinish(msg.datasetUuid,
                                                             msg.fileUuid,
                                                             error) == false)
        {
            // TODO: error-handling
            return false;
        }
    }

    return true;
}

/**
 * @brief streamDataCallback
 * @param data
 * @param dataSize
 */
void streamDataCallback(void*,
                        Kitsunemimi::Sakura::Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    if(dataSize <= 40) {
        return;
    }

    if(Kitsunemimi::Hanami::isHanamiProtocol(data, dataSize)) {
        handleHanamiFileUpload(data, dataSize);
    } else {
        handleProtobufFileUpload(data, dataSize);
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
handleClusterSnapshotRequest(const Shiori::ClusterSnapshotPull_Message &msg,
                             Kitsunemimi::Sakura::Session* session,
                             const uint64_t blockerId)
{
    Kitsunemimi::ErrorContainer error;

    // init file
    Kitsunemimi::BinaryFile* targetFile = new Kitsunemimi::BinaryFile(msg.location);
    DataSetFile::DataSetHeader header;
    DataBuffer content;
    if(targetFile->readCompleteFile(content, error) == false)
    {
        //TODO: handle error
        LOG_ERROR(error);
    }

    // send data
    if(session->sendResponse(content.data,
                             content.usedBufferSize,
                             blockerId,
                             error) == false)
    {
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
handleDataSetRequest(const Shiori::DatasetRequest_Message &msg,
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
    if(payload == nullptr)
    {
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
handleResultPush(const Shiori::ResultPush_Message &msg,
                 Kitsunemimi::Sakura::Session* session,
                 const uint64_t blockerId)
{
    Kitsunemimi::ErrorContainer error;

    Kitsunemimi::Json::JsonItem dataParser;
    if(dataParser.parse(msg.results, error) == false)
    {
        error.addMeesage("Error while receivind result-data");
        LOG_ERROR(error);
        return;
    }

    Kitsunemimi::Json::JsonItem resultData;
    resultData.insert("uuid", msg.uuid);
    resultData.insert("name", msg.name);
    resultData.insert("data", dataParser.stealItemContent());
    resultData.insert("visibility", "private");

    Kitsunemimi::Hanami::UserContext userContext;
    userContext.userId = msg.userId;
    userContext.projectId = msg.projectId;

    if(ShioriRoot::requestResultTable->addRequestResult(resultData, userContext, error) == false)
    {
        LOG_ERROR(error);

        const std::string ret = "fail";
        if(session->sendResponse(ret.c_str(), ret.size(), blockerId, error) == false) {
            LOG_ERROR(error);
        }
        return;
    }

    const std::string ret = "success";
    if(session->sendResponse(ret.c_str(), ret.size(), blockerId, error) == false) {
        LOG_ERROR(error);
    }
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
    const std::string resultLocation = GET_STRING_CONFIG("shiori", "error_location", success);
    std::string filePath = resultLocation + "/";
    if(msg.userId == "") {
        filePath += "generic";
    } else {
        filePath += msg. userId;
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
handleAuditLog(const Shiori::AuditLog_Message &msg)
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;

    // TODO: handle result
    const std::string resultLocation = GET_STRING_CONFIG("shiori", "audit_location", success);
    std::string filePath = resultLocation + "/";
    if(msg.userId == "") {
        filePath += "generic";
    } else {
        filePath += msg.userId;
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

    switch(u8Data[6])
    {
        case Shiori::CLUSTER_SNAPSHOT_PULL_MESSAGE_TYPE:
            {
                Shiori::ClusterSnapshotPull_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken cluster-snapshot-message", session, blockerId);
                    return;
                }

                handleClusterSnapshotRequest(msg, session, blockerId);
            }
            break;
        case Shiori::DATASET_REQUEST_MESSAGE_TYPE:
            {
                Shiori::DatasetRequest_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken dataset-requests-message", session, blockerId);
                    return;
                }

                handleDataSetRequest(msg, session, blockerId);
            }
            break;
        case Shiori::RESULT_PUSH_MESSAGE_TYPE:
            {
                Shiori::ResultPush_Message msg;
                if(msg.read(data, dataSize) == false)
                {
                    handleFail("Receive broken result-push-message", session, blockerId);
                    return;
                }

                handleResultPush(msg, session, blockerId);
            }
            break;
        case Shiori::AUDIT_LOG_MESSAGE_TYPE:
            {
                Shiori::AuditLog_Message msg;
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

#endif // SHIORIARCHIVE_CALLBACKS_H
