//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to N-dimensional function minimization routines
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GFUNCMIN_H
#define GFUNCMIN_H

#include "libgambit/gvector.h"

template <class T> class gFunction   {
  public:
    virtual ~gFunction()    { }
    virtual T Value(const gbtVector<T> &) const = 0;
};

template <class T> class gC1Function : public gFunction<T> {
public:
  virtual ~gC1Function() { }

  virtual bool Gradient(const gbtVector<T> &, gbtVector<T> &) const = 0;
};

class gbtFuncMinError : public gbtException {
public:
  virtual ~gbtFuncMinError() { }
  std::string GetDescription(void) const 
    { return "Internal error in minimization code"; }
};

//
// gFunctionMinimizer is an abstract base class for function minimization
//
class gFunctionMinimizer {
public:
  virtual ~gFunctionMinimizer() { }

  virtual void Set(const gC1Function<double> &fdf,
		   const gbtVector<double> &x, double &f,
		   gbtVector<double> &gradient, double step_size,
		   double p_tol) = 0;
  virtual void Restart(void) = 0;

  virtual bool Iterate(const gC1Function<double> &fdf,
		       gbtVector<double> &x, double &f,
		       gbtVector<double> &gradient, gbtVector<double> &dx) = 0;
};

//
// gConjugatePR: implements Polak-Ribiere conjugate gradient descent
//
class gConjugatePR : public gFunctionMinimizer {
private:
  int iter;
  double step;
  double max_step;
  double tol;
  gbtVector<double> x1;
  gbtVector<double> dx1;
  gbtVector<double> x2;
  double pnorm;
  gbtVector<double> p;
  double g0norm;
  gbtVector<double> g0;

public:
  gConjugatePR(int n);
  virtual ~gConjugatePR() { }

  void Set(const gC1Function<double> &fdf,
	   const gbtVector<double> &x, double &f,
	   gbtVector<double> &gradient, double step_size,
	   double p_tol);
  void Restart(void);

  bool Iterate(const gC1Function<double> &fdf,
	       gbtVector<double> &x, double &f,
	       gbtVector<double> &gradient, gbtVector<double> &dx);
};

class gFuncMinException { };

#endif  // GFUNCMIN_H