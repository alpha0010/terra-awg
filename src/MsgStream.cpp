#ifdef __EMSCRIPTEN__
#include "MsgStream.h"

#include <sstream>

class MsgBuf : public std::stringbuf
{
public:
    ~MsgBuf()
    {
        pubsync();
    }

protected:
    virtual int_type sync()
    {
        // TODO: push `str()`
        str({});
        return 0;
    }
};

namespace
{
MsgBuf msgBuffer;
}

std::ostream msgStrm(&msgBuffer);
#endif
