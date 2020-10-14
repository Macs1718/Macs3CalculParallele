// Produit matrice-vecteur
# include <cassert>
# include <vector>
# include <iostream>
# include <mpi.h>

// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix (int dim);
    /** @brief Assemble une matrice nrows x ncols
     *  
     *  start représente le début d'indice de la colonne en
     *  numérotation globale dans la matrice locale qu'on
     *  construit
     */
    Matrix( int nrows, int ncols, int start=0 );
    Matrix( const Matrix& A ) = delete;
    Matrix( Matrix&& A ) = default;
    ~Matrix() = default;

    Matrix& operator = ( const Matrix& A ) = delete;
    Matrix& operator = ( Matrix&& A ) = default;
    
    double& operator () ( int i, int j ) {
        return m_arr_coefs[i + j*m_nrows];
    }
    double  operator () ( int i, int j ) const {
        return m_arr_coefs[i + j*m_nrows];
    }
    
    std::vector<double> operator * ( const std::vector<double>& u ) const;
    
    std::ostream& print( std::ostream& out ) const
    {
        const Matrix& A = *this;
        out << "[\n";
        for ( int i = 0; i < m_nrows; ++i ) {
            out << " [ ";
            for ( int j = 0; j < m_ncols; ++j ) {
                out << A(i,j) << " ";
            }
            out << " ]\n";
        }
        out << "]";
        return out;
    }
private:
    int m_nrows, m_ncols;
    std::vector<double> m_arr_coefs;
};
// ---------------------------------------------------------------------
inline std::ostream& 
operator << ( std::ostream& out, const Matrix& A )
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream&
operator << ( std::ostream& out, const std::vector<double>& u )
{
    out << "[ ";
    for ( const auto& x : u )
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double> 
Matrix::operator * ( const std::vector<double>& u ) const
{
    const Matrix& A = *this;
    assert( u.size() == unsigned(m_ncols) );
    std::vector<double> v(m_nrows, 0.);
    for ( int i = 0; i < m_nrows; ++i ) {
        for ( int j = 0; j < m_ncols; ++j ) {
            v[i] += A(i,j)*u[j];
        }            
    }
    return v;
}

// =====================================================================
Matrix::Matrix (int dim) : m_nrows(dim), m_ncols(dim),
                           m_arr_coefs(dim*dim)
{
    for ( int i = 0; i < dim; ++ i ) {
        for ( int j = 0; j < dim; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix( int nrows, int ncols, int start ) : m_nrows(nrows), m_ncols(ncols),
                                                    m_arr_coefs(nrows*ncols)
{
    int dim = (nrows > ncols ? nrows : ncols );
    for ( int i = 0; i < nrows; ++ i ) {
        for ( int j = 0; j < ncols; ++j ) {
            (*this)(i,j) = (i+j+start)%dim;
        }
    }    
}
// =====================================================================
int main( int nargs, char* argv[] )
{
    const int N = 12000;
    MPI_Init(&nargs, &argv);
    int rank, nbp;
    MPI_Comm_size(MPI_COMM_WORLD,&nbp);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    // On va assemble qu'un bloc colonne de la matrice par processus
    // Le processus de rang rank, assemble les colonnes qui vont
    // de (N/nbp)*rank à (N/nb)*(rank+1) (non compris)
    // => Calcul du nombre de colonne local
    int ncols = N/nbp;
    // Nombre de colonne restant <= remainder
    int remainder = N%nbp;
    // Si mon rang est inférieur à remainder, je rajoute une colonne à ma sous-matrice
    if (rank<remainder) ncols += 1;
    // Calcul de l'indice globale de la première colonne de la sous matrice du processus
    int start = rank*ncols;
    // Si mon rang est supérieur à remainder, j'ai oublié de rajouter remainder colonnes contenus par les remainder premiers processus
    if (rank>=remainder) start += remainder;
    /* Exemple :
     * J'ai une matrice de dimension 7 et deux processus
     * Alors Pour le processus rank = 0 :
     *     ncols = 3 + 1 = 4 colonnes
     *     start = rank*ncols = 0
     * Pour le processus rank = 1 :
     *     ncols = 3 + 0 = 3 colonnes
     *     start = rank*ncols + 1 = 4 <== Je commence à la 5e colonne de la matrice globale
     */
    Matrix Aloc(N, ncols, start);
    //Matrix A(N);
    //std::cout  << "Aloc : " << Aloc << std::endl;
    /** Pas besoin d'assembler tout le vecteur. On va assembler uniquement 
     *  les coefficients allant de start à ncols + start
     */
    std::vector<double> u_loc( ncols );

    for ( int i_loc = 0; i_loc < ncols; ++i_loc ) u_loc[i_loc] = i_loc + start +1;
    //std::cout << " u_loc : " << u_loc << std::endl;
    // Pour calculer V, on va utiliser la relation suivante :
    //      nbp
    // v = sum( A_loc(i)*u_loc(i) )
    //     i=1
    std::vector<double> v_loc = Aloc * u_loc;
    std::vector<double> v_glob(N,0.);
    MPI_Allreduce(v_loc.data(), v_glob.data(), N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  
    //std::cout << "A.u = " << v_glob << std::endl;
    MPI_Finalize();
    return EXIT_SUCCESS;
}

