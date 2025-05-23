#ifndef thread_box_h
#define thread_box_h

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

using uint = unsigned int;

class thread_box
{
public:
	volatile bool stopper = false;
	std::mutex mtx;
	std::condition_variable cv;

	/// smart_policy - если поток был запущен и не закончил работу, пробует остановать 3 секунды и выдаёт ошибки если не получилось
	/// user_must_call_finish - если поток был запущен и не закончил работу, выдаёт ошибки и пробует остановать 3 секунды
	/// no_checks_in_destructor - ничего не делает
	enum destructor_policies { smart_policy, user_must_call_finish, no_checks_in_destructor };

	explicit inline thread_box(): m_name {}, m_finish_policy {smart_policy}  {}
	explicit inline thread_box(std::string aName, destructor_policies finish_policy = smart_policy):
		m_name {std::move(aName)}, m_finish_policy {finish_policy}  {}
	inline ~thread_box();

	/// если в поток нужно передать условно бесконечную функцию, флаг завершения её работы ей ставим thread_box::stopper
	/// тогда мы можем сами прервать её работу вызвав finish(wait_time) в нужный момент
	/// или при уничтожении thread_box поток будет завершен автоматически вызовом finish(3000) - политика smart_policy
	/// после выполнения task флаг завершения потока будет установлен в true
	inline void start(std::function<void()> task);

	/// устанавливает флаг stopper, уведомляет всех клиентов cv и ожидает пока флаг thread_ended не подтвердит завершение
	/// выдаёт ошибки если поток и не был запущен, если не завершился
	inline bool finish(uint wait_for_milliseconds = 1000);

	void set_name(std::string aName) { m_name = std::move(aName); }

	std::string name() { return m_name; }
	destructor_policies finish_policy() { return m_finish_policy; }
	bool was_started() { return thread_was_started; }
	bool ended() { return thread_ended; }
	bool finish_executed() { return m_finish_executed; }

	inline static void test_thread_box(std::string name, std::string description, bool doFinish, std::function<void()> task);
	inline static void do_thread_box_tests();

private:
	std::string m_name;
	destructor_policies m_finish_policy = smart_policy;

	volatile bool thread_was_started = false;
	volatile bool thread_ended = false;
	volatile bool m_finish_executed = false;

	//std::string errors;

	thread_box(const thread_box &) = delete;
	thread_box(thread_box &&) = delete;
	thread_box& operator= (const thread_box& other) = delete;
	thread_box& operator= (thread_box&& other) = delete;
};

/*
чат ревью 26 июля 2024 в 16:27 
Класс не предоставляет возможности для присоединения к потоку или ожидания его завершения. Это может привести к утечкам ресурсов, если поток не завершится корректно.
	в классе есть функция finish, для безопасного завершения работы потока и проверки на утечки ресурсов
Ограничение на тип функции: Методы start принимают только функции без аргументов или с одним аргументом stopper_t. Это ограничивает гибкость использования класса.
	шаблон std::function позволяет использовать лямбды с захватом аргументов, что достаточно гибко. Может потом сделаю шаблонный метод start
Обработка ошибок: Обработка ошибок в классе ограничена выводом сообщений в консоль. Было бы лучше использовать исключения или более структурированный подход к обработке ошибок.
	?
Неоптимальное ожидание завершения потока: В методе finish используется цикл ожидания с фиксированным временем ожидания, что может привести к блокировке основного потока на неопределенное время.
	Не на неопределенное, а пока не истечёт максимально установленный срок.
	2024.12 оптимизировано ожидание завершения, с постепенно увеличивающимися интервалами, для эффективного завершения быстрозавершаемых потоков
Проблемы с безопасностью: Использование volatile для переменных thread_was_started и др может привести к проблемам с безопасностью данных, поскольку volatile не гарантирует атомарность операций
	?
Недостаточная документация: Класс не содержит достаточной документации, что затрудняет его использование и поддержку.
	Документация добавлена
Создание mutex и condition_variable, хотя не факт, что они будут использованы при работе task
	Можно хранить их в private по умного указателю и создавать когда потребуются, а в public сделать функции доступа к ним,
	но тогда будет лишний вызов функции (допустим она будет встроена) и в этой функции будет проверка их существования
	Возможно ли создать шаблонный класс, чтобы можно было при инстанцировании шаблона указать есть ли в нём mutex и condition_variable?
*/

