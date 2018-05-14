#pragma once

#include "backtrace.h"

#include <unordered_map>

#include <boost/circular_buffer.hpp>

class MallocProfiler
{
public:
	static constexpr std::size_t MaxFramesCount = 10;

	explicit MallocProfiler() :
		mLocalStorage(100000)
	{}

	void Poll()
	{
		for (const auto& p : mLocalStorage)
		{
			Allocation& alloc = mAllocations[p.second];
			alloc.mCount += 1;
			alloc.mBytes += p.first;
		}
	}

	template <typename StreamT>
	void ToStream(StreamT& stream) const
	{
		for (const auto& p : mAllocations)
		{
			const auto& backtrace = p.first;
			const int calls = p.second.mCount;
			const Bytes bytes = p.second.mBytes;

			stream << "called " << calls << ", allocated " << bytes << " bytes:\n";
			backtrace.VisitDemangledSymbols([&stream](const void*, std::string_view symbol)
			{
				stream << symbol << "\n";
			});

			//backtrace.VisitSymbols([&stream](const void*, std::string_view symbol)
			//{
			//	stream << symbol << "\n";
			//});

			stream << "\n\n";
		}
	}


	// mtrace handler
	void pre_malloc(size_t size)
	{
		mLocalStorage.push_back(std::make_pair(size, Backtrace<MaxFramesCount>::ReadCurrentBacktrace()));
	}
	void post_malloc(size_t /*size*/, const void* /*mem*/) {}

	void pre_free(const void* /*mem*/) {}
	void post_free(const void* /*mem*/) {}

	void pre_realloc(const void* /*mem*/, size_t size) { pre_malloc(size); }
	void post_realloc(const void* /*mem*/, size_t /*size*/, const void* /*new_mem*/) { }

private:
	using Stacktrace = Backtrace<MaxFramesCount>;
	using Bytes = std::size_t;

	boost::circular_buffer<std::pair<Bytes, Stacktrace>> mLocalStorage;

	struct Allocation
	{
		int mCount;
		Bytes mBytes;
	};
	std::unordered_map<Stacktrace, Allocation> mAllocations;
};

