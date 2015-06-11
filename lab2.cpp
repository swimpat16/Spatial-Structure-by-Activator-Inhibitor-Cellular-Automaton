#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
using namespace std;

int H, R_1, R_2, runID, P;
double J_1, J_2;

float correlation_avg[14];
float entropy_avg;
float joint_entropy_avg[15];
float mutual_info_avg[15];

class CA
{
	public:
	
		CA(int x, int y)
		{
			srand(time(NULL));
			
			rows = x;
			cols = y;
			
			board.resize(rows);
			for (int i = 0; i < rows; i++)
			{
				board[i].resize(cols);
				for (int j = 0; j < cols; j++)
				{
					board[i][j] = rand() % 2;
					if (board[i][j] == 0)
						board[i][j] = -1;
				}
			}
		}
		
		void printToConsole()
		{
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					if (board[i][j] == 1)
						cout << "X";
					else
						cout << " ";
				}
				cout << endl;
			}
		}
		
		void savePGM(char * filename)
		{
			ofstream fout(filename, ios_base::out);
			
			if (!fout.is_open())
				exit(1);
				
			fout << "P2" << endl;
			fout << rows * P << " " << cols * P << endl;
			fout << "255" << endl;
				
			for (int i = 0; i < rows; i++)
			{
				for (int a = 0; a < P; a++)
				{
					for (int j = 0; j < cols; j++)
					{
						if (j != 0)
							fout << " ";
						if (board[i][j] == 1)
						{
							for (int b = 0; b < P; b++)
							{
								if (b != 0)
									fout << " ";
								fout << "255";
							}
						}
						else
						{
							for (int b = 0; b < P; b++)
							{
								if (b != 0)
									fout << " ";
								fout << "0";
							}
						}
					}
					fout << endl;
				}
			}
			
			fout.close();
		}
		
		void saveCSV(char * filename)
		{
			ofstream fout(filename, ios_base::out);
			
			if (!fout.is_open())
				exit(1);
				
			char line[1024];
			
			fout <<  "correlation, entropy, joint_entropy, mutual_info, J_1, J_2, R_1, R_2, H, variance\n";
			
			sprintf(line, ", %f, %f, %f, %f, %f, %d, %d, %d, %s\n", entropy, joint_entropy[0], mutual_info[0], J_1, J_2, R_1, R_2, H, getVariance());
			
			fout << line;
			
			for (int i = 1; i < 15; i++)
			{
				sprintf(line, "%f, , %f, %f\n", correlation[i - 1], joint_entropy[i], mutual_info[i]);
				fout << line;
			}
			
			fout.close();
		}
		
		void stabilizeBoard()
		{
			vector< vector<int> > oldBoard;
			bool keepGoing = true;
			
			while (keepGoing)
			{
				oldBoard = board;
				updateBoard();
				keepGoing = false;
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						if (board[i][j] != oldBoard[i][j])
						{
							keepGoing = true;
							break;
						}
					}
					if (keepGoing)
						break;
				}
			}
			
			calculateCorrelation();
			calculateEntropy();
			calculateJointEntropy();
			calculateMutualInfo();
		}
		
		void printCorrelation()
		{
			for (int i = 0; i < 14; i++)
			{
				printf("Correlation[%2d]: %f\n", i + 1, correlation[i]);
			}
		}
		
		void printEntropy()
		{
			printf("Entropy: %f\n", entropy);
		}
		
		void printJointEntropy()
		{
			for (int i = 0; i < 15; i++)
			{
				printf("Joint_Entropy[%2d]: %f\n", i, joint_entropy[i]);
			}
		}
		
		void printMutualInfo()
		{
			for (int i = 0; i < 15; i++)
			{
				printf("Mutual_Info[%2d]: %f\n", i, mutual_info[i]);
			}
		}
		
		void updateGlobals()
		{
			for (int i = 0; i < 14; i++)
			{
				correlation_avg[i] += correlation[i];
			}
			
			entropy_avg += entropy;
			
			for (int i = 0; i < 15; i++)
			{
				joint_entropy_avg[i] += joint_entropy[i];
				mutual_info_avg[i] += mutual_info[i];
			}
		}
	
	private:
	
		void updateBoard()
		{
			int index, nextUpdate;
			vector<int> toBeUpdated;
			toBeUpdated.resize(0);
			
			for (int i = 0; i < rows * cols; i++)
			{
				toBeUpdated.push_back(i);
			}
			
			while(toBeUpdated.size() != 0)
			{
				index = rand() % toBeUpdated.size();					// get random un-updated index
				nextUpdate = toBeUpdated[index];						// get random un-updated cell from index
				calculateUpdate(nextUpdate / cols, nextUpdate % cols);	// update cell
				toBeUpdated.erase(toBeUpdated.begin() + index);			// remove index from vector
			}
			
			return;
		}
		
		void calculateUpdate(int x, int y)
		{
			double s1 = 0, s2 = 0;
			int farCells = 0;
			int nearCells = 0;
			int d = 0;
			
			for(int j = 0; j < cols; j++)
			{
				for(int i = 0; i < rows; i++)
				{
					d = distance(i, j, x, y);
					if(d < R_1)
						nearCells += board[i][j];
					else if(d < R_2)
						farCells += board[i][j];
							
				}
			}
			
			s1 = farCells * J_2;
			s2 = nearCells * J_1;
			
			
			if(H + s1 + s2 >= 0)
				board[x][y] = 1;
			else
				board[x][y] = -1;
		}
		
		int distance(int i_x, int i_y, int j_x, int j_y)
		{
			int a, b;
	
			a = abs(i_x - j_x);
			b = abs(i_y - j_y);
	
			if(a > 15)
				a = 30 - a;
		
			if(b > 15)
				b = 30 - b;
		
			return a + b;
		}
		
		int beta(int s)
		{
			return (s + 1) / 2;
		}
		
		void calculateCorrelation()
		{
			int sum1[14], sum2[14];
			double c1[14];
			double c2;
			
			c2 = (1.0 / (rows * cols));
			
			for (int a = 0; a < 14; a++)
			{
				sum1[a] = 0;
				sum2[a] = 0;
				c1[a] = (2.0 / (rows * cols * 4 * (a + 1)));
				
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						for (int m = 0; m < rows; m++)
						{
							for (int n = 0; n < cols; n++)
							{
								if ((i < m) || ((i == m) && (j < n)))
								{
									if (distance(i, j, m, n) == (a + 1))
									{
										sum1[a] += board[i][j] * board[m][n];
									}
								}
							}
						}
						sum2[a] += board[i][j];
					}
				}
				correlation[a] = abs((c1[a] * sum1[a]) - (pow((c2 * sum2[a]), 2)));
			}
		}
		
		void calculateEntropy()
		{
			int sum = 0;
			double p_live;
			double p_dead;
			
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					if (board[i][j] == 1)
						sum++;
				}
			}
			
			p_live = ((1.0 * sum) / (rows * cols));
			p_dead = 1.0 - p_live;
			
			if(p_live == 0.0)
			{
				entropy = 0.0 - (p_dead * log(p_dead));
			}
			else if(p_dead == 0.0)
			{
				entropy = 0.0 - (p_live * log(p_live));
			}
			else
			{
				entropy = 0.0 - (p_live * log(p_live)) - (p_dead * log(p_dead));
			}
		}
	
		void calculateJointEntropy()
		{
			int sum1[15], sum2[15];
			double c[15];
			double p_ll[15], p_dd[15], p_ld[15];
			
			
			for (int a = 0; a < 15; a++)
			{
				sum1[a] = 0;
				sum2[a] = 0;
				
				if (a == 0)
					c[1] = (1.0 / (rows * cols));
				else
					c[a] = (2.0 / (rows * cols * 4 * a));
				
				for (int i = 0; i < rows; i++)
				{
					for (int j = 0; j < cols; j++)
					{
						for (int m = 0; m < rows; m++)
						{
							for (int n = 0; n < cols; n++)
							{
								if ((i < m) || ((i == m) && (j < n)))
								{
									if (distance(i, j, m, n) == a)
									{
										sum1[a] += beta(board[i][j]) * beta(board[m][n]);
										sum2[a] += beta(-board[i][j]) * beta(-board[m][n]);
									}
								}
							}
						}
					}
				}
				p_ll[a] = 1.0 * c[a] * sum1[a];
				p_dd[a] = 1.0 * c[a] * sum2[a];
				p_ld[a] = 1.0 - p_ll[a] - p_dd[a];
				
				if (p_ll[a] != 0.0)
					p_ll[a] *= log(p_ll[a]);
				if (p_dd[a] != 0.0)
					p_dd[a] *= log(p_dd[a]);
				if (p_ld[a] != 0.0)
					p_ld[a] *= log(p_ld[a]);
				
//				joint_entropy[a] = 1.0 - p_ll[a] - p_dd[a] - p_ld[a];
				joint_entropy[a] = -(p_ll[a] + p_dd[a] + p_ld[a]);
			}
		}
		
		void calculateMutualInfo()
		{
			for(int a = 0; a < 15; a++)
			{
				mutual_info[a] = (2.0 * entropy) - joint_entropy[a];	
			}
		}
		
		char * getVariance()
		{
			bool all_white = true;
			bool all_black = true;
			
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					if (board[i][j] == 1)
						all_black = false;
					if (board[i][j] == -1)
						all_white = false;
				}
			}
			
			if (all_white)
				return "all_white";
			else if (all_black)
				return "all_black";
			else
				return "varied";
		}
		
		double correlation[14];
		double entropy;
		double joint_entropy[15];
		double mutual_info[15];
		int rows;
		int cols;
		vector <vector <int> > board;
};

