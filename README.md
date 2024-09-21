# Agave (TM) C++20 Standard Coroutine Library Framework



- Following the C++20 standard, it has excellent performance, is easy to use, and is suitable for use by coroutine library (middleware) developers and end users.

- Designed universal Coroutine Return types: AsyncAction, AsyncActionWithProgress, AsyncOperation, AsyncOperationWithProgress ...

- Almost all operations, such as environment switching, feature settings, etc., can be completed using only the standard keyword: co_await. Such as: co_await agave::resume_background(); co_await agave::resume_foreground(); Enter the background or foreground thread environment, co_wait agave::get_cancellation_token(); Obtain stop tokens, etc.

- Support custom progress types for progress mechanism, which can also be obtained and manipulated through the standard keyword: co_wait.

- Highly scalable design, all execution environments can be configured, such as front-end, back-end, time scheduling, etc., all of which can be configured to connect to custom efficient thread pools.


### Quick Start


```
//--------------------------------------------------------------------
//	demo.cpp.
//	09/27/2022.				created.
//	09/20/2024.				last modified.
//--------------------------------------------------------------------
//	*	Demonstrations of Agave(TM) Coroutine Framework 
//		(based on ISO C++20 or later).
//	*	if has any questions, 
//	*	please contact me at 'full1900@outlook.com'.
//	*	by bubo.
//--------------------------------------------------------------------
#include "Agave.hpp"
#include <iostream>

using namespace std::chrono_literals;

//--------------------------------------------------------------------
agave::AsyncOperation<int> foo(void);

//--------------------------------------------------------------------
int main(void)
{
	auto v = foo().get();           // classical blocked method.    
	std::cout << "got the value on thread: " << std::this_thread::get_id() << std::endl;
	std::cout << v << std::endl;
	return 0;
}

//--------------------------------------------------------------------
agave::AsyncAction action_async(void)
{
	std::cout << "action started on thread: " << std::this_thread::get_id() << std::endl;
	co_await agave::resume_background();
	std::cout << "do action in 2s... on thread: " << std::this_thread::get_id() << std::endl;
	co_await 2s;
	std::cout << "do action in 2s done... on thread: " << std::this_thread::get_id() << std::endl;
}

//--------------------------------------------------------------------
agave::AsyncOperation<int> operation_async(void)
{
	std::cout << "operation started on thread: " << std::this_thread::get_id() << std::endl;
	co_await agave::resume_background();
	std::cout << "do operation in 3s... on thread: " << std::this_thread::get_id() << std::endl;
	co_await 3s;

	co_return 51;
}

//--------------------------------------------------------------------
agave::AsyncOperation<int> foo(void)
{
    co_await action_async();
    auto&& v = co_await operation_async();    
    co_return v + 10;
}

//--------------------------------------------------------------------

```


  ### Contract and Approvals

   Submitted by          | Role | Contact | Date
  :--------------------: | :---: | :---: | :---:
  Bo Bu (full1900) | Agave (TM) Designer <br> and Programming Leader | full1900@outlook.com <br> full1900@vip.163.com | 22 Sep 2024




