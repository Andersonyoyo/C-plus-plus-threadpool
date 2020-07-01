//ע�⣺
//1.Ϊ�̳߳��������֮ǰһ��Ҫ����setMaxQueueSize������������пɴ�ŵ����������
//2 ��������start�����̣߳����̳߳��˻�Ϊ���߳�
#include<thread>
#include<mutex>
#include<functional>
#include<string>
#include<condition_variable>
#include<deque>
#include<vector>
#include<memory>


namespace zqw
{
	class ThreadPool
	{
	public:
		typedef std::function<void()> Task;		//��functionģ�嶨��һ��void�͵�Task����

		explicit ThreadPool(const std::string &name = std::string());
		~ThreadPool();

		void start(int numThreads);//�����߳���������numThreads���߳�
		void stop();//�̳߳ؽ���
		void run(const Task& f);//����f���̳߳�������
		void setMaxQueueSize(int maxSize);//����������пɴ�����������

	private:
		bool isFull();//��������Ƿ�����
		void runInThread();//�̳߳���ÿ��threadִ�е�function
		Task take();//���������ȡ��һ������

		std::mutex m_mutex;
		std::condition_variable m_notEmpty;
		std::condition_variable m_notFull;
		std::string m_name;
		std::vector<std::thread> m_threads;
		std::deque<Task> m_queue;
		size_t m_maxQueueSize;
		bool m_running;

	private:
		ThreadPool(const ThreadPool& x) = delete;//�������俽��
		ThreadPool& operator=(const ThreadPool& x) = delete;
	};
}