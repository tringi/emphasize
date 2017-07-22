#ifndef RESOURCES_ERRORMESSAGE_HPP
#define RESOURCES_ERRORMESSAGE_HPP

/* Emphasize Resources library standard Error Message support
// Resources_ErrorMessage.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      29.05.2011 - initial version
*/

#include <cstddef>

namespace Resources {
    
    // ErrorMessageAugmentFunction
    //  - called with error code and a parameter, and expected to return valid
    //    string id corresponding to the error code or zero for unknown code
    
    typedef unsigned int (* ErrorMessageAugmentFunction) (unsigned int, void *);
    
    // SetErrorMessageOverride
    // SetErrorMessageSupplement
    //  - augments the ErrorMessage call of another error messages
    
    void SetErrorMessageOverride (ErrorMessageAugmentFunction, void * = NULL);
    void SetErrorMessageSupplement (ErrorMessageAugmentFunction, void * = NULL);
    
    // ErrorMessage
    //  - length is number of characters in buffer
    //  - returns 0 is no suitable message could be retrieved
    
    unsigned int ErrorMessage (wchar_t * buffer, unsigned int length,
                               unsigned int code, unsigned int language = 0u);
};

#endif
