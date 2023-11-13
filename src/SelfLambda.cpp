/*
   Author: Matthew Might
   Site:   http://matt.might.net/

   This file contains a proof-of-concept demonstration of
   self-inlining anonymous functions in C++.
 
   A self-inlining anonymous function inlines itself wherever it ends
   up being used, which avoids run-time creation and function
   invocation penalties.

   Self-inlining anonymous functions work by folding the syntax tree
   of their definition into their type.
   
   Then, an evaluator template meta-program can unfold the definition
   of the function anywhere it gets used.

   To be more useful, the implementation here could be extended to
   handle multi-argument lambdas.

   In addition, free variables and other values outside the DSEL could
   also be stored in the structure itself, leading essentially to
   static closure allocation.

 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>


// Forward declarations:
template <typename R1, typename R2> 
struct Prod ;

template <typename R1, typename R2> 
struct Sum ;

template <int i>
struct I ;



/* A template that can produce a value of any type */
template <typename T>
struct Dummy {
  static T value ;
} ;


/* Type-based syntax tree nodes */
template <typename T>
struct Exp {

  template <typename M>
  Prod<T,M> operator * (M m) {
    (void)m ; 
    return Dummy< Prod<T,M> >::value ;
  } 

  template <typename M>
  Sum<T,M> operator + (M m) {
    (void)m ; 
    return Dummy< Sum<T,M> >::value ;
  } 
} ;

template <int i>
struct I : public Exp<I<i> > {
  static I<i> v ;
} ;

template <typename R1, typename R2> 
struct Sum : public Exp<Sum<R1,R2> > {} ;

template <typename R1, typename R2> 
struct Prod : public Exp<Prod<R1,R2> > {} ;

template <typename T>
struct Arg : public Exp< Arg<T> > {} ;


/* An inliner as a template meta-program */
template <typename E>
struct Inline {
} ;

// Inline constants:
template < int i >
struct Inline< I<i> > {
  static inline int value (int arg) {
    (void)arg ;
    return i ;
  } 
} ;

// Inline arguments:
template < typename X1 >
struct Inline<Arg<X1> > {
  static inline X1 at (X1 arg) {
    return arg ;
  } 
} ;

// Inline sums:
template < typename E1, typename E2 >
struct Inline<Sum<E1,E2> > {

  template <typename A>
  static inline A at (A arg) {
    return Inline<E1>::at(arg) + Inline<E2>::at(arg) ;
  }

} ;

// Inline products:
template < typename E1, typename E2 >
struct Inline<Prod<E1,E2> > {

  template <typename A>
  static inline A at (A arg) {
    return Inline<E1>::at(arg) * Inline<E2>::at(arg) ;
  }
} ;


// Syntactic sugar for single-argument lambdas:
template <typename T, typename A>
static inline T lambda (A arg, T body) {
  (void) arg ;
  (void) body ;
  return Dummy<T>::value ;
}



// Example; numeric integration, the old-fashioned way:
double integrate_fp (double (*f)(double), double a, double b, int n) {
  (void)f ;
  double delta = (b - a) / n ;
  double area = 0.0 ;
  double x = a ;
  for (int i = 0; i < n; ++i, x += delta) {
    double y = f(x) ;
    area += (y * delta) ;
  }

  return area ;
}

// Example; numeric integration, with anonymous functions:
template < typename F >
static inline double integrate (F f, double a, double b, int n) {
  (void)f ;
  double delta = (b - a) / n ;
  double area = 0.0 ;
  double x = a ;
  for (int i = 0; i < n; ++i, x += delta) {
    double y = Inline<F>::at(x) ;
    area += (y * delta) ;
  }

  return area ;
}


double square (double x) {
  return x * x ;
}

int main (int argc, char* argv) {
  (void)argc ; // Not used 
  (void)argv ; // Not used

  Arg<double> x ;

  double area = integrate(lambda(x, x * x), 0.0, 1.0, 10000) ;

  printf("output: %f\n", area) ;

  return EXIT_SUCCESS ;
}
