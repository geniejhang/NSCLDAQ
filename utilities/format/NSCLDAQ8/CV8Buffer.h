#ifndef CV8BUFFER_H
#define CV8BUFFER_H

#include <bheader.h>

namespace DAQ
{
  namespace V8 {

    class CV8Buffer
    {
    public:
      virtual ~CV8Buffer() {}
      virtual bheader getHeader() const = 0;
      virtual BufferTypes type() const = 0;
    };

  }
}

#endif // CV8BUFFER_H
