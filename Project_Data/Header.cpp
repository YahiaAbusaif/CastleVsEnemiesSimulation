#include "Header.h"

void ReadTheData (castle &castle, float &c1, float &c2, float &c3, char filename[])
{
	ifstream input;
	input.open (filename , ios::in);
	if(!(input.is_open()))
	{
		cout << "File not found !";
		return;
	}
	char region = 'A';
	float towerHealth;
	int n, towerPower; //n for number of enemies a tower can shoot.
	input >> towerHealth;
	input >> n;
	input >> towerPower;
	for(int i = 0 ; i < 4; i++){
		castle.towers[i].Region = region; 
		castle.towers[i].Health = towerHealth;
		castle.towers[i].first_health = towerHealth;
		castle.towers[i].N = n;
		castle.towers[i].Tower_Fire_Power = towerPower;
		region++;
	}
	input >> c1 >> c2 >> c3;
	int index;
	input >> index;
	while (index != -1){
		enemy E;
		E.S = index;
		input >> E.Type;
		input >> E.Time;
		input >> E.Health;
		input >> E.Enemy_Fire_Power;
		input >> E.Reload_period;
		input >> E.Region;
		init_enemy(E);
		enqueue(E, castle.InActive);
		input >> index;
	}
}

//A function to fill array of total active enemies.
void FillEnemies (tower towers[], int &Ecount, enemy Enemies[])
{
	int j = 0;
	for (int i = 0; i < 4; i++)
	{
		enemy* temp = towers[i].Active;
		while (temp)
		{
			Enemies[j] = *temp;
			j++;
			temp = temp->link;
		}
	}
}

//A function to transport the enemies from their region if they damage their tower.
void transport (tower & tower1, tower & tower2, queue InActive)
{
	while (tower1.Active)
	{
		tower1.Active->Region = tower2.Region;
		if (tower1.Active->Distance < tower2.unpaved) tower1.Active->Distance = tower2.unpaved + 1;
		if (tower1.Active->Type == 2) InsertAtHead (Delete(tower1.Active), tower2.Active);
		else InsertAtTail (Delete(tower1.Active), tower2.Active);
		tower2.nActive++;
		tower1.nActive--;
	}
	enemy * temp = front(InActive);
	while (temp)
	{
		if (temp->Region == tower1.Region) temp->Region = tower2.Region;
		temp = temp->link;
	}
}

//A function to initialize the needed characterstics of enemy.
void init_enemy (enemy &E)
{
	E.Fight_Delay = -1;
	E.Kill_Delay = -1;
	E.Total_Fight_Time = 0;
	E.first_time = 0;
	E.Kill_time_step = 0;
	E.Distance = 60;
	if(E.Type != 2)
	{
		E.priority = 1;
		E.k = 1;
	}
	else 
	{
		E.priority = 0;
		E.k = 2;
	}
}

//A function to initialize the needed characterstics of castle.
void init_castle (castle &castle)
{
	castle.Xstrt = CastleXStrt;
	castle.Ystrt = CastleYStrt;
	castle.W = CastleWidth;
	castle.L = CastleLength;
	castle.enemy_victory = false;
	initqueue (castle.InActive);
	castle.total_killed = NULL;
	for (int i = 0; i < 4; i++)
	{
		castle.towers[i].unpaved = 30;
		castle.towers[i].nActive = 0;
		castle.towers[i].count_killed = 0;
		castle.towers[i].temp_count = 0;
		castle.towers[i].TL = TowerLength;
		castle.towers[i].TW = TowerWidth;
		castle.towers[i].Active = NULL;
	}
}

//A function to edit the priority of Shielded enemies.
void Set_priority (tower towers[], float c1, float c2, float c3, int TimeStep)
{
	enemy* temp = NULL;
	for (int i = 0; i < 4; i++)
	{
		temp = towers[i].Active;
		while (temp && temp->Type == 2)
		{
			int remaining_time = (temp->Reload_period - ((TimeStep - temp->Time) % temp->Reload_period));
			if (((TimeStep - temp->Time) % temp->Reload_period) == 0) remaining_time = 0;
			temp->priority = (((temp->Enemy_Fire_Power / temp->Distance)* c1) + (c2 / (remaining_time + 1)) + (temp->Health* c3));
			temp = temp->link;
		}
	}
}

