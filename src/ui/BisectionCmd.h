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
// BisectionCmd.h:
//
////////////////////////////////////////////////////////////////////////////////

#ifndef BISECTIONCMD_H
#define BISECTIONCMD_H

#include <iostream>
#include <ui/UserInterface.h>
#include <qball/Sample.h>

#include <cstdlib>
#include <bitset>
#include <qball/Bisection.h>
#include <qball/Timer.h>

class BisectionCmd : public Cmd
{
  public:
  Sample *s;

  BisectionCmd(Sample *sample) : s(sample) {};

  const char *name(void) const { return "bisection"; }

  const char *help_msg(void) const
  {
    return
    "\n bisection\n\n"
    " syntax: bisection lx ly lz threshold\n\n"
    "   The bisection command performs recursive bisection on the\n"
    "   wavefunctions.\n\n";
  }

  int action(int argc, char **argv)
  {
    if ( argc != 5 )
    {
      if ( ui->oncoutpe() )
      {
        cout << " use: bisection lx ly lz threshold" << endl;
      }
      return 1;
    }

    Timer tm;

    Wavefunction &wf=s->wf;
    double epsilon=atof(argv[4]);
    int nLevels[3];
    nLevels[0]=atoi(argv[1]);
    nLevels[1]=atoi(argv[2]);
    nLevels[2]=atoi(argv[3]);

    if ( epsilon < 0.0 )
    {
      if ( ui->oncoutpe() )
      {
        cout << " BisectionCmd: threshold must be non-negative" << endl;
      }
      return 1;
    }

    if ( wf.nkp() > 1 )
    {
      if ( ui->oncoutpe() )
      {
        cout << " BisectionCmd: only implemented for k=0" << endl;
      }
      return 1;
    }

    if ( nLevels[0] < 0 || nLevels[0] > 5 ||
         nLevels[1] < 0 || nLevels[1] > 5 ||
         nLevels[2] < 0 || nLevels[2] > 5 )
    {
      if ( ui->oncoutpe() )
      {
        cout << " BisectionCmd: levels must be in [0,5]" << endl;
      }
      return 1;
    }

    tm.reset();
    for ( int ispin = 0; ispin < wf.nspin(); ispin++ )
    {
      SlaterDet& sd = *wf.sd(0,0);
      Bisection bisection(sd,nLevels);
      const int maxsweep = 20;
      const double tol = 1.e-8;
      tm.start();
      bisection.compute_transform(sd);
      bisection.compute_localization(epsilon);
      bisection.forward(sd);
      tm.stop();
      vector<long int> localization = bisection.localization();

      if ( ui->oncoutpe() )
      {
        cout << " BisectionCmd: lx=" << nLevels[0]
             << " ly=" << nLevels[1]
             << " lz=" << nLevels[2]
             << " threshold=" << epsilon << endl;

        // print localization vectors and overlaps
        int sum = 0;
        const int nst = localization.size();
        for ( int i = 0; i < nst; i++ )
        {
          int count = 0;
          for ( int j = 0; j < nst; j++ )
          {
            if ( bisection.overlap(i,j) )
              count++;
          }
          cout << "localization[" << i << "]: "
               << localization[i] << " "
               << bitset<30>(localization[i])
               << " size: " << bisection.size(i)
               << " overlaps: " << count << endl;
          sum += count;
        }
        cout << " Bisection: total size:    ispin=" << ispin
             << ": " << bisection.total_size() << endl;
        cout << " Bisection: pair fraction: ispin=" << ispin
             << ": " << bisection.pair_fraction() << endl;
      }
    }
    if ( ui->oncoutpe() )
      cout << " Bisection: time=" << tm.real() << endl;
    return 0;
  }
};
#endif
