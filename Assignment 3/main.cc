#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;
/*
    Compiler and linker options, in order to run OpenMP programs:
    -fopenmp

    Algorithm:
    1. Create test matrices - 100 * 100
    2. Test with 4 threads
    3. Identify problem areas
*/

vector<vector<int>> create_matrix(int length)
{
    vector<vector<int>> matrix(length, vector<int>(length));

    for(int row = 0; row < length; row++)
    {
        for(int col = 0; col < length; col++)
        {
            matrix[row][col] = col;
        }
    }

    return matrix;
}


vector<vector<int>> create_null_matrix(int length)
{
    vector<vector<int>> matrix(length, vector<int>(length));

    // Fill the matrix with 0s
    for(int row = 0; row < length; row++)
    {
        for(int col = 0; col < length; col++)
        {
            matrix[row][col] = 0;
        }
    }

    return matrix;
}

void print_matrix(vector<vector<int>> matrix)
{
    int matrix_size = matrix.size();

    for(int row = 0; row < matrix_size; row++)
    {
        for(int col = 0; col < matrix_size; col++)
        {
           cout << matrix[row][col] << ' ';
        }

        cout << endl << endl;
    }
}

/*** Functions that validate matrix positions and generate borders */

/*
    Provides placeholder values for elemets that have less than 4 neighbours.
    Accepts an array with
*/
bool is_pos_valid(int pos[], int matrix_size)
{
    int y = pos[0];
    int x = pos[1];
    int limit = matrix_size - 1;

    cout << "Y: " << y << " and X:" << x;

    if(x > 0 && x < limit && y > 0 && y < limit)
    {
        cout << " is VALID" << endl;
        return true;
    }
    else
    {
        cout << " is INVALID" << endl;
        return false;
    }
}

vector<int> generate_border(int pos[2], int matrix_size)
{
    int y = pos[0];
    int x = pos[1];

    int X_END = matrix_size - 1;
    int Y_END = matrix_size - 1;
    /*
i
        [up, right, down, left] - ordered just the way the evolve function accepts them.
        Values that haven't been generated will be -1.
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

int evolve(int up, int right, int down, int left)
{
    int sum = up + right + down + left;
    int value = (sum / 4);

    return value;
}

/*** END ***/
int main(int argc, char *argv[])
{
    /* Allocate matrix */
    int matrix_size = 4;

    vector<vector<int>> global_matrix = create_matrix(matrix_size);

    print_matrix(global_matrix);

    /*******************/

    /* Variables for the parallel construct and loop */
    int current_pos[2] = {0, 0};
    int row_start;
    int row_limit = matrix_size;
    int col_limit = matrix_size;
    int up, right, down, left;
    vector<int> generated_borders;

    omp_set_num_threads(1);
    #pragma omp parallel default(shared) private(row_start)
    {
        cout << "THREAD NUMBER: " << omp_get_thread_num() << ' ' << endl;

        /* Create a local buffer matrix for each thread */
        vector<vector<int>> local_matrix = create_null_matrix(matrix_size);

        int evolved_value = 0;

        /* Split work according to thread number */
        int thread = omp_get_thread_num();

        if(thread == 0)
        {
            row_start = 0;
            row_limit = 2;
        }
        else
        {
            row_start = 20;
            row_limit = matrix_size;
        }

        #pragma omp parallel for default(shared) \
                                private(generated_borders, current_pos, up, right, down, left)
        /* Go through the matrix and evolve each cell */
        for(int row = row_start; row < row_limit; row++)
        {
            current_pos[0] = row;

            #pragma omp parallel for default(shared)
            for(int col = 0; col < col_limit; col++)
            {
                current_pos[1] = col;

                if(is_pos_valid(current_pos, matrix_size))
                {
                    up = global_matrix[row-1][col];
                    right = global_matrix[row][col+1];
                    down = global_matrix[row+1][col];
                    left = global_matrix[row][col-1];
                }
                else
                {
                    generated_borders = generate_border(current_pos, matrix_size);
                        /*
                            Iterate over received array with generated values.
                            No cell needs more than 2 borders generated, so the array will always contain at least two -1 values.
                            -1 denotes a border value that's already in the matrix.
                        */
                    cout << "Generated borders: ";
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
                                    break;
                            }
                        }
                        else
                        {
                            switch(i)
                            {
                                case 0:
                                    up = global_matrix[row-1][col];
                                    break;
                                case 1:
                                    right = global_matrix[row][col+1];
                                    break;
                                case 2:
                                    down = global_matrix[row+1][col];
                                    break;
                                case 3:
                                    left = global_matrix[row][col-1];
                                    break;
                                default:
                                    break;
                            }
                        }

                        cout << generated_borders[i] << ' ';
                    }
                    cout << endl;
                }
                /* Evolve cell */
                evolved_value = evolve(up, right, down, left);
                cout << "Evolved value: " << evolved_value << endl;

                /* Write to buffer matrix after each cell evolution is complete */
                local_matrix[row][col] = evolved_value;

                print_matrix(local_matrix);
            }
        }
        /* END OF FOR LOOP */
        cout << "END" << endl;
    }

    return 0;
}
