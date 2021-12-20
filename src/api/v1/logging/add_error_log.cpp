#include "add_error_log.h"

using namespace Kitsunemimi;

AddErrorLog::AddErrorLog()
    : Kitsunemimi::Sakura::Blossom("Add new error-entry.")
{

}

/**
 * @brief runTask
 */
bool
AddErrorLog::runTask(Sakura::BlossomLeaf &blossomLeaf,
                     const Kitsunemimi::DataMap &context,
                     Sakura::BlossomStatus &status,
                     ErrorContainer &error)
{
    return true;
}
