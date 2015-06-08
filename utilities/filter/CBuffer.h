
#ifndef CBUFFER_H
#define CBUFFER_H

#include <iostream>
#include <type_traits>

class CDataSource;
class CDataSink;

class CBuffer
{
	private:
		char m_buffer[8192];

		char*	m_begin;
		char* m_cursor;
		char* m_end;

	public:

	CBuffer(size_t n=8192);
	CBuffer(const char* begin, const char* end); 
	CBuffer(const CBuffer& rhs);
	~CBuffer();


  // memory allocation 
  void reserve(size_t nbytes);

  size_t capacity() const {
    return (m_end - m_begin);
  }

  void resizeWithoutInit(size_t nbytes);
  
  // content size
	size_t size() const { 
		return (m_cursor - m_begin);
	}

  // insertion
	template<typename T> void push_back(const T& val) {

//    static_assert(std::is_trivially_copyable<T>::value, 
//        "Cannot do a byte-wise copy a type that is not trivially copyable");

		size_t n = sizeof(val);
		const char* begin = reinterpret_cast<const char*>(&val);
		const char* end = begin+n;
		std::copy(begin, end, m_cursor);
		m_cursor += n;
	}
	
  // element access
	char* begin()             { return m_begin; }
	const char* begin() const { return m_begin; }
	char* end()               { return m_cursor;}
	const char* end() const   { return m_cursor;}
		
	protected:
	bool usingSBO() const { return (m_begin == m_buffer);}
};



#endif
