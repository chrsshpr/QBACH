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
// Save2ndProjFreq.h
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SAVE2NDPROJFREQ_H
#define SAVE2NDPROJFREQ_H

#include<iostream>
#include<iomanip>
#include<sstream>
#include<stdlib.h>

#include <qball/Sample.h>

class Save2ndProjFreq : public Var
{
  Sample *s;

  public:

  char const*name ( void ) const { return "save2ndprojfreq"; };

  int set ( int argc, char **argv )
  {
    if ( argc != 2 && argc != 3)
    {
      if ( ui->oncoutpe() )
      cout << " save2ndprojfreq takes only one or two values" << endl;
      return 1;
    }
    int v = atoi(argv[1]);
    s->ctrl.save2ndprojfreq = v;
    if (argc == 3)
       s->ctrl.save2ndprojfilebase = argv[2];

    return 0;
  }

  string print (void) const
  {
     ostringstream st;
     st.setf(ios::left,ios::adjustfield);
     st << name() << ":  ";
     st.setf(ios::right,ios::adjustfield);
     st << s->ctrl.save2ndprojfreq;
     return st.str();
  }

  Save2ndProjFreq(Sample *sample) : s(sample) {
     s->ctrl.save2ndprojfreq = -1 ;
     s->ctrl.save2ndprojfilebase = "proj";
  }
};
#endif

//CS
// Local Variables:
// mode: c++
// End:
