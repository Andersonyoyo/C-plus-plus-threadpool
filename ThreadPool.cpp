#include <cassert>  
#include "ThreadPool.h"  

using namespace zqw;
using namespace std;

ThreadPool::ThreadPool(const string &name) :
	m_name(name),
	m_maxQueueSize(0),
	m_running(false)
{

}

ThreadPool::~ThreadPool()
{
	if (m_running)
	{
		stop();
	}
}

void ThreadPool::setMaxQueueSize(int maxSize)
{
	m_maxQueueSize = maxSize;
}

void ThreadPool::start(int numThreads)
{
	assert(m_threads.empty());
	m_running = true;
	m_threads.reserve(numThreads);

	for (int i = 0; i < numThreads; ++i)
	{
		m_threads.push_back(thread(&ThreadPool::runInThread, this));
	}
}

void ThreadPool::stop()
{
	{
		unique_lock<mutex>  lock(m_mutex);
		m_running = false;
		m_notEmpty.notify_all();
	}

	for (size_t i = 0; i < m_threads.size(); ++i)
	{
		m_threads[i].join();
	}
}

void ThreadPool::run(const Task &f)
{
	if (m_threads.empty())
	{
		f();
	}
	else
	{
		unique_lock<mutex> lock(m_mutex);
		while (isFull())
		{
			m_notFull.wait(lock);
		}
		assert(!isFull());
		m_queue.push_back(f);
		m_notEmpty.notify_one();
	}
}

ThreadPool::Task ThreadPool::take()
{
	unique_lock<mutex> lock(m_mutex);

	while (m_queue.empty() && m_running)
	{
		m_notEmpty.wait(lock);
	}

	Task task;
	if (!m_queue.empty())
	{
		task = m_queue.front();
		m_queue.pop_front();

		if (m_maxQueueSize > 0)
		{
			m_notFull.notify_one();
		}
	}
	return task;
}

bool ThreadPool::isFull()
{
	return m_maxQueueSize > 0 && m_queue.size() >= m_maxQueueSize;
}


void ThreadPool::runInThread()
{
	try
	{
		while (m_running)
		{
			Task task = take();
			if (task)
			{
				task();
			}
		}
	}
	catch (const exception& ex)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
		fprintf(stderr, "reason: %s\n", ex.what());
		abort();
	}
	catch (...)
	{
		fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
	}
}