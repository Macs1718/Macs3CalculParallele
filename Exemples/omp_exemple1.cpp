#include <iostream>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <vector>

int main()
{
	using namespace std::chrono;

	// Suite de syrachuse : u_{n+1} = u_{n}/2 si u_{n} est pair, et (3u_{n}+1)/2 si u_{n} est impair
	// Conjecture : Toute suite de syrachuse (avec u_{0} quelconque) arrive toujours sur un cycle : 1 -> 2 -> 1 -> 2 -> 1 ...
	// Le temps de vol : Nombre d'itérations pour tomber sur la valeur 1
	// Hauteur du vol  : La plus grande valeur prise par la suite
	//
	//
	const std::int64_t N = 10000000;
	std::vector<std::int64_t> temps_vol(N);
	std::vector<std::int64_t> hauteur_vol(N);
	
	std::int64_t temps_total = 0;// Nombre total d'itérations de toutes mes suites

        high_resolution_clock::time_point t1 = high_resolution_clock::now();
	// Sans reduction : temps_total peut être mis à jour en même temps par deux threads : chacun prend l'ancienne valeur de temps_total (par exemple 0)
	// et lui rajoute son temps_vol[isuite]. DAns ce cas, temps_total vaudra (s'il valait 0 avant) le dernier temps de vol rajouté par un des deux threads
	// et non la somme des temps de vol de chaque thread. (Data Race)
	// Plusieur solutions pour éviter le data race :
	//  Première solution : Mettre temps_total comme variable globale (mais elle y est déjà) et mettre une section critique lors de la somme. Assez pénalisant,
	//                      et cela se ressent dans le temps de calcul
	//  Deuxième solution : Puisqu'une somme est une opération élémentaire, remplacer la section critique par une opération atomique. Plus de problème de
	//                      pénalité de temps dans les calculs, nettement plus efficace qu'une section critique (on divise par deux le temps de calcul !)
	//  Troisième solution : Avec réduction : temps_total est dupliqué  et Open MP calcule une somme partielle. A la sortie de la boucle, OpenMP fait la somme des
	//                       sommes partielles pour obtenir la somme globale.
#       pragma omp parallel for reduction(+:temps_total)
	//schedule(dynamic,500) reduction(+:temps_total)
	for ( std::int64_t isuite = 0; isuite < N; ++isuite)
	{
		std::int64_t un = 2*isuite+1;
		temps_vol[isuite] = 0;
		hauteur_vol[isuite] = un;

		while (un != 1)
		{
			if (un%2==0) un = un / 2;
			else un = (3*un+1)/2;
			temps_vol[isuite] += 1;
			hauteur_vol[isuite] = std::max(hauteur_vol[isuite], un);
		}
//                Première solution
//#               pragma critical
//                Deuxième solution
//#               pragma omp atomic
//                Pour la troisième solution, rajouter le reduction a l'instruction parallel for....
		temps_total = temps_total + temps_vol[isuite];
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::cout << "Temps pour le calcul : " << time_span.count() << " secondes." << std::endl;
	std::cout << "Temps le plus long de vol des suites : ";
	std::int64_t temps_max = temps_vol[0];
	for ( std::int64_t i = 1; i < N; ++i )
		temps_max = std::max(temps_max, temps_vol[i]);
	std::cout << "temps de vol maximum : " << temps_max << std::endl;
	std::int64_t hauteur_max = hauteur_vol[0];
	for ( std::int64_t i = 1; i < N; ++i )
		hauteur_max = std::max(hauteur_max, hauteur_vol[i]);
	std::cout << "Hauteur maximale atteinte : " << hauteur_max << std::endl;
	std::cout << "Nombre d'iterations total calculees : " << temps_total << std::endl;
	//for ( auto val : temps_vol ) std::cout << val << " ";

	//std::cout << "Hauteur de vol de chaque suite : ";
	//for ( auto val : hauteur_vol ) std::cout << val << " ";

	return EXIT_SUCCESS;
}
