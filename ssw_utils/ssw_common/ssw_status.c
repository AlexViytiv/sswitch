#include "ssw_status.h"

char * status_code_str[] = {
    "OK",                               /**< Operation has been done successfully */
    "UNKNOWN_ERROR",                    /**< Operation has failed for unknown reason */
    "FLOW_UNSUPPORTED",                 /**< Operation has failed because flow is unsupported */
    "FLOW_DEPRECATED",                  /**< Operation has failed because flow is deprecated */
    "MODULE_UNINITIALIZED",             /**< Operation has failed because module is uninitialized */
    "MODULE_ALREADY_INITIALIZED",       /**< Operation has failed because module is already initialized */
    "PARAM_NULL",                       /**< Operation has failed because parameter is NULL pointer */
    "PARAM_VALUE_INVALID",              /**< Operation has failed because parameter value is invalid */
    "PARAM_EXCEEDS_RANGE",              /**< Operation has failed because parameter value exceeds valid range */
    "OBJECT_EXIST",                     /**< Operation has failed because object exist */
    "OBJECT_NOT_EXIST",                 /**< Operation has failed because object not exist */
    "OBJECT_EQUAL",                     /**< Operation has failed because object is equal to another by some property */
    "OBJECT_NOT_EQUAL",                 /**< Operation has failed because object is not equal to another by some property */
    "OBJECT_NOT_FOUND",                 /**< Operation has failed because object was not found */
    "OBJECT_INITIALIZED",               /**< Operation has failed because object is already initialized */
    "OBJECT_UNINITIALIZED",             /**< Operation has failed because object is not initialized */
    "OBJECT_REFERENCED",                /**< Operation has failed because object used by other objects */
    "OBJECT_UNREFERENCED",              /**< Operation has failed because object is not used by other objects */
    "OBJECT_NOT_EMPTY",                 /**< Operation has failed because object contain other objects/references */
    "OBJECT_EMPTY",                     /**< Operation has failed because object doesn`t contain other objects/references */
    "NO_FREE_MEMORY",                   /**< Operation has failed because function cannot allocate memory */
    "FILE_OPEN_FAILED",                 /**< Operation has failed because function cannot open a file */
    "FILE_CLOSE_FAILED",                /**< Operation has failed because function cannot close a file */
    "FILE_READ_FAILED",                 /**< Operation has failed because function cannot read a file */
    "FILE_WRITE_FAILED",                /**< Operation has failed because function cannot write a file */
};

char * status_get_str(ssw_status_t status)
{
    if ((status < SC_MIN) || (status > SC_MAX)) {
        return "Unknown status";
    }

    return status_code_str[status];
}