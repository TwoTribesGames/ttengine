#if !defined(INC_TT_ASSERT_STRING_H)
#define INC_TT_ASSERT_STRING_H

#include <stdarg.h>
#include <string.h>
#include <crtdbg.h>

namespace tt {
namespace assert {

template< class TYPE >
class String
{
	public:	
		String( void );														// default constructor
		String( const char* pSource );										// construct from a char*
		String( const wchar_t* pSource );									// widechar version
		String( const String<TYPE>& source );								// construct from a string object of this type
		String( const int numChars );										// construct and allocate numChars of character space

		~String( void );

		void clear( void );													// clears the string, deallocates memory
		void zero( void );													// zero's the current block of memory
		void compact( void );												// reallocates the buffer to the length needed for the current string

		int length( void ) const;											// returns the length of the current string

		TYPE* allocate( const int numChars );								// allocates x number of CHARACTERS (not bytes). terminator not included	

		operator const TYPE*() const;										// cast back to our native type
		operator TYPE*();													// cast back to our native type
		operator bool() const;												// if the string has lenght, e.g if (string)

		const TYPE* data( void ) const;

		const String<TYPE>& operator = (const String<char>& rhs);			// assignemnt for char strings
		const String<TYPE>& operator = (const String<wchar_t>& rhs);		// assignemnt for wchar strings

		const String<TYPE>& operator = (const char* rhs);					// assignemnt for char*
		const String<TYPE>& operator = (const wchar_t* rhs);				// assignemnt for whar*


		bool operator == (const TYPE* rhs) const;							// only type comparisons, no class. Compiler should cast
		bool operator != (const TYPE* rhs) const;							// as aboce

		bool operator == (const String<TYPE>& rhs) const;					// only type comparisons, no class. Compiler should cast
		bool operator != (const String<TYPE>& rhs) const;					// as aboce

		const String<TYPE>& operator += (const TYPE* rhs);					// append a string of our type to us
		const String<TYPE>& operator += (const TYPE& letter);				// append a character of our type to us
	
		TYPE& operator[] (const int index);									// returns a reference to an indexed character, str[1] = 'h';
		const TYPE&  operator[] (const int index) const;					// returns a const ref to our character, char = str1[1];

		int sprintf(const TYPE* pFormat, ...);								// formats a string into this class


		void upper( void );													// converts a string to uppercase
		void lower( void );													// converts a string to lowercase


	private:
		
		void InitClass( void );												// initalise the class, assign variables default calues
		void EndClass( void );												// ends the class, deallocates mem & clears up

		// util routines - fairly obvious. Versions for both char & wchar
		// so the compiler will pick the correct one depending on our
		// templated type. We need these with specific types for assigning widechar
		// to ansi and vice-versae
		void util_strcpy( const char* pSource );
		void util_strcpy( const wchar_t* pSource );

		int util_strlen( const char* pSource ) const;
		int util_strlen( const wchar_t* pSource ) const;


		void util_strcat(const TYPE* pSource);
		void util_strcat(const TYPE& letter);

		bool util_stricmp(const TYPE* rhs) const;

		int util_vsprintf( char* buffer, const char* format, va_list argptr );
		int util_vsprintf( wchar_t* buffer, const wchar_t* format, va_list argptr );

