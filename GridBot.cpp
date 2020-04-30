/****************************************************************************************************
*	Program Name:	GridBot
*	Author:			Ariana Burns
*	Date:			10/3/19
****************************************************************************************************/
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>
#include <vector>
using namespace std;

enum direction { up_left, up, up_right, lleft, stay, rright, down_left, down, down_right };	//Direction bot will go

int ROW_LENGTH = 10;
int COL_LENGTH = 10;

struct Square
{
	int xCoord;	//X coordinate of square
	int yCoord;	//Y coordinate of square
	bool isBlocked;	//If square is part of obstacle
	bool hasBeenTo;	//If bot has been to square before
	char type;	//Type of square: 'O' = accessible or open (bot can go through), 'C' = obstacle (no-go), 'R' = breadcrumb (bot has been here before), 'D' = dead end (is or leads to dead end)
	string display;	//How string is displayed, usually ' |' or 'X|'

	//Constructor
	Square(int x, int y, char t)
	{
		//Sets up x and y values
		xCoord = x;
		yCoord = y;

		//Determines display, isBlocked, and hasBeenTo with type
		type = t;
		switch (t)
		{
			//Open
			case 'O':
			case 'o':
				isBlocked = false;
				hasBeenTo = false;
				display = " |";
				break;
			//Closed
			case 'C':
			case 'c':
				isBlocked = true;
				hasBeenTo = false;
				display = "X|";
				break;
			//Breadcrumb, mostly coded for potential debug purposes
			case 'R':
			case 'r':
				isBlocked = false;
				hasBeenTo = true;
				display = "*|";
				break;
			//Dead end, mostly coded for potential debug purposes
			case 'D':
			case 'd':
				isBlocked = true;
				hasBeenTo = true;
				display = "-|";
				break;
			//Start square
			case 'S':
			case 's':
				isBlocked = false;
				hasBeenTo = true;
				display = "S|";
				break;
			//End square, the goal
			case 'E':
			case 'e':
				isBlocked = false;
				hasBeenTo = false;
				display = "E|";
				break;
			//Current square bot is in, mostly coded for debugging
			case 'B':
			case 'b':
				isBlocked = false;
				hasBeenTo = true;
				display = "&|";
				break;
			default:
				isBlocked = false;
				hasBeenTo = false;
				display = "?|";	//If this comes up in the grid, something went wrong
		}
	}
};

void createGrid(vector<vector<Square>> &grid);
void displayGrid(const vector<vector<Square>> &grid);
Square getStart();
Square getEnd();
void addObstacles(vector<vector<Square>> &grid, Square start, Square end);
void changeSquareInGrid(vector<vector<Square>> &grid, int i, int j, char type);
bool botDecideTime(vector<vector<Square>> &grid, bool reset = false);
bool tryBotMove(vector<vector<Square>> &grid, int &botX, int &botY, direction dir, char &prevSquareState, bool breadMode);
int myRandom(int i) { return rand() % i; }
direction findLight(vector<vector<Square>> &grid, int botX, int botY);

int main()
{
	vector<vector<Square>> grid;	//THE world the bot will navigate through
	//vector<vector<Square>> gridZero = grid;
	srand(time(NULL));

	//int count = 1000;
	//do
	//{
		createGrid(grid);	//Creates the grid
		displayGrid(grid);	//Displays the grid
		//cout << "Number: " << count;

		//Initialize bot
		botDecideTime(grid);

		//Loop for bot decide time
		do
		{
			this_thread::sleep_for(chrono::milliseconds(500));	//Pauses computer for a moment in time, so user can see grid and bot decisions
			cout << flush;
			system("CLS");
			displayGrid(grid);
			//cout << "Number: " << count;
		} while (botDecideTime(grid));

		this_thread::sleep_for(chrono::milliseconds(500));	//Pauses computer for a moment in time, so user can see grid and bot decisions
		cout << flush;
		system("CLS");
		displayGrid(grid);
		//cout << "Number: " << count;

	//	count--;
	//	if (count)
	//	{
	//		//this_thread::sleep_for(chrono::milliseconds(1000));
	//		cout << flush;
	//		system("CLS");
	//		grid = gridZero;
	//	}
	//} while (count);

	system("Pause");
	return 0;
}

