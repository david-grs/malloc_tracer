#include "malloc_tracer.h"
#include "mtrace.h"

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

std::unique_ptr<int> ss;

void bar();

struct A
{
	void foo() { bar(); }
};

void bar()
{
	ss = std::make_unique<int>(5);
}

void buz(A&);

TEST(MallocTracer, Read)
{
	static const int Iterations = 1000;

	mtrace<MallocTracer> mt;
	A a;

	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < Iterations; ++i)
	{
		std::malloc(10);
		buz(a);
	}
	auto end = std::chrono::steady_clock::now();
	mt.handler().Poll();
	mt.handler().ToStream(std::cout);

	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / Iterations << " us" << std::endl;
}

void buz(A& a)
{
	a.foo();
}
