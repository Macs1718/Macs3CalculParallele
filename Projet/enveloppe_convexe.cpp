/** Calcul d'enveloppe convexe */

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <ctime>
#include <cmath>
#include <chrono>
#include <algorithm>

/*
     structure de base
*/
struct point
{
    double _x ;
    double _y ;
    point(double x, double y) : _x(x), _y(y) {}
};
/*
     predicat qui de deux points renvoie le plus bas
*/
bool le_plus_bas( const point & a , const point & b )
{
    return a._y < b._y ;
}
/*
  foncteur qui permet de classer deux points suivant l'angle entre l'axe des x et le vecteur
  forme par chacun des points et un p donne.
*/
class sens_trigo
{
private:
    point _p ;

public:
    sens_trigo( const point & p )
        :_p( p )
    {}
    bool operator()( const point & a , const point & b )
    {
        double tta_a = atan2( a._y - _p._y , a._x - _p._x ) ;
        double tta_b = atan2( b._y - _p._y , b._x - _p._x ) ;
        return  tta_a > tta_b ;
    }
};
/*
  droite construite a partir de deux points
  dont l'equation est de la forme ax + by + c = 0
*/
class droite
{
private:
    double _a ;
    double _b ;
    double _c ;
public:
    droite( const point & p1 , const point & p2 )
        :_a( p2._y - p1._y )
        ,_b( -p2._x + p1._x )
        ,_c( p1._y * p2._x - p1._x * p2._y )
    {}
    /* Renvoie true si p1 et p2 sont du meme cote de la droite,
       c'est-a-dire si le signe de ax + by + c pour chacun des points est le meme
    */
    bool meme_cote( const point & p1 , const point & p2 )
    {
        return ( _a * p1._x + _b * p1._y + _c ) * ( _a * p2._x + _b * p2._y + _c ) > 0 ;
    }
};
/*
  algorithme de calcul de l'enveloppe
*/
std::vector< point > enveloppe( const std::vector<point>& nuage )
{
    std::list<point> nuage_lst(nuage.begin(), nuage.end());
    //Recherche du point le plus bas
    nuage_lst.sort(le_plus_bas);
    point bas = nuage_lst.front() ;
    nuage_lst.pop_front();

    //Tri du reste du nuage en fonction des angles par rapport au point le plus bas
    nuage_lst.sort(sens_trigo(bas));

    //On place le point le plus bas dans l'enveloppe
    std::vector< point > env ;
    env.push_back( bas ) ;

    //On replace egalement le point le plus bas a la fin du nuage
    nuage_lst.push_back( bas ) ;

    //Tant qu'il y a des points dans le nuage...
    while ( !nuage_lst.empty() )
    {
        //On retire le premier point du nuage et on le place dans l'enveloppe
        env.push_back( nuage_lst.front() ) ;
        nuage_lst.pop_front() ;

        //Tant qu'il y a au moins 4 points dans l'enveloppe...
        while ( env.size() >= 4 )
        {
            size_t n = env.size() - 1 ;
            const point & b = env[ n ] ;
            const point & p2 = env[ n - 1 ] ;
            const point & p1 = env[ n - 2 ] ;
            const point & a = env[ n - 3 ] ;

            //Si les points a et b sont du meme cote que la droite passant par p1 et p2
            if ( ! droite( p1 , p2 ).meme_cote( a , b ) )
            {
                env.erase( env.begin() + n - 1 ) ;
            }
            else
            {
                //sinon on quitte la boucle
                break ;
            }
        }
    }
    return env ;
}

void write_svg(const char* fileName, const std::vector<point>& nuage, const std::vector<point>& enveloppe)
{
    FILE* fich = fopen(fileName, "w");
    fprintf(fich, "<?xml version=\"1.0\" standalone=\"no\"?>\n"),
    fprintf(fich, 
        "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"" 
        "\n \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    fprintf(fich, "<svg width=\"10cm\" height=\"10cm\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    fprintf(fich, "<desc>Nuage de points\n");
    fprintf(fich, "</desc>\n");
    for ( const auto& v : nuage )
        fprintf(fich, "<rect x=\"%gcm\" y=\"%gcm\" width=\"0.1cm\" height=\"0.1cm\"/>\n",0.45+v._x/100., 0.45+v._y/100.);
    fprintf(fich, "<desc>Enveloppe convexe\n");
    fprintf(fich, "</desc>\n");
    std::size_t nb_nodes = enveloppe.size();
    for ( int i = 1; i < nb_nodes; ++i )
    {
        fprintf(fich, "<line x1=\"%gcm\" y1=\"%gcm\" x2=\"%gcm\" y2=\"%gcm\" stroke=\"red\"/>\n",
            0.5+enveloppe[i-1]._x/100., 0.5+enveloppe[i-1]._y/100.,0.5+enveloppe[i]._x/100., 0.5+enveloppe[i]._y/100.);
    }
    fprintf(fich, "<line x1=\"%gcm\" y1=\"%gcm\" x2=\"%gcm\" y2=\"%gcm\" stroke=\"red\"/>\n",
            0.5+enveloppe[nb_nodes-1]._x/100., 0.5+enveloppe[nb_nodes-1]._y/100.,0.5+enveloppe[0]._x/100., 0.5+enveloppe[0]._y/100.);

    fprintf(fich, "<!-- Show outline of canvas using 'rect' element -->\n");
    fprintf(fich, "<rect x=\".01cm\" y=\".01cm\" width=\"9.98cm\" height=\"9.98cm\""
                  "\n fill=\"none\" stroke=\"blue\" stroke-width=\".02cm\" />\n");
    fprintf(fich, "</svg>\n");
    fclose(fich);
}

/* programme illustrant l'utilisation de l'algorithme */
int main()
{
    const int taille_nuage = 500;
    const int nb_repet     = 1000;
    //generation d'un nuage de points
    unsigned int seed = time(0);
    srand( (unsigned int) seed ) ;

    int elapsed_generation = 0;
    int elapsed_convexhull = 0;

    for ( int r = 0; r < nb_repet; ++r )
    {
        std::vector< point > nuage;
        nuage.reserve( taille_nuage ) ;
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        {
            for ( int i = 0; i < taille_nuage; ++i )
            {
                nuage.emplace_back(rand() % 900, rand() % 900) ;
            }
        }
        end = std::chrono::system_clock::now();
        elapsed_generation += std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();

        //execution de l'algorithme
        start = std::chrono::system_clock::now();
        std::vector< point > env = enveloppe( nuage ) ;
        end = std::chrono::system_clock::now();
        elapsed_convexhull += std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
        if (r == 0 )     write_svg("Enveloppe.svg", nuage, env);
    }
    std::cout << "Temps pris pour la generation : " << elapsed_generation/double(nb_repet) << " micro seconds" << std::endl;
    std::cout << "Temps pris pour le calcul de l'enveloppe : " << elapsed_convexhull/double(nb_repet) << " micro seconds" << std::endl;


    return 0;
}
