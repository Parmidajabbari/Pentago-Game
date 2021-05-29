#include <iostream>
#include <windows.h>  
#include <conio.h>

using namespace std;

#define Board_Margin_X 10
#define Board_Margin_Y 5

#define Cell_Width 5
#define Cell_Height 3

#define Padding_Width 2.8
#define Padding_Height 1

#define Shape_One 'X'
#define Shape_Two 'O'


struct Vector2
{
	float x;
	float y;
};

struct Cell
{
	Vector2 logicalCoordinate = {0 , 0};
};

struct BoardSection
{
	Cell cells[3][3];
};

struct Bead
{
	char presenter;
	Vector2 logicalCoordinate = {0 , 0};
};

struct Board
{
	int beadsCount = 0;
	Bead beads[36];
	BoardSection sections[2][2];
};

struct Player
{
	char name[100] = "";
	char presentor;
	
	int winCount = 0;
	int looseCount = 0;
	int drawCount = 0;
	int playedGamesCount = 0;
	int score = 0;
};

Vector2 AddVectors(Vector2 one, Vector2 two)
{
	return {one.x + two.x , one.y + two.y};
}

bool Vector2sAreEqual(Vector2 one, Vector2 two)
{
	return one.x == two.x && one.y == two.y;
}

Vector2 RotateVector2(Vector2 coord, bool clockWise)
{
	int x, y;
	
	if(clockWise)
	{
		x = (2-coord.y);
		y = coord.x;
	}
	else
	{
		x = coord.y;
		y = (2- coord.x);
	}
	
	return {x,y};
}

Vector2 ConvertLogicalCoordToVisual(Vector2 logicalCoord)
{
	float x = Board_Margin_X + logicalCoord.x * Cell_Width + (logicalCoord.x >= 3 ? Padding_Width : 0);
	float y = Board_Margin_Y + logicalCoord.y * Cell_Height + (logicalCoord.y >= 3 ? Padding_Height : 0);
	
	return {x, y};
}

// I found this fucntion from the internet
void gotoxy(int x, int y)
{
  static HANDLE h = NULL;  
  if(!h)
    h = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD c = { x, y };  
  SetConsoleCursorPosition(h,c);
}

void gotoxy(Vector2 coord)
{
	gotoxy(coord.x , coord.y);
}

void Init();
void ShowMenu();
char HandleMenuSelection();
void StartGame();
void ShowRanking();
Board CreateBoard();
void InitializePlayers(Player players[]);
void HandleGame(Board board, Player players[]);
void HandleVisual(Player currentPlayer, Board board, Vector2 focusedSectionCoord, Vector2 focusedCellCoord, bool isOnRotation);
void ShowPlayerInfo(Player player);
void DrawBoard(Board board, Vector2 focusedSectionCoord, Vector2 focusedCellCoord, bool isSectionOnFocus);
void DrawBoardSection(BoardSection section, Vector2 focusedCellCoord, bool isSectionOnFocus, bool showCellFocus);
void DrawCell(Cell cell, bool isFocused);
void DrawBeads(Bead beads[], int beadsCount);
void HandlePlayerTurn(Board* board, Player currentPlayer, int* playerIndex, Vector2* focusedSectionCoord, Vector2* focusedCellCoord, bool* isOnRotation);
void RotateSection(Vector2 sectionCoord, Bead beads[], int beadsCount, bool clockWise);
bool HandleCellChoosing(Vector2* focusedCellCoord); // return type of this func means if any cells where choosen or not;
int HandleSectionChoosing(Vector2* focusedCellCoord); // return type of this func means if any section where choosen or not;
bool CanInstertBeadAt(Board board, Vector2 coordinate);
void InsertBead(Board* board, Player currentPlayer, Vector2 logicalCoordinate);
int CheckGameEnd(Bead beads[], int beadsCount);
bool IsAnySameBeadAt(Vector2 coord, Bead beads[], int beadsCount, char presnter);
void ShowGameResult(int gameResult, Player players[]);
void SaveGameResult(int gameResult, Player players[]);

int main(int argc, char** argv) 
{
	Init();
	return 0;
}

void Init()
{
	ShowMenu();
	char selectedOption = HandleMenuSelection();
	
	if(selectedOption == '1')
		StartGame();
	else if(selectedOption == '2')
		ShowRanking();
	else if(selectedOption == '3')
		return;
}

void ShowMenu()
{
	system("cls");
	cout << "1.Start\n";
	cout << "2.Ranking\n";
	cout << "3.Exit\n";
}

