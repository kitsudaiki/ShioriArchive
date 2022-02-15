/**
 * @file        image_data_set_file.cpp
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

#include "image_data_set_file.h"

#include <libKitsunemimiCommon/common_methods/file_methods.h>

/**
 * @brief constructor
 *
 * @param filePath path to file
 */
ImageDataSetFile::ImageDataSetFile(const std::string &filePath)
    : DataSetFile(filePath) {}

/**
 * @brief constructor
 *
 * @param file pointer to binary-file object
 */
ImageDataSetFile::ImageDataSetFile(Kitsunemimi::BinaryFile* file)
    : DataSetFile(file) {}

/**
 * @brief destructor
 */
ImageDataSetFile::~ImageDataSetFile() {}

/**
 * @brief init header-sizes
 */
void
ImageDataSetFile::initHeader()
{
    m_headerSize = sizeof(DataSetHeader) + sizeof(ImageTypeHeader);

    const uint64_t lineSize = (imageHeader.numberOfInputsX * imageHeader.numberOfInputsY)
                              + imageHeader.numberOfOutputs;
    m_totalFileSize = m_headerSize + (lineSize * imageHeader.numberOfImages * sizeof(float));
}

/**
 * @brief read header from buffer
 *
 * @param u8buffer buffer to read
 */
void
ImageDataSetFile::readHeader(const uint8_t* u8buffer)
{
    // read image-header
    m_headerSize = sizeof(DataSetHeader) + sizeof(ImageTypeHeader);
    memcpy(&imageHeader, &u8buffer[sizeof(DataSetHeader)], sizeof(ImageTypeHeader));

    // get sizes
    const uint64_t lineSize = (imageHeader.numberOfInputsX * imageHeader.numberOfInputsY)
                              + imageHeader.numberOfOutputs;
    m_totalFileSize = m_headerSize + (lineSize * imageHeader.numberOfImages * sizeof(float));
}

/**
 * @brief update header in file
 *
 * @return true, if successful, else false
 */
bool
ImageDataSetFile::updateHeader()
{
    // write image-header to file
    if(m_targetFile->writeDataIntoFile(&imageHeader,
                                       sizeof(DataSetHeader),
                                       sizeof(ImageTypeHeader)) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief split data-set at a specific point
 *
 * @param newFilePath path of the file with the second part
 * @param ratio ratio-value where to split the file
 *
 * @return true, if successful, else false
 */
bool
ImageDataSetFile::split(const std::string &newFilePath, const float ratio)
{
    // calculate number of images for each part
    const uint64_t numberImagesP1 = ratio * imageHeader.numberOfImages;
    const uint64_t numberImagesP2 = imageHeader.numberOfImages - numberImagesP1;

    // calculate number of values for each part
    const uint64_t lineSize = (imageHeader.numberOfInputsX * imageHeader.numberOfInputsY)
                              + imageHeader.numberOfOutputs;
    const uint64_t numberValuesP1 = numberImagesP1 * lineSize;
    const uint64_t numberValuesP2 = numberImagesP2 * lineSize;

    // create new file
    ImageDataSetFile p2(newFilePath);
    p2.type = DataSetFile::IMAGE_TYPE;
    p2.name = name;
    p2.imageHeader = imageHeader;
    p2.imageHeader.numberOfImages = numberImagesP2;
    if(p2.initNewFile() == false) {
        return false;
    }

    // init buffer
    float* bufferP1 = new float[numberValuesP1];
    float* bufferP2 = new float[numberValuesP2];

    // calculate number of bytes for each part
    const float numberBytesP1 = numberValuesP1 * sizeof(float);
    const float numberBytesP2 = numberValuesP2 * sizeof(float);

    // read data
    m_targetFile->readDataFromFile(bufferP1, m_headerSize,                 numberBytesP1);
    m_targetFile->readDataFromFile(bufferP2, m_headerSize + numberBytesP1, numberBytesP2);

    m_targetFile->closeFile();
    const std::string filePathP1 = m_targetFile->m_filePath;

    bool ret = false;
    do {
        // remove old file
        Kitsunemimi::ErrorContainer error;
        if(Kitsunemimi::deleteFileOrDir(filePathP1, error) == false) {
            break;
        }
        delete m_targetFile;

        // reinit file with correct size
        m_targetFile = new Kitsunemimi::BinaryFile(filePathP1);
        imageHeader.numberOfImages = numberImagesP1;
        if(initNewFile() == false) {
            break;
        }

        // write data to part1
        if(addBlock(0, bufferP1, numberValuesP1) == false) {
            break;
        }

        // write data to part 2
        if(p2.addBlock(numberValuesP1, bufferP2, numberValuesP2) == false) {
            break;
        }

        ret = true;
    }
    while(false);

    // free memory
    delete[] bufferP1;
    delete[] bufferP2;

    return ret;
}
