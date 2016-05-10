#include "stdafx.h"
#include <Windows.h>
#include <atomic>


// distribute int to various buckets.
struct SharedAtomicInt {
	// power of 2
	static const int64_t kBuckets_ = 16;
	std::atomic<int64_t> buckets_[kBuckets_];

	void inc(int64_t val) {
		//  this works only if kBuckets_ is a power of 2
		size_t bucket = std::hash<DWORD>()(GetCurrentThreadId()) & (kBuckets_ - 1);
		std::atomic_fetch_add(&buckets_[bucket], val);
	}

	// read couple of them and then extrapolate
	int64_t readFast() {	
		static const int numToRead = 4;
		int64_t value = 0;
		for (size_t i = 0; i < numToRead; i++) {
			value += buckets_[i].load(std::memory_order_relaxed);
		}
		// extrapolate
		return value  * (kBuckets_ / numToRead);
	}
	// this is slow...but accurate
	int64_t readFull() {
		int64_t value = 0;
		for (const auto& i : buckets_) {
			value += i.load(std::memory_order_relaxed);
		}
		return value;
	}

}; 