void MoveEnemies (tower towers[], int TimeStep)
{
	enemy *temp = NULL;
	for (int i = 0; i < 4; i++)
	{
		temp = towers[i].Active;
		while (temp)
		{
			if (temp->Type == 0)
			{
				if (((TimeStep - temp->Time) % temp->Reload_period) == 0) 
				{
					towers[i].unpaved -= temp->Enemy_Fire_Power;
					if (towers[i].unpaved < 2) towers[i].unpaved = 2;
					temp->Distance -= temp->Enemy_Fire_Power;
					if (temp->Distance < 2) temp->Distance = 2;
				}
				else 
					if (temp->Distance > towers[i].unpaved && temp->Distance > 2) temp->Distance--;
			}
			else
				if (temp->Distance > towers[i].unpaved && temp->Distance > 2) temp->Distance--;
			temp = temp->link;
		}
	}
}

//A function to check the active enemies in inactive list 
void active_enemy (queue &InActive, tower towers[], int TimeStep)
{
	while(front(InActive) && front(InActive)->Time == TimeStep)
	{
		if (front(InActive)->Region == 'A') 
		{
			if (front(InActive)->Type == 2) InsertAtHead (dequeue(InActive), towers[0].Active);
			else InsertAtTail (dequeue(InActive), towers[0].Active);
			towers[0].nActive++;
		}
		else if (front(InActive)->Region == 'B') 
		{
			if (front(InActive)->Type == 2) InsertAtHead (dequeue(InActive), towers[1].Active);
			else InsertAtTail (dequeue(InActive), towers[1].Active);
			towers[1].nActive++;
		}
		else if (front(InActive)->Region == 'C') 
		{
			if (front(InActive)->Type == 2) InsertAtHead (dequeue(InActive), towers[2].Active);
			else InsertAtTail (dequeue(InActive), towers[2].Active);
			towers[2].nActive++;
		}
		else
		{
			if (front(InActive)->Type == 2) InsertAtHead (dequeue(InActive), towers[3].Active);
			else InsertAtTail (dequeue(InActive), towers[3].Active);
			towers[3].nActive++;
		}
	}
}

//A function to shoot at most N enemies.
void Fight (tower towers[], int TimeStep)
{
	int count; // to calculate the number of enemies a tower shot.
	enemy *temp = NULL;

	for (int i = 0; i < 4; i++)
	{
		count = 0; 
		temp = towers[i].Active; 

		//Tower attacks N enemies.
		while (count <= towers[i].N && temp)
		{
			temp->Health -= (towers[i].Tower_Fire_Power / (temp->Distance * temp->k));
			if (temp->first_time == 0) temp->first_time = TimeStep;
			if (temp->Fight_Delay == -1) temp->Fight_Delay = TimeStep - temp->Time;
			temp = temp->link;
			count++;
		}

		//Enemies attack the tower.
		temp = towers[i].Active; //Reset temp to the beginning of Active list.
		while (temp)
		{
			if (((TimeStep - temp->Time) % temp->Reload_period) == 0)
				if (temp->Type != 0) towers[i].Health -= ((temp->k * temp->Enemy_Fire_Power) / temp->Distance);
			temp = temp->link;
		}
	}
}

