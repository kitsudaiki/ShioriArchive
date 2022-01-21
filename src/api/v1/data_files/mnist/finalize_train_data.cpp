/**
 * @file        finalize_train_data.cpp
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

#include "finalize_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>

using namespace Kitsunemimi::Sakura;

FinalizeTrainData::FinalizeTrainData()
    : Kitsunemimi::Sakura::Blossom("Finalize uploaded train-data by checking completeness of the "
                                   "uploaded and convert into generic format.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
FinalizeTrainData::runTask(BlossomLeaf &blossomLeaf,
                           const Kitsunemimi::DataMap &context,
                           BlossomStatus &status,
                           Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::trainDataTable->getTrainData(result,
                                                uuid,
                                                userUuid,
                                                projectUuid,
                                                isAdmin,
                                                error,
                                                true) == false)
    {
        status.errorMessage = "Data with uuid '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // read input-data from temp-file
    Kitsunemimi::DataBuffer inputBuffer;
    const std::string inputTempFilePath = result.get("location").getString() + "_input_temp";
    Kitsunemimi::BinaryFile inputTempFile(inputTempFilePath, false);
    if(inputTempFile.readCompleteFile(inputBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to read input-data from file \"" + inputTempFilePath + "\"");
        return false;
    }
    inputTempFile.closeFile();

    // read label from temp-file
    Kitsunemimi::DataBuffer labelBuffer;
    const std::string labelTempFilePath = result.get("location").getString() + "_label_temp";
    Kitsunemimi::BinaryFile labelTempFile(labelTempFilePath, false);
    if(labelTempFile.writeCompleteFile(labelBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to read label-data from file \"" + labelTempFilePath + "\"");
        return false;
    }
    labelTempFile.closeFile();

    // write data to file
    Kitsunemimi::DataBuffer resultBuffer;
    if(convertMnistData(resultBuffer, inputBuffer, labelBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to convert mnist-data");
        return false;
    }

    // write converted data to real target
    const std::string targetFilePath = result.get("location").getString();
    Kitsunemimi::BinaryFile targetFile(targetFilePath, false);
    if(targetFile.writeCompleteFile(resultBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write train-data to file \"" + targetFilePath + "\"");
        return false;
    }
    targetFile.closeFile();

    // delete temp-files
    Kitsunemimi::deleteFileOrDir(inputTempFilePath, error);
    Kitsunemimi::deleteFileOrDir(labelTempFilePath, error);

    return true;
}

/**
 * @brief FinalizeTrainData::startMnistTask
 * @param inputBuffer
 * @param labelBuffer
 * @return
 */
bool
FinalizeTrainData::convertMnistData(Kitsunemimi::DataBuffer &resultBuffer,
                                    const Kitsunemimi::DataBuffer &inputBuffer,
                                    const Kitsunemimi::DataBuffer &labelBuffer)
{
    const uint64_t dataOffset = 16;
    const uint64_t labelOffset = 8;

    const uint8_t* dataBufferPtr = static_cast<uint8_t*>(inputBuffer.data);
    const uint8_t* labelBufferPtr = static_cast<uint8_t*>(labelBuffer.data);

    // get number of images
    uint32_t numberOfImages = 0;
    numberOfImages |= dataBufferPtr[7];
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[6]) << 8;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[5]) << 16;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[4]) << 24;

    // get number of rows
    uint32_t numberOfRows = 0;
    numberOfRows |= dataBufferPtr[11];
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[10]) << 8;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[9]) << 16;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[8]) << 24;

    // get number of columns
    uint32_t numberOfColumns = 0;
    numberOfColumns |= dataBufferPtr[15];
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[14]) << 8;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[13]) << 16;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[12]) << 24;

    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;
    const uint64_t retSize = (numberOfImages * pictureSize + 10) * 4;
    Kitsunemimi::allocateBlocks_DataBuffer(resultBuffer, Kitsunemimi::calcBytesToBlocks(retSize));
    float* resultPtr = static_cast<float*>(resultBuffer.data);
    uint64_t resultPos = 0;
    uint64_t dataPos = 0;
    uint64_t labelPos = 0;

    // check to avoid clang-warnings
    if(resultBuffer.data == nullptr) {
        return false;
    }

    for(uint32_t pic = 0; pic < numberOfImages; pic++)
    {
        // input
        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + dataOffset;
            resultPtr[resultPos] = static_cast<float>(dataBufferPtr[pos]);
            dataPos++;
            resultPos++;
        }

        // output
        for(uint32_t i = 0; i < 10; i++)
        {
            resultPtr[resultPos] = 0.0f;
            labelPos++;
            resultPos++;
        }
        const uint32_t label = labelBufferPtr[pic + labelOffset];
        resultPtr[(resultPos - 10) + label] = 1.0f;
    }

    return true;
}

