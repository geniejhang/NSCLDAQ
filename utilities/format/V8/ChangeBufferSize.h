#ifndef DAQ_V8_TEST_CHANGEBUFFERSIZE_H
#define DAQ_V8_TEST_CHANGEBUFFERSIZE_H

namespace DAQ {
  namespace V8 {
    namespace Test {

      /*!
       * \brief A simple tool to change the buffer size within a scope
       */
      struct ChangeBufferSize {
        std::size_t oldSize;
        ChangeBufferSize(std::size_t bsize) : oldSize(gBufferSize) {
          gBufferSize = bsize;
        }
        ~ChangeBufferSize() {
          gBufferSize = oldSize;
        }
      };

    } // namespace Test
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_TEST_BUFFERSIZECHANGER_H
