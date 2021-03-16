#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <conio.h>
#include <time.h>
#include <vector>
using namespace std;

typedef struct
{
	int no_of_beds;
	int capacity;
	int flowers_in_bed[100]; //ilosc kwiatow w danej rabacie
	int units_in_flower[100][100]; //ilosc jednostek nektaru w poszczegolnym kwiecie
	int distance[100]; //odleglosc od danej rabaty
	int no_of_bees;
	int honey_units;
	//int is_process_terminated;
}HIVE;

vector<DWORD> workers;
DWORD KillerFunction(LPVOID lpParam);

int main(int argc, char** argv)
{
	//printf("start queen\n");
	HANDLE ThreadKiller = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillerFunction, NULL, 0, NULL); //watek czekajacy na uplyniecie czasu w hive.exe
	if(ThreadKiller == NULL)
	{
		printf("CreateThread error: %d\n", GetLastError());
		return 1;
	}
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Local\\Object");
	if(hMapFile == NULL)
	{
		printf("Could not open file mapping object (%d).\n", GetLastError());
		return 1;
	}
	HIVE* hive = (HIVE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HIVE));
	if(hive == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		return 1;
	}

	srand(time(NULL));
	workers.reserve(20);
	while(true)
	{
		HANDLE semHive = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "semHive");
		if(semHive == NULL)
		{
			printf("OpenSemaphore semHive error: %d\n", GetLastError());
			return 1;
		}

		DWORD dwWaitResult = WaitForSingleObject(semHive, INFINITE);
		if(dwWaitResult == WAIT_OBJECT_0)
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			int wait_for_new_bee = rand() % 101 + 100;
			Sleep(wait_for_new_bee);
			hive->no_of_bees++;
			CreateProcess("C:\\Users\\drabi\\Desktop\\SystemyOperacyjne\\worker\\Debug\\worker.exe", NULL, NULL, NULL, true, 0, NULL, NULL, &si, &pi);
			if(workers.size() == workers.capacity())
			{
				workers.resize(workers.capacity() + 20);
			}
			workers.push_back(pi.dwProcessId);
		}
		else
		{
			printf("dwWaitResult (%d) in queen.\n", dwWaitResult);
		}
	}

	UnmapViewOfFile(hive);
	CloseHandle(hMapFile);
	CloseHandle(ThreadKiller);

	return 0;
}

DWORD KillerFunction(LPVOID lpParam)
{
	HANDLE semKiller = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "semKiller");
	if(semKiller == NULL)
	{
		printf("OpenSemaphore error: %d\n", GetLastError());
		return FALSE;
	}
	DWORD dwWaitResult = WaitForSingleObject(semKiller, INFINITE);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		printf("Terminating worker.exe (%d workers)\n", workers.size());
		for(int i = 0; i < workers.size(); i++)
		{
			HANDLE hWorker = OpenProcess(PROCESS_ALL_ACCESS, false, workers[i]);
			if(hWorker == NULL)
			{
				printf("OpenProcess error %d\n", GetLastError());
			}
			TerminateProcess(hWorker, 1);
		}
	}
	CloseHandle(semKiller);

	HANDLE semFinish = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "semFinish");
	if(semFinish == NULL)
	{
		printf("OpenSemaphore semFinish error: %d\n", GetLastError());
		return FALSE;
	}
	if(!ReleaseSemaphore(semFinish, 1, NULL))
	{
		printf("ReleaseSemaphore semFinish error: %d \n", GetLastError());
	}
	return 0;
}