/****************************************************************************
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2017 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Matrix SVD decomposition.
 *
 * Authors:
 * Eric Marchand
 *
 *****************************************************************************/

#include <visp3/core/vpConfig.h>
#include <visp3/core/vpMatrix.h>
#include <visp3/core/vpMath.h>
#include <visp3/core/vpColVector.h>
#include <visp3/core/vpException.h>
#include <visp3/core/vpMatrixException.h>
#include <visp3/core/vpDebug.h>

#include <cmath>    // std::fabs
#include <limits>   // numeric_limits
#include <iostream>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef VISP_HAVE_GSL
#  include <gsl/gsl_linalg.h>
#endif

#if (VISP_HAVE_OPENCV_VERSION >= 0x020101) // Require opencv >= 2.1.1
#    include <opencv2/core/core.hpp>
#endif

#ifdef VISP_HAVE_LAPACK_C
extern "C" int dgesdd_(char *jobz, int *m, int *n, double *a, int *lda, double *s, double *u, int *ldu, double *vt,
                       int *ldvt, double *work, int *lwork, int *iwork, int *info);

#  include <stdio.h>
#  include <string.h>
#endif
/*---------------------------------------------------------------------

SVD related functions

---------------------------------------------------------------------*/

#if (VISP_HAVE_OPENCV_VERSION >= 0x020101) // Require opencv >= 2.1.1

/*!

  Singular value decomposition (SVD) using OpenCV 3rd party.

  Given matrix \f$M\f$, this function computes it singular value decomposition such as

  \f[ M = U \Sigma V^{\top} \f]

  \warning This method is destructive wrt. to the matrix \f$ M \f$ to
  decompose. You should make a COPY of that matrix if needed.

  \param w : Vector of singular values: \f$ \Sigma = diag(w) \f$.

  \param V : Matrix \f$ V \f$.

  \return Matrix \f$ U \f$.

  \note The singular values are ordered in decreasing
  fashion in \e w. It means that the highest singular value is in \e w[0].

  Here an example of SVD decomposition of a non square Matrix M.

\code
#include <visp3/core/vpColVector.h>
#include <visp3/core/vpMatrix.h>

int main()
{
  vpMatrix M(3,2);
  M[0][0] = 1;
  M[1][0] = 2;
  M[2][0] = 0.5;

  M[0][1] = 6;
  M[1][1] = 8 ;
  M[2][1] = 9 ;

  vpMatrix V;
  vpColVector w;
  vpMatrix Mrec;
  vpMatrix Sigma;

  M.svdOpenCV(w, V);
  // Here M is modified and is now equal to U

  // Construct the diagonal matrix from the singular values
  Sigma.diag(w);

  // Reconstruct the initial matrix M using the decomposition
  Mrec =  M * Sigma * V.t();

  // Here, Mrec is obtained equal to the initial value of M
  // Mrec[0][0] = 1;
  // Mrec[1][0] = 2;
  // Mrec[2][0] = 0.5;
  // Mrec[0][1] = 6;
  // Mrec[1][1] = 8 ;
  // Mrec[2][1] = 9 ;

  std::cout << "Reconstructed M matrix: \n" << Mrec << std::endl;
}
  \endcode

  \sa svd(), svdLapack(), svdGsl()
*/
void vpMatrix::svdOpenCV(vpColVector &w, vpMatrix &V)
{
  int rows = (int)this->getRows();
  int cols = (int)this->getCols();
  cv::Mat m(rows, cols, CV_64F, this->data);
  cv::SVD opencvSVD(m);
  cv::Mat opencvV = opencvSVD.vt;
  cv::Mat opencvW = opencvSVD.w;
  V.resize((unsigned int)opencvV.rows, (unsigned int)opencvV.cols);
  w.resize((unsigned int)(opencvW.rows*opencvW.cols));
  
  memcpy(V.data, opencvV.data, (size_t)(8*opencvV.rows*opencvV.cols));
  V=V.transpose();
  memcpy(w.data, opencvW.data, (size_t)(8*opencvW.rows*opencvW.cols));
  this->resize((unsigned int)opencvSVD.u.rows, (unsigned int)opencvSVD.u.cols);
  memcpy(this->data, opencvSVD.u.data, (size_t)(8*opencvSVD.u.rows*opencvSVD.u.cols));
}

#endif

