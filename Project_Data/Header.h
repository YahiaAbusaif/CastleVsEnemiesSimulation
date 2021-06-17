#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;

//constants
#define CmdWidth		150
#define CmdHeight		50
#define CastleWidth		30
#define CastleLength	20
#define CastleXStrt		(CmdWidth/2-(CastleWidth/2))
#define CastleYStrt		(CmdHeight/2-(CastleLength/2))
#define TowerWidth      7
#define TowerLength     3
#define EnemyShape		219  //ASCII code of enemy char shape 


struct enemy
{
	int S;		//sequence number
	float k;   //constant for damage equation
	char Region;
	int Distance;	//Distance to the castle
	float Health;  
	int Type;		//PVR (0), FITR (1), SHLD_FITR (2)
	int Time;      // Arrival Time
	int Reload_period; 
	int Enemy_Fire_Power;
	int Kill_time_step;
	int Fight_Delay;   //the time elapsed until a enemy is first shot by a tower
	int Kill_Delay;  //the time elapsed between first time a tower shoots an enemy and its kill time
	int Total_Fight_Time; //Fight_Delay + Kill_Delay
	float priority; 
	int first_time; //first time a tower shoots the enemy
	enemy* link;
};

struct queue 
{
	enemy* front;
	int count;
	enemy* rear;
};

struct tower
{
	int TW; //Tower width
	int TL; //Tower Height
	float Health;
	float first_health; // to calculate the damage of tower.
	int N; // the maximum number of enemies a tower can attack at each time step
	int Tower_Fire_Power; //Tower fire Power
	int unpaved; // distance unpaved
	char Region;
	int count_killed; //the number of killed enemies from the beginnig.
	int nActive; //the number of active enemies.
	int temp_count; //the number of killed enemies in last time step.
	enemy* Active;
};

struct castle
{
	//starting x,y
	int Xstrt;
	int Ystrt;
	int W;	//width
	int L;  //Height
	tower towers[4];	//Castle has 4 towers
	enemy* total_killed;
	queue InActive; 
	bool enemy_victory;
};

/*A function to set the position of cursor on the screen*/
void gotoxy(int x, int y);

/*A function to set the command window lenght and height for the game specification*/
void SetWindow();

/*A function to color the cmd text*/
void color(int thecolor);

/*A function to partition the castle into regions (A,B,C,D)*/
void DrawRegions(const castle &C);

/*A function to draw the castle and the towers*/
void DrawCastle(const castle &C,int SimulationTime);

/*A function to draw a single enemy using its distance from the castle*/
void DrawEnemy(const enemy&E, int Ypos=0);

/*A function to draw a list of enemies exist in all regions and ensure there is no overflow in the drawing*/
void DrawEnemies(enemy enemies[],int size);

void initqueue (queue &);
bool Is_Empty (queue);
void enqueue (enemy, queue &);
enemy dequeue (queue &);
enemy* front (queue);
enemy* rear (queue);
void InsertAtTail (enemy, enemy *&);
void InsertAtHead (enemy, enemy *&);
void Sort (enemy *);
bool is_sorted (enemy*);
enemy Delete (enemy *&);
void insert_sort_killed (enemy, enemy *&);
void init_enemy (enemy &); // to initialize the needed characterstics of enemy
void init_castle (castle &); // to initialize the needed characterstics of castle
void FillEnemies (tower [], int &, enemy []);
void active_enemy (queue &, tower[], int); // to check the active enemies in inactive list
void Set_priority (tower [], float, float, float, int); //to edit the priority of Shielded enemies.
void MoveEnemies (tower[], int);
void Fight (tower[], int); //to attack towers and enemies
void killed_enemy (castle&, int); //to check the Active enemies if killed.
void transport (tower&, tower&, queue);
void print (tower[]);
void SaveToFile (castle);
void ReadTheData(castle &, float &,float &,float &, char []);