void resetGlobals()
{
	for (int i = 0; i < 14; i++)
	{
		correlation_avg[i] = 0.0;
	}
	
	entropy_avg = 0.0;
	
	for (int i = 0; i < 15; i++)
	{
		joint_entropy_avg[i] = 0.0;
		mutual_info_avg[i] = 0.0;
	}
}

void averageGlobals()
{
	for (int i = 0; i < 14; i++)
	{
		correlation_avg[i] /= 4.0;
	}
	
	entropy_avg /= 4.0;
	
	for (int i = 0; i < 15; i++)
	{
		joint_entropy_avg[i] /= 4.0;
		mutual_info_avg[i] /= 4.0;
	}
}

void saveAVG(char * filename)
{
	ofstream fout(filename, ios_base::out);
			
	if (!fout.is_open())
		exit(1);
		
	char line[1024];
	
	fout <<  "correlation, entropy, joint_entropy, mutual_info, J_1, J_2, R_1, R_2, H\n";
	
	sprintf(line, ", %f, %f, %f, %f, %f, %d, %d, %d\n", entropy_avg, joint_entropy_avg[0], mutual_info_avg[0], J_1, J_2, R_1, R_2, H);
	
	fout << line;
	
	for (int i = 1; i < 15; i++)
	{
		sprintf(line, "%f, , %f, %f\n", correlation_avg[i - 1], joint_entropy_avg[i], mutual_info_avg[i]);
		fout << line;
	}
	
	fout.close();
}

