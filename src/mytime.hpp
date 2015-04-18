#ifndef MYTIME_HPP_
#define MYTIME_HPP_

#include <iostream>
#include <deque>
#include <chrono>

using namespace std;
using namespace std::chrono;

template< typename D>
class MyTime
{

private:

  high_resolution_clock::time_point time_start;

	D time_ipo = 0.0f;
  D time_step = 1.0f / 30.0f;
  D time_last = 0.0f;
  D time_curr = 0.0f;
  D time_next = 0.0f;

	bool is_collecting_fps = false;
  deque<unsigned int> fpp_hold;
	float fps = 0.0f;
	unsigned int fps_framecount = 0;

public:

  MyTime()
  {
		time_start = high_resolution_clock::now();
	};

  bool update()
  {
		duration<D> time_dur = duration_cast<duration<D>>(
			high_resolution_clock::now() - time_start);

		time_last = time_curr;
		time_curr = time_dur.count();

		if (time_curr > time_next) // update
		{

			time_next += time_step;

			if (is_collecting_fps) fps_count_update();
			return true;
		}
		else // render
		{
			// calc render interpolation
			time_ipo = (time_curr - time_next + time_step ) / time_step;

			if (is_collecting_fps) fps_count_render();
			return false;
		}
	};

  void fps_count_render() { fps_framecount++; }
	void fps_count_update()
  {
		const unsigned int fppsize = fpp_hold.size();

		if ( fppsize > 29 ) fpp_hold.pop_back();

		fpp_hold.push_front( fps_framecount );

		unsigned int tally = 0;
		for (auto a : fpp_hold )
		{
			tally += a;
		}

		fps = (tally / float(fppsize)) / time_step;
		fps_framecount = 0;
	}

  void set_refresh_rate(D new_rate)
  {
		time_step = 1.0f / new_rate;
	};

	D get_change() const { return time_curr - time_last; };
  D get_current() const {return time_curr; };
	D get_fps() const { return fps; };
	D get_step() const { return time_step; };

	/// interpolations convenience

	D get_interpolation() const { return time_ipo; };
		//{ return (time_curr - time_next + time_step ) / time_step; };

	D ipo_time() const { return ipo(time_last,time_curr); };

	template<class T> T ipo(T a, T b) const
	{ return T(a + (b - a) * time_ipo); };

	template<class T> T ipoamt(T a, T b, D amt) const
	{ return T(a + (b - a) * amt); };
};

#endif
