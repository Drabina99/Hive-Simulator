#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <conio.h>
#include <time.h>
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

HIVE* hive;

int HiveLeaving(bool newborn);
int NectarCollecting();
int ComingBack(int honey);

int main(int argc, char** argv)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Local\\Object");
	if(hMapFile == NULL)
	{
		printf("Could not open file mapping object (%d).\n", GetLastError());
		return 1;
	}
	hive = (HIVE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HIVE));
	if(hive == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		return 1;
	}
	srand((time(NULL)));
	bool newborn = true;
	while(true)
	{
		HiveLeaving(newborn);

		int honey = NectarCollecting(); //ilosc miodu zebranego przez pszczole

		if(ComingBack(honey) == -1) //powrot zakonczony niepowodzeniem
		{
			break;  //smierc pszczo³y
		}
		newborn = false;
	}
	UnmapViewOfFile(hive);
	CloseHandle(hMapFile);
	return 0;
}


int HiveLeaving(bool newborn)
{
	HANDLE mutexExit = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "mutexExit");
	if(mutexExit == NULL)
	{
		printf("OpenMutex mutexExit error: %d\n", GetLastError());
		return 1;
	}
	DWORD dwWaitResult = WaitForSingleObject(mutexExit, INFINITE);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		if(!newborn) //jesli nie jest nowopowita, to zjada przed wylotem 5 jednostek miodu
		{
			hive->honey_units -= 5;
		}
		Sleep(2);
	}
	ReleaseMutex(mutexExit);
	return 0;
}

int NectarCollecting()
{
	int honey_units = 5;
	int random_bed = rand() % hive->no_of_beds + 0; //losowy indeks rabaty od 0 do no_of_beds-1
	double fly_time = hive->distance[random_bed] / 5.0; //czas lotu w jedna strone to dystans podzielony przed predkosc, czyli 5 m/s
	int used_honey_units = hive->distance[random_bed] / 100 + 1; //+1, bo na kazde zaczete 100 metrow
	honey_units -= used_honey_units;
	Sleep(20 + fly_time);
	int taken_honey = 0;
	/*if(hive->is_process_terminated)
	{
		return 0;
	}*/
	HANDLE mutexFlower = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "mutexFlower");
	if(mutexFlower == NULL)
	{
		printf("OpenMutex mutexFlower error: %d\n", GetLastError());
		return 1;
	}
	DWORD dwWaitResult = WaitForSingleObject(mutexFlower, INFINITE);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		for(int i = 0; i < hive->flowers_in_bed[random_bed]; i++)
		{
			if(hive->units_in_flower[random_bed][i] > 0)
			{
				if(honey_units + hive->units_in_flower[random_bed][i] >= 100)
					taken_honey = 100 - honey_units;
				else
					taken_honey = hive->units_in_flower[random_bed][i];
				honey_units += taken_honey;
				hive->units_in_flower[random_bed][i] -= taken_honey;
			}
			if(honey_units >= 100)
			{
				if(honey_units > 100)
					honey_units = 100;
				break;
			}
		}
	}
	ReleaseMutex(mutexFlower);
	honey_units -= used_honey_units; //pszczola w drodze powrotnej tez zuzywa miod
	return honey_units;
}

int ComingBack(int honey)
{
	int survive = rand() % 10 + 0;
	/*if(hive->is_process_terminated)
	{
		return 0;
	}*/
	if(!survive)
	{
		HANDLE semHive = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "semHive");
		if(semHive == NULL)
		{
			printf("OpenSemaphore semHive error: %d\n", GetLastError());
			return 1;
		}
		hive->no_of_bees--;
		if(!ReleaseSemaphore(semHive, 1, NULL))
		{
			printf("ReleaseSemaphore semHive error: %d \n", GetLastError());
			return 1;
		}
		printf("Smierc pszczoly!\n");
		return -1;
	}
	printf("Udany powrot do ula. Jednostki miodu: %d\n", honey);
	HANDLE mutexEnter = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "mutexEnter");
	if(mutexEnter == NULL)
	{
		printf("OpenMutex mutexExit error: %d\n", GetLastError());
		return 1;
	}

	DWORD dwWaitResult = WaitForSingleObject(mutexEnter, INFINITE);
	if(dwWaitResult == WAIT_OBJECT_0)
	{
		hive->honey_units += honey;
		printf("MIOD W ULU: %d\n", hive->honey_units);
		Sleep(2000);
	}
	ReleaseMutex(mutexEnter);
	return 0;
}