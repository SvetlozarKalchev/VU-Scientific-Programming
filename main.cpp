#include <iostream>
#include <fstream>
#include <mpi/mpi.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[])
{
    /*
        MPI Variables
    */
    int err, rank, size;
    err = MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status stat;

    /*
        Program variables
    */
    int response_length = 0;
    string line;
    string path_to_number;

    ifstream path_File("path.txt");

    // do check rank and read file
    if(rank == 0)
    {
        while(getline(path_File,line))
        {
            path_to_number += line;
            cout << line << endl;
        }
        path_File.close();

        cout << "Line contains: " << path_to_number  << " with length " << path_to_number.length()<< endl;

        // Send file path to other processes
        for(int process_number = 1; process_number < size; process_number++)
        {
            // Msg length is string.length() + 1 to have space for the null terminator
            MPI_Send(path_to_number.c_str(), path_to_number.length() + 1, MPI_CHAR, process_number, 0, MPI_COMM_WORLD);
            cout << "Sent " << path_to_number.c_str() << " to process#" << process_number <<  endl;
        }

//            string text;
//            ifstream file_with_number("path.txt");
//
//            while(getline(file_with_number, text))
//            {
//                cout << text << endl;
//            }

    }
    else
    {
        MPI_Probe(0,  0, MPI_COMM_WORLD, &stat);
        MPI_Get_count(&stat, MPI_CHAR, &response_length);

        char buffer[response_length];

        MPI_Recv(buffer, response_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        string result(buffer);
        cout << "Received " << buffer<< endl;

        // Open file, containing a number
        ifstream file_with_number(buffer);
        string line;
        int number;

        while(getline(file_with_number, line))
        {
            cout << "Read " << line << endl;
            number = atoi(line.c_str());
        }

        cout << "Converted " << number << endl;


    }
    err = MPI_Finalize();
    return 0;
}
