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

    int message[2] = {0, 0};

    // do check rank and read file
    if(rank == 0)
    {
        while(getline(path_File,line))
        {
            path_to_number += line;
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
        cout << '\n' << '\n' << endl;

        /* Receive either the number in the file or the rank of the process that has no access */
        int number_from_file = 0;
        int sum = 0;
        int rank_without_access = 0;

        for(int process_number = 1; process_number < size; process_number++)
        {
            MPI_Recv(message, 2, MPI_INT, process_number, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout << "Process #0 received " << message[1]  << " from process #" << process_number << endl;
            /* IF file can't be opened */
            if(message[0] == 0)
            {
                cout << "File does NOT exist for process #" << message[1] << endl;
            }
            else
            {
                sum += message[1];
            }
        }

        if(message[0] == 1)
        {
            cout << "Sum of received numbers is " << sum << endl;
        }

        cout << '\n' << '\n' << endl;
    }
    else
    {
        /* Probe the incoming message for its metadata */
        MPI_Probe(0,  0, MPI_COMM_WORLD, &stat);
        /* Get length of incomming message buffer */
        MPI_Get_count(&stat, MPI_CHAR, &response_length);

        char buffer[response_length];

        /* Receive message and store in in the pre-allocated buffer */
        MPI_Recv(buffer, response_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "Received " << buffer<< endl;

        /*
            Try to open the file that contains a number and send the contents to process #0.
            Message is structured like that -> [file_exists, value]
        */
        ifstream file_with_number(buffer);
        string line;
        int number;

        if(file_with_number.good())
        {
            cout << "File exists for process #" << rank << endl;

            while(getline(file_with_number, line))
            {
                cout << "Read " << line << endl;
                number = atoi(line.c_str());
            }
            file_with_number.close();

            cout << "Converted " << number << endl;

            message[0] = 1;
            message[1] = number;

            MPI_Send(message, 2, MPI_INT, 0, 1, MPI_COMM_WORLD);
            cout << "Sent " << message[1] << " to rank 0" << '\n' << endl;
            return 0;
        }
        else
        {
            cout << "File does NOT exist for process #" << rank << endl;
            file_with_number.close();

            message[0] = 0;
            message[1] = rank;

            MPI_Send(message, 2, MPI_INT, 0, 1, MPI_COMM_WORLD);
            cout << "Notified process # 0"  << '\n' << endl;
            return 0;
        }
    }
    err = MPI_Finalize();
    return 0;
}
