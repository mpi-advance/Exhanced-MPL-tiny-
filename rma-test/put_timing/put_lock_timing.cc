#include <iostream>
#include <mpi.h>
#include <vector>
#include <bits/stdc++.h>

using namespace std;

#ifndef NITERS 
#define NITERS 1000
#endif

#ifndef SKIP
#define SKIP 10
#endif

int main(int argc, char* argv[])
{
    const mpl::communicator &comm_world(mpl::environment::comm_world());
  
    MPI_Win win;

    //window addresses
    vector<int> vec1(1000, 0);
    vector<int> vec2(1000, 1);   

    //timing 
    vector<double> times;
    double start;
    double end;
    double time;
    double total;
    double avg;
    double min;
    double msg_size;
    double win_size;
    double bw1;

    if(comm_world.rank() == 0)
    { 
        cout << "Vector 1: [ ";
        for (int i = 0; i < 2; i++)
        {
            cout << vec1[i] << " ";
        }
        cout << "]" << endl;
        MPI_Win_create(vec1.data(), vec1.size() * 4, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
    }
    else if (comm_world.rank() == 1)
    {
        cout << "Vector 2: [ ";
        for (int i = 0; i < 2; i++)
        {
            cout << vec2[i] << " ";
        }
        cout << "]" << endl;
        MPI_Win_create(NULL, vec2.size() * 4, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
    }

    comm_world.barrier();

    for (int i=0; i < NITERS; i++)
    {
        if(i < SKIP)
        {
            start = wtime();
        }

        lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);

        if(comm_world.rank() == 1)
        {
            put(vec2.data(), 0, win);
        }

        unlock(0, win);        
    }

    end = wtime();

    /* if (comm_world.rank() == 0)
    {
        cout << "Vector 1 after put: [ ";
        for (int i = 0; i < vec1.size(); i++)
            {
                cout << vec1[i] << " ";
            }
            cout << "]" << endl;
    } */

    total = end - start;
	avg = total / (NITERS - SKIP);

	msg_size = sizeof(int);
	win_size = vec1.size();
	double tmp = msg_size / 1e6 * win_size;
	bw = tmp / avg; 

	//MPI_Reduce(&avg, &combinedavg, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    // maybe this is correct? need to check what param F should be
    comm_world.reduce(MPI_DOUBLE, 0, &avg, &combinedavg);
    // MPI_Reduce(&bw, &combinedbw, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    comm_world.reduce(MPI_DOUBLE, 0, &bw, &combinedbw);

    if (comm_world.rank() == 0)
    {
        cout <<"Average time taken by one process (ms): " << combinedavg * 1000 << endl;
        cout << "Average bandwidth: " << (combinedbw / 2) << endl;
    } 

    MPI_Win_free(&win);
}
