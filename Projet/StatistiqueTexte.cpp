#include <cstring>
#include <cctype>
#include <iostream>
#include <chrono>
#include "texte.hpp"
int main()
{
    const int nb_repet = 10000;
    std::size_t len_texte = std::strlen(texte);

    int cpteur_lettres = 0;
    int cpteur_chiffres= 0;
    int cpteur_autre   = 0;
    int cpteur_majuscule=0;
    int cpteur_minuscule=0;
    int elapsed_milliseconds = 0;
    for ( int r = 0; r < nb_repet; ++r )
    {
        cpteur_lettres = 0;
        cpteur_chiffres= 0;
        cpteur_autre   = 0;
        cpteur_majuscule=0;
        cpteur_minuscule=0;
       // Début de la mesure du temps
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        for ( int i = 0; i < len_texte; ++i )
        {
            if (std::isalpha(texte[i])) 
            {
                cpteur_lettres ++;
                if (std::islower(texte[i])) cpteur_minuscule ++;
                else cpteur_majuscule ++;
            }
            else if (std::isdigit(texte[i])) cpteur_chiffres ++;
            else cpteur_autre ++;
        }
        end = std::chrono::system_clock::now();
        elapsed_milliseconds += std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    }
    std::cout << "Statistique sur le texte : " << std::endl;
    std::cout << "\tNombre de lettres : " << cpteur_lettres << std::endl;
    std::cout << "\t\tNombre de minuscules : " << cpteur_minuscule << std::endl;
    std::cout << "\t\tNombre de majuscules : " << cpteur_majuscule << std::endl;
    std::cout << "\tNombre de chiffres :" << cpteur_chiffres << std::endl;
    std::cout << u8"\tAutres caractères : " << cpteur_autre << std::endl;
    std::cout << u8"Temps mesuré : " << elapsed_milliseconds/((double)nb_repet) << u8"µs\n";
    return 1;
}