//A function to check the Active enemies if killed.
void killed_enemy (castle &castle, int TimeStep)
{
	bool found;

	for (int i = 0; i < 4; i++)
	{
		castle.towers[i].temp_count = 0;
		found = false; 
		while (castle.towers[i].Active && castle.towers[i].Active->Health <= 0)
		{
			castle.towers[i].Active->Kill_time_step = TimeStep;  
			castle.towers[i].Active->Kill_Delay = TimeStep - castle.towers[i].Active->first_time;
			castle.towers[i].Active->Total_Fight_Time = castle.towers[i].Active->Kill_Delay + castle.towers[i].Active->Fight_Delay;
			insert_sort_killed (Delete(castle.towers[i].Active), castle.total_killed); 
			castle.towers[i].nActive--;
			castle.towers[i].temp_count++;
			castle.towers[i].count_killed++;
			found = true;  
		}  
		if (castle.towers[i].Active == NULL) return;  

		enemy *temp = castle.towers[i].Active;  
		enemy *& temp1 = castle.towers[i].Active->link;  
		while (temp->link != NULL)  
		{  
			found = false;  
			if (temp1->Health <= 0)  
			{  
				temp1->Kill_time_step = TimeStep;  
				temp1->Kill_Delay = TimeStep - temp1->first_time;
				temp1->Total_Fight_Time = temp1->Kill_Delay + temp1->Fight_Delay;
				insert_sort_killed (Delete(temp1), castle.total_killed); 
				temp->link = temp1; 
				castle.towers[i].nActive--;
				castle.towers[i].temp_count++;
				castle.towers[i].count_killed++;
				found = true;  
			}    
			if (! found) temp = temp->link; 
		} 
	}
}

//A function to initialize a queue.
void initqueue (queue &q)
{
	q.count = 0;
	q.front = NULL;
	q.rear = NULL;
}

bool Is_Empty (queue q)
{
	return (q.count == 0);
}

void enqueue (enemy E, queue &q)
{
	enemy* temp = new enemy; 
	*temp = E;
	temp->link = NULL; 
	if (q.front == NULL) q.front = temp; 
	else q.rear->link = temp; 
	q.rear = temp; 
	(q.count)++; 
}

enemy dequeue (queue &q)
{
	enemy * temp = new enemy;
	temp = q.front;
	q.front = q.front->link;
	if (q.front == NULL) q.rear = NULL;
	(q.count) --;
	enemy tmp = *temp;
	delete temp;
	return tmp;
}

enemy* front (queue q)
{
	if (q.front == NULL) return NULL;
	return q.front;
}

enemy* rear (queue q)
{
	if (q.rear == NULL) return NULL;
	return q.rear;
}

void print (tower towers[])
{
	cout << "Total current enemies: " << "A = " << towers[0].nActive << "  ";
	cout << "B = " << towers[1].nActive << "  ";
	cout << "C = " << towers[2].nActive << "  ";
	cout << "D = " << towers[3].nActive << endl;
	cout << "Last time step killed: " << "A = " << towers[0].temp_count << "  ";
	cout << "B = " << towers[1].temp_count << "  ";
	cout << "C = " << towers[2].temp_count << "  ";
	cout << "D = " << towers[3].temp_count << endl;
	cout << "Total killed: " << "A = " << towers[0].count_killed << "  ";
	cout << "B = " << towers[1].count_killed << "  ";
	cout << "C = " << towers[2].count_killed << "  ";
	cout << "D = " << towers[3].count_killed << endl;
	cout << "Unpaved distance: " << "A = " << towers[0].unpaved << "  ";
	cout << "B = " << towers[1].unpaved << "  ";
	cout << "C = " << towers[2].unpaved << "  ";
	cout << "D = " << towers[3].unpaved << endl;
}

