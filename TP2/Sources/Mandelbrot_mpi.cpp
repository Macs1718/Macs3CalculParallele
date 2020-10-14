# include <iostream>
# include <cstdlib>
# include <string>
# include <chrono>
# include <cmath>
# include <vector>
# include <fstream>
# include <mpi.h>

MPI_Comm globComm;
/** Une structure complexe est définie pour la bonne raison que la classe
 * complex proposée par g++ est très lente ! Le calcul est bien plus rapide
 * avec la petite structure donnée ci--dessous
 **/
struct Complex
{
    Complex() : real(0.), imag(0.)
    {}
    Complex(double r, double i) : real(r), imag(i)
    {}
    Complex operator + ( const Complex& z )
    {
        return Complex(real + z.real, imag + z.imag );
    }
    Complex operator * ( const Complex& z )
    {
        return Complex(real*z.real-imag*z.imag, real*z.imag+imag*z.real);
    }
    double sqNorm() { return real*real + imag*imag; }
    double real,imag;
};

std::ostream& operator << ( std::ostream& out, const Complex& c )
{
  out << "(" << c.real << "," << c.imag << ")" << std::endl;
  return out;
}

/** Pour un c complexe donné, calcul le nombre d'itérations de mandelbrot
 * nécessaires pour détecter une éventuelle divergence. Si la suite
 * converge, la fonction retourne la valeur maxIter
 **/
int iterMandelbrot( int maxIter, const Complex& c)
{
    Complex z{0.,0.};
    // On vérifie dans un premier temps si le complexe
    // n'appartient pas à une zone de convergence connue :
    // Appartenance aux disques  C0{(0,0),1/4} et C1{(-1,0),1/4}
    if ( c.real*c.real+c.imag*c.imag < 0.0625 )
        return maxIter;
    if ( (c.real+1)*(c.real+1)+c.imag*c.imag < 0.0625 )
        return maxIter;
    // Appartenance à la cardioïde {(1/4,0),1/2(1-cos(theta))}    
    if ((c.real > -0.75) && (c.real < 0.5) ) {
        Complex ct{c.real-0.25,c.imag};
        double ctnrm2 = sqrt(ct.sqNorm());
        if (ctnrm2 < 0.5*(1-ct.real/ctnrm2)) return maxIter;
    }
    int niter = 0;
    while ((z.sqNorm() < 4.) && (niter < maxIter))
    {
        z = z*z + c;
        ++niter;
    }
    return niter;
}

/**
 * On parcourt chaque pixel de l'espace image et on fait correspondre par
 * translation et homothétie une valeur complexe c qui servira pour
 * itérer sur la suite de Mandelbrot. Le nombre d'itérations renvoyé
 * servira pour construire l'image finale.
 
 Sortie : un vecteur de taille W*H avec pour chaque case un nombre d'étape de convergence de 0 à maxIter
 MODIFICATION DE LA FONCTION :
 j'ai supprimé le paramètre W étant donné que maintenant, cette fonction ne prendra plus que des lignes de taille W en argument.
 **/
void 
computeMandelbrotSetRow( int W, int H, int maxIter, int num_ligne, int* pixels)
{
    // Calcul le facteur d'échelle pour rester dans le disque de rayon 2
    // centré en (0,0)
    double scaleX = 3./(W-1);
    double scaleY = 2.25/(H-1.);
    //
    // On parcourt les pixels de l'espace image :
    for ( int j = 0; j < W; ++j ) {
       Complex c{-2.+j*scaleX,-1.125+ num_ligne*scaleY};
       pixels[j] = iterMandelbrot( maxIter, c );
    }
}

