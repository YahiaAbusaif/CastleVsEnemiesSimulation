#include <iostream>
#include "Header.h"
using namespace std;

void main ()
{
	int mode;
	cout << "Mode: ";
	cin >> mode;
	cin.ignore();

	if (mode == 1 || mode == 2) SetWindow();

	//Make a Castle and initialize it.
	castle castle;
	init_castle (castle);

	//Read the data from the file
	char filename[100];
	cout << "Enter name of Input file: ";
	cin.getline (filename, 100,'\n');
	float c1, c2, c3;
	ReadTheData (castle , c1 , c2 , c3, filename);

	int TimeStep = 1;  //set the time.

	//The War!!  ^_^
	while (!castle.enemy_victory){
		if (mode == 1 || mode == 2) DrawCastle(castle, TimeStep);
		active_enemy (castle.InActive, castle.towers, TimeStep);

		//to full the array of enemies for drawing them.
		int nEnemies = 0;
		for (int i = 0; i < 4; i++)
			nEnemies += castle.towers[i].nActive;

		if (nEnemies + castle.InActive.count == 0) break;
		enemy* enemies = new enemy [nEnemies];
		FillEnemies(castle.towers, nEnemies, enemies);

		if (mode == 1 || mode == 2) DrawEnemies (enemies, nEnemies);
		//to show the statistics on the window
		if (mode == 1 || mode == 2) print(castle.towers);

		//to edit the priority of shielded enemies and sort the active enemies.
		Set_priority (castle.towers, c1, c2, c3, TimeStep);
		for (int i = 0; i < 4; i++)
			Sort(castle.towers[i].Active);

		MoveEnemies (castle.towers, TimeStep);
		Fight (castle.towers, TimeStep);
		killed_enemy (castle, TimeStep);

		if (mode == 1)
		{
			cout << "\nPress ENTER to start motion demo";
			cin.get();
		}
		else if (mode == 2) Sleep(1000);

		//to check if a tower is totally damaged.
		for (int i = 0; i < 4; i++)
		{
			if (castle.towers[i].Health <= 0)
			{
				if (castle.towers[(i+1)%4].Health > 0) transport(castle.towers[i],castle.towers[(i+1)%4], castle.InActive);
				else if (castle.towers[(i+2)%4].Health > 0) transport(castle.towers[i],castle.towers[(i+2)%4], castle.InActive);
				else if (castle.towers[(i+3)%4].Health > 0) transport(castle.towers[i],castle.towers[(i+3)%4], castle.InActive);
				else
				{
					castle.enemy_victory = true;
					break;
				}
			}
		}

		TimeStep ++;
	}
	SaveToFile (castle);
}