char HandleMenuSelection()
{
	char selected;
	
	do
	{
		selected = getch();
		if(selected == '1' || selected=='2' || selected=='3')
			return selected;
		else
			cout << "\nPlease Choose A correct Answer\n";
		
	}
	while (true);

	return selected;
}

void ShowRanking()
{
	system("cls");
	
	FILE *scoreboardfile = fopen("scoreBoard.txt", "r"); 
	char c = fgetc(scoreboardfile); 
    while (c != EOF) 
    { 
        printf ("%c", c); 
        c = fgetc(scoreboardfile); 
    } 
    fclose(scoreboardfile);
    
    getch(); // we close ranking when any key where pressed
	Init();
}

void StartGame()
{
	Board board = CreateBoard();
	
	Player players[2];
	InitializePlayers(players);	

	HandleGame(board, players);
}

Board CreateBoard()
{
	Board board;
	
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{			
			BoardSection section;
			for(int ii=0; ii<3;ii++)
			{
				for(int jj=0;jj<3;jj++)
				{
					Cell cell;
					cell.logicalCoordinate = {ii + i * 3, jj + j * 3};
					section.cells[ii][jj] = cell;
				}
			}
			
			board.sections[i][j] = section;
		}
	}
	return board;
}

void InitializePlayers(Player players[])
{
	system("cls");
	for(int i=0; i<2; i++)
	{
		cout << "Player #";
		cout << i+1;
		cout << " Enter your name: ";
		cin >> players[i].name;
	}
	
	players[0].presentor= Shape_One;
	players[1].presentor= Shape_Two;
}

void HandleGame(Board board, Player players[])
{
	int gameResult = -1; // -1:inProgress 0:Draw 1:PlayerOneWin 2:PlayerTwoWin 

	int currentPlayerIndex = 0;
	Vector2 focusedSectionCoord = {0,0};
	Vector2 focusedCellCoord  = {0,0};
	bool isOnRotation = false;	
	
	while(gameResult == -1)
	{
		HandleVisual(players[currentPlayerIndex], board, focusedSectionCoord, focusedCellCoord, isOnRotation);
		HandlePlayerTurn(&board, players[currentPlayerIndex], &currentPlayerIndex, &focusedSectionCoord, &focusedCellCoord, &isOnRotation);
		
		gameResult = CheckGameEnd(board.beads, board.beadsCount);
	}
	
	HandleVisual(players[currentPlayerIndex], board, focusedSectionCoord, focusedCellCoord, isOnRotation);
	SaveGameResult(gameResult, players);
	ShowGameResult(gameResult, players);	
}

void HandleVisual(Player currentPlayer, Board board, Vector2 focusedSectionCoord, Vector2 focusedCellCoord, bool isOnRotation)
{
	system("cls");
	ShowPlayerInfo(currentPlayer);
	DrawBoard(board, focusedSectionCoord, focusedCellCoord, isOnRotation);
	DrawBeads(board.beads, board.beadsCount);
	gotoxy(0,0); // so that the user wont see the cursur at a bothering Pos
}

void ShowPlayerInfo(Player player)
{
	gotoxy(ConvertLogicalCoordToVisual({0,-1}));
	cout << player.name;
	cout << "'s Turn As: ";
	cout << player.presentor;
}

void DrawBoard(Board board,  Vector2 focusedSectionCoord, Vector2 focusedCellCoord, bool isSectionOnFocus)
{
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++)
			DrawBoardSection(board.sections[i][j], focusedCellCoord, (isSectionOnFocus && focusedSectionCoord.x == i && focusedSectionCoord.y == j), !isSectionOnFocus);
}

void DrawBoardSection(BoardSection section, Vector2 focusedCellCoord, bool isSectionOnFocus, bool showFocusedCell)
{
	for(int i=0; i<3;i++)
		for(int j=0;j<3;j++)
			DrawCell(section.cells[i][j], (showFocusedCell || isSectionOnFocus) && (isSectionOnFocus || Vector2sAreEqual(focusedCellCoord, section.cells[i][j].logicalCoordinate)));
}

