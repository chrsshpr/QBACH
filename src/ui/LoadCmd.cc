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
// LoadCmd.C
//
////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#include "LoadCmd.h"
#include <qball/Sample.h>
#include <qball/Timer.h>
#include <qball/Context.h>
#include <qball/ChargeDensity.h>
#include <qball/FourierTransform.h>
#include <qball/Basis.h>
#include "fstream"
#include <iostream>
#include <iomanip>
//ewd DEBUG
#include <qball/SlaterDet.h>
#include <qball/Wavefunction.h>
//ewd DEBUG
using namespace std;

////////////////////////////////////////////////////////////////////////////////
int LoadCmd::action(int argc, char **argv) {

  if ( !(argc>=2 && argc<=4 ) ) {
    if ( ui->oncoutpe() )
      cout << "  <!-- use: load [-dump|-fast|-states|-proj|-proj2nd|-text|-xml] [-serial] filename -->" 
           << endl;
    return 1;
  }
  
  // set default encoding
  string encoding = "dump";
  char* filename = 0;
  bool serial = false;
  bool readvel = false;
  
  // parse arguments
  for ( int i = 1; i < argc; i++ ) {
    string arg(argv[i]);
    
    if ( arg=="-text" )
      encoding = "text";
    else if ( arg=="-dump" )
      encoding = "dump";
    else if ( arg=="-fast" )
      encoding = "fast";
    else if ( arg=="-states" )
      encoding = "states";
    else if ( arg=="-proj" )
      encoding = "proj";
    else if ( arg=="-proj2nd" )
      encoding = "proj2nd";
    else if ( arg=="-full" )
      encoding = "full";
    else if ( arg=="-states-old" )
      encoding = "states-old";
    else if ( arg=="-xml" )
      encoding = "base64";
    else if ( arg=="-vel" )
      readvel = true;
    else if ( arg=="-serial" )
      serial = true;
    else if ( arg[0] != '-' && i == argc-1 )
      filename = argv[i];
    else {
      if ( ui->oncoutpe() )
        cout << "  <!-- use: load [-dump|-states|-text|-xml] [-serial] filename -->" 
             << endl;
      return 1;
    }
  }
  
  if ( filename == 0 ) {
    if ( ui->oncoutpe() )
      cout << "  <!-- use: load [-dump|-states|-text|-xml] [-serial] filename -->" 
           << endl;
    return 1;
  }

  Timer loadtm;
  loadtm.start();

  string filestr(filename);

  /////  DUMP CHECKPOINTING  /////
  if (encoding == "dump" ) {
     s->wf.read_dump(filestr);
     s->wf.read_mditer(filestr,s->ctrl.mditer);
     if ( ui->oncoutpe())
        cout << "<!-- LoadCmd:  setting MD iteration count to " << s->ctrl.mditer << ". -->" << endl;       

    if (s->ctrl.extra_memory >= 3)
      s->wf.set_highmem();    
    if (s->ctrl.ultrasoft)
      s->wf.init_usfns(&s->atoms);
      
    if (s->ctrl.tddft_involved)
    {
        string hamwffile = filestr + "hamwf";
        if ( s->hamil_wf == 0 ) {
          s->hamil_wf = new Wavefunction(s->wf);
          (*s->hamil_wf) = s->wf;
          (*s->hamil_wf).update_occ(0.0,0);
          //s->hamil_wf->clear();
        }
        s->hamil_wf->read_dump(hamwffile);
    }
    else
    {
       // ewd:  read rhor_last from file, send to appropriate pes
       ChargeDensity cd_(*s);
       cd_.update_density();

       //ewd DEBUG
       if (s->ctrl.ultrasoft)    
          cd_.update_usfns();
    
       const Context* wfctxt = s->wf.wfcontext();
       const Context* vctxt = &cd_.vcontext();
       FourierTransform* ft_ = cd_.vft();
       const double omega = cd_.vbasis()->cell().volume();
       const int nspin = s->wf.nspin();
       s->rhog_last.resize(nspin);
       for (int ispin = 0; ispin < nspin; ispin++) {
          valarray<complex<double> > rhortmp(ft_->np012loc());
          int rhorsize = cd_.rhor[ispin].size();
          ifstream is;
          string rhorfile;

          if (nspin == 1)
             rhorfile = filestr + ".lastrhor";
          else {
             ostringstream oss;
             oss.width(1);  oss.fill('0');  oss << ispin;
             rhorfile = filestr + ".s" + oss.str() + ".lastrhor";
          }

          int file_exists = 0;
          if (wfctxt->myrow() == 0) {
             is.open(rhorfile.c_str(),ifstream::binary);
             if (is.is_open()) 
                file_exists = 1;
             else 
                file_exists = -1;
             
             // send file_exists flag to other pes in column
             for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                wfctxt->isend(1,1,&file_exists,1,i,wfctxt->mycol());
             }
          }
          wfctxt->irecv(1,1,&file_exists,1,0,wfctxt->mycol());
          if (file_exists == 1) { 
             // send local charge density size from each pe in column to row 0
             if (wfctxt->mycol() == 0) {
                wfctxt->isend(1,1,&rhorsize,1,0,0);
             }
             
             if (wfctxt->onpe0()) {
                cout << "<!-- LoadCmd:  loading mixed charge density from file. -->" << endl;
                // hack to make checkpointing work w. BlueGene compilers
#ifdef HAVE_BGQLIBS
                char* tmpfilename = new char[256];
                is.read(tmpfilename,sizeof(char)*rhorfile.length());
#endif          
                for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                   int tmpsize;
                   wfctxt->irecv(1,1,&tmpsize,1,i,0);
                   vector<double> tmpr(tmpsize);
                   // read this portion of charge density, send to all pes in row i
                   is.read((char*)&tmpr[0],sizeof(double)*tmpsize);
                   if (tmpsize > 0) 
                      for ( int j = 0; j < wfctxt->npcol(); j++ ) 
                         wfctxt->dsend(tmpsize,1,&tmpr[0],1,i,j);
                }
             }
             if (rhorsize > 0) {
                vector<double> rhorrecv(rhorsize);
                wfctxt->drecv(rhorsize,1,&rhorrecv[0],1,0,0);
                // copy rhor to complex<double> array for Fourier transform
                for (int j = 0; j < rhorsize; j++)
                   rhortmp[j] = complex<double>(omega*rhorrecv[j],0.0);
             }
             int rhogsize = cd_.rhog[ispin].size();
             vector<complex<double> > rhogtmp(rhogsize);
             ft_->forward(&rhortmp[0],&rhogtmp[0]);
             s->rhog_last[ispin].resize(rhogsize);
             //complex<double> *rhogp = &s->rhog_last[ispin];
             //for (int j = 0; j < rhogsize; j++)
             //  rhogp[j] = rhogtmp[j];
             for (int j = 0; j < rhogsize; j++)
                s->rhog_last[ispin][j] = rhogtmp[j];
          }
          else {
             if ( ui->oncoutpe() )
                cout << "<!-- LoadCmd: mixed charge density checkpoint file not found. -->" << endl;
          }
          if (wfctxt->myrow() == 0) 
             is.close();
       }
    }
    
    if (readvel) { 
      const string atoms_dyn = s->ctrl.atoms_dyn;
      const bool compute_forces = ( atoms_dyn != "LOCKED" );
      if (compute_forces) {
        string wfvfile = filestr + "wfv";
        if ( s->wfv == 0 ) {
          s->wfv = new Wavefunction(s->wf);
          s->wfv->clear();
        }
        s->wfv->read_dump(wfvfile);
      }
      else {
        if ( ui->oncoutpe() )
          cout << "<WARNING>LoadCmd:  wavefunction velocities only available when atoms_dyn set, can't load.</WARNING>" << endl;
      }
    }

    if (serial)
      if ( ui->oncoutpe() )
        cout << "<!-- LoadCmd:  serial flag only used with xml input, ignoring. -->" << endl;
  }
  /////  FAST CHECKPOINTING  /////
  if (encoding == "fast" ) {
     s->wf.read_fast(filestr);
     s->wf.read_mditer(filestr,s->ctrl.mditer);
     if ( ui->oncoutpe())
        cout << "<!-- LoadCmd:  setting MD iteration count to " << s->ctrl.mditer << ". -->" << endl;       

    if (s->ctrl.extra_memory >= 3)
      s->wf.set_highmem();    
    if (s->ctrl.ultrasoft)
      s->wf.init_usfns(&s->atoms);
      
    if (s->ctrl.tddft_involved)
    {
        string hamwffile = filestr + "hamwf";
        if ( s->hamil_wf == 0 ) {
          s->hamil_wf = new Wavefunction(s->wf);
          (*s->hamil_wf) = s->wf;
          (*s->hamil_wf).update_occ(0.0,0);
          //s->hamil_wf->clear();
        }
        s->hamil_wf->read_fast(hamwffile);
    }
    else
    {
       // ewd:  read rhor_last from file, send to appropriate pes
       ChargeDensity cd_(*s);

       //ewd DEBUG
       if (s->ctrl.ultrasoft)    
          cd_.update_usfns();

       cd_.update_density();
    
       const Context* wfctxt = s->wf.wfcontext();
       const Context* vctxt = &cd_.vcontext();
       FourierTransform* ft_ = cd_.vft();
       const double omega = cd_.vbasis()->cell().volume();
       const int nspin = s->wf.nspin();
       s->rhog_last.resize(nspin);
       for (int ispin = 0; ispin < nspin; ispin++) {
          valarray<complex<double> > rhortmp(ft_->np012loc());
          int rhorsize = cd_.rhor[ispin].size();
          ifstream is;
          string rhorfile;

          if (nspin == 1)
             rhorfile = filestr + ".lastrhor";
          else {
             ostringstream oss;
             oss.width(1);  oss.fill('0');  oss << ispin;
             rhorfile = filestr + ".s" + oss.str() + ".lastrhor";
          }

          int file_exists = 0;
          if (wfctxt->myrow() == 0) {
             is.open(rhorfile.c_str(),ifstream::binary);
             if (is.is_open()) 
                file_exists = 1;
             else 
                file_exists = -1;
             
             // send file_exists flag to other pes in column
             for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                wfctxt->isend(1,1,&file_exists,1,i,wfctxt->mycol());
             }
          }
          wfctxt->irecv(1,1,&file_exists,1,0,wfctxt->mycol());
          if (file_exists == 1) { 
             // send local charge density size from each pe in column to row 0
             if (wfctxt->mycol() == 0) {
                wfctxt->isend(1,1,&rhorsize,1,0,0);
             }
             
             if (wfctxt->onpe0()) {
                cout << "<!-- LoadCmd:  loading mixed charge density from file. -->" << endl;
                // hack to make checkpointing work w. BlueGene compilers
#ifdef HAVE_BGQLIBS
                char* tmpfilename = new char[256];
                is.read(tmpfilename,sizeof(char)*rhorfile.length());
#endif          
                for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                   int tmpsize;
                   wfctxt->irecv(1,1,&tmpsize,1,i,0);
                   vector<double> tmpr(tmpsize);
                   // read this portion of charge density, send to all pes in row i
                   is.read((char*)&tmpr[0],sizeof(double)*tmpsize);
                   if (tmpsize > 0) 
                      for ( int j = 0; j < wfctxt->npcol(); j++ ) 
                         wfctxt->dsend(tmpsize,1,&tmpr[0],1,i,j);
                }
             }
             if (rhorsize > 0) {
                vector<double> rhorrecv(rhorsize);
                wfctxt->drecv(rhorsize,1,&rhorrecv[0],1,0,0);
                // copy rhor to complex<double> array for Fourier transform
                for (int j = 0; j < rhorsize; j++)
                   rhortmp[j] = complex<double>(omega*rhorrecv[j],0.0);
             }
             int rhogsize = cd_.rhog[ispin].size();
             vector<complex<double> > rhogtmp(rhogsize);
             ft_->forward(&rhortmp[0],&rhogtmp[0]);
             s->rhog_last[ispin].resize(rhogsize);
             //complex<double> *rhogp = &s->rhog_last[ispin];
             //for (int j = 0; j < rhogsize; j++)
             //  rhogp[j] = rhogtmp[j];
             for (int j = 0; j < rhogsize; j++)
                s->rhog_last[ispin][j] = rhogtmp[j];
          }
          else {
             if ( ui->oncoutpe() )
                cout << "<!-- LoadCmd: mixed charge density checkpoint file not found. -->" << endl;
          }
          if (wfctxt->myrow() == 0) 
             is.close();
       }
    }
    
    if (readvel) { 
      const string atoms_dyn = s->ctrl.atoms_dyn;
      const bool compute_forces = ( atoms_dyn != "LOCKED" );
      if (compute_forces) {
        string wfvfile = filestr + "wfv";
        if ( s->wfv == 0 ) {
          s->wfv = new Wavefunction(s->wf);
          s->wfv->clear();
        }
        s->wfv->read_fast(wfvfile);
      }
      else {
        if ( ui->oncoutpe() )
          cout << "<WARNING>LoadCmd:  wavefunction velocities only available when atoms_dyn set, can't load.</WARNING>" << endl;
      }
    }

    if (serial)
      if ( ui->oncoutpe() )
        cout << "<!-- LoadCmd:  serial flag only used with xml input, ignoring. -->" << endl;
  }
  /////  STATES CHECKPOINTING  /////
  else if (encoding == "states" ) {
     s->wf.read_states(filestr);
     s->wf.read_mditer(filestr,s->ctrl.mditer);
     if ( ui->oncoutpe())
        cout << "<!-- LoadCmd:  setting MD iteration count to " << s->ctrl.mditer << ". -->" << endl;       

    if (s->ctrl.extra_memory >= 3)
      s->wf.set_highmem();    
    if (s->ctrl.ultrasoft)
      s->wf.init_usfns(&s->atoms);

    if (s->ctrl.tddft_involved)
    {
        string hamwffile = filestr + "hamwf";
        if ( s->hamil_wf == 0 ) {
          s->hamil_wf = new Wavefunction(s->wf);
          (*s->hamil_wf) = s->wf;
          (*s->hamil_wf).update_occ(0.0,0);
          //s->hamil_wf->clear();
        }
        s->hamil_wf->read_states(hamwffile);
    }
    else
    {
       // ewd:  read rhor_last from file, send to appropriate pes
       ChargeDensity cd_(*s);

       //ewd DEBUG
       if (s->ctrl.ultrasoft)    
          cd_.update_usfns();
       
       cd_.update_density();

       const Context* wfctxt = s->wf.wfcontext();
       const Context* vctxt = &cd_.vcontext();
       FourierTransform* ft_ = cd_.vft();
       const double omega = cd_.vbasis()->cell().volume();
       const int nspin = s->wf.nspin();
       s->rhog_last.resize(nspin);
       for (int ispin = 0; ispin < nspin; ispin++) {
          valarray<complex<double> > rhortmp(ft_->np012loc());
          int rhorsize = cd_.rhor[ispin].size();
          ifstream is;
          string rhorfile;

          if (nspin == 1)
             rhorfile = filestr + ".lastrhor";
          else {
             ostringstream oss;
             oss.width(1);  oss.fill('0');  oss << ispin;
             rhorfile = filestr + ".s" + oss.str() + ".lastrhor";
          }

          int file_exists = 0;
          if (wfctxt->myrow() == 0) {
             is.open(rhorfile.c_str(),ifstream::binary);
             if (is.is_open()) 
                file_exists = 1;
             else 
                file_exists = -1;

             // send file_exists flag to other pes in column
             for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                wfctxt->isend(1,1,&file_exists,1,i,wfctxt->mycol());
             }
          }
          wfctxt->irecv(1,1,&file_exists,1,0,wfctxt->mycol());
          if (file_exists == 1) { 
             // send local charge density size from each pe in column to row 0
             if (wfctxt->mycol() == 0) {
                wfctxt->isend(1,1,&rhorsize,1,0,0);
             }
             
             if (wfctxt->onpe0()) {
                cout << "<!-- LoadCmd:  loading mixed charge density from file. -->" << endl;
                // hack to make checkpointing work w. BlueGene compilers
#ifdef HAVE_BGQLIBS
                char* tmpfilename = new char[256];
                is.read(tmpfilename,sizeof(char)*rhorfile.length());
#endif
                
                for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                   int tmpsize;
                   wfctxt->irecv(1,1,&tmpsize,1,i,0);
                   vector<double> tmpr(tmpsize);
                   // read this portion of charge density, send to all pes in row i
                   is.read((char*)&tmpr[0],sizeof(double)*tmpsize);
                   if (tmpsize > 0) 
                      for ( int j = 0; j < wfctxt->npcol(); j++ ) 
                         wfctxt->dsend(tmpsize,1,&tmpr[0],1,i,j);
                }
             }
             if (rhorsize > 0) {
                vector<double> rhorrecv(rhorsize);
                wfctxt->drecv(rhorsize,1,&rhorrecv[0],1,0,0);
                // copy rhor to complex<double> array for Fourier transform
                for (int j = 0; j < rhorsize; j++)
                   rhortmp[j] = complex<double>(omega*rhorrecv[j],0.0);
             }
             int rhogsize = cd_.rhog[ispin].size();
             vector<complex<double> > rhogtmp(rhogsize);
             ft_->forward(&rhortmp[0],&rhogtmp[0]);
             s->rhog_last[ispin].resize(rhogsize);
             //complex<double> *rhogp = &s->rhog_last[ispin];
             //for (int j = 0; j < rhogsize; j++)
             //  rhogp[j] = rhogtmp[j];
             for (int j = 0; j < rhogsize; j++)
                s->rhog_last[ispin][j] = rhogtmp[j];
          }
          else {
             if ( ui->oncoutpe() )
                cout << "<!-- LoadCmd: mixed charge density checkpoint file not found. -->" << endl;
          }
          if (wfctxt->myrow() == 0) 
             is.close();
       }
    }
    
    if (readvel) { 
      const string atoms_dyn = s->ctrl.atoms_dyn;
      const bool compute_forces = ( atoms_dyn != "LOCKED" );
      if (compute_forces) {
        string wfvfile = filestr + "wfv";
        if ( s->wfv == 0 ) {
          s->wfv = new Wavefunction(s->wf);
          s->wfv->clear();
        }
        s->wfv->read_states(wfvfile);
      }
      else {
        if ( ui->oncoutpe() )
          cout << "<WARNING>LoadCmd:  wavefunction velocities only available when atoms_dyn set, can't load.</WARNING>" << endl;
      }
    }

    if (serial)
      if ( ui->oncoutpe() )
        cout << "<!-- LoadCmd:  serial flag only used with xml input, ignoring. -->" << endl;

    }
  ///// PROJ STATES /////
  if (encoding == "proj") {
  new Wavefunction(s->wf);
  s->proj_wf = new Wavefunction(s->wf);
  *(s->proj_wf) = s->wf;
  (*(s->proj_wf)).read_states(filestr);
  }

 if (encoding == "full") {
  //new Wavefunction(s->wf);
  //s->proj_wf = new Wavefunction(s->wf);
  //(s->proj_wf) = s->wf;
  (*(s->proj_wf_virtual)).read_states(filestr);
  }

  //////  2nd proj ///////
  if (encoding == "proj2nd") {
  new Wavefunction(s->wf);
  s->proj2nd_wf = new Wavefunction(s->wf);
  *(s->proj2nd_wf) = s->wf;
  (*(s->proj2nd_wf)).read_states(filestr);
  }

  /////  OLD STATES CHECKPOINTING  /////
  else if (encoding == "states-old" ) {
     s->wf.read_states_old(filestr);
     s->wf.read_mditer(filestr,s->ctrl.mditer);
     if ( ui->oncoutpe())
        cout << "<!-- LoadCmd:  setting MD iteration count to " << s->ctrl.mditer << ". -->" << endl;       

    if (s->ctrl.extra_memory >= 3)
      s->wf.set_highmem();    
    if (s->ctrl.ultrasoft)
      s->wf.init_usfns(&s->atoms);

    if (s->ctrl.tddft_involved)
    {
        string hamwffile = filestr + "hamwf";
        if ( s->hamil_wf == 0 ) {
          s->hamil_wf = new Wavefunction(s->wf);
          (*s->hamil_wf) = s->wf;
          (*s->hamil_wf).update_occ(0.0,0);
          //s->hamil_wf->clear();
        }
        s->hamil_wf->read_states_old(hamwffile);
    }
    else
    {
       // ewd:  read rhor_last from file, send to appropriate pes
       ChargeDensity cd_(*s);

       //ewd DEBUG
       if (s->ctrl.ultrasoft)    
          cd_.update_usfns();
       
       cd_.update_density();

       const Context* wfctxt = s->wf.wfcontext();
       const Context* vctxt = &cd_.vcontext();
       FourierTransform* ft_ = cd_.vft();
       const double omega = cd_.vbasis()->cell().volume();
       const int nspin = s->wf.nspin();
       s->rhog_last.resize(nspin);
       for (int ispin = 0; ispin < nspin; ispin++) {
          valarray<complex<double> > rhortmp(ft_->np012loc());
          int rhorsize = cd_.rhor[ispin].size();
          ifstream is;
          string rhorfile;

          if (nspin == 1)
             rhorfile = filestr + ".lastrhor";
          else {
             ostringstream oss;
             oss.width(1);  oss.fill('0');  oss << ispin;
             rhorfile = filestr + ".s" + oss.str() + ".lastrhor";
          }

          int file_exists = 0;
          if (wfctxt->myrow() == 0) {
             is.open(rhorfile.c_str(),ifstream::binary);
             if (is.is_open()) 
                file_exists = 1;
             else 
                file_exists = -1;

             // send file_exists flag to other pes in column
             for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                wfctxt->isend(1,1,&file_exists,1,i,wfctxt->mycol());
             }
          }
          wfctxt->irecv(1,1,&file_exists,1,0,wfctxt->mycol());
          if (file_exists == 1) { 
             // send local charge density size from each pe in column to row 0
             if (wfctxt->mycol() == 0) {
                wfctxt->isend(1,1,&rhorsize,1,0,0);
             }
             
             if (wfctxt->onpe0()) {
                cout << "<!-- LoadCmd:  loading mixed charge density from file. -->" << endl;
                // hack to make checkpointing work w. BlueGene compilers
#ifdef HAVE_BGQLIBS
                char* tmpfilename = new char[256];
                is.read(tmpfilename,sizeof(char)*rhorfile.length());
#endif
                
                for ( int i = 0; i < wfctxt->nprow(); i++ ) {
                   int tmpsize;
                   wfctxt->irecv(1,1,&tmpsize,1,i,0);
                   vector<double> tmpr(tmpsize);
                   // read this portion of charge density, send to all pes in row i
                   is.read((char*)&tmpr[0],sizeof(double)*tmpsize);
                   if (tmpsize > 0) 
                      for ( int j = 0; j < wfctxt->npcol(); j++ ) 
                         wfctxt->dsend(tmpsize,1,&tmpr[0],1,i,j);
                }
             }
             if (rhorsize > 0) {
                vector<double> rhorrecv(rhorsize);
                wfctxt->drecv(rhorsize,1,&rhorrecv[0],1,0,0);
                // copy rhor to complex<double> array for Fourier transform
                for (int j = 0; j < rhorsize; j++)
                   rhortmp[j] = complex<double>(omega*rhorrecv[j],0.0);
             }
             int rhogsize = cd_.rhog[ispin].size();
             vector<complex<double> > rhogtmp(rhogsize);
             ft_->forward(&rhortmp[0],&rhogtmp[0]);
             s->rhog_last[ispin].resize(rhogsize);
             //complex<double> *rhogp = &s->rhog_last[ispin];
             //for (int j = 0; j < rhogsize; j++)
             //  rhogp[j] = rhogtmp[j];
             for (int j = 0; j < rhogsize; j++)
                s->rhog_last[ispin][j] = rhogtmp[j];
          }
          else {
             if ( ui->oncoutpe() )
                cout << "<!-- LoadCmd: mixed charge density checkpoint file not found. -->" << endl;
          }
          if (wfctxt->myrow() == 0) 
             is.close();
       }
    }
    
    if (readvel) { 
      const string atoms_dyn = s->ctrl.atoms_dyn;
      const bool compute_forces = ( atoms_dyn != "LOCKED" );
      if (compute_forces) {
        string wfvfile = filestr + "wfv";
        if ( s->wfv == 0 ) {
          s->wfv = new Wavefunction(s->wf);
          s->wfv->clear();
        }
        s->wfv->read_states_old(wfvfile);
      }
      else {
        if ( ui->oncoutpe() )
          cout << "<WARNING>LoadCmd:  wavefunction velocities only available when atoms_dyn set, can't load.</WARNING>" << endl;
      }
    }

    if (serial)
      if ( ui->oncoutpe() )
        cout << "<!-- LoadCmd:  serial flag only used with xml input, ignoring. -->" << endl;

    }
  /////  XML CHECKPOINTING  /////
  else {
    if( ui->oncoutpe() ) ui->error("LoadCmd: XML checkpointing is deprecated.");
  }

  if (s->ctrl.extra_memory >= 3)
    s->wf.set_highmem();    
  if (s->ctrl.ultrasoft)
    s->wf.init_usfns(&s->atoms);
        
  loadtm.stop();

  double time = loadtm.cpu();
  double tmin = time;
  double tmax = time;
    
  s->ctxt_.dmin(1,1,&tmin,1);
  s->ctxt_.dmax(1,1,&tmax,1);
  if ( ui->oncoutpe() ) {
    cout << "<!--  load timing : " << setprecision(4) << setw(9) << tmin
         << " " << setprecision(4) << setw(9) << tmax << " -->" << endl;
  }

  return 0;
}
