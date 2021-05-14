#pragma once

#include <sstream>
#include <stdint.h>
#include <iostream>
#include <inttypes.h>
#include <functional>

#define DEFAULT_MEASURES_FORMATTER measures::measure_formatter_mks_c

#if defined(_DEBUG) || defined(DEBUG)
#	define DEBUG_SCOPED_MEASURE(measure_name) measures::scoped_measure_c<> msr(measure_name);
#	define DEBUG_SCOPED_FUNCTION_MEASURE DEBUG_SCOPED_MEASURE(__FUNCTION__);
#else
#	define DEBUG_SCOPED_MEASURE(measure_name)
#	define DEBUG_SCOPED_FUNCTION_MEASURE
#endif

#define SCOPED_FUNCTION_MEASURE measures::scoped_measure_c<> msr(__FUNCTION__);
#define SCOPED_MEASURE(measure_name) measures::scoped_measure_c<> msr(measure_name);

namespace measures
{

#ifdef WIN32
typedef void(__stdcall *g_measure_output_target_fn_t)(const char*);
#else
typedef void(*g_measure_output_target_fn_t)(const char*);
#endif

#ifdef WIN32
static LARGE_INTEGER g_msr_frequency =
	[]() -> LARGE_INTEGER { LARGE_INTEGER freq; QueryPerformanceFrequency(&freq); return freq; }();
#endif
extern g_measure_output_target_fn_t g_measure_output_target_fn;

static void set_measure_output_target(g_measure_output_target_fn_t target)
{
	g_measure_output_target_fn = target;
}


#ifndef WIN32
union LARGE_INTEGER
{
	struct u
	{
		uint32_t LowPart;
		uint32_t HighPart;
	};

	uint64_t QuadPart;
};

void QueryPerformanceCounter(LARGE_INTEGER* performance_count_ptr)
{
	struct timespec ts;
#if 1
	clock_gettime(CLOCK_REALTIME, &ts);
#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

	performance_count_ptr->QuadPart = ts.tv_nsec / 1000;
	performance_count_ptr->QuadPart += ts.tv_sec * 1000000;
}
#endif


template <typename T>
std::string split_digit_by_triads(T value)
{
	std::stringstream ss;

	ss.imbue(std::locale(""));
	ss << std::fixed << value;

	return ss.str();
}


class basic_measure_c
{
	friend class measure_formatter_secs_c;
	friend class measure_formatter_ms_c;
	friend class measure_formatter_mks_c;

protected:
	std::string m_name;

	LARGE_INTEGER m_starting_time, m_ending_time, m_elapsed_microseconds;
};


class measure_formatter_secs_c
{
public:
	static std::string format(basic_measure_c* measure_ptr)
	{
		std::string buf;
		buf.resize(64);

#ifdef WIN32
		sprintf_s(&buf[0],
			buf.size(),
#else
		sprintf(&buf[0],
#endif
			"[%s] measured: %" PRIu64 " seconds\n",
			measure_ptr->m_name.c_str(),
			(uint64_t)(measure_ptr->m_elapsed_microseconds.QuadPart / 1000. / 1000.));
		return buf;
	}
};


class measure_formatter_ms_c
{
public:
	static std::string format(basic_measure_c* measure_ptr)
	{
		std::string buf;
		buf.resize(64);

#ifdef WIN32
		sprintf_s(&buf[0],
			buf.size(),
#else
		sprintf(&buf[0],
#endif
			"[%s] measured: %s milliseconds\n",
			measure_ptr->m_name.c_str(),
			split_digit_by_triads(
				(uint64_t)(measure_ptr->m_elapsed_microseconds.QuadPart / 1000.)
				).c_str());
		return buf;
	}
};


class measure_formatter_mks_c
{
public:
	static std::string format(basic_measure_c* measure_ptr)
	{
		std::string buf;
		buf.resize(128);

#ifdef WIN32
		sprintf_s(&buf[0],
			buf.size(),
#else
		sprintf(&buf[0],
#endif
			"[%s] measured: %s microseconds\n",
			measure_ptr->m_name.c_str(),
			split_digit_by_triads(measure_ptr->m_elapsed_microseconds.QuadPart).c_str());
		return buf;
	}
};


template <class FormatterT = DEFAULT_MEASURES_FORMATTER>
class scoped_measure_c
	: public basic_measure_c
{
public:
	scoped_measure_c(const std::string& name)
	{
		m_name = name;
		QueryPerformanceCounter(&m_starting_time);
	}

	~scoped_measure_c()
	{
		QueryPerformanceCounter(&m_ending_time);

		m_elapsed_microseconds.QuadPart = m_ending_time.QuadPart - m_starting_time.QuadPart;
#ifdef WIN32
		m_elapsed_microseconds.QuadPart *= 1000000;
		m_elapsed_microseconds.QuadPart /= g_msr_frequency.QuadPart;
#endif

		g_measure_output_target_fn(FormatterT::format(this).c_str());
	}

private:
};


template <class FormatterT = DEFAULT_MEASURES_FORMATTER>
class no_scoped_measure_c
	: public basic_measure_c
{
public:
	no_scoped_measure_c() {}
	no_scoped_measure_c(const std::string& name) { m_name = name; }
	~no_scoped_measure_c() {}

	void set_name(const std::string& name) { m_name = name; }

	uint64_t get_duration_mks()
	{
		return m_elapsed_microseconds.QuadPart;
	}

	uint64_t get_duration_ms()
	{
		return (uint64_t)(m_elapsed_microseconds.QuadPart / 1000.);
	}

	uint64_t get_duration_secs()
	{
		return (uint64_t)(m_elapsed_microseconds.QuadPart / 1000. / 1000.);
	}

	void start_measure()
	{
		QueryPerformanceCounter(&m_starting_time);
	}

	void end_measure()
	{
		QueryPerformanceCounter(&m_ending_time);

		m_elapsed_microseconds.QuadPart = m_ending_time.QuadPart - m_starting_time.QuadPart;
#ifdef WIN32
		m_elapsed_microseconds.QuadPart *= 1000000;
		m_elapsed_microseconds.QuadPart /= g_msr_frequency.QuadPart;
#endif
	}

	void output_result()
	{
		g_measure_output_target_fn(FormatterT::format(this).c_str());
	}
};

}


namespace benchmark
{

#define RUN_BENCHMARK(func_call, n_times, benchmark_name)							\
{																					\
	measures::scoped_measure_c<DEFAULT_MEASURES_FORMATTER> measure(benchmark_name);	\
	for (uint64_t i = 0; i < n_times; ++i)											\
	{																				\
		func_call;																	\
	}																				\
}

template <class FormatterT = DEFAULT_MEASURES_FORMATTER>
static void run_benchmark(std::function<void()> func, uint64_t n_times,
	const std::string& benchmark_name)
{
	measures::scoped_measure_c<FormatterT> measure(benchmark_name);

	for (uint64_t i = 0; i < n_times; ++i)
	{
		func();
	}
}

}
