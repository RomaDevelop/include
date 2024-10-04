#ifndef thread_box_h
#define thread_box_h

#include <iostream>
#include <thread>
#include <mutex>
#include <functional>

using stopper_t = volatile bool &;

class thread_box
{

public:
    volatile bool stopper = false;
    std::mutex mtx;

    inline thread_box(std::string name, int finish_policy = smart_policy):
	_name {name}, finish_policy {finish_policy}  {}
    inline ~thread_box();

    enum finish_policies { smart_policy, user_must_call_finish, no_checks_in_destructor };

    /// если в поток нужно передать условно бесконечную функцию
    /// флаг выхода ей ставим thread_box::stopper
    /// тогда мы можем сами прервать её работу вызвав finish(wait_time) в нужный момент
    /// или при уничтожении thread_box поток будет завершен автоматически вызовом finish(3000) - политика smart_policy
    /// после выполнения task флаг завершения потока будет установлен в true
    inline void start(std::function<void()> task);

    /// устанавливает флаг остановки работы потока и ожидает пока флаг завершения потока не подтвердит завершение
    inline bool finish(unsigned int wait_for_milliseconds = 1000);

    std::string name() { return _name; }
    bool was_started() { return thread_was_started; }
    bool ended() { return thread_ended; }
    bool finish_executed() { return _finish_executed; }

    inline static void test_thread_box(std::string name, std::string description, bool doFinish, std::function<void()> task);
    inline static void do_thread_box_tests();

private:
    volatile bool thread_was_started = false;
    volatile bool thread_ended = false;
    bool _finish_executed = false;
    std::string _name;
    int finish_policy = smart_policy;
    std::string errors;

    thread_box() = delete;
    thread_box(const thread_box &) = delete;
    thread_box(thread_box &&) = delete;
    thread_box& operator= (const thread_box& other) = delete;
    thread_box& operator= (thread_box&& other) = delete;
};

/*
GigaChat 26 июля в 16:27 
- видишь ли ты какие-либо проблемы и недостатки у класса thread_box?
- Да, я вижу несколько потенциальных проблем и недостатков у класса thread_box:
1. Неправильное использование stopper: Переменная stopper должна быть инициализирована в потоке, чтобы отмена работала корректно. В текущем виде, если stopper не будет инициализирована в потоке, отмена может не работать.
2. Отсутствие контроля над потоком: Класс не предоставляет возможности для присоединения к потоку или ожидания его завершения. Это может привести к утечкам ресурсов, если поток не завершится корректно.
3. Ограничение на тип функции: Методы start принимают только функции без аргументов или с одним аргументом stopper_t. Это ограничивает гибкость использования класса.
4. Обработка ошибок: Обработка ошибок в классе ограничена выводом сообщений в консоль. Было бы лучше использовать исключения или более структурированный подход к обработке ошибок.
5. Неоптимальное ожидание завершения потока: В методе finish используется цикл ожидания с фиксированным временем ожидания, что может привести к блокировке основного потока на неопределенное время.
6. Проблемы с безопасностью: Использование volatile для переменных thread_was_started, thread_ended и stopper может привести к проблемам с безопасностью данных, поскольку volatile не гарантирует атомарность операций.
7. Недостаточная документация: Класс не содержит достаточной документации, что затрудняет его использование и поддержку.
Эти проблемы могут быть решены путем улучшения дизайна класса и добавления дополнительных функций и механизмов безопасности.
*/

thread_box::~thread_box()
{
    if(finish_policy == no_checks_in_destructor) return;

    if(finish_policy == smart_policy)
    {
	if(thread_was_started && !thread_ended)
	    if(!finish(3000))
		std::cerr << "thread "+_name+" smart_policy can't finish thread\n";

	if(!errors.empty()) std::cerr << "thread "+_name+" errors: " + errors + "\n";
	return;
    }

    if(finish_policy == user_must_call_finish)
    {
	if(thread_was_started)
	{
	    if(!thread_ended)
	    {
		std::cerr << "thread "+_name+" destructor called, policy is user_must_call_finish, but thread is going\n";
		if(!_finish_executed)
		{
		    std::cerr << "thread "+_name+" finish was not did. Call finish(3000)\n";
		}
		else
		{
		    std::cerr << "thread "+_name+" finish did earlier, call finish again\n";
		}

		if(finish(3000)) std::cerr << "thread "+_name+" now finish did successful\n";
		else std::cerr << "thread "+_name+" call finish(3000) unsuccessfully\n";
	    }
	}

	if(!errors.empty()) std::cerr << "thread "+_name+" errors: " + errors + "\n";
	return;
    }
}

//void thread_box::start(std::function<void (stopper_t)> task)
//{
//    thread_was_started = true;
//    std::thread thread([this, task](){
//	task(stopper);
//	thread_ended = true;
//    });
//    thread.detach();
//}

void thread_box::start(std::function<void ()> task)
{
    thread_was_started = true;
    std::thread thread([this, task](){
	task();
	thread_ended = true;
    });
    thread.detach();
}

bool thread_box::finish(unsigned int wait_for_milliseconds)
{
    if(thread_was_started == false)
    {
	errors += "thread "+_name+" finish executed, but thread was not started\n";
	return errors.empty();
    }

    stopper = true;
    int count_waits = 100;
    int one_wait = wait_for_milliseconds / count_waits;

    for(int i=0; i<count_waits && !thread_ended; i++)
    {
	std::this_thread::sleep_for(std::chrono::milliseconds(one_wait));
    }

    if(!thread_ended)
	errors += "thread "+_name+" was not finished after waiting " + std::to_string(wait_for_milliseconds) + " milliseconds\n";

    _finish_executed = true;
    return errors.empty();
}



void thread_box::test_thread_box(std::string name, std::string description, bool doFinish, std::function<void ()> task)
{
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << name + " test start (" + description + ")\n";
    thread_box box(name);
    box.start(task);
    if(doFinish)
    {
	if(!box.finish()) std::cout << "finish error" << box.errors.c_str() << std::endl;
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
