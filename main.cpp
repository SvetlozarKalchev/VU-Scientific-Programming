#include <iostream>
#include <fstream>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

string read_File_Path()
{
    ifstream path_File("path.txt");
    string line;
    string path;

    while(getline(path_File,line))
    {
        path += line;
    }
    path_File.close();

    return path;
}

void send_File_Path_To_All_Ranks(const string &path_to_number, int size)
{
    /**
        C++ strings are convertible to const void * , but MPI.Send() needs void *, so
        convert to char[].
    **/
    char *text = new char[path_to_number.length()+1];

    for(int i = 0; i < path_to_number.length()+1; i++)
    {
        text[i] = path_to_number[i];
    }

    /** Send file path to all other processes **/
        for(int process_number = 1; process_number < size; process_number++)
        {
            /** Message length is path.length() + 1 to have space for the null terminator **/
            MPI_Send(&text, path_to_number.length() + 1, MPI_CHAR, process_number, 0, MPI_COMM_WORLD);
        }
        cout << '\n' << endl;
}

void receive_Data_From_All_Ranks(int message[2], int size)
{
        int sum = 0;

        for(int process_number = 1; process_number < size; process_number++)
        {
            MPI_Recv(message, 2, MPI_INT, process_number, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            /*************************************************************************
                If file can't be opened -> print process#
                Else                    -> calculate the sum of all received numbers
             ************************************************************************/
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
}

int main(int argc, char *argv[])
{
    /********************
        MPI Variables
    ********************/
    int err, rank, size;
    err = MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status stat;

    /*********************
        Program variables
    *********************/
    int response_length = 0;
    int message[2] = {0, 0};

    if(rank == 0)
    {
        string path_to_number = read_File_Path();

        send_File_Path_To_All_Ranks(path_to_number, size);

        receive_Data_From_All_Ranks(message, size);
    }
    else
    {
        /** Probe the incoming message for its metadata **/
        MPI_Probe(0,  0, MPI_COMM_WORLD, &stat);

        /** Get length of incomming message, so the exact amount of memory can be allocated**/
        MPI_Get_count(&stat, MPI_CHAR, &response_length);

        char buffer[response_length];

        /** Receive message and store in in the pre-allocated buffer **/
        MPI_Recv(buffer, response_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        /***
            Try to open the file that contains a number and send the contents to process #0.
            Message structure -> [file_exists, number_value/process#]
        ***/
        ifstream file_with_number(buffer);
        string line;
        int number;

        if(file_with_number.good())
        {
            while(getline(file_with_number, line))
            {
                number = atoi(line.c_str());
            }
            file_with_number.close();

            message[0] = 1;
            message[1] = number;

            MPI_Send(message, 2, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
        else
        {
            file_with_number.close();

            message[0] = 0;
            message[1] = rank;

            MPI_Send(message, 2, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }

    err = MPI_Finalize();
    return 0;
}
