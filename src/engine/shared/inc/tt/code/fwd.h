#ifndef INC_TT_CODE_FWD_H
#define INC_TT_CODE_FWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

class Buffer;
typedef tt_ptr<const Buffer>::shared BufferPtr;           //!< Use this pointer in client code.
typedef tt_ptr<      Buffer>::shared BufferPtrForCreator; //!< Use this pointer as creator (so data can be changed).

struct BufferReadContext;
struct BufferWriteContext;

class ErrorStatus;


// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_FWD_H)
