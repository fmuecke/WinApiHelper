// Copyright 2015 Florian Muecke. All rights reserved.
#include <vector>

namespace StdHelper
{
	// Vector with range checking.
	// copied from Stroustrup: "A Tour of C++"
	template<typename T>
	class Vector : public std::vector<T>
	{
	public:
		using vector<T>::vector; // use the constructors from vector (under the name Vec)
		T& operator[](int i) // range check
		{ 
			return vector<T>::at(i); 
		}
		
		const T& operator[](int i) const // range check const objects; §4.2.1
		{ 
			return vector<T>::at(i); 
		}
	};
}