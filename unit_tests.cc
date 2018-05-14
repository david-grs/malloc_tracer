#include "stack_inspector.h"

#include "gtest/gtest.h"

#include <iostream>
#include <chrono>
#include <unordered_map>

TEST(BacktraceTest, Init)
{
	Backtrace<10> backtrace;
	EXPECT_EQ(0, backtrace.size());
}

TEST(BacktraceTest, InitRange)
{
	Backtrace<10> backtrace{{(void*)&::std::malloc, (void*)&std::free}};
	EXPECT_EQ(2, backtrace.size());
}

TEST(BacktraceTest, Hash)
{
	Backtrace<10> backtrace{{(void*)&::std::malloc, (void*)&std::free}};


	std::unordered_map<Backtrace<10>, int> ss;
	ss[backtrace]++;
	ss[backtrace]++;
	ss[backtrace]++;
	ss[backtrace]++;
	EXPECT_EQ(1, ss.size());
}

void bar(StackInspector&);

struct A
{
	A(StackInspector& ss) :
		mSS(ss)
	{}

	void foo() { bar(mSS); }

	StackInspector& mSS;
};

void bar(StackInspector& ss)
{
	ss.StoreBacktrace();
}

void buz(A&);

TEST(StackInspector, Read)
{
	static const int Iterations = 2;

	StackInspector ss;
	A a(ss);

	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < Iterations; ++i)
	{
		buz(a);
		ss.ToStream(std::cout);
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / Iterations << " us" << std::endl;
}

void buz(A& a)
{
	a.foo();
}
