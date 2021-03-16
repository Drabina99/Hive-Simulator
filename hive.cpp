#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <conio.h>
#include <time.h>
using namespace std;

typedef struct
{
	int no_of_beds; //liczba rabat
	int capacity; //pojemnosc ula
	int flowers_in_bed[100]; //ilosc kwiatow w danej rabacie
	int units_in_flower[100][100]; //ilosc jednostek nektaru w poszczegolnym kwiecie
	int distance[100]; //odleglosc od danej rabaty
	int no_of_bees; //aktualna ilosc pszczol
	int honey_units;
	//int is_process_terminated;
}HIVE;

HIVE* hive;
void printBeds(HIVE hive);
int nectar_before[100] = {0}; //tablica do zapisania ilosci nektaru w kazdej rabacie przed wylotem pszczol

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Nalezy podac 3 parametry!\nP1 - liczba rabat kwiatowych\nP2 - pojemnosc ula\nP3 - czas zycia roju\n\n");
		return 1;
	}
	HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HIVE), "Local\\Object");
	if(hMapFile == NULL)
	{
		printf("Could not create file mapping object (%d).\n", GetLastError());
		return 1;
	}
	hive = (HIVE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HIVE));
	if(hive == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		return 1;
	}
	hive->no_of_beds = atoi(argv[1]);
	hive->capacity = atoi(argv[2]);
	int wait_for_queen = atoi(argv[3]);
	HANDLE semHive = CreateSemaphore(NULL, hive->capacity, hive->capacity, "semHive");
	if(semHive == NULL)
	{
		printf("CreateSemaphore semHive error: %d\n", GetLastError());
		return 1;
	}
	HANDLE mutexExit = CreateMutex(NULL, FALSE, "mutexExit");
	if(mutexExit == NULL)
	{
		printf("CreateMutex mutexExit error: %d\n", GetLastError());
		return 1;
	}
	HANDLE mutexEnter = CreateMutex(NULL, FALSE, "mutexEnter");
	if(mutexEnter == NULL)
	{
		printf("CreateMutex mutexEnter error: %d\n", GetLastError());
		return 1;
	}
	HANDLE mutexFlower = CreateMutex(NULL, FALSE, "mutexFlower");
	if(mutexFlower == NULL)
	{
		printf("CreateMutex mutexFlower error: %d\n", GetLastError());
		return 1;
	}
	HANDLE semKiller = CreateSemaphore(NULL, 0, 1, "semKiller"); //semafor do usmiercenia roju, czekajacy w queen na zwolnienie z hive
	if(semKiller == NULL)
	{
		printf("CreateSemaphore semKiller error: %d\n", GetLastError());
		return 1;
	}
	HANDLE semFinish = CreateSemaphore(NULL, 0, 1, "semFinish"); //semafor do zakonczenia hive, czeka w hive na zwolnienie z queen
	if(semFinish == NULL)
	{
		printf("CreateSemaphore semFinish error: %d\n", GetLastError());
		return 1;
	}
	hive->no_of_bees = 0;
	hive->honey_units = 0;
	//hive->is_process_terminated = 0;
	
	srand((time(NULL)));
	
	for(int i = 0; i < hive->no_of_beds; i++)
	{
		hive->flowers_in_bed[i] = rand() % 100 + 1;
		hive->distance[i] = rand() % 241 + 10;
		
		for(int j = 0; j < hive->flowers_in_bed[i]; j++)
		{
			hive->units_in_flower[i][j] = rand() % 21 + 5;
			nectar_before[i] += hive->units_in_flower[i][j];
		}
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess("C:\\Users\\drabi\\Desktop\\SystemyOperacyjne\\queen\\Debug\\queen.exe", NULL, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
	Sleep(wait_for_queen);
	//hive->is_process_terminated = 1;
	if(!ReleaseSemaphore(semKiller, 1, NULL))
	{
		printf("ReleaseSemaphore semKiller error: %d \n", GetLastError());
	}

	DWORD dwWaitResult = WaitForSingleObject(semFinish, INFINITE);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		//printf("Terminating queen.exe \n");
		TerminateProcess(pi.hProcess, 0);
	}
	printf("------------------------------------------ \n");
	printf("Zebrano miodu: %d jednostek \n", hive->honey_units);
	printf("------------------------------------------ \n");
	CloseHandle(pi.hProcess);
	printBeds(*hive);
	//_getch();
	UnmapViewOfFile(hive);
	CloseHandle(hMapFile);
	CloseHandle(semKiller);
	CloseHandle(semHive);
	CloseHandle(semFinish);
	CloseHandle(mutexExit);
	CloseHandle(mutexEnter);
	
	return 0;
}

void printBeds(HIVE hive)
{
	printf("RABATA\tKWIATY\tNEKTAR PRZED\tNEKTAR PO\n");
	int nectar_after[100] = {0};
	for(int i = 0; i < hive.no_of_beds; i++)
	{
		for(int j = 0; j < hive.flowers_in_bed[i]; j++)
		{
			nectar_after[i] += hive.units_in_flower[i][j];
		}
		printf("%d\t%d\t%d\t\t%d\n", i, hive.flowers_in_bed[i], nectar_before[i], nectar_after[i]);
	}
	cout << endl;
}