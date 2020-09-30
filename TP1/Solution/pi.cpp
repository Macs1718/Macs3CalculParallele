# include <chrono>
# include <random>
# include <iostream>
# include <cstdio>
# include <fstream>
# include <cstdlib>
# include <sstream>
# include <string>
# include <iomanip>
# include <mpi.h>


// Attention , ne marche qu'en C++ 11 ou supérieur :
double approximate_pi ( unsigned long nbSamples, int nb_samples_total, int rank, int size, const MPI_Comm& glob ) {

    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now ();    
    myclock :: duration d = myclock :: now()-beginning;
    unsigned seed = rank+d.count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution ( -1.0 ,1.0);
    unsigned long nbDarts = 0;
    // Throw nbSamples darts in the unit square [ -1:1] x [ -1:1]
    for ( unsigned sample = 0; sample < nbSamples ; ++sample ) {
        double x = distribution ( generator );
        double y = distribution ( generator );
        // Test if the dart is in the unit disk
        if ( x*x+y*y <=1 ) nbDarts ++;
    }
    // Récuper le nombre total de nbDarts qu'on a obtenu sur tous les processeurs sur le proc zéro qui aura seul la solution
    if (rank == 0)
	    for ( int p = 1; p < size; ++p)
	    {
		    MPI_Status status;
		    int nb_darts;
		    MPI_Recv(&nb_darts, 1, MPI_UNSIGNED_LONG, p, 101, glob, &status);
		    //std::cout << "recu de " << p << " sont nombre de flechettes" << std::flush << std::endl;
		    nbDarts += nb_darts;
	    }
    else
    {
    MPI_Send(&nbDarts, 1, MPI_UNSIGNED_LONG, 0, 101, glob);
    }
    // Number of nbDarts throwed in the unit disk
    double ratio = double (nbDarts)/ double (nb_samples_total);

    return ratio ;
}


int main( int nargs, char* argv[] )
{
        // On initialise le contexte MPI qui va s'occuper :
        //    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
        //       et assurer la cohésion de l'ensemble des processus créés par MPI;
        //    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
        //       le communicateur COMM_WORLD
        //    3. etc...
        MPI_Init( &nargs, &argv );
        // Pour des raisons de portabilité qui débordent largement du cadre
        // de ce cours, on préfère toujours cloner le communicateur global
        // MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
        MPI_Comm globComm;
        MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
        // On interroge le communicateur global pour connaître le nombre de processus
        // qui ont été lancés par l'utilisateur :
        int nbp;
        MPI_Comm_size(globComm, &nbp);
        // On interroge le communicateur global pour connaître l'identifiant qui
        // m'a été attribué ( en tant que processus ). Cet identifiant est compris
        // entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
        // l'utilisateur )
        int rank;
        MPI_Comm_rank(globComm, &rank);
        // Création d'un fichier pour ma propre sortie en écriture :
        std::stringstream fileName;
        fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
        std::ofstream output( fileName.str().c_str() );

        int nbSamples = 1000;
	if (nargs > 1) nbSamples = std::stoi(argv[1]);
	int nbSamples_local = nbSamples/nbp;
	if (rank < nbSamples%nbp) nbSamples_local += 1;

        double r = approximate_pi(nbSamples_local, nbSamples, rank, nbp, globComm);
	output << "pi = " << std::setprecision(10) << 4*r << std::endl;

        output.close();
        // A la fin du programme, on doit synchroniser une dernière fois tous les processus
        // afin qu'aucun processus ne se termine pendant que d'autres processus continue à
        // tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
        // qui ne seront pas encore terminés.
        MPI_Finalize();
        return EXIT_SUCCESS;
}

