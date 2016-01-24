#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include "matrix_evolver.h"

using namespace std;

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

/*** END ***/
int main(int argc, char *argv[])
{
    /* Set matrix size and allocate it */
    int matrix_size = 500;

    vector<vector<int>> global_matrix = create_matrix(matrix_size);

    //print_matrix(global_matrix);

    /** Number of iterations is equal to the matrix size **/
    int iterations = 200;

    /** Set number of threads and how many rows each thread will be getting **/
    int number_of_threads = 2;

    int chunk_size = matrix_size / number_of_threads;

    omp_set_num_threads(number_of_threads);

    /** Allocate evolver object and call main method for the set number of iterations **/
    Matrix_Evolver evolver;

    /** Start measuring time here. **/
    auto start_time = chrono::high_resolution_clock::now();

    for(int i = 0; i < iterations; i++)
    {
        evolver.work(global_matrix, matrix_size, chunk_size);
    }

    /** Finish measuring time here. **/
    auto end_time = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> elapsed_time = end_time - start_time;

    auto elapsed_time_seconds = elapsed_time.count() / 1000;

    cout << "Finished " << iterations << " iterations on a " << matrix_size << "x" << matrix_size << " matrix " << " with " << number_of_threads << " threads in " << elapsed_time_seconds << " seconds." << endl;

    //print_matrix(global_matrix);

    return 0;
}
