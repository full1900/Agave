# Agave (TM) C++20 Standard Coroutine Library Framework    (by bubo)



- Following the C++20 standard, it has excellent performance, is easy to use, and is suitable for use by coroutine library (middleware) developers and end users.

- Designed universal Coroutine Return types: AsyncAction, AsyncActionWithProgress, AsyncOperation, AsyncOperationWithProgress ...

- Almost all operations, such as environment switching, feature settings, etc., can be completed using only the standard keyword: co_await. Such as: co_await agave::resume_background(); co_await agave::resume_foreground(); Enter the background or foreground thread environment, co_wait agave::get_cancellation_token(); Obtain stop tokens, etc.

- Support custom progress types for progress mechanism, which can also be obtained and manipulated through the standard keyword: co_wait.

- Highly scalable design, all execution environments can be configured, such as front-end, back-end, time scheduling, etc., all of which can be configured to connect to custom efficient thread pools.