		TYPE*	mpDataBuffer;
		int		mBufferSize;

};



typedef String<char> StringA;
typedef String<wchar_t> StringW;


// initialisation
template< class TYPE >
void String<TYPE>::InitClass( void )
{
	_ASSERTE(sizeof(TYPE) == 1 || sizeof(TYPE) == 2);		// must be one of these!

	mpDataBuffer = NULL;
	mBufferSize = 0;
}


// destruction
template< class TYPE >
void String<TYPE>::EndClass( void )
{
	if (mpDataBuffer)
	{
		delete [] mpDataBuffer;
		mpDataBuffer = NULL;
	}
		
	mBufferSize = 0;
}

// memory allocation
template< class TYPE >
TYPE* String<TYPE>::allocate( const int numChars )
{
	if (mpDataBuffer)
	{
		delete [] mpDataBuffer;
		mpDataBuffer = NULL;
	}

	if (numChars)
	{

		mBufferSize = numChars;
		mpDataBuffer = new TYPE[mBufferSize];
		
		zero();
	}

	return mpDataBuffer;
}


// default constructor
template< class TYPE >
String<TYPE>::String( void )
{
	InitClass();
}

// constructor
template< class TYPE >
String<TYPE>::String( const char* pSource )
{
	InitClass();

	util_strcpy(pSource);
}

template< class TYPE >
String<TYPE>::String( const wchar_t* pSource )
{
	InitClass();

	util_strcpy(pSource);
}

// constructor
template< class TYPE >
String<TYPE>::String( const String<TYPE>& source )
{
	InitClass();

	util_strcpy(source);
}

// constructor
template< class TYPE >
String<TYPE>::String( const int numChars )
{
	InitClass();

	allocate(numChars);

	memset(mpDataBuffer, 0, sizeof(TYPE) * numChars);
}


// destructor
template< class TYPE >
String<TYPE>::~String( void )
{
	EndClass();
}


template< class TYPE >
void String<TYPE>::clear( void )
{
	EndClass();
	InitClass();
}


template< class TYPE >
void String<TYPE>::zero( void )
{
	if (mpDataBuffer)
	{
		memset(mpDataBuffer, 0, sizeof(TYPE) * mBufferSize);
	}
}

// removes any remaining whitespace
template< class TYPE >
void String<TYPE>::compact( void )
{
	String<TYPE> temp = *this;

	clear();

	*this = temp;
}


// assignment operators
template< class TYPE >
const String<TYPE>& String<TYPE>::operator = (const String<char>& rhs)			// assignment for strings
{
	util_strcpy(rhs);

	return *this;
}

// assignment operators
template< class TYPE >
const String<TYPE>& String<TYPE>::operator = (const String<wchar_t>& rhs)			// assignment for strings
{
	util_strcpy(rhs);

	return *this;
}

template< class TYPE >
const String<TYPE>& String<TYPE>::operator = (const char* rhs)					// assignment for strings
{
	util_strcpy(rhs);

	return *this;
}


template< class TYPE >
const String<TYPE>& String<TYPE>::operator = (const wchar_t* rhs)					// assignment for strings
{
	util_strcpy(rhs);

	return *this;
}


template< class TYPE >
TYPE& String<TYPE>::operator[] (const int index)
{
	// do not check length as this can be used to set
	// elements that have been allocated, check on buffersize
	_ASSERTE(index < mBufferSize);

	return mpDataBuffer[index];
}


template< class TYPE >
const TYPE& String<TYPE>::operator[] (const int index) const
{
	// boundry check on used length
	_ASSERTE(index <= length());
		
	return mpDataBuffer[index];
}

	

// conversion to our type
template< class TYPE >
String<TYPE>::operator const TYPE*() const
{
	return mpDataBuffer;
}

// conversion to our type
template< class TYPE >
String<TYPE>::operator TYPE*()
{
	return mpDataBuffer;
}

template< class TYPE >
const TYPE* String<TYPE>::data( void ) const
{
	return mpDataBuffer;
}

// test for validity
template< class TYPE >
String<TYPE>::operator bool() const
{
	return (mpDataBuffer != NULL);
}

// length
template< class TYPE >
int String<TYPE>::length( void ) const
{
	return util_strlen(mpDataBuffer);
}


// comparison operators
template< class TYPE >
bool String<TYPE>::operator == (const TYPE* rhs) const
{
	return (util_stricmp(rhs)==true);
}


template< class TYPE >
bool String<TYPE>::operator != (const TYPE* rhs) const
{
	return (util_stricmp(rhs) != true);
}

// comparison operators
template< class TYPE >
bool String<TYPE>::operator == (const String<TYPE>& rhs) const
{
	return (util_stricmp(rhs)==true);
}


template< class TYPE >
bool String<TYPE>::operator != (const String<TYPE>& rhs) const
{
	return (util_stricmp(rhs) != true);
}

template< class TYPE >
const String<TYPE>& String<TYPE>::operator += (const TYPE* rhs)
{
	util_strcat(rhs);

	return *this;
}

template< class TYPE >
const String<TYPE>& String<TYPE>::operator += (const TYPE& letter)
{
	util_strcat(letter);

	return *this;
}


template< class TYPE >
int String<TYPE>::sprintf(const TYPE* pFormat, ...)
{
	TYPE buffer[16384];		// hope this is big enough...
	
	va_list vItems;

	// init the list
	va_start(vItems, pFormat);

	// get the formatting
	int chars = util_vsprintf(buffer, pFormat, vItems);

	// end the var list
	va_end(vItems);

	// if we overwrote stuff we should still be able to see how much by
	_ASSERTE(util_strlen(buffer) < 16384);

	*this = buffer;

	return chars;
}
                                                                 


template< class TYPE >
void String<TYPE>::upper( void )													// converts a string to uppercase
{
	TYPE* data = mpDataBuffer;

	if (NULL == data) return;

	while (*data)
	{
		if (*data >= 'a' && *data <= 'z')
		{
			*data = *data - ('a' - 'A');
		}

		data++;
	}
}

template< class TYPE >
void String<TYPE>::lower( void )													// converts a string to lowercase
{
	TYPE* data = mpDataBuffer;

	if (NULL == data) return;

	while (*data)
	{
		if (*data >= 'A' && *data <= 'Z')
		{
			*data = *data + ('a' - 'A');
		}

		data++;
	}
}





//
//
// util routines - some of these have char and wchar versions of each named the same for
// assignment & copy constructors from different types
//
//



template< class TYPE >
void String<TYPE>::util_strcpy( const char* pSource )
{

	if (((void*)mpDataBuffer) == ((void*)pSource))					// do not copy if this is us!
	{
		return;
	}

	// if null, blank this string
	if (pSource == NULL)
	{
		if (mBufferSize)
		{
			memset(mpDataBuffer, 0, mBufferSize * sizeof(TYPE));
		}
	}
	else
	{
		int len = util_strlen(pSource) +1;

		if (len > mBufferSize)
		{
			allocate( len );
		}

		TYPE* pDest = mpDataBuffer;

		while (*pSource)
		{
			*pDest++ = static_cast<TYPE>(*pSource++);
		}

		*pDest = 0;
	}
}

template< class TYPE >
void String<TYPE>::util_strcpy( const wchar_t* pSource )
{

	if (((void*)mpDataBuffer) == ((void*)pSource))					// do not copy if this is us!
	{
		return;
	}

	// if null, blank this string
	if (pSource == NULL)
	{
		if (mBufferSize)
		{
			memset(mpDataBuffer, 0, mBufferSize * sizeof(TYPE));
		}
	}
	else
	{
		int len = util_strlen(pSource) +1;

		if (len > mBufferSize)
		{
			allocate( len );
		}

		TYPE* pDest = mpDataBuffer;

		while (*pSource)
		{
			*pDest++ = static_cast<TYPE>(*pSource++);
		}

		*pDest = 0;
	}
}

template< class TYPE >
int String<TYPE>::util_strlen( const char* pSource ) const
{
	if (pSource == NULL)
	{
		return 0;
	}

	const char* start = pSource;
	const char* end = pSource;

	while (*end)
	{
		end++;
	}

	return static_cast<int>(end-start);
}

template< class TYPE >
int String<TYPE>::util_strlen( const wchar_t* pSource ) const
{
	if (pSource == NULL)
	{
		return 0;
	}

	const wchar_t* start = pSource;
	const wchar_t* end = pSource;

	while (*end)
	{
		end++;
	}

	return (end-start);
}


template< class TYPE >
bool String<TYPE>::util_stricmp( const TYPE* rhs) const
{
	if (mpDataBuffer == NULL || rhs == NULL) return false;

	const TYPE* plhs = mpDataBuffer;
	const TYPE* prhs = rhs;

	// while we have strings
	while (*plhs && *prhs)
	{
		if (*plhs != *prhs)
		{
			return false;
		}

		plhs++;
		prhs++;
	}

	// if one was shorter
	if (*plhs != *prhs)
	{
		return false;
	}

	return true;
}


/*template< class TYPE >
int String<TYPE>::util_stricmp(const wchar_t* lhs, const wchar_t* rhs)
{
	if (lhs == NULL || rhs == NULL) return false;

	return wcsicmp(lhs, rhs);
}*/


template< class TYPE >
void String<TYPE>::util_strcat(const TYPE* pSource)
{
	if (NULL == pSource) return;

	_ASSERTE(pSource != mpDataBuffer);

	if (pSource == mpDataBuffer) return;
	

	int srcLen = util_strlen(pSource) + 1;
	int destLen = length();

	if (srcLen + destLen > mBufferSize)
	{
		// need to allocate a new buffer
		String<TYPE> temp = *this;

		allocate(srcLen + destLen);

		util_strcpy(temp);
	}

	TYPE* pDest = mpDataBuffer;

	// find the end of the string
	while (*pDest)
	{
		pDest++;
	}

	// copy the new one in
	while (*pSource)
	{
		*pDest++ = *pSource++;
	}

	*pDest = 0;
}


template< class TYPE >
void String<TYPE>::util_strcat(const TYPE& letter)
{
	int destLen = length() + 1;

	if (destLen + 1 > mBufferSize)
	{
		// need to allocate a new buffer
		String<TYPE> temp = *this;

		allocate(destLen + 1);

		util_strcpy(temp);
	}

	// we have room, append the letter
	mpDataBuffer[destLen-1] = letter;
	mpDataBuffer[destLen] = 0;
}


template< class TYPE >
int String<TYPE>::util_vsprintf( char* buffer, const char* format, va_list argptr )
{
	return vsprintf(buffer, format, argptr);
}

template< class TYPE >
int String<TYPE>::util_vsprintf( wchar_t* buffer, const wchar_t* format, va_list argptr )
{
	return vswprintf(buffer, format, argptr);
}

// namespace
}
}

#endif // !defined(INC_TT_ASSERT_STRING_H)

