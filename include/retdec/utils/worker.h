/**
* @file include/retdec/llvmir2hll/llvm/llvmir2bir_converters/worker.h
* @brief A multi thread worker
*/

#ifndef RETDEC_WORKER_H
#define RETDEC_WORKER_H


#include <thread>
#include <vector>
#include <queue>
#include "retdec/llvmir2hll/support/visitable.h"
#include "retdec/llvmir2hll/support/singleton.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"

namespace retdec {
namespace utils {
/**
 * @brief Make the standard queue thread safe
 */
template<typename T>
class Queue {
public:
	bool push(T const &value) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(value);

		return true;
	}

	bool pop(T &result) {
		std::unique_lock<std::mutex> lock(mutex);
		if (queue.empty()) {

			return false;
		}
		result = queue.front();
		queue.pop();

		return true;
	}

	bool empty() {
		std::unique_lock<std::mutex> lock(mutex);

		return queue.empty();
	}

private:
	std::queue<T> queue;
	std::mutex mutex;
};

/**
 * @brief A visitor manager handle the visit calls in multiply threads
 */
class Worker {
public:
	Worker();

	Worker(unsigned cores);

	virtual ~Worker() {
		stop(false);
	}

	/**
	 * @brief Clear the queue and free the pointer
	 */
	virtual void clear() {
		std::function<void()> func;
		while (queue.pop(func));
	}

	/**
	 * @brief Wait until all threads complete
	 * @param wait: if `true` then the function will wait until the queue is empty
	 * 				if `false` then the function will clear the queue and wait that all
	 * 				threads complete
	 */
	virtual void stop(bool wait = false);

	/**
	 * @brief Return the number of cores for this manager
	 * @return
	 */
	unsigned int getCores() const {
		return cores;
	}

	/**
	 * @brief Insert a task that should be completed
	 */
	virtual void push(std::function<void()> func);

protected:
	/**
	 * @brief Init the threads by given core count
	 */
	virtual void init(unsigned cores);

	/**
	 * @brief This worker wait for new elements in the queue and execute it
	 */
	virtual void doWork();

	std::vector<std::thread> threads;
	//the queue with our function and the parameter that we will call
	Queue<std::function<void()>> queue;
	unsigned cores;
	std::mutex mutex, waitMutex, fMutex;
	unsigned threadWaiting = 0;
	bool stopped = false, completed = true;
	std::condition_variable cvComplete, cv;
private:
	// this function should not call
	Worker(const Worker &) = delete;

	Worker(Worker &&) = delete;

	Worker &operator=(const Worker &) = delete;

	Worker &operator=(Worker &&) = delete;
};
} // namespace utils
} // namespace retdec

#endif //RETDEC_WORKER_H
