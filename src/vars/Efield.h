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
// Efield.h
//
////////////////////////////////////////////////////////////////////////////////

#ifndef EFIELD_H
#define EFIELD_H

#include<iostream>
#include<iomanip>
#include<sstream>
#include<stdlib.h>
#include <math/d3vector.h>

#include <qball/Sample.h>

class Efield : public Var
{
  Sample *s;

  public:

  char const*name ( void ) const { return "e_field"; };

  int set ( int argc, char **argv )
  {
    if ( argc != 4 )
    {
      //if ( ui->onpe0() )
      cout << " e_field takes 3 values" << endl;
      return 1;
    }

    double v0 = atof(argv[1]);
    double v1 = atof(argv[2]);
    double v2 = atof(argv[3]);

    s->ctrl.e_field[0] = v0;
    s->ctrl.e_field[1] = v1;
    s->ctrl.e_field[2] = v2;

    return 0;
  }

  string print (void) const
  {
     ostringstream st;
     st.setf(ios::left,ios::adjustfield);
     st << setw(10) << name() << " = ";
     st.setf(ios::right,ios::adjustfield);
     st << s->ctrl.e_field[0] << " "
        << s->ctrl.e_field[1] << " "
        << s->ctrl.e_field[2] << " ";
     return st.str();
  }

  Efield(Sample *sample) : s(sample)
  {
    s->ctrl.e_field[0] = 0.0;
    s->ctrl.e_field[1] = 0.0;
    s->ctrl.e_field[2] = 0.0;
  }
};
#endif
