extern "C"
{
#include <execinfo.h> // for backtrace
#include <dlfcn.h>    // for dladdr
#include <cxxabi.h>   // for __cxa_demangle
}

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <array>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <string_view>

#include <boost/functional/hash.hpp>

template <std::size_t MaxFramesCount>
struct Backtrace
{
	// this function *does* allocate every time you call it.
	// Callable: fn(std::string_view symbol)
	template <typename Callable>
	void VisitSymbols(Callable visitor)
	{
		char **symbols = ::backtrace_symbols(mCallstack.data(), mFramesCount);

		for (int i = 1; i < mFramesCount; i++)
		{
			const std::size_t symbolLen = std::strlen(symbols[i]);
			const std::string_view symbol(symbols[i], symbolLen);
			visitor(symbol);
		}
	}

	bool operator==(const Backtrace& rhs) const
	{
		return mFramesCount == rhs.mFramesCount
				&& std::equal(mCallstack.cbegin(), mCallstack.cbegin() + mFramesCount, rhs.mCallstack.cbegin());
	}

	std::array<void*, MaxFramesCount> mCallstack;
	int mFramesCount;
};

namespace std
{

template <std::size_t MaxFramesCount>
struct hash<::Backtrace<MaxFramesCount>>
{
	std::size_t operator()(const Backtrace<MaxFramesCount>& bt) const
	{
		return boost::hash_range(bt.mCallstack.cbegin(), bt.mCallstack.cbegin() + bt.mFramesCount);
	}
};

}

class StackInspector2
{
public:
	static constexpr std::size_t MaxFramesCount = 5;

	void StoreBacktrace()
	{
		Backtrace<MaxFramesCount> bt;
		bt.mFramesCount = ::backtrace(bt.mCallstack.data(), MaxFramesCount);
		++s[bt];
	}


private:
	std::unordered_map<Backtrace<MaxFramesCount>, int> s;
};



// This function produces a stack backtrace with demangled function & method names.
template <std::size_t MaxFramesCount>
struct StackInspector
{
	std::array<void*, MaxFramesCount> mCallstack;

	std::string GetBacktrace()
	{
	std::ostringstream trace_buf;
	std::array<void*, MaxFramesCount> callstack;
	char buf[1024];
	const int nFrames = ::backtrace(callstack.data(), MaxFramesCount);

	for (int i = 1; i < nFrames; i++)
		trace_buf << std::hex << callstack[i] << "\n";

/*
	char **symbols = ::backtrace_symbols(callstack.data(), nFrames);

	for (int i = 1; i < nFrames; i++)
	{
		//printf("%s\n", symbols[i]);

		Dl_info info;
		if (::dladdr(callstack[i], &info) && info.dli_sname) {
			char *demangled = NULL;
			int status = -1;
			if (info.dli_sname[0] == '_')
				demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
			snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
					 i, int(2 + sizeof(void*) * 2), callstack[i],
					 status == 0 ? demangled :
					 info.dli_sname == 0 ? symbols[i] : info.dli_sname,
					 (char *)callstack[i] - (char *)info.dli_saddr);
			free(demangled);
		}
 else {
		   snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
					 i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
		}
		trace_buf << buf;
	}
	free(symbols);
	if (nFrames == MaxFramesCount)
		trace_buf << "[truncated]\n";
*/
return trace_buf.str();
	}

private:

};

void RRBacktrace()
{
 StackInspector2 ss;

auto start = std::chrono::steady_clock::now();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
ss.StoreBacktrace();
//	std::string bt = ss.GetBacktrace();
auto end = std::chrono::steady_clock::now();


//std::cout << bt << std::endl;
 std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
}


void foo()
{
RRBacktrace();
}

struct FooObject
{
	void bar() { foo(); std::cout << _i << std::endl; }

	int _i;
};


void bar(FooObject& bb) { bb.bar(); }

int main()
{
FooObject bb;
bar(bb);
}