void DrawCell(Cell cell, bool isFocused)
{
	for(int i=0; i<Cell_Width;i++)
		for(int j=0; j<Cell_Height; j++)
			{
				gotoxy(AddVectors(ConvertLogicalCoordToVisual(cell.logicalCoordinate), {i,j}));
				if(i==0 && j==0)
					cout << (isFocused ? "\u2554":"\u250C");
				else if(i==0 && j == Cell_Height-1)
					cout << (isFocused ? "\u255A":"\u2514");
				else if(i==Cell_Width-1 && j == 0)
					cout << (isFocused ? "\u2557":"\u2510");		
				else if(i==Cell_Width-1 && j == Cell_Height-1)
					cout << (isFocused ? "\u255D":"\u2518");
				else if(i==0 || i==Cell_Width-1)
					cout << (isFocused ? "\u2551":"\u2502");
				else if(j==0 || j==Cell_Height-1)
					cout << (isFocused ? "\u2550":"\u2500");		
			}
	
}


void DrawBeads(Bead beads[], int beadsCount)
{
	for(int i = 0; i< beadsCount; i++ )
	{
		float x = beads[i].logicalCoordinate.x + 0.4;
		float y = beads[i].logicalCoordinate.y + 0.4;
		
		gotoxy(ConvertLogicalCoordToVisual({x,y}));
		cout << beads[i].presenter;
	}
}

void HandlePlayerTurn(Board* board, Player currentPlayer, int* currentPlayerIndex, Vector2* focusedSectionCoord, Vector2* focusedCellCoord, bool* isOnRotation)
{	
	if(*isOnRotation)
	{
		int sectionSelected = HandleSectionChoosing(focusedSectionCoord);
		if(sectionSelected != 0)
		{
			RotateSection(*focusedSectionCoord, board->beads , board->beadsCount, sectionSelected == 1);
			*isOnRotation = false;
			*currentPlayerIndex = ((*currentPlayerIndex) == 0 ? 1 : 0);
		}
	}
	else
	{
		bool cellSelected = HandleCellChoosing(focusedCellCoord);
		if(cellSelected && CanInstertBeadAt(*board, *focusedCellCoord))
	    {
			InsertBead(board, currentPlayer, *focusedCellCoord);	
			*isOnRotation = true;
		}
	}	
}

void RotateSection(Vector2 sectionCoord, Bead beads[], int beadsCount, bool clockWise)
{
	int minValidX = sectionCoord.x * 3;
	int minValidY = sectionCoord.y * 3;
	int maxValidX = sectionCoord.x * 3 + 3 - 1;
	int maxValidY = sectionCoord.y * 3 + 3 - 1;
	
	for(int i=0; i<beadsCount; i++)
	{
		Vector2 beadCoord = beads[i].logicalCoordinate;
		if(beadCoord.x < minValidX || beadCoord.x > maxValidX || beadCoord.y < minValidY || beadCoord.y > maxValidY)
			continue;
		Vector2 pureCoord = { beadCoord.x - (sectionCoord.x * 3) , beadCoord.y - (sectionCoord.y * 3)};
		Vector2 pureResult = RotateVector2(pureCoord , clockWise); 
		beads[i].logicalCoordinate = {pureResult.x + (sectionCoord.x * 3) , pureResult.y + (sectionCoord.y * 3)};
	}
}

int HandleSectionChoosing(Vector2* focusedSectionCoord) // return type of this func means if any Section where choosen ( 0:none 1:clockWise 2:antiClockWise)
{
	bool correctInputRecieved = false;
	while(correctInputRecieved == false)
	{
	    char input = getch();
		switch(input)
		{
			case 'w':
				if(focusedSectionCoord->y == 0)
					break;
				focusedSectionCoord->y--;
				correctInputRecieved = true;
				break;
			case 's':
				if(focusedSectionCoord->y == 1)
					break;
				focusedSectionCoord->y++;
				correctInputRecieved = true;
				break;
			case 'a':
				if(focusedSectionCoord->x == 0)
					break;
				focusedSectionCoord->x--;
				correctInputRecieved = true;
				break;
			case 'd':
				if(focusedSectionCoord->x == 1)
					break;
				focusedSectionCoord->x++;
				correctInputRecieved = true;
				break;
			case 'r':
				return 1;
			case 'l':
				return 2;	
		}
	}
	return 0;
}

bool HandleCellChoosing(Vector2* focusedCellCoord) // return type of this func means if any cells where choosen;
{
	bool correctInputRecieved = false;
	while(correctInputRecieved == false)
	{
	    char input = getch();
		switch(input)
		{
			case 'w':
				if(focusedCellCoord->y == 0)
					break;
				focusedCellCoord->y--;
				correctInputRecieved = true;
				break;
			case 's':
				if(focusedCellCoord->y == 5)
					break;
				focusedCellCoord->y++;
				correctInputRecieved = true;
				break;
			case 'a':
				if(focusedCellCoord->x == 0)
					break;
				focusedCellCoord->x--;
				correctInputRecieved = true;
				break;
			case 'd':
				if(focusedCellCoord->x == 5)
					break;
				focusedCellCoord->x++;
				correctInputRecieved = true;
				break;
			case '\r':
				return true;
		}
	}
	return false;
}

