/**
 * \file
 * \author Thomas Fischer
 * \date   2011-05-06
 * \brief  Definition of the GaussAlgorithm class.
 *
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#ifndef GAUSSALGORITHM_H_
#define GAUSSALGORITHM_H_

#include <cstddef>


#include "BaseLib/ConfigTree.h"
#include "../Dense/DenseMatrix.h"
#include "TriangularSolve.h"

namespace MathLib {

/**
 * This is a class for the direct solution of (dense) systems of
 * linear equations, \f$A x = b\f$. During the construction of
 * the object the matrix A is factorized in matrices L and U using
 * Gauss-Elimination with partial pivoting (rows are exchanged). In doing so
 * the entries of A change! The solution for a specific
 * right hand side is computed by the method execute().
 */
template <typename MAT_T, typename VEC_T = typename MAT_T::FP_T*>
class GaussAlgorithm
{
public:
	typedef typename MAT_T::FP_T FP_T;
	typedef typename MAT_T::IDX_T IDX_T;

public:
	/**
	 * A direct solver for the (dense) linear system \f$A x = b\f$.
	 * @param A at the beginning the matrix A, at the end of the application of
	 * method solve the matrix contains the factor L (without the diagonal)
	 * in the strictly lower part and the factor U in the upper part.
	 * The diagonal entries of L are all 1.0 and are not explicitly stored.
	 * @attention The entries of the given matrix will be changed!
	 * @param solver_name A name used as a prefix for command line options
	 *                    if there are such options available.
	 * @param option For some solvers the user can give parameters to the
	 * algorithm. GaussAlgorithm has to fulfill the common interface
	 * of all solvers of systems of linear equations. For this reason the
	 * second argument was introduced.
	 */
	GaussAlgorithm(MAT_T &A, const std::string solver_name = "",
                   BaseLib::ConfigTree const*const option = nullptr);
	/**
	 * destructor, deletes the permutation
	 */
	~GaussAlgorithm();

	/**
	 * Method solves the linear system \f$A x = b\f$ (based on the LU factorization)
	 * using forward solve and backward solve.
	 * @param b at the beginning the right hand side, at the end the solution
	 * @param decompose Flag that signals if the LU decomposition should be
	 *        performed or not. If the matrix \f$A\f$ does not change, the LU
	 *        decomposition needs to be performed once only!
	 * @attention The entries of the given matrix will be changed!
	 */
	template <typename V> void solve (V & b, bool decompose = true);
	void solve(FP_T* & b, bool decompose = true);

	/**
	 * Method solves the linear system \f$A x = b\f$ (based on the LU factorization)
	 * using forward solve and backward solve.
	 * @param b (input) the right hand side
	 * @param x (output) the solution
	 * @param decompose see documentation of the other solve methods.
	 * @attention The entries of the given matrix will be changed!
	 */
	void solve(VEC_T const& b, VEC_T & x, bool decompose = true);

private:
	void performLU();
	/**
	 * permute the right hand side vector according to the
	 * row permutations of the LU factorization
	 * @param b the entries of the vector b are permuted
	 */
	template <typename V> void permuteRHS(V & b) const;
	void permuteRHS (VEC_T& b) const;

	/**
	 * a reference to the matrix
	 */
	MAT_T& _mat;
	/**
	 * the size of the matrix
	 */
	IDX_T _n;
	/**
	 * the permutation of the rows
	 */
	IDX_T* _perm;
};

} // end namespace MathLib

#include "GaussAlgorithm-impl.h"

#endif /* GAUSSALGORITHM_H_ */
