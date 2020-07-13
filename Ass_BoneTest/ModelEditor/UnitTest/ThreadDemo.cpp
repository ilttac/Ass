#include "stdafx.h"
#include "ThreadDemo.h"

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

void ThreadDemo::Initialize()
{
	//Thread();
	//Thread2();
	//Join();
	//Mutex();

	timer.Start([]()
	{
		printf("Timer\n");
	}, 2000, 2);

	timer2.Start([]()
	{
		printf("Timer2\n");
	}, 3000, 0);

	Performace perfomence;
	int arr[10000];

	perfomence.Start();
	{
		for (int i = 0; i < 10000; i++)
			arr[i] = Math::Random(0, 100000);

		sort(arr, arr + 10000);
	}
	printf("Running : %f\n", perfomence.Stop());
}

void ThreadDemo::Destroy()
{
}

void ThreadDemo::Update()
{
	m.lock();
	{
		progress += 0.01f;
	}
	m.unlock();
	
	ImGui::ProgressBar(progress / 1000);
}

void ThreadDemo::Render()
{
	

}

void ThreadDemo::Thread()
{
	for (int i = 0; i < 100; i++)
		printf("1 : %d\n", i);
	printf("반복문1 종료\n");

	for (int i = 0; i < 100; i++)
		printf("2 : %d\n", i);
	printf("반복문2 종료\n");
}

void ThreadDemo::Thread2()
{
	
	thread t(bind(&ThreadDemo::Thread2_1, this));
	thread t2(bind(&ThreadDemo::Thread2_2, this));

	t2.join();
	t.join();
	
}

void ThreadDemo::Thread2_1()
{
	for (int i = 0; i < 100; i++)
		printf("1 : %d\n", i);
	printf("반복문1 종료\n");
}

void ThreadDemo::Thread2_2()
{
	for (int i = 0; i < 100; i++)
		printf("2 : %d\n", i);
	printf("반복문2 종료\n");
}

void ThreadDemo::Join()
{
	thread t([]()
	{
		for (int i = 0; i < 100; i++)
			printf("1 : %d\n", i);
		printf("반복문1 종료\n");
	});

	thread t2([]()
	{
		int a = 0;

		while (true)
		{
			a++;
			printf("A : %d\n", a);

			Sleep(100);

			if (a > 30)
				break;
		}
	});

	t2.join();
	printf("Thread2 종료\n");

	t.join();
	printf("Thread1 종료\n");
}

void ThreadDemo::Mutex()
{
	thread t([=]
	{
		while (true)
		{
			Sleep(1000);
			printf("Progress : %f\n", progress);
		}

	});

	t.detach();
}


