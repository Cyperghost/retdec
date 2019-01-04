/**
* @file include/retdec/llvmir2hll/support/visitors/visitor_manager.h
* @brief A manager for the visitor, that visits everything, in multiply threads
*/

#ifndef RETDEC_LLVMIR2HLL_SUPPORT_MANAGER_VISITOR_MANAGER_H
#define RETDEC_LLVMIR2HLL_SUPPORT_MANAGER_VISITOR_MANAGER_H

#include <thread>
#include <vector>
#include <queue>
#include <retdec/llvmir2hll/support/visitable.h>

namespace retdec {
namespace llvmir2hll {
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
class VisitorManager {
public:
	VisitorManager();

	VisitorManager(unsigned cores);

	virtual ~VisitorManager() {
		stop(false);
	}

	/**
	 * @brief Clear the queue and free the pointer
	 */
	virtual void clear() {
		std::pair<Visitable *, std::function<void(Visitable *)> *> pair;
		while (queue.pop(pair))
			delete pair.second; // free function pointer, but not the element!
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
	virtual void push(Visitable * var, std::function<void(Visitable *)> * func);

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
	Queue<
			std::pair<
					Visitable *,
					std::function<void(Visitable *)> *
			>
	> queue;
	unsigned cores;
	std::mutex mutex;
	unsigned threadWaiting = 0;
	bool stopped = false, completed = false;
	std::condition_variable cvComplete, cv;
private:
	// this function should not call
	VisitorManager(const VisitorManager &) = delete;

	VisitorManager(VisitorManager &&) = delete;

	VisitorManager &operator=(const VisitorManager &) = delete;

	VisitorManager &operator=(VisitorManager &&) = delete;
};
} // namespace llvmir2hll
} // namespace retdec

#endif //RETDEC_VISITOR_MANAGER_H