#ifdef VISP_HAVE_LAPACK_C
/*!

  Singular value decomposition (SVD) using Lapack 3rd party.

  Given matrix \f$M\f$, this function computes it singular value decomposition such as

  \f[ M = U \Sigma V^{\top} \f]

  \warning This method is destructive wrt. to the matrix \f$ M \f$ to
  decompose. You should make a COPY of that matrix if needed.

  \param w : Vector of singular values: \f$ \Sigma = diag(w) \f$.

  \param V : Matrix \f$ V \f$.

  \return Matrix \f$ U \f$.

  \note The singular values are ordered in decreasing
  fashion in \e w. It means that the highest singular value is in \e w[0].

  Here an example of SVD decomposition of a non square Matrix M.

\code
#include <visp3/core/vpColVector.h>
#include <visp3/core/vpMatrix.h>

int main()
{
  vpMatrix M(3,2);
  M[0][0] = 1;
  M[1][0] = 2;
  M[2][0] = 0.5;

  M[0][1] = 6;
  M[1][1] = 8 ;
  M[2][1] = 9 ;

  vpMatrix V;
  vpColVector w;
  vpMatrix Mrec;
  vpMatrix Sigma;

  M.svdLapack(w, V);
  // Here M is modified and is now equal to U

  // Construct the diagonal matrix from the singular values
  Sigma.diag(w);

  // Reconstruct the initial matrix M using the decomposition
  Mrec =  M * Sigma * V.t();

  // Here, Mrec is obtained equal to the initial value of M
  // Mrec[0][0] = 1;
  // Mrec[1][0] = 2;
  // Mrec[2][0] = 0.5;
  // Mrec[0][1] = 6;
  // Mrec[1][1] = 8 ;
  // Mrec[2][1] = 9 ;

  std::cout << "Reconstructed M matrix: \n" << Mrec << std::endl;
}
  \endcode

  \sa svd(), svdOpenCV(), svdGsl()
*/
void vpMatrix::svdLapack(vpColVector &w, vpMatrix &V)
{
  w.resize( this->getCols() );
  V.resize( this->getCols(), this->getCols() );

  /* unsigned */ int m = static_cast<int>(this->getCols()), n = static_cast<int>(this->getRows()), lda = m, ldu = m, ldvt = std::min(m,n);
  int info, lwork;

  double wkopt;
  double *work;

  int* iwork = new int[8*static_cast<unsigned int>(std::min(n,m))];

  double *s = w.data;
  double *a = new double[static_cast<unsigned int>(lda*n)];
  memcpy(a, this->data, this->getRows()*this->getCols()*sizeof(double));
  double *u = V.data;
  double *vt = this->data;

  lwork = -1;
  dgesdd_( (char*)"S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, &wkopt, &lwork, iwork, &info );
  lwork = (int)wkopt;
  work = new double[static_cast<unsigned int>(lwork)];

  dgesdd_( (char*)"S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, work, &lwork, iwork, &info );

  if( info > 0 ) {
   throw(vpMatrixException(vpMatrixException::fatalError,
         "The algorithm computing SVD failed to converge.")) ;

  }

  V = V.transpose();
  delete[] work;
  delete[] iwork;
  delete[] a;
}
#endif

#ifdef VISP_HAVE_GSL

/*!

  Singular value decomposition (SVD) using GSL 3rd party.

  Given matrix \f$M\f$, this function computes it singular value decomposition such as

  \f[ M = U \Sigma V^{\top} \f]

  \warning This method is destructive wrt. to the matrix \f$ M \f$ to
  decompose. You should make a COPY of that matrix if needed.

  \param w : Vector of singular values: \f$ \Sigma = diag(w) \f$.

  \param V : Matrix \f$ V \f$.

  \return Matrix \f$ U \f$.

  \note The singular values are ordered in decreasing
  fashion in \e w. It means that the highest singular value is in \e w[0].

  Here an example of SVD decomposition of a non square Matrix M.

\code
#include <visp3/core/vpColVector.h>
#include <visp3/core/vpMatrix.h>

int main()
{
  vpMatrix M(3,2);
  M[0][0] = 1;
  M[1][0] = 2;
  M[2][0] = 0.5;

  M[0][1] = 6;
  M[1][1] = 8 ;
  M[2][1] = 9 ;

  vpMatrix V;
  vpColVector w;
  vpMatrix Mrec;
  vpMatrix Sigma;

  M.svdGsl(w, V);
  // Here M is modified and is now equal to U

  // Construct the diagonal matrix from the singular values
  Sigma.diag(w);

  // Reconstruct the initial matrix M using the decomposition
  Mrec =  M * Sigma * V.t();

  // Here, Mrec is obtained equal to the initial value of M
  // Mrec[0][0] = 1;
  // Mrec[1][0] = 2;
  // Mrec[2][0] = 0.5;
  // Mrec[0][1] = 6;
  // Mrec[1][1] = 8 ;
  // Mrec[2][1] = 9 ;

  std::cout << "Reconstructed M matrix: \n" << Mrec << std::endl;
}
  \endcode

  \sa svd(), svdOpenCV(), svdLapack()
*/
void
vpMatrix::svdGsl(vpColVector &w, vpMatrix &V)
{
  w.resize( this->getCols() );
  V.resize( this->getCols(), this->getCols() );

  unsigned int nc = getCols() ;
  unsigned int nr = getRows() ;
  gsl_vector *work = gsl_vector_alloc(nc) ;

  gsl_matrix A;
  A.size1 = nr;
  A.size2 = nc;
  A.tda = A.size2;
  A.data = this->data;
  A.owner = 0;
  A.block = 0;
  
  gsl_matrix V_;
  V_.size1 = nc;
  V_.size2 = nc;
  V_.tda = V_.size2;
  V_.data = V.data;
  V_.owner = 0;
  V_.block = 0;
  
  gsl_vector S;
  S.size = nc;
  S.stride = 1;
  S.data = w.data;
  S.owner = 0;
  S.block = 0;
  
  gsl_linalg_SV_decomp(&A, &V_, &S, work) ;
  
  gsl_vector_free(work) ;
}
#endif // # #GSL

#endif // #ifndef DOXYGEN_SHOULD_SKIP_THIS
