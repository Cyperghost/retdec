/**
* @file include/retdec/llvmir2hll/support/manager/visitor_manager.h
* @brief A manager for the visitor, that visits everything, in multiply threads
*/

#ifndef RETDEC_LLVMIR2HLL_SUPPORT_MANAGER_VISITOR_MANAGER_H
#define RETDEC_LLVMIR2HLL_SUPPORT_MANAGER_VISITOR_MANAGER_H

#include <thread>
#include <vector>
#include <queue>
#include "retdec/llvmir2hll/support/visitable.h"
#include "retdec/llvmir2hll/support/singleton.h"
#include "retdec/llvmir2hll/support/smart_ptr.h"

#define VISIT(param, obj) VisitorManager::getInstance().push(param, obj);
#define VISIT_THIS(obj) VISIT(this,obj)

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
class VisitorManagerWorker {
public:
	VisitorManagerWorker();

	VisitorManagerWorker(unsigned cores);

	virtual ~VisitorManagerWorker() {
		stop(false);
	}

	/**
	 * @brief Clear the queue and free the pointer
	 */
	virtual void clear() {
		std::pair<Visitor *, ShPtr<Visitable>> pair;
		while (queue.pop(pair));
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
	virtual void push(Visitor * var, ShPtr<Visitable> func);

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
					Visitor *,
					ShPtr<Visitable>
			>
	> queue;
	unsigned cores;
	std::mutex mutex;
	unsigned threadWaiting = 0;
	bool stopped = false, completed = false;
	std::condition_variable cvComplete, cv;
private:
	// this function should not call
	VisitorManagerWorker(const VisitorManagerWorker &) = delete;

	VisitorManagerWorker(VisitorManagerWorker &&) = delete;

	VisitorManagerWorker &operator=(const VisitorManagerWorker &) = delete;

	VisitorManagerWorker &operator=(VisitorManagerWorker &&) = delete;
};

using VisitorManager = Singleton<VisitorManagerWorker>;
} // namespace llvmir2hll
} // namespace retdec

#endif //RETDEC_VISITOR_MANAGER_H