void SaveToFile (castle castle)
{
	ofstream outputfile ("output.txt", ios::out);
	enemy *& temp = castle.total_killed;
	float sum_kill_delay = 0, sum_fight_delay = 0;
	int count = castle.towers[0].count_killed + castle.towers[1].count_killed + castle.towers[2].count_killed + castle.towers[3].count_killed;
	int count1 = castle.towers[0].nActive + castle.towers[1].nActive + castle.towers[2].nActive + castle.towers[3].nActive + castle.InActive.count;

	outputfile << "Kill_Time_Step  Sequence_number  Fight_Delay  Kill_Delay  Fight_Time" << endl;
	while(temp)
	{
		outputfile << temp->Kill_time_step << "  ";
		outputfile << temp->S << "   ";
		outputfile << temp->Fight_Delay << "   ";
		outputfile << temp->Kill_Delay << "   ";
		outputfile << temp->Total_Fight_Time << "   ";
		outputfile << endl;
		sum_kill_delay += temp->Kill_Delay;
		sum_fight_delay += temp->Fight_Delay;
		temp = temp->link;
	}

	float average_fight_delay = sum_fight_delay / count, average_kill_delay = sum_kill_delay / count;
	if (count == 0) {average_fight_delay = 0; average_kill_delay = 0;} //to avoid infinity !

	outputfile << "T1_Total_Damage  T2_Total_Damage  T3_Total_Damage  T4_Total_Damage" << endl;
	for (int i = 0; i < 4; i++)
		outputfile << castle.towers[i].first_health - castle.towers[i].Health << "   ";
	outputfile << endl;
	outputfile << "R1_Distance  R2_Distance  R3_Distance  R4_Distance" << endl;
	for (int i = 0; i < 4; i++)
		outputfile << castle.towers[i].unpaved << "   ";
	outputfile << endl;

	if (!castle.enemy_victory)
	{
		outputfile << "Game is WIN" << endl;
		outputfile << "Total Enemies = " << count << endl;
		outputfile << "Average Fight Delay = " << average_fight_delay << endl;
		outputfile << "Average Kill Delay = " << average_kill_delay << endl;
	}
	else 
	{
		outputfile << "Game is LOSS" << endl;
		outputfile << "Killed Enemies = " << count << endl;
		outputfile << "Alive Enemies = " << count1 << endl;
		outputfile << "Average Fight Delay = " << average_fight_delay << endl;
		outputfile << "Average Kill Delay = " << average_kill_delay << endl; 
	}
}

void InsertAtTail (enemy E, enemy *& head)
{
	enemy * temp = new enemy;
	*temp = E;
	temp->link = NULL;
	if (head == NULL) head = temp;
	else 
	{
		enemy * temp1 = head;
		while (temp1->link)
			temp1 = temp1 -> link;
		temp1 -> link = temp;
	}
}

void InsertAtHead (enemy E, enemy *&head)
{
	enemy * temp = new enemy;
	*temp = E;
	temp->link = NULL;
	if (head != NULL) temp -> link = head;
	head = temp;
}

enemy Delete (enemy *& head)
{
	enemy temp = *head;
	head = head -> link;
	return temp;
}
 
void insert_sort_killed (enemy E, enemy *& total_killed)
{
	enemy* head = total_killed;
	enemy* temp = new enemy;
	*temp = E;
	if (total_killed == NULL) 
	{
		temp->link = NULL;
		total_killed = temp;
		return;
	}
	if (total_killed->Kill_time_step > E.Kill_time_step)
	{
		temp->link = total_killed;
		total_killed = temp;
		return;
	}
	else if (total_killed->Kill_time_step == E.Kill_time_step)
	{
		if (total_killed->Fight_Delay >= E.Fight_Delay)
		{
			temp->link = total_killed; 
			total_killed = temp; 
			return;
		}
		temp->link = total_killed->link;
		total_killed->link = temp;
		return;
	}
	enemy* temp1 = total_killed->link;
	while (temp1 &&head->link->Kill_time_step < E.Kill_time_step)
	{
		head = head->link;
		temp1 = temp1->link;
	}
	while (temp1 &&head->link->Kill_time_step == E.Kill_time_step &&head->link->Fight_Delay <= E.Fight_Delay)
	{
		head = head->link;
		temp1 = temp1->link;
	}
	temp->link = head->link;
	head->link = temp;
} 

bool is_sorted (enemy * head)
{
	if (head == NULL || head->link == NULL) return true;
	enemy *& temp = head;
	while (temp->link && temp->link->Type == 2)
	{
		if (temp->priority > temp->link->priority) temp = temp->link;
		else if (temp->priority == temp->link->priority && temp->Time <= temp->link->Time) temp = temp->link;
		else return false;
	}
	return true;
}

void Sort (enemy * head)
{
	if (is_sorted(head)) return;
	enemy *& temphead = head;
	enemy tempenemy;
	int counter = 0;
	while (temphead && temphead->Type == 2)
	{
		temphead = temphead->link;
		counter++;
	}
	temphead = head;
	
	for (int i = 0; i < counter; i++)
	{
		if (is_sorted(head)) return;
		while (temphead->link && temphead->link->Type == 2)  //iterate through list until next is null or not shielded 
		{
			if (is_sorted(head)) return;
			if (temphead->priority < temphead->link->priority)
			{
				tempenemy = *temphead;
				*temphead = *(temphead->link);
				*(temphead->link) = tempenemy;
			}
			else if (temphead->priority == temphead->link->priority && temphead->Time > temphead->link->Time)
			{
				tempenemy = *temphead;
				*temphead = *(temphead->link);
				*(temphead->link) = tempenemy;
			}
			temphead = temphead->link; //increment node
		}	
		temphead = head;//reset temphead
	}
}

