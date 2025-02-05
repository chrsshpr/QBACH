////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013, Lawrence Livermore National Security, LLC.
// qb@ll:  Qbox at Lawrence Livermore
//
// This file is part of qb@ll.
//
// Produced at the Lawrence Livermore National Laboratory.
// Written by Erik Draeger (draeger1@llnl.gov) and Francois Gygi (fgygi@ucdavis.edu).
// Based on the Qbox code by Francois Gygi Copyright (c) 2008
// LLNL-CODE-635376. All rights reserved.
//
// qb@ll is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details, in the file COPYING in the
// root directory of this distribution or <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////
//
// GaussField.h
//
////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#ifndef GAUSSFIELD_H
#define GAUSSFIELD_H

#include<iostream>
#include<iomanip>
#include<sstream>
#include<stdlib.h>

#include <qball/Sample.h>

class GaussField : public Var
{
  Sample *s;

  public:

  const char *name ( void ) const { return "gauss_field"; };

  int set ( int argc, char **argv )
  {
    if ( argc != 4 )
    {
      if ( ui->oncoutpe() )
      cout << " gauss_field takes three value: t0, full width half max and freq" << endl;
      return 1;
    }
    
    double ev2au = 0.0367493;
    double sig = 2.3548;  //2 * sqrt(2*log(2))
    double convert = atof(argv[2])*ev2au;
    double v0 = atof(argv[1]); //t0
    double v1 = sig /convert; //FWHM
    double v2 = atof(argv[3])*ev2au; //Freq 

    if ( v1 < 0.0 )
    {
      if ( ui->oncoutpe() )
        cout << "full width half max value must be > 0" << endl;
      return 1;
    }

    s->ctrl.compute_gaussian_field = true;
    s->ctrl.gauss_field[0] = v0;
    s->ctrl.gauss_field[1] = v1;
    s->ctrl.gauss_field[2] = v2;

    return 0;
  }

  string print (void) const
  {
     ostringstream st;
     st.setf(ios::left,ios::adjustfield);
     st << setw(10) << name() << " = ";
     st.setf(ios::right,ios::adjustfield);
     st << s->ctrl.gauss_field[0] << " " 
        << s->ctrl.gauss_field[1] << " "
        << s->ctrl.gauss_field[2] << " ";
     return st.str();
  }

  GaussField(Sample *sample) : s(sample)
  {
    s->ctrl.compute_gaussian_field = false;
    s->ctrl.gauss_field[0] = 0;
    s->ctrl.gauss_field[1] = 0;
    s->ctrl.gauss_field[2] = 0;
  }
};
#endif

