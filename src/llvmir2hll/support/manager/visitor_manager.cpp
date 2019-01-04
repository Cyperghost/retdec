/**
* @file include/retdec/llvmir2hll/support/visitors/visitor_manager.h
* @brief A manager for the visitor, that visits everything, in multiply threads
*/

#include <retdec/llvmir2hll/support/manager/visitor_manager.h>
#include <iostream>

namespace retdec {
namespace llvmir2hll {


VisitorManager::VisitorManager() {
	init(std::thread::hardware_concurrency());
}

VisitorManager::VisitorManager(unsigned cores) {
	init(cores);
}

void VisitorManager::init(unsigned cores) {
	this->cores = cores;
	for (unsigned i = 0; i < cores; i++) {
		threads.push_back(std::thread(
				[this] {
					doWork();
				}
		));
	}
}

void VisitorManager::doWork() {
	std::pair<Visitable *, std::function<void(Visitable *)> *> result;
	bool success = queue.pop(result);
	while (!stopped) {
		while (success) {
			// will free the pointer automatically
			std::unique_ptr<std::function<void(Visitable *)>> uniquePtr(result.second);
			(*result.second)(result.first);
			if (!stopped) {
				return;
			} else {
				success = queue.pop(result);
			}
		}
		std::unique_lock<std::mutex> lock(mutex);
		threadWaiting++;
		//should all threads wait and the queue is empty.
		//Set the flag the work is done
		if(threadWaiting == cores && queue.empty()){
			cvComplete.notify_all();
			completed = true;
		}
		this->cv.wait(lock, [this, &success, &result]() {
			              success = queue.pop(result);
			              return success || stopped || completed;
		              }
		);
		threadWaiting--;
	}
}

void VisitorManager::push(Visitable *var, std::function<void(Visitable *)> *func) {
	queue.push(std::make_pair(var, func));
	std::unique_lock<std::mutex> lock(mutex);
	completed = false;
	cv.notify_one();
}

void VisitorManager::stop(bool wait) {
	if (!wait) {
		if (stopped) {
			return;
		}
		stopped = true;
		clear();
		{
			std::unique_lock<std::mutex> lock(mutex);
			cv.notify_all();
		}
		//wait that all threads will stop
		for (int i = 0; i < static_cast<int>(threads.size()); ++i) {
			if (threads[i].joinable())
				threads[i].join();
		}
	} else {
		if (completed || stopped) {
			return;
		}
	}
	std::unique_lock<std::mutex> lock(mutex);
	cvComplete.wait(lock);
}
} // namespace llvmir2hll
} // namespace retdec