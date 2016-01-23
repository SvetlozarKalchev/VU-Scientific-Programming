#include <iostream>
#include <vector>

using namespace std;

class Matrix_Evolver
{
    bool is_pos_valid(int pos[], int matrix_size);
    vector<int> generate_border(int pos[2], int matrix_size);
    int evolve(int up, int right, int down, int left);
    void set_limits(int &row_start, int &row_limit, int number_of_threads);

    public:
    void work(vector<vector<int>> &matrix, int matrix_size, int chunk_size);
};


/*** Functions that validate matrix positions and generate borders */

/*
    Provides placeholder values for elemets that have less than 4 neighbours.
    Accepts an array with
*/
bool Matrix_Evolver::is_pos_valid(int pos[], int matrix_size)
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

vector<int> Matrix_Evolver::generate_border(int pos[2], int matrix_size)
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

int Matrix_Evolver::evolve(int up, int right, int down, int left)
{
    int sum = up + right + down + left;
    int value = (sum / 4);

    return value;
}

void Matrix_Evolver::work(vector<vector<int>> &global_matrix, int matrix_size, int chunk_size)
{
    /* Variables for the parallel construct and loop */
    int current_pos[2] = {0, 0};
    int rows_left = matrix_size;
    int row_start, row_limit;
    int col_limit = matrix_size;
    int up, right, down, left;
    vector<int> generated_borders;

    #pragma omp parallel default(shared) private(row_start)
    {
        cout << "THREAD NUMBER: " << omp_get_thread_num() << ' ' << endl;

        /* Create a local buffer matrix for each thread */
        //vector<vector<int>> local_matrix = create_null_matrix(matrix_size);

        int evolved_value = 0;
        /*
            Split work according to thread number. Each thread gets an even size of the matrix.
        */
        int number_of_threads = omp_get_num_threads();
        int thread_number = omp_get_thread_num();

        row_start = thread_number * chunk_size;
        row_limit = row_start + chunk_size;

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
                    //cout << "Generated borders: ";
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

                        //cout << generated_borders[i] << ' ';
                    }
                    //cout << endl;
                }
                /* Evolve cell */
                evolved_value = evolve(up, right, down, left);
                cout << "Evolved value: " << evolved_value << endl;

                /*
                    Write to buffer matrix after each cell evolution is complete.
                    Write to global matrix too.
                */
                //local_matrix[row][col] = evolved_value;
                //print_matrix(local_matrix);

                global_matrix[row][col] = evolved_value;
                //print_matrix(global_matrix);
            }
        }

        #pragma omp barrier
        /* END OF FOR LOOP */
        cout << "END" << endl;

    /* END OF WORKSHARING CONTRUCT*/
    }
}
