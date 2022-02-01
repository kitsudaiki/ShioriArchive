/**
 * @file        finalize_data_set.cpp
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

#include "finalize_data_set.h"

#include <sagiri_root.h>
#include <database/data_set_table.h>
#include <core/temp_file_handler.h>

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

FinalizeMnistDataSet::FinalizeMnistDataSet()
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
    registerInputField("uuid_input_file",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identify the file for date upload of input-data.");
    assert(addFieldRegex("uuid_input_file", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                            "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));
    registerInputField("uuid_label_file",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identify the file for date upload of label-data.");
    assert(addFieldRegex("uuid_label_file", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                            "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
FinalizeMnistDataSet::runTask(BlossomLeaf &blossomLeaf,
                              const Kitsunemimi::DataMap &context,
                              BlossomStatus &status,
                              Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    const std::string inputUuid = blossomLeaf.input.get("uuid_input_file").getString();
    const std::string labelUuid = blossomLeaf.input.get("uuid_label_file").getString();

    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get location from database
    Kitsunemimi::Json::JsonItem result;
    if(SagiriRoot::dataSetTable->getDataSet(result,
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
    if(SagiriRoot::tempFileHandler->getData(inputBuffer, inputUuid) == false)
    {
        status.errorMessage = "Input-data with uuid '" + inputUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // read label from temp-file
    Kitsunemimi::DataBuffer labelBuffer;
    if(SagiriRoot::tempFileHandler->getData(labelBuffer, labelUuid) == false)
    {
        status.errorMessage = "Label-data with uuid '" + inputUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // write data to file
    Kitsunemimi::DataBuffer resBuffer;
    ImageTypeHeader imageHeader;
    if(convertMnistData(imageHeader, resBuffer, inputBuffer, labelBuffer) == false)
    {
        status.statusCode =Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to convert mnist-data");
        return false;
    }

    // write converted data to real target
    const std::string targetFilePath = result.get("location").getString();
    Kitsunemimi::BinaryFile targetFile(targetFilePath, false);

    // allocate storage
    const uint64_t dataPos = sizeof(DataSetHeader) + sizeof(ImageTypeHeader);
    if(targetFile.allocateStorage(resBuffer.usedBufferSize + dataPos, 1) == false)
    {
        status.statusCode = Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Allcating storage for file '" + targetFilePath + "\' failed");
        return false;
    }

    // write dataset-hader
    DataSetHeader dataSetHeader;
    dataSetHeader.type = DataSetType::IMAGE_TYPE;
    std::strncpy(dataSetHeader.name, result.get("name").getString().c_str(), 256);
    if(targetFile.writeDataIntoFile(&dataSetHeader, 0, sizeof(DataSetHeader)) == false)
    {
        status.statusCode = Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write data-set-header to file '" + targetFilePath + "'");
        return false;
    }

    // write picture-header
    if(targetFile.writeDataIntoFile(&imageHeader,
                                    sizeof(DataSetHeader),
                                    sizeof(ImageTypeHeader)) == false)
    {
        status.statusCode = Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write data-set-header to file '" + targetFilePath + "'");
        return false;
    }

    // write payload to file
    if(targetFile.writeDataIntoFile(resBuffer.data, dataPos, resBuffer.usedBufferSize) == false)
    {
        status.statusCode = Kitsunemimi:: Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to write payload to file '" + targetFilePath + "'");
        return false;
    }
    targetFile.closeFile();

    // delete temp-files
    SagiriRoot::tempFileHandler->removeData(inputUuid);
    SagiriRoot::tempFileHandler->removeData(labelUuid);

    // create output
    blossomLeaf.output.insert("uuid", uuid);

    return true;
}

/**
 * @brief convert mnist-data into generic format
 *
 * @param header reference for header with meta-information
 * @param resultBuffer buffer for the resulting file, which should be written back to disc
 * @param inputBuffer buffer with input-data
 * @param labelBuffer buffer with label-data
 *
 * @return true, if successfull, else false
 */
bool
FinalizeMnistDataSet::convertMnistData(ImageTypeHeader &header,
                                       Kitsunemimi::DataBuffer &resultBuffer,
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

    // set information in header
    header.numberOfInputsX = numberOfColumns;
    header.numberOfInputsY = numberOfRows;
    header.numberOfImages = numberOfImages;

    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;
    const uint64_t retSize = (numberOfImages * (pictureSize + 10)) * 4;
    Kitsunemimi::allocateBlocks_DataBuffer(resultBuffer, Kitsunemimi::calcBytesToBlocks(retSize));
    resultBuffer.usedBufferSize = retSize;
    float* resultPtr = static_cast<float*>(resultBuffer.data);
    uint64_t resultPos = 0;
    uint64_t dataPos = 0;
    uint64_t labelPos = 0;

    // check to avoid clang-warnings
    if(resultBuffer.data == nullptr) {
        return false;
    }

    // copy values of each pixel into the resulting file
    for(uint32_t pic = 0; pic < numberOfImages; pic++)
    {
        // input
        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + dataOffset;
            resultPtr[resultPos] = (static_cast<float>(dataBufferPtr[pos]) / 255.0f);
            dataPos++;
            resultPos++;
        }

        // label
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

