#ifndef _CLOCK_H_
#define _CLOCK_H_

// by yuwf qingting.water@gmail.com

// 时钟
#include <stdlib.h>
#if defined(_MSC_VER)
#include <intrin.h>  
#else
#include <x86intrin.h>
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define	FORCE_INLINE inline __attribute__((always_inline))
#endif

// 获取CPU当前频率值
FORCE_INLINE int64_t TSC()
{
	// rdtsc
	//return (int64_t)__rdtsc();

	// rdtscp
	uint32_t aux;
	return (int64_t)__rdtscp(&aux);
}

// 获取CPU妙的频率
extern int64_t TSCPerS();
// 获取CPU毫妙的频率
extern int64_t TSCPerMS();
// 获取CPU微妙的频率
extern int64_t TSCPerUS();

// struct Clock
// {
// 	int64_t epoch = 0;
// };

struct Ticker
{
	// 间隔tsc
	int64_t interval = 0;
	// 开始tsc
	int64_t last = 0;

	Ticker(int64_t milliseconds)
	{	
		interval = TSCPerMS() * milliseconds;
		last = LTSC() - rand()%interval; // 防止太固定了
	}

	bool Hit()
	{
		int64_t now = LTSC();
		if (LTSC() - last >= interval)
		{
			last = now;
			return true;
		}
		return false;
	}

	explicit operator bool()
	{
		return Hit();
	}
};


#endif