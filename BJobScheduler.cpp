//--------------------------------------------------------------------
//	BJobScheduler.cpp.
//	09/27/2022.				created.
//	08/05/2024.				last modified.
//--------------------------------------------------------------------
//	*	Job Scheduler - A Part of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "BJobScheduler.h"
#include <algorithm>
#include <execution>


//--------------------------------------------------------------------
using namespace std::chrono_literals;


//--------------------------------------------------------------------
// initialize static variables.
//--------------------------------------------------------------------
constinit std::shared_ptr<agave::details::BJobScheduler> agave::details::BJobScheduler::_b_job_scheduler{ nullptr };
std::mutex agave::details::BJobScheduler::_instance_mx;
constinit unsigned long long agave::details::BJobScheduler::_next_tok_id{ 1ull };


//--------------------------------------------------------------------
namespace agave::details
{
	//--------------------------------------------------------------------
	//	global entries.
	//--------------------------------------------------------------------
	std::function<void(std::function<void(void)>)>		__JobThread;
	std::function<void(std::function<void(void)>)>		__BGThread;
	std::function<void(std::function<void(void)>)>		__FGThread;

	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
agave::BJobToken::~BJobToken()
{
	//
}


//--------------------------------------------------------------------
agave::BJobToken::BJobToken(std::nullptr_t)
{
	//
}


//--------------------------------------------------------------------
agave::BJobToken::BJobToken(BJobToken const& other) : _tok_id{ other._tok_id }
{
	//
}


//--------------------------------------------------------------------
agave::BJobToken::BJobToken(BJobToken&& other) noexcept :
	_tok_id{ other._tok_id }
{
	//
}


//--------------------------------------------------------------------
agave::BJobToken&
agave::BJobToken::operator = (BJobToken const& other)
{
	_tok_id = other._tok_id;
	return *this;
}


//--------------------------------------------------------------------
agave::BJobToken&
agave::BJobToken::operator = (BJobToken&& other)
noexcept
{
	_tok_id = other._tok_id;
	return *this;
}


//--------------------------------------------------------------------
bool
agave::BJobToken::operator == (BJobToken const& other)
const
{
	return _tok_id == other._tok_id;
}


//--------------------------------------------------------------------
agave::BJobToken::operator bool()
const
{
	return _tok_id;
}


//--------------------------------------------------------------------
agave::BJobToken::BJobToken(unsigned long long tok_id) : _tok_id{ tok_id }
{
	//
}


//--------------------------------------------------------------------
auto
agave::details::BJobScheduler::instance_ptr(void) ->
std::shared_ptr<agave::details::BJobScheduler>
{
	if (!_b_job_scheduler)
	{
		std::unique_lock lck{ _instance_mx };
		if (!_b_job_scheduler)
			_b_job_scheduler = espresso::utilities::make_obj<BJobScheduler>(
				&BJobScheduler::delete_self);
	}

	return _b_job_scheduler;

}


//--------------------------------------------------------------------
void
agave::details::BJobScheduler::destroy_instance(void)
{
	std::unique_lock lck(_instance_mx);
	if (_b_job_scheduler)
		_b_job_scheduler = nullptr;

}


//--------------------------------------------------------------------
agave::BJobToken
agave::details::BJobScheduler::add_job(
	BDuration dur,
	BCallBack cb)
{
	std::unique_lock lck(_mx);
	auto job_tok = insert_new_job(dur, cb);
	_cv.notify_all();

	return job_tok;

}


//--------------------------------------------------------------------
bool
agave::details::BJobScheduler::remove_job(BJobToken const& tok)
{
	std::unique_lock lck(_mx);

	if (remove_job_by_token(tok))
	{
		_cv.notify_all();
		return true;
	}
	else if (std::get<BJobToken>(_cur_item) == tok)
	{
		_cur_item = { nullptr, std::chrono::high_resolution_clock::now(), nullptr };
		_cv.notify_all();
		return true;
	}

	return false;

}


//--------------------------------------------------------------------
bool
agave::details::BJobScheduler::clear_all_jobs(void)
{
	std::unique_lock lck(_mx);
	if (size_t size = _pending_jobs.size(); size > 0u)
	{
		_pending_jobs.clear();
		_cv.notify_all();
		return true;
	}

	return false;

}


//--------------------------------------------------------------------
agave::details::BJobScheduler::BJobScheduler(void)
{
	loop_jobs();
}


//--------------------------------------------------------------------
void
agave::details::BJobScheduler::delete_self(BJobScheduler* p)
{
	if (p)
		delete p;
}


//--------------------------------------------------------------------
agave::details::BJobScheduler::~BJobScheduler(void)
{
	_is_exit = true;
	_cv.notify_all();

	if (_th.joinable())
		_th.join();

}


//--------------------------------------------------------------------
inline
auto
agave::details::BJobScheduler::insert_new_job(
	BDuration& dur,
	BCallBack& fn) -> agave::BJobToken
{
	auto cur_tp = std::chrono::high_resolution_clock::now() + dur;
	auto it = std::find_if(
		//std::execution::par,
		_pending_jobs.begin(),
		_pending_jobs.end(),
		[&cur_tp](std::tuple<BJobToken, BTimePoint, BCallBack>& tup) -> bool
		{
			return cur_tp > std::get<BTimePoint>(tup);
		});

	auto tok = agave::BJobToken{ _next_tok_id++ };

	if (it == _pending_jobs.end())
		_pending_jobs.emplace_back(tok, cur_tp, fn);
	else
		_pending_jobs.emplace(it, tok, cur_tp, fn);

	return tok;

}


//--------------------------------------------------------------------
inline
void
agave::details::BJobScheduler::insert_new_job(std::tuple<BJobToken, BTimePoint, BCallBack> task_tup)
{
	auto cur_tp = std::get<BTimePoint>(task_tup);
	auto it = std::find_if(
		//std::execution::par,
		_pending_jobs.begin(),
		_pending_jobs.end(),
		[&cur_tp](std::tuple<BJobToken, BTimePoint, BCallBack>& tup) -> bool
		{
			return cur_tp > std::get<BTimePoint>(tup);
		});

	if (it == _pending_jobs.end())
		_pending_jobs.emplace_back(task_tup);
	else
		_pending_jobs.emplace(it, task_tup);

}


//--------------------------------------------------------------------
inline
bool
agave::details::BJobScheduler::remove_job_by_token(BJobToken const& tok)
{
	auto it = std::remove_if(
		//std::execution::par,
		_pending_jobs.begin(),
		_pending_jobs.end(),
		[&tok](std::tuple<BJobToken, BTimePoint, BCallBack>& tup) -> bool
		{
			return std::get<BJobToken>(tup) == tok;
		});

	if (it == _pending_jobs.end())
		return false;

	while (it != _pending_jobs.end())
		it = _pending_jobs.erase(it);

	return true;

}


//--------------------------------------------------------------------
void
agave::details::BJobScheduler::loop_jobs(void)
{
	std::thread th([this](void) -> void
		{
			while (true)
			{
				std::unique_lock lck(_mx);

				if (_is_exit.load())
				{
					_pending_jobs.clear();
					break;
				}
				else if (_pending_jobs.size() == 0)
				{
					_cv.wait(lck);
				}
				else
				{
					_cur_item = _pending_jobs.front();
					_pending_jobs.pop_front();

					if (std::get<BTimePoint>(_cur_item) - std::chrono::high_resolution_clock::now() > 1ms)
						_cv.wait_until(lck, std::get<BTimePoint>(_cur_item));

					if (std::get<BJobToken>(_cur_item))
					{
						if (std::get<BTimePoint>(_cur_item) - std::chrono::high_resolution_clock::now() < 1ms)
						{
							auto bcb = std::get<BCallBack>(_cur_item);

							if (__JobThread)
								__JobThread([bcb]() { bcb(); });
							else
							{
								std::thread t(bcb);
								t.detach();
							}

						}
						else // insert into the task pending queue again.
							insert_new_job(_cur_item);

						_cur_item = { nullptr, std::chrono::high_resolution_clock::now(), nullptr };

					}

				}
			}

		});

	_th = std::move(th);

}


//--------------------------------------------------------------------





