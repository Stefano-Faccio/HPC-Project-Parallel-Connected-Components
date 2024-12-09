#pragma once

#include <vector>
#include <iostream>
#include <chrono>
#include <string>
#include <functional>
#include <iomanip>

template<typename OutStream, typename T>
OutStream& operator<< (OutStream& out, const std::vector<T>& v)
{
	for (auto const& tmp : v)
		out << tmp << ", ";
	return out;
}

namespace TimeUtils {
	template<class T>
	inline T measure(std::function<T()> timeable, double &timer) {
		auto start = std::chrono::high_resolution_clock::now();

		T res = timeable();

		std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
		timer = diff.count();

		return res;
	}

	template<>
	inline void measure(std::function<void()> timeable, double &timer) {
		auto start = std::chrono::high_resolution_clock::now();

		timeable();

		std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
		timer = diff.count();
	}

	template<class T>
	inline T profile(std::function<T()> timeable, std::string tag) {
#ifdef PROFILE_TIMING
		double timer;
		T res = measure<T>(timeable, timer);
		std::cout << tag << "," << timer << std::endl;
		return res;
#else
		return timeable();
#endif
	}

	template<>
	inline void profile(std::function<void()> timeable, std::string tag) {
#ifdef PROFILE_TIMING
		double timer;
		measure<void>(timeable, timer);
		std::cout << tag << "," << timer << std::endl;
#else
		return timeable();
#endif
	}

	inline void profileStep(std::function<void()> timeable, int rank, std::string tag) {
		double timer;
		measure<void>(timeable, timer);

#ifdef PROFILE_STEPS
		std::cout << rank << ","
				  << tag << ","
				  << timer << std::endl;
#endif
	}

	template<class T>
	inline T profileStep(std::function<T()> timeable, int rank, std::string tag) {
		double timer;
		T result = measure<T>(timeable, timer);

#ifdef PROFILE_STEPS
		std::cout << rank << ","
				  << tag << ","
				  << timer << std::endl;
#endif
		return result;
	}
}

namespace DebugUtils {
	inline void print(int rank, std::function<void(std::ostream &)> lambda) {
#ifdef DAINT_DEBUG
		std::cout << rank << "\t";
		lambda(std::cout);
		std::cout << std::endl;
#endif
	}
}

namespace MPIUtils {
	/**
	 * Calculate offset specification for non-uniform collectives
	 * @param sizes
	 * @return Memory offsets of groups of elements of the give `sizes`
	 */
	std::vector<int> prefix_offsets(const std::vector<int> & sizes);

	/**
	 * Calculate interval specification for CSR-style interval sepcification
	 * @param sizes
	 * @return Memory offsets of groups of elements of the give `sizes`
	 */
	std::vector<int> prefix_intervals(const std::vector<int> & sizes);
}

namespace CacheUtils {
	void trashCache(unsigned cache_size = 20 /* MB */);
}


