#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <omp.h>

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
void assemblage_matrice_lhs()
{
	std::cout << "Assemblage matrice LHS" << std::endl;
	sleep_for(seconds(2));
	std::cout << "Fin assemblage matrice LHS" << std::endl;
}

void assemblage_matrice_rhs()
{
	std::cout << "Assemblage matrice RHS" << std::endl;
	sleep_for(seconds(3));
	std::cout << "Fin assemblage matrice RHS" << std::endl;
}

int main(int nargs, char* argv[])
{
#pragma omp parallel num_threads(2)
{
#pragma omp sections nowait
{
#pragma omp section
	assemblage_matrice_lhs();
#pragma omp section
	assemblage_matrice_rhs();
}
# pragma omp critical
  std::cout << "Fin assemblage pour thread num. " << omp_get_thread_num() << std::endl;
}
  std::cout << "Et calcul d'un tableau" << std::endl;
  int n = 100;
  if (nargs>1) n = std::atoi(argv[1]);
  std::vector<double> array(n);
#pragma omp parallel num_threads(2) if(n>5000)
{
  double mean;
# pragma omp critical
  std::cout << "Nombre de threads : " << omp_get_num_threads() << std::endl;
#pragma omp for 
  //nowait
  for ( int i = 0; i < n; ++i )
  {
	  array[i] = std::cos(3.1415*i/n);
  }
  mean = 0.5*(array[0]+array[n-1]);
# pragma omp critical
  std::cout << "Mean pour thread num. " << omp_get_thread_num() << " : " << mean << std::endl; 
}


  return EXIT_SUCCESS;
}
