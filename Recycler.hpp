//--------------------------------------------------------------------
//	Recycler.hpp.
//	09/10/2022.				created.
//	09/03/2025.				last modified.
//--------------------------------------------------------------------
//	*	Recycler - A Part of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _RECYCLER_HPP__
#define _RECYCLER_HPP__


//--------------------------------------------------------------------
//	headers.
//--------------------------------------------------------------------
#include "LockFreeQueue.hpp"


//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	//	Generic Object Pool.
	//--------------------------------------------------------------------
	template <typename T>
	class Recycler
	{
	public:
		//--------------------------------------------------------------------
		Recycler(long long threshold, long long init_count = 0) :
			_threshold{ threshold }
		{
			_cache = std::shared_ptr<Queue<T>>(new Queue<T>);

			for (long long i = 0; i < init_count; ++i)
				_cache->push(new T);

			_cur_cnt.store(init_count);
		}

		//--------------------------------------------------------------------
		T* create(void)
		{
			if (auto data = _cache->pop())
			{
				--_cur_cnt;
				return data;
			}

			return new T;
		}

		//--------------------------------------------------------------------
		void recycle(T* v)
		{
			if (!v)
				return;

			if (_cur_cnt < _threshold)
			{
				_cache->push(v);
				++_cur_cnt;
				return;
			}

			delete v;
		}

		//--------------------------------------------------------------------
		std::list<T*> pop_all(void)
		{
			std::list<T*> data_list;
			while (!_cache->is_empty())
				if (auto data = _cache->pop())
					data_list.push_back(data);

			_cur_cnt = 0;

			return data_list;
		}

		//--------------------------------------------------------------------

	private:
		std::shared_ptr<Queue<T>>			_cache;
		long long							_threshold;
		std::atomic<long long>				_cur_cnt;

	};


	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------
#endif // !_RECYCLER_HPP__





