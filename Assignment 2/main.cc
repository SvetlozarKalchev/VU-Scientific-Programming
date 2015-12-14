#include <iostream>
#include <mpi/mpi.h>
#include <vector>

using namespace std;
/*
    Checks if provided position is a valid one. Conditions for a valid position:
    1. y > 0 && y < end
    2. x > 0 && x < end
*/
int Y_END = 0;
int X_END = 0;

// A valid position is one for which no borders need to be generated
bool is_pos_valid(int pos[])
{
    int y = pos[0];
    int x = pos[1];

    cout << "Y: " << y << " and X:" << x;

    if(x > 0 && x < X_END && y > 0 && y < Y_END)
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
// Provides placeholder values for elemets that have less than 4 neighbours
vector<int> generate_border(int pos[2])
{
    int y = pos[0];
    int x = pos[1];
    /*
i
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
    /********************
        MPI Variables
    ********************/
    int err, rank, size;

    err = MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status stat;

    /********************
        Program Code
    ********************/
    int x = 8, y = 4;
    X_END = x;
    Y_END = y;

    /*
        Rank 0 will be the one that coordinates all other ranks.
        It will receive the calculated values, update the matrix with them
        and give the permission to start work on the updated values.

        Stuff to do:
        1. Read file
        2. Create matrix
        3. Decide how many workers to use
        4. Coordinate data between workerss

        Split the matrix on iteration 0.
        Send the each part to the corresponding worker
        Worker sends back its part
        Rank 0 rebuilds the global matrix
        Rank 0 sends OK message to other ranks to notify them they can begin working
    */
    //cout << "Hello! Size is: " << size << " and rank is: " << rank << endl;

    if(rank == 0)
    {
        vector<vector<int>> global_matrix(y, vector<int>(x));

        int iteration = 0;
        int fields = 0;
        int vector_size[2] = {y, x};

        // Populates test matrix
        for(int i = 0; i < y; i++)
        {
            for(int j = 0; j < x; j++)
            {
                global_matrix[i][j] = j+1;
            }
        }

        // Prints test matrix
        for(int i = 0; i < y; i++)
        {
            for(int j = 0; j < x; j++)
            {
                cout << global_matrix[i][j];
            }
            cout << endl;
        }
        cout << "Finished populating matrix" << endl;
        // As many workers as # of processes.
        int workers = size - 1;
        /*
            Matrix range for each worker to work on.
            Always contains values for 1 axis, because we are doing either a
            column-wise or a row-wise 1D distribution.

            Example: [0, 0, 7]
                First number indicates if we are splitting x or y.
                Numbers two and three are the range of the opposite axis that we are working on
        */
        int coordinates[3], matrix_row[x];
        bool split_x = false;
        int num_of_worker = 1;

        // Determine which axis has more elements and split it in n equal parts. n is a power of 2
        (x > y) ? split_x = true : split_x = false;

        // If the x axis has more fields, we do a column-wise 1D distribution.
        if(split_x)
        {
            coordinates[0] = 0;
            // Split the x axis fields in equal parts
            fields = x / workers;
            cout << "fields to split in: " << fields << endl;

            // For each row, get n number of positions to give to each worker.
            // Store the positions in a vector and give it to the worker.
            for(int col = 0; col < x; col+=fields)
            {
                // Start value
                coordinates[1] = col;

                for(int counter = 0; counter < fields; counter++)
                {
                    // End value
                    coordinates[2] = col + counter;
                }
                cout << "Worker " << num_of_worker << " gets all rows" << " and fields " << coordinates[1] << " to " << coordinates[2] << endl;
                // Send vector size to each worker
                MPI_Send(&vector_size, 2, MPI_INT, num_of_worker, 0, MPI_COMM_WORLD);
                // Send coordinates to worker
                MPI_Send(coordinates, 3, MPI_INT, num_of_worker, 1, MPI_COMM_WORLD);

                // Send initial matrix to each worker. We can't send a multidimensional vector at once. Every row must be copied to an array and sent separately.
                cout << "Matrix size " << global_matrix[0].size() << endl;
                for(int i = 0; i < global_matrix.size(); i++)
                {
                    for(int j = 0; j < x; j++)
                    {
                        matrix_row[j] = global_matrix[i][j];
                    }

                    MPI_Send(matrix_row, global_matrix[i].size(), MPI_INT, num_of_worker, 2, MPI_COMM_WORLD);
                }

                num_of_worker++;
            }
        }
        // If the x axis has more fields, we do a row-wise 1D distribution.
        else
        {

        }

        /*
            The data is already sent to each worker.
            1. Now process #0 must receive confirmation that each process has finished.
            2. Give them permission to update the global matrix.
            3. Receive confirmation that they finished updating.
            4. If there are more iterations, it must send permission to start work, using the updated values from the global matrix.
            5. Repeat untill iterations are over.
        */

    }
    else
    {
        // 1. Receive the matrix size. We need a local copy of the matrix, because we can't access the global one, since every process has its own memory.
        int vector_size[2];
        MPI_Recv(&vector_size, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Received vector size " << vector_size[0] << " and " << vector_size[1] << endl;
        Y_END = vector_size[0] - 1, X_END = vector_size[1] - 1;

        // Allocate a local copy of the global matrix with the received demensions
        vector<vector<int>> global_matrix(vector_size[0], vector<int>(vector_size[1]));
        vector<vector<int>> global_buffer(vector_size[0], vector<int>(vector_size[1]));

        int y = vector_size[0], x = vector_size[1];

        // Allocate a buffer for each row of the matrix
        int matrix_buffer[vector_size[1]];
        int coordinates[3];
        bool split_x = false;

        // 2. Receive coordinates
        MPI_Recv(coordinates, 3, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "Process #" << rank << " received " << coordinates[1] << " to " << coordinates[2] << endl;

        // 3. Receive matrix
        for(int i = 0; i < vector_size[0]; i++)
        {
            MPI_Recv(matrix_buffer, vector_size[1], MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Insert received row
            for(int el = 0; el < vector_size[1]; el++)
            {
                global_matrix[i][el] = matrix_buffer[el] ;
            }
        }

        // What kind of a split are we doing?
        (coordinates[0] == 0) ? split_x = true : split_x = false;

        /*
            Variables for the stencil operation
        */
        int current_pos[2] = {0, 0};
        bool valid = false;
        int up, right, down, left;
        vector<int> positions;
        vector<int> generated_borders;

        // If column-wise split, work on all rows
        if(split_x)
        {
            for(int row = 0; row <= Y_END; row++)
            {
                current_pos[0] = row;
                for(int col = coordinates[1]; col <= coordinates[2]; col++)
                {
                    current_pos[1] = col;
                    valid = is_pos_valid(current_pos);

                    if(valid)
                    {
                        up = global_matrix[row-1][col];
                        right = global_matrix[row][col+1];
                        down = global_matrix[row+1][col];
                        left = global_matrix[row][col-1];
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

                            cout << "Generated borders: " << generated_borders[i] << endl;
                        }
                    }
                    cout << "Values to evolve: " << up << " " << right << " " << down << " " << left << endl;
                    // Put evolved value in temporary array
                    global_buffer[row][col] = evolve(up, right, down, left);
                    cout << "Evolved value " << global_buffer[row][col] << endl;
                }
            }

        }
        else
        {

        }
    }
    err = MPI_Finalize();
    return 0;
}
