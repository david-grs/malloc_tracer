extern "C"
{
#include <execinfo.h> // backtrace
#include <dlfcn.h>    // dladdr
#include <cxxabi.h>   // __cxa_demangle
}

#include <array>
#include <string_view>
#include <cstring> // std::strlen
#include <cstdlib> // std::free
#include <algorithm> // std::equal
#include <initializer_list>

#include <boost/functional/hash.hpp>

template <std::size_t MaxFramesCount>
class Backtrace
{
public:
	Backtrace() :
		mFramesCount(0)
	{}

	template <typename Iterator>
	explicit Backtrace(Iterator begin, Iterator end) :
		mFramesCount(0)
	{
		auto itStack = mCallstack.begin();
		for (auto current = begin; current != end; ++current, ++itStack, ++mFramesCount)
		{
			*itStack = *current;
		}
	}

	explicit Backtrace(std::initializer_list<void*> range) :
		Backtrace(std::cbegin(range), std::cend(range))
	{ }

	static Backtrace ReadCurrentBacktrace()
	{
		Backtrace backtrace;
		backtrace.mFramesCount = ::backtrace(backtrace.mCallstack.data(), MaxFramesCount);
		return backtrace;
	}

	std::size_t size() const { return static_cast<std::size_t>(mFramesCount); }

	// Callable: fn(const void* address)
	template <typename Callable>
	void VisitAddresses(Callable visitor) const
	{
		for (int i = 1; i < mFramesCount; i++)
		{
			visitor(mCallstack[i]);
		}
	}

	// this function *does* allocate every time you call it.
	// Callable: fn(const void* address, std::string_view symbol)
	template <typename Callable>
	void VisitSymbols(Callable visitor) const
	{
		char **symbols = ::backtrace_symbols(mCallstack.data(), mFramesCount);

		for (int i = 1; i < mFramesCount; i++)
		{
			const std::size_t symbolLen = std::strlen(symbols[i]);
			const std::string_view symbol(symbols[i], symbolLen);
			visitor(mCallstack[i], symbol);
		}

		std::free(symbols);
	}

	// this function *does* allocate every time you call it.
	// Callable: fn(const void* address, std::string_view symbol)
	template <typename Callable>
	void VisitDemangledSymbols(Callable visitor) const
	{
		char **symbols = ::backtrace_symbols(mCallstack.data(), mFramesCount);

		for (int i = 1; i < mFramesCount; i++)
		{
			std::string_view symbol;
			Dl_info info;

			if (::dladdr(mCallstack[i], &info) && info.dli_sname && info.dli_sname[0] == '_')
			{
				// NOTE: this buffer is supposed to be on the heap, to allow abi::__cxa_demangle to realloc()
				// in case it is not big enough. this will crash. if this requirement isn't good enough for you,
				// simply change the below line to allocate.
				char buff[4096];
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

			visitor(mCallstack[i], symbol);
		}

		std::free(symbols);
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
	std::size_t operator()(const Backtrace<MaxFramesCount>& backtrace) const
	{
		std::size_t seed = 0;
		backtrace.VisitAddresses([&seed](const void* addr)
		{
			boost::hash_combine(seed, addr);
		});
		return seed;
	}
};

}
