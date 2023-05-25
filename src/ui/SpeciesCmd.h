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
// SpeciesCmd.h:
//
////////////////////////////////////////////////////////////////////////////////

#include <config.h>

#ifndef SPECIESCMD_H
#define SPECIESCMD_H

#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

#include <ui/UserInterface.h>
#include <qball/Sample.h>
#include <qball/SpeciesReader.h>
#include <qball/Species.h>
#include <qball/Messages.h>
#include <pseudo/set.hpp>

class SpeciesCmd : public Cmd
{
  public:

  Sample *s;

  SpeciesCmd(Sample *sample) : s(sample) { s->ctrl.ultrasoft = false; s->ctrl.nlcc = false; };

  char const*name(void) const { return "species"; }

  char const*help_msg(void) const
  {
    return 
    "\n species\n\n"
    " syntax: species <name> <filename>\n\n"
    "     or: species collection <collection_name>\n\n"
    "   The species command defines a species name based on a file, or\n";
    "   if the 'collection' keyword is specified it will load a group of\n";
    "   species. Valid options are:\n";
    "      sg15\n";
    "      hscv_lda\n";
    "      hscv_pbe\n";
    "      pseudodojo_pbe\n";
    "      pseudodojo_pbe_stringent\n";
    "      pseudodojo_pbesol\n";
    "      pseudodojo_pbesol_stringent\n";
    "      pseudodojo_lda\n";
    "      pseudodojo_lda_stringent\n";
  }

  int action(int argc, char **argv) {
    if (! (argc == 3 || argc == 4)) {
      if ( ui->oncoutpe() ) {
	cout << "  <!-- use: species <name> <filename> [ewald_width] -->" << endl;
      	cout << "  <!--  or: species collection <collection_name> [ewald_width] -->" << endl;
      }
      return 1;
    }

    if(argv[1] != std::string("collection")){
    
      if ( ui->oncoutpe() ) cout << "  <!-- SpeciesCmd: defining species " << argv[1] << " as " << argv[2] << " -->" << endl;
      
      SpeciesReader sp_reader(s->ctxt_);
      
      Species* sp = new Species(s->ctxt_, argv[1]);
      
      try {
	sp_reader.readSpecies(*sp, argv[2]);
	sp_reader.bcastSpecies(*sp);
	if (argc == 4) {
	  const double rcpsin = atof(argv[3]);
	  s->atoms.addSpecies(sp, argv[1], rcpsin);
	} else {
	  s->atoms.addSpecies(sp,argv[1]);
	}
	
	if (sp->ultrasoft()) {
	  s->ctrl.ultrasoft = true;
	  s->wf.set_ultrasoft(true);
	}
	
	if (sp->nlcc()) {
	  s->ctrl.nlcc = true;
	}
	
      }
      catch ( const SpeciesReaderException& e ) {
	cout << " SpeciesReaderException caught in SpeciesCmd" << endl;
	cout << " SpeciesReaderException: cannot define Species" << endl;
      }
      catch (...) {
	cout << " SpeciesCmd: cannot define Species" << endl;
      }

    } else {
      
      std::string colname(argv[2]);
      std::string dirname = std::string(SHARE_DIR) + "/pseudopotentials/";

      if(colname == "SG15" || colname == "sg15") {
	dirname += "quantum-simulation.org/sg15/";

      } else if(colname == "HSCV_LDA" || colname == "hscv_lda") {
	dirname += "quantum-simulation.org/hscv/lda/";
	
      } else if(colname == "HSCV_PBE" || colname == "hscv_pbe") {
	dirname += "quantum-simulation.org/hscv/pbe/";
	
      } else if(colname == "pseudodojo_pbe" || colname == "PSEUDODOJO_PBE") {
	dirname += "pseudo-dojo.org/nc-sr-04_pbe_standard/";
	
      } else if(colname == "pseudodojo_pbe_stringent" || colname == "PSEUDODOJO_PBE_STRINGENT") {
	dirname += "pseudo-dojo.org/nc-sr-04_pbe_stringent/";
	
      } else if(colname == "pseudodojo_pbesol" || colname == "PSEUDODOJO_PBESOL") {
	dirname += "pseudo-dojo.org/nc-sr-04_pbesol_standard/";
	
      } else if(colname == "pseudodojo_pbesol_stringent" || colname == "PSEUDODOJO_PBESOL_STRINGENT") {
	dirname += "pseudo-dojo.org/nc-sr-04_pbesol_stringent/";
	
      } else if(colname == "pseudodojo_lda" || colname == "PSEUDODOJO_LDA") {
	dirname += "pseudo-dojo.org/nc-sr-04_pw_standard/";
	
      } else if(colname == "pseudodojo_lda_stringent" || colname == "PSEUDODOJO_LDA_STRINGENT") {
	dirname += "pseudo-dojo.org/nc-sr-04_pw_stringent/";
	
      } else {
	ui->error("Unknown species collection '" + colname + "'");
	return 1;
      }

      pseudopotential::set collection(dirname);

      for(pseudopotential::set::iterator it = collection.begin(); it != collection.end(); ++it){
	pseudopotential::element el = *it;

	if ( ui->oncoutpe() ) cout << "  <!-- SpeciesCmd: defining species " << argv[1] << " as " << argv[2] << " -->" << endl;
	
	SpeciesReader sp_reader(s->ctxt_);
	
	Species* sp = new Species(s->ctxt_, el.symbol());
	
	try {
	  sp_reader.readSpecies(*sp, collection.file_path(el));
	  sp_reader.bcastSpecies(*sp);
	  s->atoms.addSpecies(sp, el.symbol());
	  
	  if (sp->ultrasoft()) {
	    s->ctrl.ultrasoft = true;
	    s->wf.set_ultrasoft(true);
	  }
	  
	  if (sp->nlcc()) {
	    s->ctrl.nlcc = true;
	  }
	  
	}
	catch ( const SpeciesReaderException& e ) {
	  cout << " SpeciesReaderException caught in SpeciesCmd" << endl;
	  cout << " SpeciesReaderException: cannot define Species" << endl;
	}
	catch (...) {
	  cout << " SpeciesCmd: cannot define Species" << endl;
	}
       
      }
      
    }
      
    return 0;
  }

};
#endif

// Local Variables:
// mode: c++
// End:
