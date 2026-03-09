#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <optional>

namespace ostd
{
	template<typename T>
	class AsyncJob
	{
		public:
		    AsyncJob() = default;

		    // Start a new job (callable must return T)
		    template<typename Callable, typename... Args>
		    void start(Callable&& func, Args&&... args)
		    {
		        // Reset state
		        m_done.store(false);
		        {
		            std::lock_guard<std::mutex> lock(m_mutex);
		            m_result.reset();
		        }

		        // Launch detached worker thread
		        std::thread([this, func = std::forward<Callable>(func), ... args = std::forward<Args>(args)]() mutable {
		            T value = func(args...);
		            {
		                std::lock_guard<std::mutex> lock(m_mutex);
		                m_result = std::move(value);
		            }
		            m_done.store(true);
		        }).detach();
		    }

		    inline bool isReady(void) const { return m_done.load(); }

		    // Retrieve the result (only valid if is_ready() == true)
		    inline T get(void)
		    {
		        std::lock_guard<std::mutex> lock(m_mutex);
		        return *m_result;
		    }

		    // Retrieve result as optional (safe even if not ready)
		    std::optional<T> tryGet(void)
			{
		        if (!m_done.load())
		            return std::nullopt;
		        std::lock_guard<std::mutex> lock(m_mutex);
		        return m_result;
		    }

		private:
			std::atomic<bool> m_done { false };
		    std::optional<T> m_result;
		    mutable std::mutex m_mutex;
	};
}