//This function creates the grid, which is a 2D matrix of Squares
void createGrid(vector<vector<Square>> &grid)
{
	//Get length of column and row
	cout << "Enter column length: ";
	cin >> COL_LENGTH;
	while (cin.fail() || COL_LENGTH < 2)
	{
		cout << "Invalid. Value must be numeric, and cannot be less than 2.\n";
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter column length: ";
		cin >> COL_LENGTH;
	}
	cout << "Enter row length: ";
	cin >> ROW_LENGTH;
	while (cin.fail() || ROW_LENGTH < 2)
	{
		cout << "Invalid. Value must be numeric, and cannot be less than 2.\n";
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter row length: ";
		cin >> ROW_LENGTH;
	}

	//Get start and end squares
	Square start = getStart();
	Square end = getEnd();

	//In case start and end get assigned same square
	while (start.xCoord == end.xCoord && start.yCoord == end.yCoord)
		end = getEnd();

	vector<Square> column;	//A column in the grid
	for (int i = 0; i < COL_LENGTH; i++)
	{
		grid.push_back(column);
		for (int j = 0; j < ROW_LENGTH; j++)
		{
			//Creates generic square
			Square temp(j, i, 'o');

			//If square is not generic, modify what gets put in the grid
			if (temp.xCoord == start.xCoord && temp.yCoord == start.yCoord)	//If current square is supposed to be the starting square
				temp = start;
			else if (temp.xCoord == end.xCoord && temp.yCoord == end.yCoord)	//If current square is supposed to be the ending square
				temp = end;

			//Add square to grid
			grid[i].push_back(temp);
		}
	}

	addObstacles(grid, start, end);
}

//This function displays the grid to the screen
void displayGrid(const vector<vector<Square>> &grid)
{
	for (int i = 0; i < grid.size(); i++)
	{
		cout << '|';
		for (int j = 0; j < grid[i].size(); j++)
			cout << grid[i][j].display;
			//cout << grid[i][j].xCoord << ',' << grid[i][j].yCoord << '|';
		cout << endl;
	}
	cout << endl;
}

//Generates the starting position
Square getStart()
{
	//int x = rand() % ROW_LENGTH;
	//int y = rand() % COL_LENGTH;
	//Square start(x, y, 's');
	int x;
	int y;
	cout << "Enter coordinates for start: \"x y\" : ";
	cin >> x >> y;
	while (cin.fail() || x < 0 || x >= ROW_LENGTH || y < 0 || y >= COL_LENGTH)
	{
		cout << "Invalid coordinates.\n";
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter coordinates for start: \"x y\" : ";
		cin >> x >> y;
	}
	Square start(x, y, 's');
	return start;
}

//Generates the end (or goal) position
Square getEnd()
{
	//int x = rand() % ROW_LENGTH;
	//int y = rand() % COL_LENGTH;
	//Square end(x, y, 'e');
	int x;
	int y;
	cout << "Enter coordinates for end: \"x y\" : ";
	cin >> x >> y;
	while (cin.fail() || x < 0 || x >= ROW_LENGTH || y < 0 || y >= COL_LENGTH)
	{
		cout << "Invalid coordinates.\n";
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter coordinates for end: \"x y\" : ";
		cin >> x >> y;
	}
	Square end(x, y, 'e');
	return end;
}

