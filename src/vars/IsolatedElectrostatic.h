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
// IsolatedElectrostatic.h
//  YY : command for isolated electrostatic methods.
//  only MT avaiable now
//  MT method: JCP 110 (6),2810-2821
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ISOLATEDELECTROSTATIC_H
#define ISOLATEDELECTROSTATIC_H

#include<iostream>
#include<iomanip>
#include<sstream>
#include<stdlib.h>

#include "qball/Sample.h"

class IsolatedElectrostatic : public Var
{
  Sample *s;

  public:

  char const*name ( void ) const { return "isolated_electrostatic"; };

  int set ( int argc, char **argv )
  {
    if ( argc != 2 )
    {
      if ( ui->oncoutpe() )
      cout << " isolated_electrostatic takes only one value" << endl;
      return 1;
    }

    string v = argv[1];
    if ( !( v == "mt" ) )
    {
      if ( ui->oncoutpe() )
        cout << " isolated_electrostatic must be mt" << endl;
      return 1;
    }

    s->ctrl.isolated_electrostatic = v;

    return 0;
  }

  string print (void) const
  {
     ostringstream st;
     st.setf(ios::left,ios::adjustfield);
     st << setw(10) << name() << " = ";
     st.setf(ios::right,ios::adjustfield);
     st << setw(10) << s->ctrl.isolated_electrostatic;
     return st.str();
  }

  IsolatedElectrostatic(Sample *sample) : s(sample) { s->ctrl.isolated_electrostatic = ""; };
};
#endif

// Local Variables:
// mode: c++
// End:
