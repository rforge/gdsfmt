// ===========================================================
//     _/_/_/   _/_/_/  _/_/_/_/    _/_/_/_/  _/_/_/   _/_/_/
//      _/    _/       _/             _/    _/    _/   _/   _/
//     _/    _/       _/_/_/_/       _/    _/    _/   _/_/_/
//    _/    _/       _/             _/    _/    _/   _/
// _/_/_/   _/_/_/  _/_/_/_/_/     _/     _/_/_/   _/_/
// ===========================================================
//
// dPlatform.h: Functions for independent platforms
//
// Copyright (C) 2007 - 2014	Xiuwen Zheng
//
// This file is part of CoreArray.
//
// CoreArray is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License Version 3 as
// published by the Free Software Foundation.
//
// CoreArray is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with CoreArray.
// If not, see <http://www.gnu.org/licenses/>.

/**
 *	\file     dPlatform.h
 *	\author   Xiuwen Zheng [zhengx@u.washington.edu]
 *	\version  1.0
 *	\date     2007 - 2014
 *	\brief    Functions for independent platforms
 *	\details
 *  \todo     Need to improve: UTF8toUTF32
**/

#ifndef _HEADER_COREARRAY_PLATFORM_
#define _HEADER_COREARRAY_PLATFORM_

#include <dType.h>
#include <dString.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <limits>
#include <vector>
#include <queue>

#if defined(COREARRAY_WINDOWS)
#   include <windows.h>
#elif defined(COREARRAY_UNIX)
#   ifdef COREARRAY_R_LINK
#       include <R_ext/Riconv.h>
#   else
#       include <iconv.h>
#   endif
#endif

#ifdef COREARRAY_POSIX_THREAD
#   include <pthread.h>
#endif



namespace CoreArray
{
	using namespace std;

	#ifndef COREARRAY_NO_EXTENDED_TYPES

	#ifndef COREARRAY_HAVE_INT128

	struct int128_t;
	struct uint128_t;

	/// Signed integer of 128 bits
	struct int128_t
	{
		#if defined(COREARRAY_LITTLE_ENDIAN)
		C_UInt64 Low; C_Int64 High;
		#elif defined(COREARRAY_BIG_ENDIAN)
		C_Int64 High; C_UInt64 Low;
		#else
		#  error "Unsupported Endianness!"
		#endif
		int128_t() {}
		int128_t(const C_Int64 val) { *this = val; }
		int128_t(const uint128_t &val) { *this = val; }

		int128_t & operator= (C_Int64 val);
		int128_t & operator= (const uint128_t &val);
		operator C_Int64() const;

		void toStr(char Out[]) const {}
	};

	/// Unsigned integer of 128 bits
	struct uint128_t
	{
		#if defined(COREARRAY_LITTLE_ENDIAN)
		C_UInt64 Low; C_UInt64 High;
		#elif defined(COREARRAY_BIG_ENDIAN)
		C_UInt64 High; C_UInt64 Low;
		#else
		#  error "Unsupported Endianness!"
		#endif
		uint128_t() {}
		uint128_t(const C_UInt64 val) { *this = val; }
		uint128_t(const int128_t &val) { *this = val; }

		uint128_t & operator= (C_UInt64 val);
		uint128_t & operator= (const int128_t &val);
		operator C_UInt64() const;

		void toStr(char Out[]) const {}
	};

	#endif


	/// Signed integer of 128 bits
	typedef int128_t    Int128;
	/// Unsigned integer of 128 bits
	typedef uint128_t   UInt128;


	/// Float number of quadruple precision
	#ifndef COREARRAY_HAVE_FLOAT128
	struct Float128
	{
	public:
		/// \todo To support Float128
		Float128() {}
		#ifndef COREARRAY_HAVE_INT128
		Float128(const Int128 &val);
		Float128(const UInt128 &val);
		#endif
		Float128(long double val);

		Float128 & operator= (long double val);
		operator long double() const;

		static Float128 min();
		static Float128 max();
	private:
		char buf[16];
	};
	#else
	typedef long float Float128;
	#endif


	template<> struct TdTraits<Int128>
	{
		typedef Int128 TType;
		static const int trVal = COREARRAY_TR_INTEGER;
		static const unsigned BitOf = 128u;
		static const bool isClass = false;
		static const C_SVType SVType = svCustomInt;

		static const char * TraitName() { return "Int128"; }
		static const char * StreamName() { return "dInt128"; }

	#ifndef COREARRAY_HAVE_INT128
		static Int128 Min()
			{
				Int128 rv;
				rv.Low = TdTraits<C_UInt64>::Max();
				rv.High = TdTraits<C_Int64>::Min();
				return rv;
			}
		static Int128 Max()
			{
				Int128 rv;
				rv.Low = TdTraits<C_UInt64>::Max();
				rv.High = TdTraits<C_Int64>::Max();
				return rv;
			}
	#else
	#  error "Int128"
	#endif
	};

	template<> struct TdTraits<UInt128>
	{
		typedef UInt128 TType;
		static const int trVal = COREARRAY_TR_INTEGER;
		static const unsigned BitOf = 128u;
		static const bool isClass = false;
		static const C_SVType SVType = svCustomInt;

		static const char * TraitName() { return "UInt128"; }
		static const char * StreamName() { return "dUInt128"; }

	#ifndef COREARRAY_HAVE_INT128
		static UInt128 Min()
			{ return UInt128(0); }
		static UInt128 Max()
			{ UInt128 rv; rv.Low = rv.High = -1; return rv; }
	#else
	#  error "UInt128"
	#endif
	};

	#ifndef COREARRAY_HAVE_FLOAT128
	template<> struct TdTraits<Float128>
	{
		typedef UInt128 TType;
		static const int trVal = COREARRAY_TR_FLOAT;
		static const unsigned BitOf = 128u;
		static const bool isClass = false;
		static const C_SVType SVType = svCustomFloat;

		static const char * TraitName() { return "Float128"; }
		static const char * StreamName() { return "Float128"; }

		static Float128 Min()
			{ return Float128::min(); }
		static Float128 Max()
			{ return Float128::max(); }
	};
	#endif

	#endif


	// Floating point number

	extern const double NaN;
	extern const double Infinity;
	extern const double NegInfinity;

	enum TFPClass { fpFinite=0, fpPosInf, fpNegInf, fpNaN };

	TFPClass FloatClassify(const float val);
	TFPClass FloatClassify(const double val);
	TFPClass FloatClassify(const long double val);

	// whether it is finite
	bool IsFinite(const float V);
	bool IsFinite(const double V);
	bool IsFinite(const long double V);

	// whether it is NaN
	bool IsNaN(const float V);
	bool IsNaN(const double V);
	bool IsNaN(const long double V);

	// whether it is +infinity
	bool IsInf(const float V);
	bool IsInf(const double V);
	bool IsInf(const long double V);

	// whether it is -infinity
	bool IsNegInf(const float V);
	bool IsNegInf(const double V);
	bool IsNegInf(const long double V);

	// whether v1 == v2
	bool EqaulFloat(const float v1, const float v2); // consider NaN
	bool EqaulFloat(const double v1, const double v2); // consider NaN
	bool EqaulFloat(const long double v1, const long double v2); // consider NaN

	/// Make FPU exception enable
	void EnableFPUException();
	/// Make FPU exception disable
	void DisableFPUException();
	/// Reset default FPU exception
	void DefaultFPUControl();

	// get a string from floating point number
	std::string FloatToStr(const float val);
	std::string FloatToStr(const double val);
	std::string FloatToStr(const long double val);

	#ifndef COREARRAY_NO_EXTENDED_TYPES
	std::string FloatToStr(const Float128 val);
	#endif


	// get a string from an integer
	std::string IntToStr(const C_Int8 val);
	std::string IntToStr(const C_UInt8 val);
	std::string IntToStr(const C_Int16 val);
	std::string IntToStr(const C_UInt16 val);
	std::string IntToStr(const C_Int32 val);
	std::string IntToStr(const C_UInt32 val);
	std::string IntToStr(const C_Int64 val);
	std::string IntToStr(const C_UInt64 val);

	#ifndef COREARRAY_NO_EXTENDED_TYPES
	std::string IntToStr(const Int128 val);
	std::string IntToStr(const UInt128 val);
	#endif


	// UTF functions

	UTF16String UTF7toUTF16(const char *str);
	UTF16String UTF7toUTF16(const UTF8String &s);
	UTF32String UTF7toUTF32(const char *str);
	UTF32String UTF7toUTF32(const UTF8String &s);

	UTF16String PChartoUTF16(const char* str);
	UTF16String UTF8toUTF16(const UTF8String &s);
	UTF32String UTF8toUTF32(const UTF8String &s);

	UTF8String UTF16toUTF8(const UTF16String &ws);
	UTF32String UTF16toUTF32(const UTF16String &ws);

	UTF8String UTF32toUTF8(const UTF32String &ws);
	UTF16String UTF32toUTF16(const UTF32String &ws);


	COREARRAY_INLINE UTF16String T(const char *s)
	{
		return PChartoUTF16(s);
	}

	COREARRAY_INLINE UTF16String UTF7(const char *s)
	{
		return UTF7toUTF16(s);
	}


	extern const char *const sLineBreak;
	extern const char *const sFileSep;

#ifdef COREARRAY_UNIX
	class COREARRAY_DLL_DEFAULT TdICONV
	{
	public:
		#ifdef COREARRAY_R_LINK
			typedef void* TIconv;
		#else
			typedef iconv_t TIconv;
		#endif

		TdICONV(const char *to, const char *from);
		~TdICONV();
		void Reset();
		size_t Cvt(const char * &inbuf, size_t &inbytesleft,
			char* &outbuf, size_t &outbytesleft);

		COREARRAY_INLINE TIconv Handle() const { return fHandle; }
//		static std::vector<std::string> List();
	protected:
		TIconv fHandle;
	};
#endif


	// Exception

	/// Error Macro
	#define _COREARRAY_ERRMACRO_(x) { \
		va_list args; va_start(args, x); \
		Init(x, args); \
		va_end(args); \
	}

	/// The root class of exception for CoreArray library
	class ErrCoreArray: public std::exception
	{
	public:
		ErrCoreArray(): std::exception()
			{ }
		ErrCoreArray(const char *fmt, ...): std::exception()
			{ _COREARRAY_ERRMACRO_(fmt); }
		ErrCoreArray(const std::string &msg): std::exception()
			{ fMessage = msg; }
		virtual const char *what() const throw()
			{ return fMessage.c_str(); }
		virtual ~ErrCoreArray() throw()
			{ }

	protected:
		std::string fMessage;
		void Init(const char *fmt, va_list arglist);
	};





	// Thread-safe functions



	// System Error information

	/// Exception for system error
	class ErrOSError: public ErrCoreArray
	{
	public:
		ErrOSError(): ErrCoreArray()
			{ };
		ErrOSError(const char *fmt, ...): ErrCoreArray()
			{ _COREARRAY_ERRMACRO_(fmt); }
		ErrOSError(const std::string &msg): ErrCoreArray()
			{ fMessage = msg; }
	};

	/// Return the last code from the OS
	int GetLastOSError();
	/// Format a system error message
	UTF8String SysErrMessage(int err);
	/// Format the last system error message
	UTF8String LastSysErrMsg();
	/// Raise an exception with the last Operating System error code
	template<class X> void RaiseLastOSError() { throw X(LastSysErrMsg()); }

	/// convert the date and time information to a string
	std::string NowDateToStr();


	// File Functions

	#if defined(COREARRAY_WINDOWS)
		typedef HANDLE TSysHandle;
	#elif defined(COREARRAY_UNIX)
		typedef int TSysHandle;
	#else
		"TSysHandle"
	#endif
	extern const TSysHandle NullSysHandle;

	enum TSysOpenMode { fmRead, fmWrite, fmReadWrite };
	enum TSysShareMode { saNone, saRead, saWrite, saReadWrite };
	enum TdSysSeekOrg { soBeginning=0, soCurrent, soEnd };

	TSysHandle SysCreateFile(char const* const AFileName, C_UInt32 Mode);
	TSysHandle SysOpenFile(char const* const AFileName, enum TSysOpenMode mode,
		enum TSysShareMode smode);

	bool SysCloseHandle(TSysHandle Handle);
	size_t SysHandleRead(TSysHandle Handle, void *Buffer, size_t Count);
	size_t SysHandleWrite(TSysHandle Handle, const void* Buffer, size_t Count);
	C_Int64 SysHandleSeek(TSysHandle Handle, C_Int64 Offset, enum TdSysSeekOrg sk);
	bool SysHandleSetSize(TSysHandle Handle, C_Int64 NewSize);


	std::string TempFileName(const char *prefix, const char *tempdir);
	bool FileExists(const std::string &FileName);


	namespace Mach
	{
		/// Return the number of available CPU cores in the system
		/** return -1, if unable to determine. **/
		int GetNumberOfCPU();

		/// Return the size in byte of level-1 cache memory
		/** return 0, if unable to determine. **/
		size_t GetL1CacheMemory();

		/// Return the size in byte of level-2 cache memory
		/** return 0, if unable to determine. **/
		size_t GetL2CacheMemory();
	}


	// Processes
	
	#if defined(COREARRAY_UNIX)
		typedef pid_t TProcessID;
	#elif defined(COREARRAY_WINDOWS)
		typedef DWORD TProcessID;
	#endif

	/// Get the current process id
	TProcessID GetCurrentProcessID();



	// Thread structure, classes, functions

	/// The thread mutex object
	struct CdThreadMutex
	{
	public:
	#ifdef COREARRAY_WINDOWS
		#ifdef COREARRAY_GNU_MINGW32
			typedef CRITICAL_SECTION TdMutex;
			// typedef HANDLE TdMutex;
		#else
			typedef RTL_CRITICAL_SECTION TdMutex;
		#endif
	#elif defined(COREARRAY_POSIX_THREAD)
		typedef pthread_mutex_t TdMutex;
	#else
		"..."
	#endif
		CdThreadMutex();
		~CdThreadMutex();
		void Lock();
		void Unlock();
		bool TryLock();
		COREARRAY_INLINE TdMutex &Mutex() { return mutex; }
	private:
		TdMutex mutex;
	};

	/// The pointer to a thread mutex object
	typedef CdThreadMutex* PdThreadMutex;



	/// The auto object for locking and unlocking a mutex object
	struct TdAutoMutex
	{
		CdThreadMutex * mutex;
		TdAutoMutex(CdThreadMutex *m) { mutex = m; if (m) m->Lock(); }
		~TdAutoMutex() { if (mutex) mutex->Unlock(); }

		/// Reset the mutex object
		void Reset(CdThreadMutex *m)
		{
			if (m != mutex)
			{
				if (mutex) mutex->Unlock();
				mutex = m;
				if (m) m->Lock();
			}
		}
	};



	class CdThread;

	typedef int (*TdThreadProc)(CdThread *Thread, void *Data);

	namespace _INTERNAL
	{
		class CdThBasic {
        public:
			virtual ~CdThBasic() {}
		};

		template<typename Tx> class CdThBasicEx: public CdThBasic {
		public:
			Tx Data;
		};

		struct TdThreadData
		{
			CdThread *thread;
			TdThreadProc proc;
			void *Data;
		};

		template<typename Tx> struct TdThreadDataEx
		{
			typedef int (*TdProc)(CdThread *Thread, Tx Data);
			TdProc proc;
			Tx Data;
		};

		template<typename Tx>
		int _pTdThreadEx(CdThread *Thread, void *Data) {
			CdThBasicEx< _INTERNAL::TdThreadDataEx<Tx> > *p =
				(CdThBasicEx< _INTERNAL::TdThreadDataEx<Tx> >*)Data;
			return (*p->Data.proc)(Thread, p->Data.Data);
		}
	}

    /// Thread class
	class COREARRAY_DLL_DEFAULT CdThread
	{
	public:
		friend class CdThreadsSuspending;

	#if defined(COREARRAY_WINDOWS)
		typedef struct {
			HANDLE Handle;
			DWORD ThreadID;
		} TStruct;
	#elif defined(COREARRAY_POSIX_THREAD)
		typedef pthread_t TStruct;
	#else
		"..."
	#endif

		CdThread();
		CdThread(TdThreadProc proc, void *Data);
		virtual ~CdThread();

		void BeginThread();
		template<typename Tx>
			void BeginThread(int (*proc)(CdThread *, Tx), Tx val)
		{
        	_INTERNAL::CdThBasicEx< _INTERNAL::TdThreadDataEx<Tx> > *p =
				new _INTERNAL::CdThBasicEx< _INTERNAL::TdThreadDataEx<Tx> >;
			p->Data.proc = proc; p->Data.Data = val;
			vData.thread = this; vData.proc = _INTERNAL::_pTdThreadEx<Tx>;
			vPrivate = p; vData.Data = (void*)p;
			_BeginThread();
		}

		int RunThreadSafe();
		virtual int RunThread();
		int EndThread();
		void Terminate();

		COREARRAY_INLINE bool Terminated() const { return terminated; }
		COREARRAY_INLINE TStruct &Thread() { return thread; }
		COREARRAY_INLINE int &ExitCode() { return fExitCode; }
        COREARRAY_INLINE std::string &ErrorInfo() { return fErrorInfo; }

	protected:
		TStruct thread;
		int fExitCode;
		std::string fErrorInfo;
		bool terminated;
		_INTERNAL::TdThreadData vData;
		void _BeginThread(); // need vData

	private:
		_INTERNAL::CdThBasic *vPrivate;
		void Done();
	};

	/// The pointer to a thread object
	typedef CdThread* PdThread;


	/// Closure or delegate for C++
	template<class Tx> struct COREARRAY_DLL_DEFAULT TdThreadObjProc
	{
		void (Tx::*proc)(CdThread *);
		Tx * obj;
		COREARRAY_INLINE void Proc(CdThread *thread) { (obj->*proc)(thread); }
	};


	/// Class for suspending and resuming thread
	class COREARRAY_DLL_DEFAULT CdThreadsSuspending
	{
    public:
		CdThreadsSuspending();
        ~CdThreadsSuspending();

		void Suspend();
        void WakeUp();

	protected:

	#if defined(COREARRAY_WINDOWS)

		// portable "pthread_cond_t" in Win32
		// http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
		//     3.3. The Generation Count Solution
		int waiters_count_;  //< Count of the number of waiters.
		CRITICAL_SECTION waiters_count_lock_;  //< Serialize access to <waiters_count_>.
		int release_count_;  //< Number of threads to release via a <pthread_cond_broadcast>
		int wait_generation_count_;  //< Keeps track of the current "generation" so that
				// we don't allow one thread to steal all the "releases" from the broadcast.
		HANDLE event_;  //< A manual-reset event that's used to block and release waiting threads.

	#elif defined(COREARRAY_POSIX_THREAD)

		pthread_mutex_t mutex;
		pthread_cond_t threshold;

	#endif
	};

	typedef CdThreadsSuspending* PdThreadsSuspending;



	/// Thread pool
/*	class COREARRAY_DLL_DEFAULT CdThreadPool
	{
	public:
    	// There is no limit of thread numbers, if MaxThreads = 0
		CdThreadPool(size_t MaxThreads=0, size_t InitThreads=0);
		virtual ~CdThreadPool();

		template<typename Tx>
			void BeginThread(int (*proc)(CdThread *, Tx), Tx val)
		{

		}

		COREARRAY_INLINE size_t MaxThreads() const { return fMaxThreads; }
		void SetMaxThreads(size_t NewMaxThreads);
	protected:
        CdThreadMutex fPoolMutex;
    	size_t fMaxThreads;
		std::queue<CdThread*> fWorkingList, fWaitingList;
	};
*/

	/// Exceptions for threads
	class ErrThread: public ErrOSError
	{
	public:
		ErrThread(): ErrOSError()
			{ }
		ErrThread(const char *fmt, ...): ErrOSError()
			{ _COREARRAY_ERRMACRO_(fmt); }
		ErrThread(const std::string &msg): ErrOSError()
			{ fMessage = msg; }
	};


	/// Exceptions for conversion
	class ErrConvert: public ErrCoreArray
	{
	public:
		ErrConvert(): ErrCoreArray()
			{ }
		ErrConvert(const char *fmt, ...): ErrCoreArray()
			{ _COREARRAY_ERRMACRO_(fmt); }
		ErrConvert(const std::string &msg): ErrCoreArray()
			{ fMessage = msg; }
	};


	void SwapData(void *d1, void *d2, size_t len);

	UTF8String Format(const char *fmt, ...);
	UTF8String _FmtNum(const char *fmt, ...); // with small buffer

	long StrToInt(char const* str);
	bool StrToInt(char const* str, long *rv);
	long StrToIntDef(char const* str, const long Default);

	double StrToFloat(char const* str);
	bool StrToFloat(char const* str, double *rv);
	double StrToFloatDef(char const* str, const double Default);




	namespace _INTERNAL
	{
		// Type Convert

		template<typename DestT, typename SourceT,
			int DTrait = TdTraits<DestT>::trVal,
			int STrait = TdTraits<SourceT>::trVal >
		struct TValCvt
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const SourceT &val) { return val; }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = *s++; }
		};

		template<typename DestT, typename SourceT>
			struct TValCvt<DestT, SourceT, COREARRAY_TR_INTEGER, COREARRAY_TR_FLOAT>
		{
		#ifdef COREARRAY_GNU_CC
			COREARRAY_FORCE_INLINE static DestT Cvt(const SourceT &val)
				{ return DestT(typename TdTraits<DestT>::TType(val)); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = DestT(typename TdTraits<DestT>::TType(*s++)); }
		#else
			COREARRAY_FORCE_INLINE static DestT Cvt(const SourceT &val)
				{ return DestT(TdTraits<DestT>::TType(val)); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = DestT(TdTraits<DestT>::TType(*s++)); }
		#endif
		};

		// Type Convert: Source UTF8String

		template<typename DestT> struct TValCvt<DestT, UTF8String,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF8String &val)
				{ return StrToInt(val.c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF8String *s, ssize_t L)
				{ for (; L > 0; p++, s++, L--) *p = StrToInt(s->c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF8*,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF8 * val)
				{ return StrToInt(val); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF8 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToInt(*s++); }
		};

		template<typename SourceT> struct TValCvt<UTF8String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_INTEGER>
		{
			COREARRAY_FORCE_INLINE static UTF8String Cvt(const SourceT val)
				{ return IntToStr(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF8String *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = IntToStr(*s++); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF8String,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF8String &val)
				{ return StrToFloat(val.c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF8String *s, ssize_t L)
				{ for (; L > 0; p++, s++, L--) *p = StrToFloat(s->c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF8*,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF8 *val)
				{ return StrToFloat(val); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF8 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToFloat(*s++); }
		};

		template<typename SourceT> struct TValCvt<UTF8String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_FLOAT>
		{
			COREARRAY_FORCE_INLINE static UTF8String Cvt(const SourceT &val)
				{ return FloatToStr(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF8String *p, const SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = FloatToStr(*s++); }
		};

		template<> struct TValCvt<UTF16String, UTF8String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF16String Cvt(const UTF8String &val)
				{ return CoreArray::UTF8toUTF16(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, UTF8String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF8toUTF16(*s++); }
		};

		template<> struct TValCvt<UTF32String, UTF8String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF32String Cvt(const UTF8String &val)
				{ return CoreArray::UTF8toUTF32(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF32String *p, UTF8String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF8toUTF32(*s++); }
		};

		// Type Convert: Source UTF16String

		template<typename DestT> struct TValCvt<DestT, UTF16String,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF16String &val)
				{ return StrToInt(UTF16toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF16String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToInt(UTF16toUTF8(*s++).c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF16*,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF16 *val)
				{ return StrToInt(UTF16toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF16 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToInt(UTF16toUTF8(*s++).c_str()); }
		};

		template<typename SourceT> struct TValCvt<UTF16String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_INTEGER>
		{
			COREARRAY_FORCE_INLINE static UTF16String Cvt(const SourceT val)
				{ return UTF7toUTF16(IntToStr(val)); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = UTF7toUTF16(IntToStr(*s++)); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF16String,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF16String &val)
				{ return StrToFloat(UTF16toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF16String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToFloat(UTF16toUTF8(*s++).c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF16*,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF16 *val)
				{ return StrToFloat(UTF16toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF16 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToFloat(UTF16toUTF8(*s++).c_str()); }
		};

		template<typename SourceT> struct TValCvt<UTF16String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_FLOAT>
		{
			COREARRAY_FORCE_INLINE static UTF16String Cvt(const SourceT &val)
				{ return UTF7toUTF16(FloatToStr(val)); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, const SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = UTF7toUTF16(FloatToStr(*s++)); }
		};

		template<> struct TValCvt<UTF8String, UTF16String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF8String Cvt(const UTF16String &val)
				{ return CoreArray::UTF16toUTF8(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, UTF8String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF8toUTF16(*s++); }
		};

		template<> struct TValCvt<UTF32String, UTF16String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF32String Cvt(const UTF16String &val)
				{ return CoreArray::UTF16toUTF32(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, UTF32String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF32toUTF16(*s++); }
		};

		// Type Convert: Source UTF32String

		template<typename DestT> struct TValCvt<DestT, UTF32String,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF32String &val)
				{ return StrToInt(UTF32toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF32String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToInt(UTF32toUTF8(*s++).c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF32*,
			COREARRAY_TR_INTEGER, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF32 *val)
				{ return StrToInt(UTF32toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF32 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToInt(UTF32toUTF8(*s++).c_str()); }
		};

		template<typename SourceT> struct TValCvt<UTF32String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_INTEGER>
		{
			COREARRAY_FORCE_INLINE static UTF32String Cvt(const SourceT val)
				{ return UTF7toUTF32(IntToStr(val)); }
			COREARRAY_FORCE_INLINE static void Array(UTF32String *p, SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = UTF7toUTF32(IntToStr(*s++)); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF32String,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF32String &val)
				{ return StrToFloat(UTF32toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF32String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToFloat(UTF32toUTF8(*s++).c_str()); }
		};

		template<typename DestT> struct TValCvt<DestT, UTF32*,
			COREARRAY_TR_FLOAT, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static DestT Cvt(const UTF32 *val)
				{ return StrToFloat(UTF32toUTF8(val).c_str()); }
			COREARRAY_FORCE_INLINE static void Array(DestT *p, UTF32 **s, ssize_t L)
				{ for (; L > 0; L--) *p++ = StrToFloat(UTF32toUTF8(*s++).c_str()); }
		};

		template<typename SourceT> struct TValCvt<UTF32String, SourceT,
			COREARRAY_TR_STRING, COREARRAY_TR_FLOAT>
		{
			COREARRAY_FORCE_INLINE static UTF32String Cvt(const SourceT &val)
				{ return UTF7toUTF32(FloatToStr(val)); }
			COREARRAY_FORCE_INLINE static void Array(UTF32String *p, const SourceT *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = UTF7toUTF32(FloatToStr(*s++)); }
		};

		template<> struct TValCvt<UTF8String, UTF32String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF8String Cvt(const UTF32String &val)
				{ return CoreArray::UTF32toUTF8(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF8String *p, UTF32String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF32toUTF8(*s++); }
		};

		template<> struct TValCvt<UTF16String, UTF32String,
			COREARRAY_TR_STRING, COREARRAY_TR_STRING>
		{
			COREARRAY_FORCE_INLINE static UTF16String Cvt(const UTF32String &val)
				{ return CoreArray::UTF32toUTF16(val); }
			COREARRAY_FORCE_INLINE static void Array(UTF16String *p, UTF32String *s, ssize_t L)
				{ for (; L > 0; L--) *p++ = CoreArray::UTF32toUTF16(*s++); }
		};
	}

	/// Conversion from SourceT to DestT
	/** \tparam  DestT    type of destination
	 *  \tparam  SourceT  type of source
	**/
	template<typename DestT, typename SourceT>
	static COREARRAY_FORCE_INLINE DestT ValCvt(const SourceT &val)
		{ return _INTERNAL::TValCvt<DestT, SourceT>::Cvt(val); }

	/// Conversion from SourceT to DestT
	/** \tparam  DestT    type of destination
	 *  \tparam  SourceT  type of source
	**/
	template<typename DestT, typename SourceT>
	static COREARRAY_FORCE_INLINE void ValCvtArray(DestT *p, SourceT *s, ssize_t L)
		{ _INTERNAL::TValCvt<DestT, SourceT>::Array(p, s, L); }



	// --------------------------------------------------------------------

	/// Unaligned C_Int16 -- get
	static COREARRAY_FORCE_INLINE C_Int16 GET_VAL_UNALIGNED_PTR(const C_Int16 *p)
	{
		if (size_t(p) & 0x01)
		{
			const C_UInt8 *s = (const C_UInt8*)p;
			return C_Int16(s[0]) | (C_Int16(s[1]) << 8);
		} else
			return *p;
	}

	/// Unaligned C_UInt16 -- get
	static COREARRAY_FORCE_INLINE C_UInt16 GET_VAL_UNALIGNED_PTR(const C_UInt16 *p)
	{
		if (size_t(p) & 0x01)
		{
			const C_UInt8 *s = (const C_UInt8*)p;
			return C_UInt16(s[0]) | (C_UInt16(s[1]) << 8);
		} else
			return *p;
	}

	/// Unaligned C_Int32 -- get
	static COREARRAY_FORCE_INLINE C_Int32 GET_VAL_UNALIGNED_PTR(const C_Int32 *p)
	{
		if (size_t(p) & 0x03)
		{
			const C_UInt8 *s = (const C_UInt8*)p;
			return C_Int32(s[0]) | (C_Int32(s[1]) << 8) |
				(C_Int32(s[2]) << 16) | (C_Int32(s[3]) << 24);
		} else
			return *p;
	}

	/// Unaligned C_UInt32 -- get
	static COREARRAY_FORCE_INLINE C_UInt32 GET_VAL_UNALIGNED_PTR(const C_UInt32 *p)
	{
		if (size_t(p) & 0x03)
		{
			const C_UInt8 *s = (const C_UInt8*)p;
			return C_UInt32(s[0]) | (C_UInt32(s[1]) << 8) |
				(C_UInt32(s[2]) << 16) | (C_UInt32(s[3]) << 24);
		} else
			return *p;
	}

	/// Unaligned C_Int16 -- set
	static COREARRAY_FORCE_INLINE void SET_VAL_UNALIGNED_PTR(C_Int16 *p, C_Int16 val)
	{
		if (size_t(p) & 0x01)
		{
			const C_UInt8 *s1 = (const C_UInt8*)&val;
			C_UInt8 *s2 = (C_UInt8*)p;
			s2[0] = s1[0]; s2[1] = s1[1];
		} else
			*p = val;
	}

	/// Unaligned C_UInt16 -- set
	static COREARRAY_FORCE_INLINE void SET_VAL_UNALIGNED_PTR(C_UInt16 *p, C_UInt16 val)
	{
		if (size_t(p) & 0x01)
		{
			const C_UInt8 *s1 = (const C_UInt8*)&val;
			C_UInt8 *s2 = (C_UInt8*)p;
			s2[0] = s1[0]; s2[1] = s1[1];
		} else
			*p = val;
	}

	/// Unaligned C_Int32 -- set
	static COREARRAY_FORCE_INLINE void SET_VAL_UNALIGNED_PTR(C_Int32 *p, C_Int32 val)
	{
		if (size_t(p) & 0x03)
		{
			const C_UInt8 *s1 = (const C_UInt8*)&val;
			C_UInt8 *s2 = (C_UInt8*)p;
			s2[0] = s1[0]; s2[1] = s1[1]; s2[2] = s1[2]; s2[3] = s1[3];
		} else
			*p = val;
	}

	/// Unaligned C_UInt32 -- set
	static COREARRAY_FORCE_INLINE void SET_VAL_UNALIGNED_PTR(C_UInt32 *p, C_UInt32 val)
	{
		if (size_t(p) & 0x03)
		{
			const C_UInt8 *s1 = (const C_UInt8*)&val;
			C_UInt8 *s2 = (C_UInt8*)p;
			s2[0] = s1[0]; s2[1] = s1[1]; s2[2] = s1[2]; s2[3] = s1[3];
		} else
			*p = val;
	}




	// Endian

	#if defined(COREARRAY_LITTLE_ENDIAN)

		#define COREARRAY_ENDIAN_CVT(x, size)
		#define COREARRAY_ENDIAN_CVT16(x)      x
		#define COREARRAY_ENDIAN_CVT32(x)      x
		#define COREARRAY_ENDIAN_CVT64(x)      x
		#define COREARRAY_ENDIAN_CVT128(x)     x
		#define COREARRAY_ENDIAN_VAL(x)        x
		#define COREARRAY_ENDIAN_ARRAY(x, size)

	#elif defined(COREARRAY_BIG_ENDIAN)

		void COREARRAY_ENDIAN_CVT(void *x, size_t size);
		C_UInt16 COREARRAY_ENDIAN_CVT16(C_UInt16 x);
		C_UInt32 COREARRAY_ENDIAN_CVT32(C_UInt32 x);
		C_UInt64 COREARRAY_ENDIAN_CVT64(C_UInt64 x);
		#ifndef COREARRAY_NO_EXTENDED_TYPES
		UInt128 COREARRAY_ENDIAN_CVT128(UInt128 x);
		#endif


		namespace _INTERNAL
		{
			// Endianness Conversion
			template<typename TYPE> struct TEndianValCvt
			{
				COREARRAY_FORCE_INLINE static TYPE Cvt(const TYPE &val) { return val; }
				COREARRAY_FORCE_INLINE static void Array(TYPE *p, ssize_t L) { }
			};

			template<> struct TEndianValCvt<C_Int16>
			{
				COREARRAY_FORCE_INLINE static C_Int16 Cvt(C_Int16 val)
					{ return COREARRAY_ENDIAN_CVT16(val); }
				COREARRAY_FORCE_INLINE static void Array(C_Int16 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT16(*p); }
			};
			template<> struct TEndianValCvt<C_UInt16>
			{
				COREARRAY_FORCE_INLINE static C_UInt16 Cvt(C_UInt16 val)
					{ return COREARRAY_ENDIAN_CVT16(val); }
				COREARRAY_FORCE_INLINE static void Array(C_UInt16 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT16(*p); }
			};

			template<> struct TEndianValCvt<C_Int32>
			{
				COREARRAY_FORCE_INLINE static C_Int32 Cvt(C_Int32 val)
					{ return COREARRAY_ENDIAN_CVT32(val); }
				COREARRAY_FORCE_INLINE static void Array(C_Int32 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT32(*p); }
			};
			template<> struct TEndianValCvt<C_UInt32>
			{
				COREARRAY_FORCE_INLINE static C_UInt32 Cvt(C_UInt32 val)
					{ return COREARRAY_ENDIAN_CVT32(val); }
				COREARRAY_FORCE_INLINE static void Array(C_UInt32 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT32(*p); }
			};

			template<> struct TEndianValCvt<C_Int64>
			{
				COREARRAY_FORCE_INLINE static C_Int64 Cvt(C_Int64 val)
					{ return COREARRAY_ENDIAN_CVT64(val); }
				COREARRAY_FORCE_INLINE static void Array(C_Int64 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT64(*p); }
			};
			template<> struct TEndianValCvt<C_UInt64>
			{
				COREARRAY_FORCE_INLINE static C_UInt64 Cvt(C_UInt64 val)
					{ return COREARRAY_ENDIAN_CVT64(val); }
				COREARRAY_FORCE_INLINE static void Array(C_UInt64 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT64(*p); }
			};

		#ifndef COREARRAY_NO_EXTENDED_TYPES
			template<> struct TEndianValCvt<Int128>
			{
				COREARRAY_FORCE_INLINE static Int128 Cvt(const Int128 &val)
					{ return COREARRAY_ENDIAN_CVT128(val); }
				COREARRAY_FORCE_INLINE static void Array(Int128 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT128(*p); }
			};
			template<> struct TEndianValCvt<UInt128>
			{
				COREARRAY_FORCE_INLINE static UInt128 Cvt(const UInt128 &val)
					{ return COREARRAY_ENDIAN_CVT128(val); }
				COREARRAY_FORCE_INLINE static void Array(UInt128 *p, ssize_t L)
					{ for (; L > 0; L--, p++) *p = COREARRAY_ENDIAN_CVT128(*p); }
			};
		#endif
		}


		template<typename TYPE>
		COREARRAY_FORCE_INLINE TYPE COREARRAY_ENDIAN_VAL(const TYPE &val)
			{ return _INTERNAL::TEndianValCvt<TYPE>::Cvt(val); }

		template<typename TYPE>
		COREARRAY_FORCE_INLINE void COREARRAY_ENDIAN_ARRAY(TYPE *p, ssize_t L)
			{ return _INTERNAL::TEndianValCvt<TYPE>::Array(p, L); }

	#else
	#  error "Unknown endianness"
    #endif
}

#endif /* _HEADER_COREARRAY_PLATFORM_ */