bool CanInstertBeadAt(Board board, Vector2 coordinate)
{
	for(int i=0; i<board.beadsCount; i++)
	{
		if((board.beads[i].presenter == Shape_One || board.beads[i].presenter == Shape_Two)  && board.beads[i].logicalCoordinate.x == coordinate.x && board.beads[i].logicalCoordinate.y == coordinate.y)
			return false;
	}
	
	return true;
}

void InsertBead(Board* board, Player currentPlayer, Vector2 logicalCoordinate)
{
	Bead newBead;
	newBead.presenter = currentPlayer.presentor;
	newBead.logicalCoordinate = logicalCoordinate;

	board->beads[board->beadsCount] = newBead;
	board->beadsCount++;	
}

//Result Value Meaning: -1:inProgress 0:Draw 1:PlayerOneWin 2:PlayerTwoWin 
int CheckGameEnd(Bead beads[], int beadsCount)
{
	for(int i = 0 ; i < beadsCount; i++)
	{
		int beadX = beads[i].logicalCoordinate.x;
		int beadY = beads[i].logicalCoordinate.y;

		int inRowCount = 0;	
		for(int n= beadX-1 ; n>= 0 ; n--)
		{
			if(IsAnySameBeadAt({n, beadY}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		for(int n= beadX + 1 ; n< 6 ; n++)
		{
			if(IsAnySameBeadAt({n, beadY}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		if(inRowCount >= 4)
			return (beads[i].presenter == Shape_One ? 1 : 2);
			
		inRowCount = 0;
		for(int n= beadY - 1 ; n>= 0 ; n--)
		{
			if(IsAnySameBeadAt({beadX, n}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		for(int n= beadY + 1 ; n< 6 ; n++)
		{
			if(IsAnySameBeadAt({beadX, n}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		if(inRowCount >= 4)
			return (beads[i].presenter == Shape_One ? 1 : 2);
			
		inRowCount = 0;
		for(int n= 1 ; (beadX + n < 6 && beadY + n < 6)  ; n++)
		{
			if(IsAnySameBeadAt({beadX + n, beadY + n}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		for(int n= 1 ; (beadX - n >= 0 && beadY - n >= 0)  ; n++)
		{
			if(IsAnySameBeadAt({beadX - n, beadY - n}, beads, beadsCount, beads[i].presenter))
				inRowCount++;
			else
				break;
		}
		if(inRowCount >= 4)
			return (beads[i].presenter == Shape_One ? 1 : 2);
	}
	
	if(beadsCount == 36)
		return 0;
		
	return -1;
}

bool IsAnySameBeadAt(Vector2 coord, Bead beads[], int beadsCount, char presenter)
{
	for(int i=0; i<beadsCount ; i++)
	{
		if(Vector2sAreEqual(beads[i].logicalCoordinate, coord) && beads[i].presenter == presenter)
			return true;
	}
	
	return false;
}

void ShowGameResult(int gameResult, Player players[])
{
	gotoxy(ConvertLogicalCoordToVisual({1, 7}));
	if(gameResult == 0)
		cout<< "GAME FINISHED WITH RESULT OF DRAW.";
	else
	{
		cout << (gameResult == 1? players[0].name : players[1].name);
		cout << " Won The Game";
	}
	
	char input;
	while(input != '\r')
	{
		input = getch();
	}
	Init();
}

void SaveGameResult(int gameResult, Player players[])
{
    FILE *scoreboardfile = fopen("scoreBoard.txt", "a");
    
	if(gameResult == 1)
	{
		players[0].score+=3;
		players[0].winCount++;
		players[1].looseCount++;	
	}
	else if(gameResult == 2)
	{
		players[1].score+=3;
		players[1].winCount++;
		players[0].looseCount++;	
	}
	else 
	{
		players[0].score+=1;
		players[1].score+=1;
		players[0].drawCount++;
		players[1].drawCount++;
	}

	for(int i=0; i<2; i++)
		fprintf(scoreboardfile, "Name:%s win:%d loose:%d draw:%d Score:%d\n", players[i].name, players[i].winCount, players[i].looseCount, players[i].drawCount, players[i].score);	

    
	fclose(scoreboardfile);

}
