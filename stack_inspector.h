#pragma once

#include "backtrace.h"

#include <unordered_map>

class StackInspector
{
public:
	static constexpr std::size_t MaxFramesCount = 10;

	void StoreBacktrace()
	{
		Backtrace<MaxFramesCount> backtrace = Backtrace<MaxFramesCount>::ReadCurrentBacktrace();
		++s[backtrace];
	}

	template <typename StreamT>
	void ToStream(StreamT& stream)
	{
		for (const auto& p : s)
		{
			const auto& backtrace = p.first;
			const int calls = p.second;

			stream << "called " << calls << ":\n";
			backtrace.VisitDemangledSymbols([&stream](const void*, std::string_view symbol)
			{
				stream << symbol << "\n";
			});

			backtrace.VisitSymbols([&stream](const void*, std::string_view symbol)
			{
				stream << symbol << "\n";
			});
		}
	}

private:
	std::unordered_map<Backtrace<MaxFramesCount>, int> s;
};

