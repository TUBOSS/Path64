C
C Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
C
C
C
C  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
C
C  This program is free software; you can redistribute it and/or modify it
C  under the terms of version 2.1 of the GNU Lesser General Public License 
C  as published by the Free Software Foundation.
C
C  This program is distributed in the hope that it would be useful, but
C  WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
C
C  Further, this software is distributed without any warranty that it is
C  free of the rightful claim of any third person regarding infringement 
C  or the like.  Any license provided herein, whether implied or 
C  otherwise, applies only to this software file.  Patent licenses, if
C  any, provided herein do not apply to combinations of this program with 
C  other software, or any other product whatsoever.  
C
C  You should have received a copy of the GNU Lesser General Public 
C  License along with this program; if not, write the Free Software 
C  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, 
C  USA.
C
C  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
C  Mountain View, CA 94043, or:
C
C  http://www.sgi.com
C
C  For further information regarding this notice, see:
C
C  http://oss.sgi.com/projects/GenInfo/NoticeExplan
C
C


#if defined(BUILD_COMPILER_GNU)
      SUBROUTINE underscore_CGEMMX( ICONJA, ICONJB, M, N, K, ALPHA, A,
     $ INCCA, INCRA, B, INCCB, INCRB, BETA, C, INCCC, INCRC)
#else /* defined(BUILD_COMPILER_GNU) */
      SUBROUTINE _CGEMMX( ICONJA, ICONJB, M, N, K, ALPHA, A, INCCA,
     $                   INCRA, B, INCCB, INCRB, BETA, C, INCCC, INCRC)
#endif /* defined(BUILD_COMPILER_GNU) */
!DIR$ ID "@(#) libfi/matrix/cgemmx.f	92.0	"
!DIR$ ID "10/08/98 14:37:14"
*     ENTRY CGEMMX_( ICONJA, ICONJB, M, N, K, ALPHA, A, INCCA,
*    $                   INCRA, B, INCCB, INCRB, BETA, C, INCCC, INCRC)
*
*  -- X-interface BLAS specification --
*     E. Anderson, Cray Research Inc.
*     February 25, 1993
*
*     .. Scalar Arguments ..
      INTEGER            ICONJA, ICONJB, INCCA, INCCB, INCCC, INCRA,
     $                   INCRB, INCRC, K, M, N
      COMPLEX(KIND=8)    ALPHA, BETA
C     COMPLEX*16         ALPHA, BETA
*     ..
*     .. Array Arguments ..
      COMPLEX(KIND=8)    A( * ), B( * ), C( * )
C     COMPLEX*16         A( * ), B( * ), C( * )
*     ..
*
*  Purpose
*  =======
*
*  CGEMMX performs the matrix-matrix operation
*
*     C := alpha*A*B + beta*C,
*
*  where alpha and beta are scalars, A is an m-by-k matrix,
*  B is a k-by-n matrix, and C is an m-by-n matrix.  Arbitrary row
*  and column increments are permitted for the matrices, which allows
*  transposes.  For example, a matrix A with leading dimension LDA is
*  specified by INCCA = 1, INCRA = LDA, while the transpose of A is
*  specified by INCCA = LDA, INCRA = 1.
*
*  Arguments
*  =========
*
*  ICONJA  (input) INTEGER
*          Specifies whether the matrix A or its conjugate will be used.
*          = 0:  No conjugate for A
*          = 1:  Use conjugate of A
*
*  ICONJB  (input) INTEGER
*          Specifies whether the matrix B or its conjugate will be used.
*          = 0:  No conjugate for B
*          = 1:  Use conjugate of B
*
*  M       (input) INTEGER
*          The number of rows of the matrix A and of the matrix C.
*          M >= 0.
*
*  N       (input) INTEGER
*          The number of columns of the matrix B and the number of
*          columns of the matrix C.  N >= 0.
*
*  K       (input) INTEGER
*          The number of columns of the matrix A and the number of
*          rows of the matrix B.  K >= 0.
*
*  ALPHA   (input) COMPLEX
*          The scalar alpha.
*
*  A       (input) COMPLEX array, dimension
*                  (1 + abs(INCCA)*(M-1) + abs(INCRA)*(K-1))
*          The m-by-k matrix A with column and row increments specified
*          by INCCA and INCRA.
*
*  INCCA   (input) INTEGER
*          The increment between elements in a column of A.
*
*  INCRA   (input) INTEGER
*          The increment between elements in a row of A.
*
*  B       (input) COMPLEX array, dimension
*                  (1 + abs(INCCB)*(K-1) + abs(INCRB)*(N-1))
*          The k-by-n matrix B with column and row increments specified
*          by INCCB and INCRB.
*
*  INCCB   (input) INTEGER
*          The increment between elements in a column of B.
*
*  INCRB   (input) INTEGER
*          The increment between elements in a row of B.
*
*  BETA    (input) COMPLEX
*          The scalar beta.
*
*  C       (input/output) COMPLEX array, dimension
*                         (1 + abs(INCCC)*(M-1) + abs(INCRC)*(N-1))
*          On entry, the m-by-n matrix C with column and row increments
*          specified by INCCC and INCRC.  If beta = 0. on input, then C
*          is initialized to zero and need not be set on input.
*
*          On exit, the array C is overwritten by the m-by-n matrix
*          alpha*A*B + beta*C.
*
*  INCCC   (input) INTEGER
*          The increment between elements in a column of C.
*
*  INCRC   (input) INTEGER
*          The increment between elements in a row of C.
*
*  =====================================================================
*
*     .. Parameters ..
      COMPLEX(KIND=8)           ONE, ZERO
C     COMPLEX*16                ONE, ZERO
      PARAMETER          ( ONE = 1.0E+0, ZERO = 0.0E+0 )
*     ..
*     .. Local Scalars ..
      INTEGER            I, IA, IB, IC, J, JA, JB, JC, L
      COMPLEX(KIND=8)    TEMP
C     COMPLEX*16         TEMP
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          CONJG
*     ..
*     .. Executable Statements ..
*
*     Quick return if possible
*
      IF( ( M.EQ.0 ) .OR. ( N.EQ.0 ) .OR.
     $    ( (ALPHA.EQ.ZERO) .AND. (BETA.EQ.  ONE) ) ) RETURN

*     Set the result to zeros and exit
      IF ( K.EQ.0 ) THEN
           DO, J = 1, N
               DO, I = 1, M
                   IC = INCRC*(J - 1) + INCCC*(I - 1) + 1
                   C( IC ) = ZERO
               END DO
           END DO
           RETURN
       END IF
*
*     Compute C := alpha*A*B + beta*C
*
      JB = 1
      JC = 1
      DO 70 J = 1, N
         JA = 1
         IB = JB
*
*        Scale C by beta (or set to zero if beta is zero).
*
         IF( BETA.EQ.ZERO ) THEN
            IC = JC
            DO 10 I = 1, M
               C( IC ) = ZERO
               IC = IC + INCCC
   10       CONTINUE
         ELSE IF( BETA.NE.ONE ) THEN
            IC = JC
            DO 20 I = 1, M
               C( IC ) = BETA*C( IC )
               IC = IC + INCCC
   20       CONTINUE
         END IF
*
         IF( ICONJA.EQ.0 ) THEN
            DO 40 L = 1, K
               IA = JA
               IC = JC
               IF( ICONJB.EQ.0 ) THEN
                  TEMP = ALPHA*B( IB )
               ELSE
                  TEMP = ALPHA*CONJG( B( IB ) )
               END IF
               IB = IB + INCCB
               DO 30 I = 1, M
                  C( IC ) = C( IC ) + TEMP*A( IA )
                  IA = IA + INCCA
                  IC = IC + INCCC
   30          CONTINUE
               JA = JA + INCRA
   40       CONTINUE
         ELSE
            DO 60 L = 1, K
               IA = JA
               IC = JC
               IF( ICONJB.EQ.0 ) THEN
                  TEMP = ALPHA*B( IB )
               ELSE
                  TEMP = ALPHA*CONJG( B( IB ) )
               END IF
               IB = IB + INCCB
               DO 50 I = 1, M
                  C( IC ) = C( IC ) + TEMP*CONJG( A( IA ) )
                  IA = IA + INCCA
                  IC = IC + INCCC
   50          CONTINUE
               JA = JA + INCRA
   60       CONTINUE
         END IF
         JB = JB + INCRB
         JC = JC + INCRC
   70 CONTINUE
*
      RETURN
*
*     End of CGEMMX
*
      END
