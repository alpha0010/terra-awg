#ifndef MSGSTREAM_H
#define MSGSTREAM_H

#ifdef __EMSCRIPTEN__
#include <ostream>
extern std::ostream msgStrm;
#else
#include <iostream>
inline auto &msgStrm = std::cout;
#endif

#endif // MSGSTREAM_H