thread_box::~thread_box()
{
	if(m_finish_policy == smart_policy)
	{
		if(thread_was_started && !thread_ended)
			if(!finish(3000))
				std::cerr << "thread "+m_name+" smart_policy can't finish thread\n";
	}
	else if(m_finish_policy == user_must_call_finish)
	{
		if(thread_was_started && !thread_ended)
		{
			std::cerr << "thread "+m_name+" destructor called, policy is user_must_call_finish, but thread is going\n";
			if(!m_finish_executed)
			{
				std::cerr << "thread "+m_name+" finish was not did. Call finish(3000)\n";
			}
			else
			{
				std::cerr << "thread "+m_name+" finish did earlier, call finish again\n";
			}

			if(finish(3000)) std::cerr << "thread "+m_name+" now finish did successful\n";
			else std::cerr << "thread "+m_name+" call finish(3000) unsuccessfully\n";
		}
	}
	else if(m_finish_policy == no_checks_in_destructor) { }
	else std::cerr << "thread "+m_name+" wrong policy " << m_finish_policy << "\n";
}

void thread_box::start(std::function<void ()> task)
{
	if(m_name.empty()) std::cerr << "thread_box with empty name starting\n";
	if(thread_was_started)
	{
		std::cerr << "thread "+m_name+" start executed, but thread already started\n";
		return;
	}

	thread_was_started = true;
	std::thread thread([this, task](){
		task();
		thread_ended = true;
	});
	thread.detach();
}

bool thread_box::finish(uint wait_for_milliseconds)
{
	m_finish_executed = true;

	if(thread_was_started == false)
	{
		std::cerr << "thread "+m_name+" finish executed, but thread was not started\n";
		return false;
	}

	stopper = true;
	cv.notify_all();
	uint one_wait = 1;
	uint wait_fakt = 0;
	uint add = 1;

	while(!thread_ended && wait_fakt < wait_for_milliseconds)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(one_wait));
		wait_fakt += one_wait;
		one_wait += add;
		add < 10 ? add += 1 : add < 100 ? add += 10 : add += 30;
	}

	if(thread_ended)
	{
		std::cout << "thread " << m_name << " " << "waited " << wait_fakt << " milliseconds for finish and finished\n";
		return true;
	}
	else
	{
		std::cerr << "thread "+m_name+" was not finished after waiting " + std::to_string(wait_fakt) + " milliseconds\n";
		return false;
	}
}

void thread_box::test_thread_box(std::string name, std::string description, bool doFinish, std::function<void ()> task)
{
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << name + " test start (" + description + ")\n";
	thread_box box(name);
	box.start(task);
	if(doFinish)
	{
		if(!box.finish()) std::cout << "finish error" << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	std::cout << name + " test end\n";
	std::cout << "--------------------------------------------------------------------------------\n";
}

void thread_box::do_thread_box_tests()
{
	//    std::array<bool,6> tests { 0,0,0,0,0,1 };

	//    if(tests[0])
	//    {
	//	auto task = [](){ while(1) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); } };
	//	test_thread_box("a","endless cycle, no stopper, no call finish", false, task);
	//    }

	//    if(tests[1])
	//    {
	//	auto task = [](){ while(1) { std::this_thread::sleep_for(std::chrono::milliseconds(500));} };
	//	test_thread_box("b","endless cycle, no stopper, has call finish", true, task);
	//    }

	//    if(tests[2])
	//    {
	//	auto task = [](){ for(int i=0; i<3; i++) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); } };
	//	test_thread_box("c","ending cycle, no stopper, has call finish", true, task);
	//    }

	//    if(tests[3])
	//    {
	//	auto task = [](stopper_t stopper){ while(!stopper) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); } };
	//	test_thread_box("d","endless cycle, has stopper, no call finish", false, task);
	//    }

	//    if(tests[4])
	//    {
	//	auto task = [](stopper_t stopper){ while(!stopper) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); } };
	//	test_thread_box("e","endless cycle, has stopper, has call finish", true, task);
	//    }

	//    if(tests[5])
	//    {
	//	auto task = [](){ for(int i=0; i<10; i++) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); } };
	//	test_thread_box("f","ending cycle, no stopper, no call finish", false, task);
	//    }
}

#endif // thread_box_h
