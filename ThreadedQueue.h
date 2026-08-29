#include <deque>
#include <cstddef> //size_t
#include <stdexcept>
#include <tuple> //oder pair <utility>

#include <type_traits>
#include <condition_variable>
#include <mutex>

#include <optional>
#include <concepts>


/// <summary>
/// movebar und mit destruktor,
/// und per requires spaeter ggf mehr schablonenvorgaben
/// </summary>
template <typename ty>
concept QueueAble = std::move_constructible<ty> && std::destructible<ty>;

template<QueueAble ty, std::size_t uCapacity> requires (uCapacity > 0ull)
struct ThreadedQueue final{
	using value_type = ty;

	std::deque<ty> _data{};
	bool _closed{ false }; //stopped

	std::condition_variable _NotEmpty;
	std::condition_variable _NotFull;

	mutable std::mutex _mtx;
	static constexpr std::size_t capacity{ uCapacity };
public:
	ThreadedQueue() = default;
	ThreadedQueue(ThreadedQueue const&) = delete; //nocopy
	ThreadedQueue(ThreadedQueue&&) noexcept = delete; //nomove
	~ThreadedQueue() = default; //novirtual <- final

	ThreadedQueue& operator=(ThreadedQueue const&) = delete;
	ThreadedQueue& operator=(ThreadedQueue&&) noexcept = delete;

	//aufrufen mit std::move(element)
	[[nodiscard]] bool push(value_type input) {

		std::unique_lock guard{ _mtx };
		//solange voll, warten..
		_NotFull.wait(guard, [&]() { return _closed || _data.size() < capacity; });
		if (_closed) { 
			return false; 
		}
		
		_data.emplace_back(std::move<value_type>(input));
		guard.unlock();

		_NotEmpty.notify_one();

		return true;
	}

	//non-blocking push
	[[nodiscard]] bool try_push(value_type input) {
		{ //guard_scope
			std::lock_guard guard{ _mtx };
			//solange voll, warten..
			if (_closed || _data.size() < capacity)
			{
				return false;
			}

			_data.emplace_back(std::move<value_type>(input));
		}

		_NotEmpty.notify_one();

		return true;
	}

	template<typename... Args>
	requires std::constructible_from<value_type, Args&&...>
	[[nodiscard]] bool emplace(Args&&... args) {

		std::unique_lock guard{ _mtx };
		//solange voll, warten..
		_NotFull.wait(guard, [&]() { return _closed 
			|| (_data.size() + sizeof...(Args)) < capacity; 
			});
		if (_closed) {
			return false;
		}

		_data.emplace_back(std::forward<Args>(args)...);
		guard.unlock();

		_NotEmpty.notify_one();

		return true;
	}

	//Falls Laune: TODO: Try_Emplace, an sich wie der try_push, halt mit args..


	[[nodiscard]] std::optional<value_type> pop() {
		std::unique_lock guard{ _mtx };
		//solange leer, warten..
		_NotEmpty.wait(guard, [&]() {return _closed || _data.size()>0ull; });
		
		if (_closed) { return {}; };

		std::optional<value_type> front{ std::move(_data.front()) };
		_data.pop_front();

		guard.unlock();
		_NotFull.notify_one();

		return front;
	}

	[[nodiscard]] std::optional<value_type> try_pop() {
		
		std::lock_guard guard{ _mtx };
		if (_closed) { return {}; };
		if (_data.size() == 0ull) { return {}; }

		std::optional<value_type> front{ std::move(_data.front()) };
		_data.pop_front();

		_NotFull.notify_one();
		return front;
	}

	void close() {
		{
			std::lock_guard guard{ _mtx };
			if (_closed) { return; }
			_closed = true;
		}
		_NotFull.notify_all();
		_NotEmpty.notify_all();
	}



	[[nodiscard]] std::size_t size() const {
		std::lock_guard guard{ _mtx };
		return _data.size();
	}

	[[nodiscard]] std::size_t Capacity() const {
		std::lock_guard guard{ _mtx };
		return capacity;
	}

	[[nodiscard]] bool IsClosed() const {
		std::lock_guard guard{ _mtx };
		return _closed;
	}

	[[nodiscard]] bool empty() const {
		std::lock_guard guard{ _mtx };
		return _data.empty();
	}

	[[nodiscard]] bool full() const {
		std::lock_guard guard{ _mtx };
		return _data.size()>=capacity;
	}

};

//ThreadedQueue<int,1ull> qu;