std::vector<int>
computeMandelbrotSet( int W, int H, int maxIter )
{
    int rank, nbp;
    MPI_Status bordereau;
    MPI_Comm_rank(globComm, &rank);
    MPI_Comm_size(globComm, &nbp );
    std::vector<int> row(W);
    std::vector<int> pixels;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    if ( rank == 0 ) // Master process
    {
        std::vector<int>(W*H).swap(pixels);
        pixels.resize(W*H);
        int ind_row = 0;
        for ( int p = 1; p < nbp; ++p )
        {
	    // J'envoie aux esclaves le n° de ligne à calculer
	    //  (différent pour chacun)
            MPI_Send(&ind_row, 1, MPI_INT, p, 101, globComm);
            ind_row += 1;
        }
	// Tant qu'il y a des lignes à calculer
        while ( ind_row < H )
        {
		// Je reçois un résultat d'un des esclaves
            MPI_Recv(row.data(), W, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm, &bordereau);
	    // Qui me l'a envoyé ?
            int sender = bordereau.MPI_SOURCE;
	    // Le tag du message contient le n° de ligne calculée : je récupère le tag
            int irow    = bordereau.MPI_TAG;
	    // J'envoie une nouvelle ligne à calculer au processus dont je viens de recevoir le résultat.
            MPI_Send(&ind_row, 1, MPI_INT, sender, 101, globComm);
            ind_row += 1;
            std::copy(row.begin(), row.end(), pixels.data() + (H-irow-1) * W );
        }
	// Arrivé ici, j'ai demandé aux esclaves toutes les lignes à calculer, plus de travail à faire
        ind_row = -1; // Signal de terminaison
        for ( int p = 1; p < nbp; ++ p )
        {
		// J'attends les résultats de chaque esclave (dans le désordre)
            MPI_Recv(row.data(), W, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm, &bordereau);
	    // Qui m'a envoyé le résultat ?
            int sender = bordereau.MPI_SOURCE;
	    // Quel était le n° de ligne qu'il m'a calculé
            int irow    = bordereau.MPI_TAG;
	    // J'envoie un signal de terminaison à l'esclave
            MPI_Send(&ind_row, 1, MPI_INT, sender, 101, globComm ); 
            std::copy(row.begin(), row.end(), pixels.data() + (H-irow-1) * W );
        }
    }
    else // Slaves
    {
        MPI_Status bordereau;
        int irow;
	// Je reçois la première ligne à calculer
        MPI_Recv(&irow, 1, MPI_INT, 0, 101, globComm, &bordereau);
        while (irow >= 0)
        {
		// Je calcule la ligne
            computeMandelbrotSetRow(W, H, maxIter, irow, row.data() );
	    // J'envoie le résultat au maître (de rang 0) avec pour tag le n° de la ligne que j'ai calculé
            MPI_Send(row.data(), W, MPI_INT, 0, irow, globComm);
	    // J'attend une nouvelle ligne à calculer
            MPI_Recv(&irow, 1, MPI_INT, 0, 101, globComm, &bordereau);
        }// Je sors de cette boucle si le n° de ligne donné est < 0 (signal de terminaison)
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Temps calcul ensemble mandelbrot : " << elapsed_seconds.count() 
              << std::endl;
    return pixels;
}

/** Construit et sauvegarde l'image finale **/
void savePicture( const std::string& filename, int W, int H, const std::vector<int>& nbIters, int maxIter )
{
    double scaleCol = 1./maxIter;//16777216
    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
    ofs << "P6\n"
        << W << " " << H << "\n255\n";
    for ( int i = 0; i < W * H; ++i ) {
        double iter = scaleCol*nbIters[i];
        unsigned char r = (unsigned char)(256 - (unsigned (iter*256.) & 0xFF));
        unsigned char b = (unsigned char)(256 - (unsigned (iter*65536) & 0xFF));
        unsigned char g = (unsigned char)(256 - (unsigned( iter*16777216) & 0xFF));
        ofs << r << g << b;
    }
    ofs.close();
}

int main(int argc, char *argv[] ) 
 { 
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm, &rank );
    const int W = 800;
    const int H = 600;
    // Normalement, pour un bon rendu, il faudrait le nombre d'itérations
    // ci--dessous :
    //const int maxIter = 16777216;
    const int maxIter = 8*65536;
    auto iters = computeMandelbrotSet( W, H, maxIter );
    if ( rank == 0 )
        savePicture("mandelbrot.ppm", W, H, iters, maxIter);
    MPI_Finalize();
    return EXIT_SUCCESS;
 }
    