//Adds obstacles to grid
void addObstacles(vector<vector<Square>> &grid, Square start, Square end)
{
	int numObs;	//Number of obstacles
	int obsLen;	//Length of obstacles

	int tempX;
	int tempY;

	int origX;
	int origY;

	direction dir;

	cout << "Enter number of obstacles: ";
	cin >> numObs;
	while (cin.fail() || numObs > ROW_LENGTH * COL_LENGTH - 2)
	{
		cout << "Invalid. Value must be numeric, and cannot be more than " << (ROW_LENGTH * COL_LENGTH - 2) << endl;
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter number of obstacles: ";
		cin >> numObs;
	}
	cout << "Enter length of obstacles: ";
	cin >> obsLen;
	while (cin.fail() || obsLen > (COL_LENGTH < ROW_LENGTH ? COL_LENGTH : ROW_LENGTH) - 1)
	{
		cout << "Invalid. Value must be numeric, and cannot be more than " << (COL_LENGTH < ROW_LENGTH ? COL_LENGTH : ROW_LENGTH) - 1 << endl;
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Enter length of obstacles: ";
		cin >> obsLen;
	}

	for (int i = 0; i < numObs; i++)
	{
		for (int j = 0; j < obsLen; j++)
		{
			//Starting point
			if (!j)
			{
				do
				{
					tempX = rand() % ROW_LENGTH;
					tempY = rand() % COL_LENGTH;
				} while ((tempX == start.xCoord && tempY == start.yCoord) || (tempX == end.xCoord && tempY == end.yCoord));	//Make sure obstacle isn't put on start or end
				changeSquareInGrid(grid, tempY, tempX, 'c');
				origX = tempX;
				origY = tempY;
			}
			else if (j == 1)	//Second iteration, need to pick direction
			{
				switch (rand() % 4)
				{
					case 0:
						dir = lleft;
						break;
					case 1:
						dir = rright;
						break;
					case 2:
						dir = up;
						break;
					case 3:
						dir = down;
						break;
					default:
						dir = stay;
				}
			}

			if (j)	//Not first iteration, obstacle needs to grow
			{
				switch (dir)
				{
					case lleft:
						tempX--;
						if (tempX < 0 || tempX >= ROW_LENGTH)
						{
							tempX = origX + 1;
							dir = rright;
						}
						if (!(tempX == start.xCoord && tempY == start.yCoord) && !(tempX == end.xCoord && tempY == end.yCoord))
							changeSquareInGrid(grid, tempY, tempX, 'c');
						break;
					case rright:
						tempX++;
						if (tempX < 0 || tempX >= ROW_LENGTH)
						{
							tempX = origX - 1;
							dir = lleft;
						}
						if (!(tempX == start.xCoord && tempY == start.yCoord) && !(tempX == end.xCoord && tempY == end.yCoord))
							changeSquareInGrid(grid, tempY, tempX, 'c');
						break;
					case up:
						tempY--;
						if (tempY < 0 || tempY >= COL_LENGTH)
						{
							tempY = origY + 1;
							dir = down;
						}
						if (!(tempX == start.xCoord && tempY == start.yCoord) && !(tempX == end.xCoord && tempY == end.yCoord))
							changeSquareInGrid(grid, tempY, tempX, 'c');
						break;
					case down:
						tempY++;
						if (tempY < 0 || tempY >= COL_LENGTH)
						{
							tempY = origY - 1;
							dir = up;
						}
						if (!(tempX == start.xCoord && tempY == start.yCoord) && !(tempX == end.xCoord && tempY == end.yCoord))
							changeSquareInGrid(grid, tempY, tempX, 'c');
						break;
					default:
						cout << "Something went wrong.\n";
				}
			}
		}
	}
}

//For handling change of Square states
void changeSquareInGrid(vector<vector<Square>> &grid, int i, int j, char type)
{
	grid[i][j].type = type;
	switch (type)
	{
		//Open
		case 'O':
		case 'o':
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = false;
			grid[i][j].display = " |";
			break;
		//Closed
		case 'C':
		case 'c':
			grid[i][j].isBlocked = true;
			grid[i][j].hasBeenTo = false;
			grid[i][j].display = "X|";
			break;
		//Breadcrumb
		case 'R':
		case 'r':
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = true;
			grid[i][j].display = "*|";
			break;
		//Dead end
		case 'D':
		case 'd':
			grid[i][j].isBlocked = true;
			grid[i][j].hasBeenTo = true;
			grid[i][j].display = "-|";
			break;
		//Start square, used here mostly for debug purposes
		case 'S':
		case 's':
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = true;
			grid[i][j].display = "S|";
			break;
		//End square, the goal, used here mostly for debug purposes
		case 'E':
		case 'e':
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = false;
			grid[i][j].display = "E|";
			break;
		//Current square bot is in
		case 'B':
		case 'b':
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = true;
			grid[i][j].display = "&|";
			break;
		default:
			grid[i][j].isBlocked = false;
			grid[i][j].hasBeenTo = false;
			grid[i][j].display = "?|";	//If this comes up in the grid, something went wrong
	}
}

