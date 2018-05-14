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
#include <boost/optional/optional.hpp>

template <std::size_t MaxFramesCount>
class Backtrace
{
public:
	static Backtrace ReadCurrentBacktrace()
	{
		Backtrace backtrace;
		backtrace.mFramesCount = ::backtrace(backtrace.mCallstack.data(), MaxFramesCount);
		return backtrace;
	}

	template <typename Callable>
	void VisitAddresses(Callable visitor) const
	{
		for (int i = 1; i < mFramesCount; i++)
		{
			visitor(mCallstack[i]);
		}
	}

	// this function *does* allocate every time you call it.
	// Callable: fn(std::string_view symbol)
	template <typename Callable>
	void VisitSymbols(Callable visitor) const
	{
		char **symbols = ::backtrace_symbols(mCallstack.data(), mFramesCount);

		for (int i = 1; i < mFramesCount; i++)
		{
			const std::size_t symbolLen = std::strlen(symbols[i]);
			const std::string_view symbol(symbols[i], symbolLen);
			visitor(symbol);
		}
	}

	template <typename Callable>
	void VisitDemangledSymbols(Callable visitor) const
	{
		char **symbols = ::backtrace_symbols(mCallstack.data(), mFramesCount);

		for (int i = 1; i < mFramesCount; i++)
		{
			std::string_view symbol;
			char buff[256];
			Dl_info info;

			if (::dladdr(mCallstack[i], &info) && info.dli_sname && info.dli_sname[0] == '_')
			{
				std::size_t length = sizeof(buff);
				int status;

				(void)abi::__cxa_demangle(info.dli_sname, buff, &length, &status);

				if (status == 0)
				{
					symbol = std::string_view(buff, std::strlen(buff));
				}
			}

			// we fallback to dli_sname, or to the symbol name
			if (symbol.empty() && info.dli_sname)
			{
				const char* symbolStr = info.dli_sname ? info.dli_sname : symbols[i];
				const std::size_t symbolLen = std::strlen(symbolStr);

				symbol = std::string_view(symbolStr, symbolLen);
			}

			visitor(symbol);
		}
	}

	bool operator==(const Backtrace& rhs) const
	{
		return mFramesCount == rhs.mFramesCount
				&& std::equal(mCallstack.cbegin(), mCallstack.cbegin() + mFramesCount, rhs.mCallstack.cbegin());
	}

private:
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
		Backtrace<MaxFramesCount> backtrace = Backtrace<MaxFramesCount>::ReadCurrentBacktrace();
		++s[backtrace];
	}

	void Dump()
	{
		for (const auto& p : s)
		{
			const auto& backtrace = p.first;
			const int calls = p.second;

			std::cout << "called " << calls << ":\n";
			backtrace.VisitDemangledSymbols([](std::string_view symbol)
			{
				std::cout << symbol << "\n";
			});
		}
	}


private:
	std::unordered_map<Backtrace<MaxFramesCount>, int> s;
};



void buz(StackInspector2& ss)
{
	ss.StoreBacktrace();
}


void RRBacktrace()
{
	StackInspector2 ss;
auto start = std::chrono::steady_clock::now();
for (int i = 0; i < 1; ++i)
buz(ss);
auto end = std::chrono::steady_clock::now();

ss.Dump();


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
