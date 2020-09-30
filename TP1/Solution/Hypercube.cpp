# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>

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

	int token;
	MPI_Status status;
	if (0 == rank) token = 101;
	/* Cas particuliers commentés 
	// Diffusion sur un hypercube de dimension 1
        if (0 == rank) MPI_Send(&token, 1, MPI_INT, 1, 101, globComm);
	if (1 == rank) MPI_Recv(&token, 1, MPI_INT, 0, 101, globComm, &status);
        // Diffusion sur un hypercube de dimension 2
	if (0 == rank) MPI_Send(&token, 1, MPI_INT, 2, 101, globComm);
	if (1 == rank) MPI_Send(&token, 1, MPI_INT, 3, 101, globComm);
	if ((rank==2) ||(rank==3)) MPI_Recv(&token, 1, MPI_INT, rank-2, 101, globComm, &status);
	// Diffusion sur un hypercube de dimension 3
	if (rank < 4) MPI_Send(&token, 1, MPI_INT, rank+4, 101, globComm);
	else MPI_Recv(&token, 1, MPI_INT, rank-4, 101, globComm, &status);
	*/
	// Cas général:
	// Selon le nombre de processus, quel est la dimension de mon cube ?
	int dimension_cube = 0;
	int n = nbp;
	while (n > 1)
	{
		dimension_cube += 1;
		n = n/2;
	}
	output << "Dimension de l'hypercube:" << dimension_cube << std::endl;
	// Diffusion du token sur les hypercubes de dimension 1, 2, 3, ...., d
	for ( int d = 0; d < dimension_cube; ++d )
	{
		// Rang maximal de ceux qui envoient
		int send_rank_max = (1<<d); // 1<<d <=> 2^d
		// Rang maximal de ceux qui recoivent
		int recv_rank_max = (1<<(d+1)); // 2^(d+1)
		if ( rank < send_rank_max ) MPI_Send(&token, 1, MPI_INT, rank + send_rank_max, 101, globComm);
		else if (rank < recv_rank_max) MPI_Recv(&token, 1, MPI_INT, rank - send_rank_max, 101, globComm, &status);
        }
	output << "Token final :" << token << std::endl;

	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
