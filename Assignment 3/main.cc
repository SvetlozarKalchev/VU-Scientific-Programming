#include <iostream>
#include <vector>
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
    /* Allocate matrix */
    int matrix_size = 400;

    vector<vector<int>> global_matrix = create_matrix(matrix_size);

    print_matrix(global_matrix);

    /** Number of iterations is equal to the matrix size **/
    int iterations = matrix_size;

    /** Set number of threads and how many rows each thread will be getting **/
    int number_of_threads = 1;
    int chunk_size = matrix_size / number_of_threads;

    omp_set_num_threads(number_of_threads);

    /** Allocate evolver object and call main method **/
    Matrix_Evolver evolver;

    for(int i = 0; i < iterations; i++)
    {
        evolver.work(global_matrix, matrix_size, chunk_size);
    }

    print_matrix(global_matrix);

    return 0;
}