//The function where the bot decisions are made, returns true if not at end, false otherwise
bool botDecideTime(vector<vector<Square>> &grid, bool reset)
{
	static int botX = -1;	//X coord of bot
	static int botY = -1;	//Y coord of bot
	static int endX = -1;	//X coord of end square
	static int endY = -1;	//Y coord of end square
	static char prevSquareState = 's';	//Square state before bot was in it
	direction idealDir;	//Best direction for bot to go
	bool triedEverything = false;
	vector<direction> potDirs;
	bool breadMode = false;	//Used to determine if algorithm should evaluate breadcrumbs as valid spots

	if (reset)
	{
		botX = -1;
		prevSquareState = 'S';
	}

	//Initialize bot if not yet initialized
	if (botX == -1)
	{
		bool found = false;
		//Find the start block
		for (int i = 0; i < grid.size() && !found; i++)
		{
			for (int j = 0; j < grid[i].size() && !found; j++)
			{
				if (grid[i][j].type == 'S' || grid[i][j].type == 's')
				{
					botX = grid[i][j].xCoord;
					botY = grid[i][j].yCoord;
					changeSquareInGrid(grid, i, j, 'b');
					found = true;
				}
			}
		}
		if (!found)	//Start block non-existant, this is a problem, end program
		{
			cout << "Error. Start not found.\n";
			cout << endl;
			exit(-1);
		}
		//Find the end block
		found = false;
		for (int i = 0; i < grid.size() && !found; i++)
		{
			for (int j = 0; j < grid[i].size() && !found; j++)
			{
				if (grid[i][j].type == 'E' || grid[i][j].type == 'e')
				{
					endX = grid[i][j].xCoord;
					endY = grid[i][j].yCoord;
					found = true;
				}
			}
		}
		if (!found)	//End block non-existant, this is a problem, end program
		{
			cout << "Error. End not found.\n";
			cout << endl;
			exit(-1);
		}
		return true;
	}
	else
	{
		//Actual decision time
		do
		{
			if (botX == endX)	//Bot is in same column as end, move up or down
			{
				if (botY < endY)	//Bot above end, ideal is to move down
					idealDir = down;
				else	//Bot below end, ideal is move up
					idealDir = up;
			}
			else if (botY == endY)	//Bot in same row as end, move left or right
			{
				if (botX < endX)	//Bot left of end, move right
					idealDir = rright;
				else	//Bot right of end, move left
					idealDir = lleft;
			}
			else	//Bot moves in diagonal direction
			{
				if (botX > endX && botY > endY)	//Bot right and below end, move upper left
					idealDir = up_left;
				else if (botX < endX && botY > endY)	//Bot left and below end, move upper right
					idealDir = up_right;
				else if (botX > endX && botY < endY)	//Bot right and above end, move lower left
					idealDir = down_left;
				else	//Bot left and above end, move lower right
					idealDir = down_right;
			}

			//Bot tries to move, if it can't, pick another direction
			if (!tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
			{
				switch (idealDir)
				{
					case up_left:
						if (botX - endX < botY - endY)	//Going left is closer than going up
							idealDir = lleft;
						else if (botX - endX > botY - endY)	//Going up closer than going left
							idealDir = up;
						else	//Both directions equidistant, pick up or left at random
							idealDir = rand() % 2 ? up : lleft;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try up if left fails, or left if up fails
							if (idealDir == lleft)
								idealDir = up;
							else
								idealDir = lleft;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_right, rright, down_left, down, down_right
									potDirs.push_back(up_right);
									potDirs.push_back(rright);
									potDirs.push_back(down_left);
									potDirs.push_back(down);
									potDirs.push_back(down_right);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case up:
						//Both directions equidistant, pick up_left or up_right at random
						idealDir = rand() % 2 ? up_left : up_right;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try up_left if up_right fails, or up_right if up_left fails
							if (idealDir == up_right)
								idealDir = up_left;
							else
								idealDir = up_right;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: lleft, rright, down_left, down, down_right
									potDirs.push_back(lleft);
									potDirs.push_back(rright);
									potDirs.push_back(down_left);
									potDirs.push_back(down);
									potDirs.push_back(down_right);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case up_right:
						if (endX - botX < botY - endY)	//Going right is closer than going up
							idealDir = rright;
						else if (endX - botX > botY - endY)	//Going up closer than going right
							idealDir = up;
						else	//Both directions equidistant, pick up or right at random
							idealDir = rand() % 2 ? up : rright;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try up if right fails, or right if up fails
							if (idealDir == rright)
								idealDir = up;
							else
								idealDir = rright;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_left, lleft, down_left, down, down_right
									potDirs.push_back(up_left);
									potDirs.push_back(lleft);
									potDirs.push_back(down_left);
									potDirs.push_back(down);
									potDirs.push_back(down_right);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case lleft:
						//Both directions equidistant, pick up_left or down_left at random
						idealDir = rand() % 2 ? up_left : down_left;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try up_left if down_left fails, or down_left if up_left fails
							if (idealDir == down_left)
								idealDir = up_left;
							else
								idealDir = down_left;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}

									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up, up_right, rright, down, down_right
									potDirs.push_back(up);
									potDirs.push_back(up_right);
									potDirs.push_back(rright);
									potDirs.push_back(down);
									potDirs.push_back(down_right);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case rright:
						//Both directions equidistant, pick up_right or down_right at random
						idealDir = rand() % 2 ? up_right : down_right;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try up_right if down_right fails, or down_right if up_right fails
							if (idealDir == down_right)
								idealDir = up_right;
							else
								idealDir = down_right;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_left, up, lleft, down_left, down
									potDirs.push_back(up_left);
									potDirs.push_back(up);
									potDirs.push_back(lleft);
									potDirs.push_back(down_left);
									potDirs.push_back(down);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case down_left:
						if (botX - endX < endY - botY)	//Going left is closer than going down
							idealDir = lleft;
						else if (botX - endX > endY - botY)	//Going down closer than going left
							idealDir = down;
						else	//Both directions equidistant, pick down or left at random
							idealDir = rand() % 2 ? down : lleft;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try down if left fails, or left if down fails
							if (idealDir == lleft)
								idealDir = down;
							else
								idealDir = lleft;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_left, up, up_right, rright, down_right
									potDirs.push_back(up_left);
									potDirs.push_back(up);
									potDirs.push_back(up_right);
									potDirs.push_back(rright);
									potDirs.push_back(down_right);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case down:
						//Both directions equidistant, pick down_left or down_right at random
						idealDir = rand() % 2 ? down_left : down_right;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try down_left if down_right fails, or down_right if down_left fails
							if (idealDir == down_right)
								idealDir = down_left;
							else
								idealDir = down_right;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_left, up, up_right, lleft, rright
									potDirs.push_back(up_left);
									potDirs.push_back(up);
									potDirs.push_back(up_right);
									potDirs.push_back(lleft);
									potDirs.push_back(rright);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					case down_right:
						if (endX - botX < endY - botY)	//Going right is closer than going down
							idealDir = rright;
						else if (endX - botX > endY - botY)	//Going down closer than going right
							idealDir = down;
						else	//Both directions equidistant, pick down or right at random
							idealDir = rand() % 2 ? down : rright;

						//Bot tries to move again
						if (tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
						{
							if (!breadMode)
								breadMode = true;	//Prevents second iteration with breadMode
							break;
						}
						else	//Bot couldn't move, pick another direction
						{
							//Try down if right fails, or right if down fails
							if (idealDir == rright)
								idealDir = down;
							else
								idealDir = rright;

							bool dirFail;
							while (dirFail = !tryBotMove(grid, botX, botY, idealDir, prevSquareState, breadMode))
							{
								//At this point, best directions have failed, time to try other less ideal directions

								if (triedEverything)
								{
									if (breadMode)
									{
										//cout << "I'm stuck. This world is unsolvable.\n";
										//cout << endl;
										if (!tryBotMove(grid, botX, botY, findLight(grid, botX, botY), prevSquareState, true))
										{
											vector<Square> errStart;
											Square errMsg(-1, -1, '?');
											errMsg.display = "I'm stuck. This world is unsolvable.";
											grid.push_back(errStart);
											grid[grid.size() - 1].push_back(errMsg);
											return false;
										}
									}
									else	//Prep for breadMode
									{
										triedEverything = false;	//Reset triedEverything
										break;	//Break out of switch
									}
								}

								//Init stack
								if (!potDirs.size())
								{
									//Directions to try: up_left, up, up_right, lleft, down_left
									potDirs.push_back(up_left);
									potDirs.push_back(up);
									potDirs.push_back(up_right);
									potDirs.push_back(lleft);
									potDirs.push_back(down_left);
									random_shuffle(potDirs.begin(), potDirs.end(), myRandom);
								}

								//Trying random directions
								idealDir = potDirs[potDirs.size() - 1];

								//Clear out about to be tested direction
								potDirs.pop_back();

								//Last direction about to be tried, set triedEverything to true
								if (!potDirs.size())
									triedEverything = true;
							}
							//One of the directions worked
							if (!dirFail)
							{
								if (!breadMode)
									breadMode = true;	//Prevents second iteration with breadMode
							}
						}
						break;
					default:
						return false;
				}
			}
			else	//First move worked
			{
				if (!breadMode)
					breadMode = true;	//Prevents second iteration with breadMode
			}
			//ASSIGNMENT INTENTIONAL
			//Loop only runs twice, once on not breadMode
			//If valid direction not found, breadMode gets turned on, meaning bot can go to space it's been to before
			//If already in breadMode, or breadMode turned on to break out of decision time, breadMode turns back off, and loop ends
		} while (breadMode = !breadMode);

		//Still decide time?
		if (botX != endX || botY != endY)	//Bot is not at end square
			return true;
		else	//Bot has reached end square
			return false;
	}
}

//This function will try to move the bot, if the direction is blocked, returns false, else true
bool tryBotMove(vector<vector<Square>> &grid, int &botX, int &botY, direction dir, char &prevSquareState, bool breadMode)
{
	int prevX = botX;	//For updating previous square
	int prevY = botY;	//For updating previous square
	char nextSquareState;	//State of square bot will move to next
	//Checks if direction is valid, if valid update bot coords
	switch (dir)
	{
		case up_left:
			if ((botX - 1) < 0 || (botX - 1) >= ROW_LENGTH || (botY - 1) < 0 || (botY - 1) >= COL_LENGTH || grid[botY - 1][botX - 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY - 1][botX - 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
			{
				botX--;
				botY--;
			}
			break;
		case up:
			if ((botY - 1) < 0 || (botY - 1) >= COL_LENGTH || grid[botY - 1][botX].isBlocked)
				return false;
			else if (!breadMode && grid[botY - 1][botX].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
				botY--;
			break;
		case up_right:
			if ((botX + 1) < 0 || (botX + 1) >= ROW_LENGTH || (botY - 1) < 0 || (botY - 1) >= COL_LENGTH || grid[botY - 1][botX + 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY - 1][botX + 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
			{
				botX++;
				botY--;
			}
			break;
		case lleft:
			if ((botX - 1) < 0 || (botX - 1) >= ROW_LENGTH || grid[botY][botX - 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY][botX - 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
				botX--;
			break;
		case stay:
			return false;
		case rright:
			if ((botX + 1) < 0 || (botX + 1) >= ROW_LENGTH || grid[botY][botX + 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY][botX + 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
				botX++;
			break;
		case down_left:
			if ((botX - 1) < 0 || (botX - 1) >= ROW_LENGTH || (botY + 1) < 0 || (botY + 1) >= COL_LENGTH || grid[botY + 1][botX - 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY + 1][botX - 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
			{
				botX--;
				botY++;
			}
			break;
		case down:
			if ((botY + 1) < 0 || (botY + 1) >= COL_LENGTH || grid[botY + 1][botX].isBlocked)
				return false;
			else if (!breadMode && grid[botY + 1][botX].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
				botY++;
			break;
		case down_right:
			if ((botX + 1) < 0 || (botX + 1) >= ROW_LENGTH || (botY + 1) < 0 || (botY + 1) >= COL_LENGTH || grid[botY + 1][botX + 1].isBlocked)
				return false;
			else if (!breadMode && grid[botY + 1][botX + 1].hasBeenTo)	//If not in breadMode, don't go to space you've been to before
				return false;
			else
			{
				botX++;
				botY++;
			}
			break;
		default:
			cout << "Invalid direction.\n";
			cout << endl;
			return false;	//Something went wrong
	}
	//Update grid
	//cout << "BotX: " << botX << endl;
	//cout << "BotY: " << botY << endl;
	nextSquareState = grid[botY][botX].type;
	changeSquareInGrid(grid, botY, botX, 'B');
	//Update previous bot location
	switch (prevSquareState)
	{
		case 'S':
		case 's':
			changeSquareInGrid(grid, prevY, prevX, 'S');
			break;
		case 'O':
		case 'o':
			changeSquareInGrid(grid, prevY, prevX, 'R');
			break;
		case 'R':
		case 'r':
			changeSquareInGrid(grid, prevY, prevX, 'D');
	}
	prevSquareState = nextSquareState;
	return true;
}

//If the bot thinks it's stuck, check to make sure it isn't caught in just dead ends
//If open squares beyond dead ends, revert dead ends to open
direction findLight(vector<vector<Square>> &grid, int botX, int botY)
{
	//Check left
	for (int x = botX - 1; x >= 0; x--)
	{
		if (!grid[botY][x].isBlocked)	//Found light
		{
			//Clear out breadcrumbs
			for (int xx = botX - 1; xx >= 0; xx--)
			{
				if (!(grid[botY][xx].type == 'c' || grid[botY][xx].type == 'C'))	//Don't change past walls
				{
					if (grid[botY][xx].type == 'D' || grid[botY][xx].type == 'd')
						changeSquareInGrid(grid, botY, xx, 'o');
				}
				else
					break;
			}
			return lleft;
		}
		if (grid[botY][x].type == 'c' || grid[botY][x].type == 'C')	//Don't look past walls
			break;
	}
	//Check right
	for (int x = botX + 1; x < ROW_LENGTH; x++)
	{
		if (!grid[botY][x].isBlocked)	//Found light
		{
			//Clear out breadcrumbs
			for (int xx = botX + 1; xx < ROW_LENGTH; xx++)
			{
				if (!(grid[botY][xx].type == 'c' || grid[botY][xx].type == 'C'))	//Don't change past walls
				{
					if (grid[botY][xx].type == 'D' || grid[botY][xx].type == 'd')
						changeSquareInGrid(grid, botY, xx, 'o');
				}
				else
					break;
			}
			return rright;
		}
		if (grid[botY][x].type == 'c' || grid[botY][x].type == 'C')	//Don't look past walls
			break;
	}
	//Check up
	for (int y = botY - 1; y >= 0; y--)
	{
		//Clear out breadcrumbs
		if (!grid[y][botX].isBlocked)	//Found light
		{
			for (int yy = botY - 1; yy >= 0; yy--)
			{
				if (!(grid[yy][botX].type == 'c' || grid[yy][botX].type == 'C'))	//Don't change past walls
				{
					if (grid[yy][botX].type == 'D' || grid[yy][botX].type == 'd')
						changeSquareInGrid(grid, yy, botX, 'o');
				}
				else
					break;
			}
			return up;
		}
		if (grid[y][botX].type == 'c' || grid[y][botX].type == 'C')	//Don't look past walls
			break;
	}
	//Check down
	for (int y = botY + 1; y < COL_LENGTH; y++)
	{
		if (!grid[y][botX].isBlocked)	//Found light
		{
			//Clear out dead ends
			for (int yy = botY + 1; yy < COL_LENGTH; yy++)
			{
				if (!(grid[yy][botX].type == 'c' || grid[yy][botX].type == 'C'))	//Don't change past walls
				{
					if (grid[yy][botX].type == 'D' || grid[yy][botX].type == 'd')
						changeSquareInGrid(grid, yy, botX, 'o');
					//displayGrid(grid);
				}
				else
					break;
			}
			return down;
		}
		if (grid[y][botX].type == 'c' || grid[y][botX].type == 'C')	//Don't look past walls
			break;
	}

	//All directions failed
	return stay;
}