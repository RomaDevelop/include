#ifndef async_mult_h
#define async_mult_h

class async_mult {
public:
	async_mult() : done(false) {
		worker = std::thread([this]() { this->run(); });
	}

	~async_mult() {
		{
			std::unique_lock<std::mutex> lock(mtx);
			done = true;
		}
		cv.notify_one();
		if (worker.joinable())
			worker.join();
	}

	template<typename Func, typename... Args>
	auto async(Func&& f, Args&&... args) {
		using RetType = std::invoke_result_t<Func, Args...>;
		auto task = std::make_shared<std::packaged_task<RetType()>>(
			std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
		);
		std::future<RetType> fut = task->get_future();
		{
			std::unique_lock<std::mutex> lock(mtx);
			tasks.push([task]() { (*task)(); });
		}
		cv.notify_one();
		return fut;
	}

private:
	void run() {
		while (true) {
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(mtx);
				cv.wait(lock, [this]() { return done || !tasks.empty(); });
				if (done && tasks.empty())
					return;
				task = std::move(tasks.front());
				tasks.pop();
			}
			task();
		}
	}

	std::thread worker;
	std::queue<std::function<void()>> tasks;
	std::mutex mtx;
	std::condition_variable cv;
	bool done;
};

#endif