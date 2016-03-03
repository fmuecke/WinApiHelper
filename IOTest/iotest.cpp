#include "stdafx.h"
#include <CppUnitTest.h>
#include "../WinUtil/System.IO.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace io = WinUtil::System::IO;

namespace IOTest
{		
	TEST_CLASS(FindFileTest)
	{
	public:
		
		TEST_METHOD(PatternMatchTest)
		{
            Assert::IsFalse(io::IsPatternMatch("", "*"), L"empty string never matches");
            Assert::IsFalse(io::IsPatternMatch("", "?"), L"empty string never matches");
            Assert::IsFalse(io::IsPatternMatch("", "*.*"), L"empty string never matches");
            Assert::IsTrue(io::IsPatternMatch("abc", "a*"), L"string not beginning with a");
            Assert::IsTrue(io::IsPatternMatch("a", "a*"), L"string not beginning with a");
            Assert::IsTrue(io::IsPatternMatch("abc", "*c"), L"string not ending with c");
            Assert::IsTrue(io::IsPatternMatch("c", "*c"), L"string not ending with c");
            Assert::IsTrue(io::IsPatternMatch("bc", "?c"), L"string not ending with c");
            Assert::IsTrue(io::IsPatternMatch("ab", "a?"), L"string not beginning with a");

		}

	};
}