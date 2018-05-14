#include "backtrace.h"

#include <iostream>
#include <unordered_map>
#include <chrono>

class StackInspector2
{
public:
	static constexpr std::size_t MaxFramesCount = 10;

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
