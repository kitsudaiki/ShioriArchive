#include "add_audit_log.h"

using namespace Kitsunemimi;

AddAuditLog::AddAuditLog()
    : Kitsunemimi::Sakura::Blossom("Add new audit-entry.")
{

}
/**
 * @brief runTask
 */
bool
AddAuditLog::runTask(Sakura::BlossomLeaf &blossomLeaf,
                     const Kitsunemimi::DataMap &context,
                     Sakura::BlossomStatus &status,
                     ErrorContainer &error)
{
    return true;
}
