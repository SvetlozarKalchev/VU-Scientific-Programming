#include <iostream>
#include <mpi/mpi.h>
#include <vector>

using namespace std;
/*
    Steps:
    -DONE 1. Unserstand the algorithm
    2. Create a sequential version
    3. Decide how to split the data between processes
    4. Create a parallel version

    Algorithm:
    1. Go over each value
    2. Calculate new value
    3. Write value to new matrix
    4. Copy new matrix to old matrix
*/

/*
    Checks if provided position is a valid one. Conditions for a valid position:
    1. y > 0 && y < end
    2. x > 0 && x < end
*/
int Y_END = 9;
int X_END = 9;

// A valid position is one for which no borders need to be generated
bool is_pos_valid(int pos[])
{
    int y = pos[0];
    int x = pos[1];

    cout << "Y: " << y << " and X:" << x;

    if(x > 0 && x < X_END && y > 0 && y < Y_END)
    {
        cout << "is VALID" << endl;
        return true;
    }
    else
    {
        cout << "is INVALID" << endl;
        return false;
    }
}
// Provides placeholder values for elemets that have less than 4 neighbours
vector<int> generate_border(int pos[2])
{
    int y = pos[0];
    int x = pos[1];

    /*
        [up, right, down, left] - ordered just right for the evolve function
        Values that haven't been generated will be -1
    */
    vector<int> missing_positions = {-1, -1, -1, -1};

    // Default border values
    int top_border = 8;
    int right_border = 16;
    int bottom_border = 32;
    int left_border = 64;

    // Array positions for each direction
    int top = 0;
    int right = 1;
    int bottom = 2;
    int left = 3;

    if(y == 0)
    {
        // Upper left element -> needs left and top border
        if(x == 0)
        {
            missing_positions[left] = left_border;
            missing_positions[top] = top_border;
        }
        // Upper right element -> needs right and top border
        else if(x == X_END)
        {
            missing_positions[right] = right_border;
            missing_positions[top] = top_border;
        }
        // All other elements on the first row need just a top border
        else
        {
            missing_positions[top] = top_border;
        }
    }
    else if(y == Y_END)
    {
        // Lowest left element -> needs left and bottom border
        if(x == 0)
        {
            missing_positions[left] = left_border;
            missing_positions[bottom] = bottom_border;
        }
        // Upper right element -> needs right and bottom border
        else if(x == X_END)
        {
            missing_positions[right] = right_border;
            missing_positions[bottom] = bottom_border;
        }
        // All other elements on the first row need just a bottom border
        else
        {
            missing_positions[bottom] = bottom_border;
        }
    }
    else
    {
         // Most left element -> needs left border
        if(x == 0)
        {
            missing_positions[left] = left_border;
        }
        // Most right element -> needs right border
        else if(x == X_END)
        {
            missing_positions[right] = right_border;
        }
    }

    return missing_positions;
}
// Calculates values for each position
int evolve(int up, int right, int down, int left)
{
    //int value = ((positions[0] + positions[1] + positions[2] + positions[3]) / 4);
    int sum = up + right + down + left;
    int value = (sum / 4);

    return value;
}

int main(int argc, char* argv[])
{
    int matrix[10][10] = {
                            1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            1, 2, 3, 4, 5, 6, 7, 8, 9, 10
                        };

    int total_iterations = 1;
    // {y, x}, just like in the matrix
    int current_pos[2] = {0, 0};
    bool valid = false;
    int up, right, down, left;
    vector<int> positions;
    vector<int> generated_borders;

    // For each iteration, go ever the matrix and evolve it
    for(int iteration = 0; iteration < total_iterations; iteration++)
    {
        for(int y = 0; y <= Y_END; y++)
        {
            for(int x = 0; x <= X_END; x++)
            {
                /*
                    Check if position is valid

                    If TRUE
                        ->  evolve
                    If FALSE
                        -> generate missing values
                        -> populate array with positions
                        -> evolve

                    Store each evolved value in the next matrix
                */
                current_pos[0] = y;
                current_pos[1] = x;
                valid = is_pos_valid(current_pos);

                if(valid)
                {
                    up = matrix[y-1][x];
                    right = matrix[y][x+1];
                    down = matrix[y+1][x];
                    left = matrix[y][x-1];
                }
                else
                {
                    generated_borders = generate_border(current_pos);
                    /*
                        Iterate over received array with generated values.
                        No cell needs more than 2 borders generated, so the array will always contain at least two -1 values.
                        -1 denotes a border value that's already in the matrix.
                    */
                    for(int i = 0; i < 4; i++)
                    {
                        if(generated_borders[i] != -1)
                        {
                            switch(i)
                            {
                                case 0:
                                    up = generated_borders[i];
                                    break;
                                case 1:
                                    right = generated_borders[i];
                                    break;
                                case 2:
                                    down = generated_borders[i];
                                    break;
                                case 3:
                                    left = generated_borders[i];
                                    break;
                                default:
                                    continue;
                            }
                        }
                        else
                        {
                            switch(i)
                            {
                                case 0:
                                    up = matrix[y-1][x];
                                    break;
                                case 1:
                                    right = matrix[y][x+1];
                                    break;
                                case 2:
                                    down = matrix[y+1][x];
                                    break;
                                case 3:
                                    left = matrix[y][x-1];
                                    break;
                                default:
                                    continue;
                            }
                        }
                        cout << "Generated borders: " << generated_borders[i] << endl;
                    }

                // Evolve the value
                cout << "Evolved value: " << evolve(up, right, down, left) << endl;
                }
            }
        }
    }

    //cout << matrix[0][2] << endl;

//    /********************
//        MPI Variables
//    ********************/
//    int err, rank, size;
//
//    err = MPI_Init(&argc, &argv);
//
//    MPI_Comm_size(MPI_COMM_WORLD, &size);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//
//    MPI_Status stat;
//
//    /********************
//        Program Code
//    ********************/
//    cout << "Hello! Size is: " << size << " and rank is: " << rank << endl;
//
//    err = MPI_Finalize();
//    return 0;
}
