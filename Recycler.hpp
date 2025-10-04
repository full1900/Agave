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
#include <atomic>
#include <memory>


//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	//	lock free mechanism.
	//--------------------------------------------------------------------
	//  Refer to the paper <Implementing Lock-Free Queues>
	//	by John D. Valois,
	//  Department of Computer Science Rensselaer Polytechnic Institute
	//	Troy, NY 12180.
	//	To appear in Proceedings of the Seventh International Conference on 
	//	Parallel and Distributed Computing Systems,
	//	Las Vegas, NV, October 1994.
	//--------------------------------------------------------------------
	template <typename T>
	class QueueNode
	{
	public:
		T* data;
		std::atomic<std::shared_ptr<QueueNode<T>>>	next;
	};


	//--------------------------------------------------------------------
	template <typename T>
	class Queue
	{
	public:
		//--------------------------------------------------------------------
		Queue(void)
		{
			_head = std::shared_ptr<QueueNode<T>>{ new QueueNode<T>{ nullptr, nullptr } };
			_tail.store(_head);
		}

		//--------------------------------------------------------------------
		void push(T* data)
		{
			/* preparing for adding a new node. */
			std::shared_ptr<QueueNode<T>> node{ new QueueNode<T>{ data, nullptr } };
			std::shared_ptr<QueueNode<T>> tail, next;

			while (true)
			{
				/*	take the pointer of tail and
					the pointer of next of tail. */
				tail = _tail.load();
				next = tail->next;

				/* try loop again, if the pointer of tail is changed. */
				if (_tail.load() != tail)
					continue;

				/* assist in advancing nodes of other threads.  */
				if (next)
				{
					_tail.compare_exchange_weak(tail, next);
					continue;
				}

				/* break the loop, if inserts node successfully. */
				if (tail->next.compare_exchange_weak(next, node))
					break;

			}

			_tail.compare_exchange_weak(tail, node);

		}

		//--------------------------------------------------------------------
		T* pop(void)
		{
			std::shared_ptr<QueueNode<T>> head, tail, next;
			T* data{ nullptr };

			while (true)
			{
				head = _head.load();
				tail = _tail.load();
				next = head->next;

				/* try loop again, if the pointer of head is changed. */
				if (_head.load() != head)
					continue;

				/* the queue is empty. */
				if (head == tail && !next)
					return nullptr;

				if (head == tail && next)
				{
					/* assist in advancing nodes of other threads.  */
					_tail.compare_exchange_weak(tail, next);
					continue;
				}

				if (_head.compare_exchange_weak(head, next))
				{
					data = next->data;
					break;
				}
			}

			return data;
		}

		//--------------------------------------------------------------------
		bool is_empty(void) const
		{
			return _head.load() == _tail.load();
		}

		//--------------------------------------------------------------------
	private:
		std::atomic<std::shared_ptr<QueueNode<T>>>		_head;
		std::atomic<std::shared_ptr<QueueNode<T>>>		_tail;

	};


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
		std::shared_ptr<Queue<T>>		_cache;
		long long						_threshold;
		std::atomic<long long>			_cur_cnt;

	};


	//--------------------------------------------------------------------

}


//--------------------------------------------------------------------
#endif // !_RECYCLER_HPP__





