#include <cmath>
#include <vector>
#include <random>
#include <cassert>
#include <functional>
#include <iostream>
#include <chrono>

/** Génération d'une famille libre de vecteurs */
std::vector<std::vector<double>> generate_free_family( int dim, int nb_vectors )
{
    assert(nb_vectors <= dim);
    std::random_device rd;
    std::mt19937 generator1(rd());
    std::mt19937 generator2(rd());
    std::uniform_real_distribution<double> normDistrib(-1.,1.);
    auto genNorm = std::bind( normDistrib, generator1 );
    std::uniform_int_distribution<int> pertDistrib(0,1);
    auto pertNorm = std::bind( pertDistrib, generator2 );
    std::vector<std::vector<double>> family;
    family.reserve(nb_vectors);
    family.emplace_back(dim);

    std::vector<double>& u0 = family.back();
    for ( int i = 0; i < dim; ++i )
    {
        u0[i] = genNorm();
    }
    for ( int j = 1; j < nb_vectors; ++j )
    {
        family.emplace_back(u0);
        std::vector<double>& u_j = family.back();
        if (pertNorm()==0) u_j[j] += 1;
        else u_j[j] -= 1;
    }
    return family;
}

/** Produit scalaire de deux vecteurs */
double ddot( const std::vector<double>& u, const std::vector<double>& v )
{
    assert(u.size() == v.size());
    double scal = 0.;
    for ( int i = 0; i < u.size(); ++i )
        scal += u[i]*v[i];
    return scal;
}

/** Opération : v = alpha u + v pour deux vecteurs : */
void daxpy( double alpha, const std::vector<double>& u, std::vector<double>& v )
{
    assert(u.size() == v.size());
    for ( int i = 0; i < u.size(); ++i )
        v[i] += alpha * u[i];
}

/** Opération : v = alpha * v */
void dscal( double alpha, std::vector<double>& v )
{
    for (int i = 0; i < v.size(); ++i )
        v[i] *= alpha;
}

int main()
{
    const int dim = 1000;
    const int nb_vectors = 1000;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    auto family = generate_free_family(dim, nb_vectors);
    end = std::chrono::system_clock::now();
    int elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    std::cout << "Temps pris pour la generation : " << elapsed_microseconds << u8"µs" << std::endl;
    /** Orthonormalisation (Gram Schmidt modifié) :*/
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < nb_vectors; ++i )
    {
        for ( int j = 0; j < i; ++j )
            daxpy(-ddot(family[i],family[j]), family[j], family[i]);
        double nrm = std::sqrt(ddot(family[i],family[i]));
        assert(nrm > 1.E-14);
        dscal(1./nrm, family[i]);
    }
    end = std::chrono::system_clock::now();
    elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    std::cout << "Temps pris pour l'orthonormalisation : " << elapsed_microseconds << "µs" << std::endl;
    /** Vérification qu'on a bien une base orthonormée : */
    start = std::chrono::system_clock::now();
    for ( int i = 0; i < nb_vectors; ++i )
    {
        for ( int j = 0; j < i; ++j )
        {
            if (std::abs(ddot(family[i],family[j])) > 1.E-10)
            {
                std::cerr << "Erreur : le vecteur " << i << "n'est pas orthonormal au vecteur  " << j << std::endl;
                std::cerr << "(u_" << i << "|u_" << j << ") = " << ddot(family[i],family[j]) << std::endl;
                exit(-1);            
            }
        }
        if (std::abs(ddot(family[i],family[i])-1.) > 1.E-10)
        {
            std::cerr << "Erreur, le vecteur " << i << " n'est pas normalisé !" << std::endl;
            exit(-1);
        }
    }
    end = std::chrono::system_clock::now();
    elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    std::cout << "Temps pris pour la verification : " << elapsed_microseconds << "µs" << std::endl;
    return EXIT_SUCCESS;
}
