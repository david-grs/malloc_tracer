#include "backtrace.h"

#include <iostream>
#include <unordered_map>
#include <chrono>

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


void buz(StackInspector& ss)
{
	ss.StoreBacktrace();
}

void RRBacktrace()
{
	static const int Iterations = 1000;

	StackInspector ss;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < Iterations; ++i)
	{
		buz(ss);
		ss.ToStream(std::cout);
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / Iterations << " us" << std::endl;
}


void foo()
{
	RRBacktrace();
}

struct FooObject
{
	void bar()
	{
		foo();
		std::cout << _i << std::endl;
	}

	int _i;
};


void bar(FooObject& bb)
{
	bb.bar();
}

int main()
{
	FooObject bb;
	bar(bb);

	return 0;
}