/*A function to set the position of cursor on the screen*/
void gotoxy(int x, int y)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	_COORD pos;
	pos.X = x;
	pos.Y = y;

	SetConsoleCursorPosition(hConsole, pos);
}

/*A function to set the command window lenght and height for the game specification*/
void SetWindow()
{
	system("mode 150,50");   //Set mode to ensure window does not exceed buffer size
	SMALL_RECT WinRect = {0, 0, CmdWidth, CmdHeight};   //New dimensions for window in 8x12 pixel chars
	SMALL_RECT* WinSize = &WinRect;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, WinSize);   //Set new size for window
}

/*A function to color the cmd text*/
void color(int thecolor)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hStdout, 
		thecolor|FOREGROUND_INTENSITY);
}

/*A function to partition the castle into regions (A,B,C,D)*/
void DrawRegions(const castle &C)
{
	color(FOREGROUND_RED|FOREGROUND_BLUE);
	gotoxy(0, C.Ystrt + C.L/2);
	for(int i=0;i<C.W+2*C.Xstrt;i++)
		cout<<"-";
	for(int j=0;j<C.L+2*C.Ystrt;j++)
	{
		gotoxy(C.Xstrt + C.W/2, j);
		cout<<"|";		
	}
	color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
}

/*A function to draw the castle and the towers*/
void DrawCastle(const castle &C,int SimulationTime)
{
	system("CLS");
	
	//Draw the castle
	cout<<endl;
	gotoxy(C.Xstrt,C.Ystrt);
	for(int i=0;i<C.W;i++)
		cout<<"=";
	for(int j=1;j<C.L; j++)
	{
		gotoxy(C.Xstrt, C.Ystrt +j);
		cout<<"|";
		gotoxy(C.Xstrt+C.W-1, C.Ystrt +j);
		cout<<"|";
	}
	gotoxy(C.Xstrt,C.Ystrt+C.L-1);
	for(int i=0;i<C.W;i++)
		cout<<"=";

	//Draw towers
	int T1X,T2X,T3X,T4X,T1Y,T2Y,T3Y,T4Y;
	for (int i=0;i<4;i++)
	{

		if(i==0)
		{
			T1X=C.Xstrt+1;
			T1Y=C.Ystrt + TowerLength;
			gotoxy(T1X, T1Y);
			for(int i=0;i<TowerWidth;i++)
				cout<<"=";
			gotoxy(T1X +1, T1Y-1);
			cout<<"T1"<<"("<<C.towers[0].Health<<")";

			gotoxy(C.Xstrt+C.W/4, C.Ystrt+C.L/4);
			cout<<"(A)";
		}
		else if(i==1)
		{
			T2X=C.Xstrt+C.W-2-TowerWidth;
			T2Y=T1Y;
			gotoxy(T2X,T2Y );
			for(int i=0;i<TowerWidth+1;i++)
				cout<<"=";
			gotoxy(T2X +1, T2Y-1);
			cout<<"T2"<<"("<<C.towers[1].Health<<")";
			gotoxy(C.Xstrt+(3*C.W/4), C.Ystrt+C.L/4);
			cout<<"(B)";
		}
		else if(i==2)
		{
			T3X=T2X;
			T3Y=C.Ystrt + C.L - TowerLength-1;;
			gotoxy(T3X,T3Y );
			for(int i=0;i<TowerWidth+1;i++)
				cout<<"=";
			gotoxy(T3X +1, T3Y+1);
			cout<<"T3"<<"("<<C.towers[2].Health<<")";
			gotoxy(C.Xstrt+(3*C.W/4), C.Ystrt+(3*C.L/4)-1);
			cout<<"(C)";

		}
		else 
		{
			T4X=T1X;
			T4Y=C.Ystrt + C.L - TowerLength-1;
			gotoxy(T4X, T4Y);
			for(int i=0;i<TowerWidth;i++)
				cout<<"=";
			gotoxy(T4X +1, T4Y+1);
			cout<<"T4"<<"("<<C.towers[3].Health<<")";
			gotoxy(C.Xstrt+(C.W/4), C.Ystrt+(3*C.L/4)-1);
			cout<<"(D)";

		}
	}


	DrawRegions(C);

	//****************************
	gotoxy(CmdWidth/2 -10, CmdHeight-1);
	cout<<"Simulation Time : " << SimulationTime<<endl;
}

