#include "create_train_data.h"

#include <sagiri_root.h>
#include <database/train_data_table.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/files/binary_file.h>

using namespace Kitsunemimi::Sakura;

CreateTrainData::CreateTrainData()
    : Kitsunemimi::Sakura::Blossom("Init new set of train-data.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the new set.");
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("data_size",
                       SAKURA_INT_TYPE,
                       true,
                       "Total size of the data-set.");
    assert(addFieldBorder("data_size", 1, 10000000000));

    registerInputField("type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the new set (options: csv or mnist)");
    assert(addFieldRegex("type", "(csv|mnist)"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new set.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new set.");
    registerOutputField("user_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the user who uploaded the data.");
    registerOutputField("type",
                        SAKURA_STRING_TYPE,
                        "Type of the new set (For example: CSV)");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

bool
CreateTrainData::runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                         const Kitsunemimi::DataMap &context,
                         Kitsunemimi::Sakura::BlossomStatus &status,
                         Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const std::string type = blossomLeaf.input.get("type").getString();
    const long dataSize = blossomLeaf.input.get("data_size").getLong();
    const std::string userUuid = context.getStringByKey("uuid");

    // get directory to store data from config
    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "train_data_location", success);
    if(success == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("file-location to store train-data is missing in the config");
        return false;
    }

    // build absolut file-path to store the file
    if(targetFilePath.at(targetFilePath.size() - 1) != '/') {
        targetFilePath.append("/");
    }
    targetFilePath.append(name + "_" + type + "_" + userUuid);

    // init empty file with the size for the complete data-set
    Kitsunemimi::BinaryFile targetFile(targetFilePath, false);
    targetFile.allocateStorage(dataSize, 1);
    targetFile.closeFile();

    // register in database
    blossomLeaf.output.insert("name", name);
    blossomLeaf.output.insert("user_uuid", userUuid);
    blossomLeaf.output.insert("type", type);
    blossomLeaf.output.insert("location", targetFilePath);

    blossomLeaf.output.insert("project_uuid", "-");
    blossomLeaf.output.insert("owner_uuid", "-");
    blossomLeaf.output.insert("visibility", 0);

    if(SagiriRoot::trainDataTable->addTrainData(blossomLeaf.output, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    blossomLeaf.output.remove("location");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