int main(int argc, char** argv)
{

	char filename1[1024];
	char filename2[1024];
	char filename3[1024];
	
	int R_1_arr[] = {1, 3, 5, 7, 9, 11};
	int R_2_arr[] = {2, 4, 6, 8, 10, 12};
	int H_arr[] = {0, 3, 5, -3, -5};
	
	J_1 = 1;
	J_2 = -0.1;
	P = 5;
	
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (R_1_arr[i] >= R_2_arr[j])
				continue;
				
			for (int k = 0; k < 5; k++)
			{
				printf("Running: R1 = %2d, R2 = %2d, H = %2d\n", R_1_arr[i], R_2_arr[j], H_arr[k]);
				
				H = H_arr[k];
				R_1 = R_1_arr[i];
				R_2 = R_2_arr[j];
				
				resetGlobals();
				
				for (int l = 0; l < 4; l++)
				{
					sprintf(filename1, "data/%d_%d_%d_%d.pgm", R_1_arr[i], R_2_arr[j], H_arr[k], l);
					sprintf(filename2, "data/%d_%d_%d_%d.csv", R_1_arr[i], R_2_arr[j], H_arr[k], l);
				
					CA ca(30,30);
					ca.stabilizeBoard();
					ca.savePGM(filename1);
					ca.saveCSV(filename2);
					ca.updateGlobals();
				}
				
				averageGlobals();
				
				sprintf(filename3, "data/%d_%d_%d_avg.csv", R_1_arr[i], R_2_arr[j], H_arr[k]);
				saveAVG(filename3);
			}
		}
	}
	
	return 1;
}