/**
* @file include/retdec/utils/worker.h
* @brief A worker that do job multi threaded
*/

#include "retdec/utils/worker.h"
#include <iostream>
namespace retdec {
namespace utils {


Worker::Worker() {
	init(std::thread::hardware_concurrency());
}

Worker::Worker(unsigned cores) {
	init(cores);
}

void Worker::init(unsigned cores) {
	this->cores = cores;
	for (unsigned i = 0; i < cores; i++) {
		threads.push_back(std::thread(
				[this] {
					doWork();
				}
		));
	}
}

void Worker::doWork() {
	std::function<void()> func;
	bool success = queue.pop(func);
	while (true) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (stopped) return;
		}
		while (success) {
			func();
			{
				std::lock_guard<std::mutex> lock(mutex);
				if (stopped) {
					return;
				} else {
					success = queue.pop(func);
				}
			}
		}
		{
			std::lock_guard<std::mutex> lock(mutex);
			threadWaiting++;
			//should all threads wait and the queue is empty.
			//Set the flag the work is done
			if (threadWaiting == cores && queue.empty()) {
				completed = true;
				cvComplete.notify_all();
			}
		}
		{
			std::unique_lock<std::mutex> lock(waitMutex);
			cv.wait(lock, [this, &success, &func]() {
				        if (stopped)
					        return true;

				        success = queue.pop(func);
				        return success;
			        }
			);
		}
		{
			std::lock_guard<std::mutex> lock(mutex);
			threadWaiting--;
		}
	}
}

void Worker::push(std::function<void()> func) {
	queue.push(func);
	std::lock_guard<std::mutex> lock(mutex);
	completed = false;
	cv.notify_one();
}

void Worker::stop(bool wait) {
	if (!wait) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			clear();
			if (stopped) {
				return;
			}
			stopped = true;
		}
		cv.notify_all();
		//wait that all threads will stop
		for (int i = 0; i < static_cast<int>(threads.size()); ++i) {
			if (threads[i].joinable())
				threads[i].join();
		}
	} else {
		{
			std::lock_guard<std::mutex> lock(mutex);
			cv.notify_all();
		}
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (completed || stopped) {
				return;
			}
			if (threadWaiting == cores && queue.empty()) {
				return;
			}
		}
		std::unique_lock<std::mutex> lock(fMutex);
		cvComplete.wait(lock);
	}
}
} // namespace utils
} // namespace retdec