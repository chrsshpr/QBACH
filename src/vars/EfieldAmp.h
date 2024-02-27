////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 The Regents of the University of California
//
// This file is part of Qbox
//
// Qbox is distributed under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 2 of
// the License, or (at your option) any later version.
// See the file COPYING in the root directory of this distribution
// or <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////
//
// EfieldAmp.h
//
////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#ifndef EFIELDAMP_H 
#define EFIELDAMP_H

#include<iostream>
#include<iomanip>
#include<sstream>
#include<stdlib.h>
#include <math/d3vector.h>

#include <qball/Sample.h>

class EfieldAmp : public Var
{
  Sample *s;

  public:

  char const*name ( void ) const { return "efield_amp"; };

  //input is x y z vector, with value correspond to amplitude 
  int set ( int argc, char **argv )
  {
    if ( argc != 4 )
    {
      cout << " e_field amp takes 3 inputs: x y z" << endl;
      return 1;
    }
    
    D3vector v(atof(argv[1]), atof(argv[2]), atof(argv[3]));

    s->ctrl.efield_amp= v;

    return 0;
  }

  string print (void) const
  {
     ostringstream st;
     st.setf(ios::left,ios::adjustfield);
     st << setw(10) << name() << " = ";
     st.setf(ios::right,ios::adjustfield);
     st << "amp"  << s->ctrl.efield_amp;
     return st.str();
  }

  EfieldAmp(Sample *sample) : s(sample)
  { 
    s->ctrl.efield_amp= D3vector(0.0, 0.0, 0.0); }
  
};
#endif
