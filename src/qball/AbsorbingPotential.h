////////////////////////////////////////////////////////////////////////////////  
//// Copyright (c) 2013, Lawrence Livermore National Security, LLC. 
//// qb@ll:  Qbox at Lawrence Livermore
////
//// This file is part of qb@ll.
////
//// Produced at the Lawrence Livermore National Laboratory. 
//// Written by Erik Draeger (draeger1@llnl.gov) and Francois Gygi (fgygi@ucdavis.edu).
//// Based on the Qbox code by Francois Gygi Copyright (c) 2008 
//// LLNL-CODE-635376. All rights reserved. 
////
//// qb@ll is free software: you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation, either version 3 of the License, or
//// (at your option) any later version.
////
//// This program is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details, in the file COPYING in the
//// root directory of this distribution or <http://www.gnu.org/licenses/>.
////
//////////////////////////////////////////////////////////////////////////////////
////
//// AbsorbingPotential.h
////
//// YY: complex absorbing potential (ref. phys. stat. sol. (b) 243, No.5, 1121-1138(2006))
////
//////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#ifndef ABSORBINGPOTENTIAL_H
#define ABSORBINGPOTENTIAL_H

#include <ChargeDensity.h>

#include <vector>
#include <valarray>
#include <complex>
using namespace std;

class Basis;
class FourierTransform;

class AbsorbingPotential
{
  private:
  
  ChargeDensity& cd_;
  FourierTransform& vft_;
  Basis& vbasis_;

  string absorbing_potential_type_;
  //
  double spherical_R0_, spherical_deltaR_, spherical_W0_;
  //
  double local_spherical_R0_, local_spherical_W0_;
  double local_spherical_center_x_;
  double local_spherical_center_y_;
  double local_spherical_center_z_;
  //

  void initialize(const string absorbing_potential);
 
  public:

  AbsorbingPotential(ChargeDensity& cd, const string absorbing_potential);
  ~AbsorbingPotential();
  void update(vector<complex<double>> & vabs);
};

class AbsorbingPotentialException
{
  public:
  string msg;
  AbsorbingPotentialException(string s) : msg(s) {}
};
#endif

// Local Variables:
//  mode: c++
// End:
