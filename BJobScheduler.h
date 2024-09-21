//--------------------------------------------------------------------
//	BJobScheduler.h.
//	09/27/2022.				created.
//	08/05/2024.				last modified.
//--------------------------------------------------------------------
//	*	Job Scheduler - A Part of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#pragma once

#ifndef _BJOB_SCHEDULER_H__
#define _BJOB_SCHEDULER_H__


//--------------------------------------------------------------------
//	headers.
//--------------------------------------------------------------------
#include <memory>
#include <mutex>
#include <functional>
#include <thread>
#include <list>
#include "B_Object.hpp"


//--------------------------------------------------------------------
namespace agave
{
	//--------------------------------------------------------------------
	// incomplete types.
	//--------------------------------------------------------------------
	namespace details
	{
		class BJobScheduler;
	}


	//--------------------------------------------------------------------
	//	 Token for BJobScheduler
	//--------------------------------------------------------------------
	class BJobToken
	{
		friend details::BJobScheduler;

	public:
		~BJobToken();
		BJobToken(std::nullptr_t);
		BJobToken(BJobToken const& other);
		BJobToken(BJobToken&& other) noexcept;

		BJobToken& operator = (BJobToken const& other);
		BJobToken& operator = (BJobToken&& other) noexcept;

		bool operator == (BJobToken const& other) const;
		operator bool() const;

	private:
		BJobToken(unsigned long long tok_id);

	private:
		unsigned long long						_tok_id{ 0ull };


	};


	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
namespace agave::details
{
	//--------------------------------------------------------------------
	//	custom types.
	//--------------------------------------------------------------------
	using BTimePoint = std::chrono::high_resolution_clock::time_point;
	using BDuration = std::chrono::high_resolution_clock::duration;
	using BCallBack = std::function<void(void)>;


	//--------------------------------------------------------------------
	//	extern global entries.
	//--------------------------------------------------------------------
	extern std::function<void(std::function<void(void)>)>		__JobThread;
	extern std::function<void(std::function<void(void)>)>		__BGThread;
	extern std::function<void(std::function<void(void)>)>		__FGThread;


	//--------------------------------------------------------------------
	//	job scheduler.
	//--------------------------------------------------------------------
	class BJobScheduler : public espresso::utilities::B_Object<BJobScheduler>
	{
		DefineMakeObjFriend;

	public:
		static auto instance_ptr(void) -> std::shared_ptr<BJobScheduler>;
		static void destroy_instance(void);

		BJobToken add_job(BDuration dur, BCallBack cb);
		bool remove_job(BJobToken const& tok);
		bool clear_all_jobs(void);

	private:
		BJobScheduler(void);

		BJobScheduler(BJobScheduler const& other) = delete;
		BJobScheduler(BJobScheduler&& other) = delete;
		static void delete_self(BJobScheduler* p);

		~BJobScheduler(void);

		auto insert_new_job(
			BDuration& dur,
			BCallBack& fn) -> BJobToken;
		void insert_new_job(std::tuple<BJobToken, BTimePoint, BCallBack> task_tup);
		bool remove_job_by_token(BJobToken const& tok);
		void loop_jobs(void);

	private:
		static std::shared_ptr<BJobScheduler>			_b_job_scheduler;
		static std::mutex								_instance_mx;
		static unsigned long long						_next_tok_id;

		std::mutex										_mx;
		std::condition_variable							_cv;
		std::list<std::tuple<BJobToken, BTimePoint, BCallBack>>
			_pending_jobs;
		std::tuple<BJobToken, BTimePoint, BCallBack>	_cur_item{ nullptr, std::chrono::high_resolution_clock::now(), nullptr };
		std::atomic<bool>								_is_exit{ false };
		std::thread										_th;


	};


	//--------------------------------------------------------------------


}


//--------------------------------------------------------------------
#endif // !_BJOB_SCHEDULER_H__