/*A function to draw a single enemy using its distance from the castle*/
void DrawEnemy(const enemy&E, int Ypos)
{
	int x,y;
	// First calcuale x,y position of the enemy on the output screen
	// It depends on the region and the enemy distance
	char EnemyRegion = E.Region;
	switch (EnemyRegion)
	{
	case 'A':
		x = CastleXStrt-E.Distance;
		y = (CmdHeight/2)-(CastleLength/4)-Ypos;
		break;
	case 'B':
		x = CastleXStrt+CastleWidth+E.Distance-1;
		y = (CmdHeight/2)-(CastleLength/4)-Ypos;
		break;
	case 'C':
		x = CastleXStrt+CastleWidth+E.Distance-1;
		y = (CmdHeight/2)+(CastleLength/4)+Ypos;
		break;
	default:
		x= CastleXStrt-E.Distance;
		y= (CmdHeight/2)+(CastleLength/4)+Ypos;

	}

	gotoxy(x, y);

	//set enemy color according to it type
	int EnemyType = E.Type;
	switch(EnemyType)
	{
	case 0:
		color(FOREGROUND_GREEN);
		break;
	case 1:
		color(FOREGROUND_RED|FOREGROUND_GREEN);//Yellow
		break;
	default:
		color(FOREGROUND_RED);
	}

	
	cout<<(char)EnemyShape; //Draw the enemy

	color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
}

/* A function to draw a list of enemies and ensure there is no overflow in the drawing
To use this function, you must prepare its input parameters as specified
Input Parameters:
enemies [ ] : array of enemy pointers (ALL enemies from all regions in one array)
size : the size of the array (total no. of enemies) */
void DrawEnemies(enemy enemies[], int size)
{
	//draw enemies at each region 
	for(char region = 'A'; region <= 'D'; region++)
	{
		int CountEnemies = 0;	//count Enemies at the same distance to draw them vertically if they are <= 15 enemy else print number of enemy in the region
		bool draw = true;
		for(int distance = ((CmdWidth/2)-(CastleWidth/2)); distance > 1; distance--)
		{
			CountEnemies = 0;

			for(int i = 0; i < size; i++)
			{	
				if(enemies[i].Distance == distance &&enemies[i].Region == region)
				{
					CountEnemies++;
				}
			}
			if(CountEnemies > 15)
			{
				draw = false;
				break;
			}

		}
		if(draw)
		{
			for(int distance = ((CmdWidth/2) - (CastleWidth/2)); distance > 1; distance--)
			{
				CountEnemies = 0;

				for(int i = 0; i < size; i++)
				{	
					if(enemies[i].Distance == distance &&enemies[i].Region == region)
					{
						DrawEnemy(enemies[i], CountEnemies);
						CountEnemies++;
					}
				}

			}


		}
		else // print message maximum reached in this region
		{
			int x; int y;
			if(region == 'A')
			{
				x = CastleXStrt - 30;
				y = (CmdHeight/2)-(CastleLength/4);
			}
			else if(region == 'B')
			{
				x = CastleXStrt + CastleWidth+30;
				y = (CmdHeight/2) - (CastleLength/4);


			}
			else if(region == 'C')
			{
				x = CastleXStrt + CastleWidth+30;
				y = (CmdHeight/2) + (CastleLength/4);

			}
			else
			{
				x = CastleXStrt - 30;
				y = (CmdHeight/2) + (CastleLength/4);

			}
			gotoxy(x, y);
			cout << "Maximum limit";
		}

	}
	gotoxy(0, CmdHeight - 1);
}