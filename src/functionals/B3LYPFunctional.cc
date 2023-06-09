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
// B3LYPFunctional.cc
//
// Includes LDA Exchange-correlation energy and potential (VWN paramters)
// S.H.Vosko, L.Wilk, M.Nusair, Can. J. Phys. 58, 1200 (1980) 
////////////////////////////////////////////////////////////////////////////////

#include <config.h>
#include <cmath>
#include <cassert>
#include "B3LYPFunctional.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
B3LYPFunctional::B3LYPFunctional(const vector<vector<double> > &rhoe) {
  _nspin = rhoe.size();
  if ( _nspin > 1 ) assert(rhoe[0].size() == rhoe[1].size());
  _np = rhoe[0].size();

  if ( _nspin == 1 )
  {
    _exc.resize(_np);
    _vxc1.resize(_np);
    _vxc2.resize(_np);
    _grad_rho[0].resize(_np);
    _grad_rho[1].resize(_np);
    _grad_rho[2].resize(_np);
    rho = &rhoe[0][0];
    grad_rho[0] = &_grad_rho[0][0];
    grad_rho[1] = &_grad_rho[1][0];
    grad_rho[2] = &_grad_rho[2][0];
    exc = &_exc[0];
    vxc1 = &_vxc1[0];
    vxc2 = &_vxc2[0];
  }
  else
  {
    _exc_up.resize(_np);
    _exc_dn.resize(_np);
    _vxc1_up.resize(_np);
    _vxc1_dn.resize(_np);
    _vxc2_upup.resize(_np);
    _vxc2_updn.resize(_np);
    _vxc2_dnup.resize(_np);
    _vxc2_dndn.resize(_np);
    _grad_rho_up[0].resize(_np);
    _grad_rho_up[1].resize(_np);
    _grad_rho_up[2].resize(_np);
    _grad_rho_dn[0].resize(_np);
    _grad_rho_dn[1].resize(_np);
    _grad_rho_dn[2].resize(_np);

    rho_up = &rhoe[0][0];
    rho_dn = &rhoe[1][0];
    grad_rho_up[0] = &_grad_rho_up[0][0];
    grad_rho_up[1] = &_grad_rho_up[1][0];
    grad_rho_up[2] = &_grad_rho_up[2][0];
    grad_rho_dn[0] = &_grad_rho_dn[0][0];
    grad_rho_dn[1] = &_grad_rho_dn[1][0];
    grad_rho_dn[2] = &_grad_rho_dn[2][0];
    exc_up = &_exc_up[0];
    exc_dn = &_exc_dn[0];
    vxc1_up = &_vxc1_up[0];
    vxc1_dn = &_vxc1_dn[0];
    vxc2_upup = &_vxc2_upup[0];
    vxc2_updn = &_vxc2_updn[0];
    vxc2_dnup = &_vxc2_dnup[0];
    vxc2_dndn = &_vxc2_dndn[0];
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::setxc(void)
{
  if ( _np == 0 ) return;

  if ( _nspin == 1 )
  {
    assert( rho != 0 );
    assert( grad_rho[0] != 0 && grad_rho[1] != 0 && grad_rho[2] != 0 );
    assert( exc != 0 );
    assert( vxc1 != 0 );
    assert( vxc2 != 0 );
    for ( int i = 0; i < _np; i++ )
    {
      double grad = sqrt(grad_rho[0][i]*grad_rho[0][i] +
                         grad_rho[1][i]*grad_rho[1][i] +
                         grad_rho[2][i]*grad_rho[2][i] );
      excb3lyp(rho[i],grad,&exc[i],&vxc1[i],&vxc2[i]);
    }
  }
  else
  {
    assert( rho_up != 0 );
    assert( rho_dn != 0 );
    assert( grad_rho_up[0] != 0 && grad_rho_up[1] != 0 && grad_rho_up[2] != 0 );
    assert( grad_rho_dn[0] != 0 && grad_rho_dn[1] != 0 && grad_rho_dn[2] != 0 );
    assert( exc_up != 0 );
    assert( exc_dn != 0 );
    assert( vxc1_up != 0 );
    assert( vxc1_dn != 0 );
    assert( vxc2_upup != 0 );
    assert( vxc2_updn != 0 );
    assert( vxc2_dnup != 0 );
    assert( vxc2_dndn != 0 );

    for ( int i = 0; i < _np; i++ )
    {
      double grx_up = grad_rho_up[0][i];
      double gry_up = grad_rho_up[1][i];
      double grz_up = grad_rho_up[2][i];
      double grx_dn = grad_rho_dn[0][i];
      double gry_dn = grad_rho_dn[1][i];
      double grz_dn = grad_rho_dn[2][i];
      double grad_up2 = grx_up*grx_up + gry_up*gry_up + grz_up*grz_up;
      double grad_dn2 = grx_dn*grx_dn + gry_dn*gry_dn + grz_dn*grz_dn;
      double grad_up_grad_dn = grx_up*grx_dn + gry_up*gry_dn + grz_up*grz_dn;

      excb3lyp_sp(rho_up[i],rho_dn[i],grad_up2,grad_dn2,grad_up_grad_dn,
                  &exc_up[i],&exc_dn[i],&vxc1_up[i],&vxc1_dn[i],
                  &vxc2_upup[i],&vxc2_dndn[i],
                  &vxc2_updn[i],&vxc2_dnup[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::excb3lyp(double rho, double grad,
  double *exc, double *vxc1, double *vxc2)
{
  // B3LYP unpolarized
  // Coefficients of the B3LYP functional
  // A. Becke, JCP 98, 5648 (1993)
  // See also X.Xu and W. Goddard, J.Phys.Chem. A108, 2305 (2004)
  // EcLSDA is the Vosko-Wilk-Nusair correlation energy
  // dExBecke88 is the difference ExB88 - ExLDA
  // 0.2 ExHF + 0.80 ExSlater + 0.19 EcLSDA + 0.81 EcLYP + 0.72 dExBecke88
  const double xlda_coeff = 0.80; // Slater exchange
  const double clda_coeff = 0.19; // LSDA correlation
  const double xb88_coeff = 0.72; // Becke88 exchange gradient correction
  const double clyp_coeff = 1.0 - clda_coeff;

  double ex_lda,vx_lda,ec_lda,vc_lda;
  double ex_b88,vx1_b88,vx2_b88;
  double ec_lyp,vc1_lyp,vc2_lyp;

  // VWN ex and ec 
  exvwn(rho,ex_lda,vx_lda);
  ecvwn(rho,ec_lda,vc_lda);

  //Becke 88 ex and LYP ec 
  exb88(rho,grad,&ex_b88,&vx1_b88,&vx2_b88);
  eclyp(rho,grad,&ec_lyp,&vc1_lyp,&vc2_lyp);

  const double dex_b88 = ex_b88 - ex_lda;
  const double dvx1_b88 = vx1_b88 - vx_lda;
  const double dvx2_b88 = vx2_b88;

  *exc = xlda_coeff  * ex_lda +
         clda_coeff  * ec_lda +
         xb88_coeff  * dex_b88 +
         clyp_coeff  * ec_lyp;

  *vxc1 = xlda_coeff * vx_lda +
          clda_coeff * vc_lda +
          xb88_coeff * dvx1_b88 +
          clyp_coeff * vc1_lyp;

  *vxc2 = xb88_coeff * dvx2_b88 +
          clyp_coeff * vc2_lyp;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::excb3lyp_sp(double rho_up, double rho_dn,
  double grad_up2, double grad_dn2, double grad_up_grad_dn,
  double *exc_up, double *exc_dn, double *vxc1_up, double *vxc1_dn,
  double *vxc2_upup, double *vxc2_dndn, double *vxc2_updn, double *vxc2_dnup)
{
  // B3LYP spin-polarized
  // Coefficients of the B3LYP functional
  // A. Becke, JCP 98, 5648 (1993)
  // See also X.Xu and W. Goddard, J.Phys.Chem. A108, 2305 (2004)
  // EcLSDA is the Vosko-Wilk-Nusair correlation energy
  // dExBecke88 is the difference ExB88 - ExLDA
  // 0.2 ExHF + 0.80 ExSlater + 0.19 EcLSDA + 0.81 EcLYP + 0.72 dExBecke88
  const double xlda_coeff = 0.80; // Slater exchange
  const double clda_coeff = 0.19; // LSDA correlation
  const double xb88_coeff = 0.72; // Becke88 exchange gradient correction
  const double clyp_coeff = 1.0 - clda_coeff;

  double ex_lda,vx_lda_up,vx_lda_dn;
  double ec_lda,vc_lda_up,vc_lda_dn;
  double ex_b88_up,ex_b88_dn,vx1_b88_up,vx1_b88_dn,
         vx2_b88_upup,vx2_b88_dndn,vx2_b88_updn,vx2_b88_dnup;
  double ec_lyp_up,ec_lyp_dn,vc1_lyp_up,vc1_lyp_dn,
         vc2_lyp_upup,vc2_lyp_dndn,vc2_lyp_updn,vc2_lyp_dnup;

  // VWN ex and ec 
  exvwn_sp(rho_up,rho_dn,ex_lda,vx_lda_up,vx_lda_dn);
  ecvwn_sp(rho_up,rho_dn,ec_lda,vc_lda_up,vc_lda_dn);

  // Becke 88 ex and LYP ec 
  exb88_sp(rho_up,rho_dn,grad_up2,grad_dn2,grad_up_grad_dn,
    &ex_b88_up,&ex_b88_dn,&vx1_b88_up,&vx1_b88_dn,
    &vx2_b88_upup,&vx2_b88_dndn,&vx2_b88_updn,&vx2_b88_dnup);
  eclyp_sp(rho_up,rho_dn,grad_up2,grad_dn2,grad_up_grad_dn,
    &ec_lyp_up,&ec_lyp_dn,&vc1_lyp_up,&vc1_lyp_dn,
    &vc2_lyp_upup,&vc2_lyp_dndn,&vc2_lyp_updn,&vc2_lyp_dnup);

  const double dex_b88_up = ex_b88_up - ex_lda;
  const double dex_b88_dn = ex_b88_dn - ex_lda;
  const double dvx1_b88_up = vx1_b88_up - vx_lda_up;
  const double dvx1_b88_dn = vx1_b88_dn - vx_lda_dn;
  const double dvx2_b88_upup = vx2_b88_upup;
  const double dvx2_b88_dndn = vx2_b88_dndn;
  const double dvx2_b88_updn = vx2_b88_updn;
  const double dvx2_b88_dnup = vx2_b88_dnup;

  *exc_up = xlda_coeff  * ex_lda +
            clda_coeff  * ec_lda +
            xb88_coeff  * dex_b88_up +
            clyp_coeff  * ec_lyp_up;

  *exc_dn = xlda_coeff  * ex_lda +
            clda_coeff  * ec_lda +
            xb88_coeff  * dex_b88_dn +
            clyp_coeff  * ec_lyp_dn;

  *vxc1_up = xlda_coeff * vx_lda_up +
             clda_coeff * vc_lda_up +
             xb88_coeff * dvx1_b88_up +
             clyp_coeff * vc1_lyp_up;

  *vxc1_dn = xlda_coeff * vx_lda_dn +
             clda_coeff * vc_lda_dn +
             xb88_coeff * dvx1_b88_dn +
             clyp_coeff * vc1_lyp_dn;

  *vxc2_upup = xb88_coeff * dvx2_b88_upup + clyp_coeff * vc2_lyp_upup;
  *vxc2_dndn = xb88_coeff * dvx2_b88_dndn + clyp_coeff * vc2_lyp_dndn;
  *vxc2_updn = xb88_coeff * dvx2_b88_updn + clyp_coeff * vc2_lyp_updn;
  *vxc2_dnup = xb88_coeff * dvx2_b88_dnup + clyp_coeff * vc2_lyp_dnup;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::exb88(double rho, double grad,
  double *ex, double *vx1, double *vx2)
{
  // Becke exchange constants
  const double fourthirds = 4.0 / 3.0;
  const double beta=0.0042;
  const double axa = -0.9305257363490999;   // -1.5*pow(3.0/(4*pi),third)

  *ex = 0.0;
  *vx1 = 0.0;
  *vx2 = 0.0;

  if ( rho < 1.e-18 ) return;

  // Becke's exchange
  // A.D.Becke, Phys.Rev. B38, 3098 (1988)

  const double rha = 0.5 * rho;
  const double grada = 0.5 * grad;

  const double rha13 = cbrt(rha);
  const double rha43 = rha * rha13;
  const double xa = grada / rha43;
  const double xa2 = xa*xa;
  const double asinhxa = asinh(xa);
  const double frac = 1.0 / ( 1.0 + 6.0 * beta * xa * asinhxa );
  const double ga = axa - beta * xa2 * frac;
  // in next line, ex is the energy density, hence rh13
  *ex = rha13 * ga;

  // potential
  const double gpa = ( 6.0*beta*beta*xa2 * ( xa/sqrt(xa2+1.0) - asinhxa )
                     - 2.0*beta*xa ) * frac*frac;
  *vx1 = rha13 * fourthirds * ( ga - xa * gpa );
  *vx2 = - 0.5 * gpa / grada;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::eclyp(double rho, double grad,
  double *ec, double *vc1, double *vc2)
{
  // LYP constants
  const double a = 0.04918;
  const double b = 0.132;
  const double ab36 = a * b / 36.0;
  const double c = 0.2533;
  const double c_third = c / 3.0;
  const double d = 0.349;
  const double d_third = d / 3.0;
  const double cf = 2.87123400018819; // (3/10)*pow(3*pi*pi,2/3)
  const double cfb = cf * b;

  *ec = 0.0;
  *vc1 = 0.0;
  *vc2 = 0.0;

  if ( rho < 1.e-18 ) return;

  // LYP correlation
  // Phys. Rev. B 37, 785 (1988).
  // next lines specialized to the unpolarized case
  const double rh13 = cbrt(rho);
  const double rhm13 = 1.0 / rh13;
  const double rhm43 = rhm13 / rho;
  const double e = exp ( - c * rhm13 );
  const double num = 1.0 + cfb * e;
  const double den = 1.0 + d * rhm13;
  const double deninv = 1.0 / den;
  const double cfrac = num * deninv;

  const double delta = rhm13 * ( c + d * deninv );
  const double ddelta = - (1.0/3.0) * ( c * rhm43
                      + d * rhm13 * rhm13 / ((d+rh13)*(d+rh13)) );
  const double rhm53 = rhm43 * rhm13;
  const double t1 = e * deninv;
  const double t2 = rhm53;
  const double t3 = 6.0 + 14.0 * delta;

  const double g = ab36 * t1 * t2 * t3;

  // ec is the energy density, hence divide the energy by rho
  *ec = - a * cfrac + 0.25 * g * grad * grad / rho;

  // potential
  const double de = c_third * rhm43 * e;
  const double dnum = cfb * de;
  const double dden = - d_third * rhm43;
  const double dfrac = ( dnum * den - dden * num ) * deninv * deninv;

  const double dt1 = de * deninv - e * dden * deninv * deninv;
  const double dt2 = - (5.0/3.0) * rhm53/rho;
  const double dt3 = 14.0 * ddelta;

  const double dg = ab36 * ( dt1 * t2 * t3 + t1 * dt2 * t3 + t1 * t2 * dt3 );

  *vc1 = - a * ( cfrac + rho * dfrac ) + 0.25 * dg * grad * grad;
  *vc2 = -0.5 * g;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::exb88_sp(double rho_up, double rho_dn,
  double grad_up2, double grad_dn2, double grad_up_grad_dn,
  double *ex_up, double *ex_dn, double *vx1_up, double *vx1_dn,
  double *vx2_upup, double *vx2_dndn, double *vx2_updn, double *vx2_dnup)
{
  *ex_up = 0.0;
  *ex_dn = 0.0;
  *vx1_up = 0.0;
  *vx1_dn = 0.0;
  *vx2_upup = 0.0;
  *vx2_updn = 0.0;
  *vx2_dnup = 0.0;
  *vx2_dndn = 0.0;

  if ( rho_up < 1.e-18 && rho_dn < 1.e-18 ) return;

  // Becke exchange constants
  const double fourthirds = 4.0 / 3.0;
  const double beta = 0.0042;
  const double cx = -0.9305257363490999;    // -1.5*pow(3.0/(4*pi),third)

  // Becke's exchange
  // A.D.Becke, Phys.Rev. B38, 3098 (1988)

  double ex1a = 0.0;
  double ex1b = 0.0;
  double vx1a = 0.0;
  double vx1b = 0.0;
  double vx2a = 0.0;
  double vx2b = 0.0;

  if ( rho_up > 1.e-18 )
  {
    const double& rha = rho_up;
    const double rha13 = cbrt(rha);
    const double rha43 = rha * rha13;
    const double grada = sqrt(grad_up2);
    const double xa = grada / rha43;
    const double xa2 = xa*xa;
    const double asinhxa = asinh(xa);
    const double fraca = 1.0 / ( 1.0 + 6.0 * beta * xa * asinhxa );
    const double ga = cx - beta * xa2 * fraca;
    // next line, ex is the energy density, hence rh13
    ex1a = rha13 * ga;
    const double gpa = ( 6.0*beta*beta*xa2 * ( xa/sqrt(xa2+1.0) - asinhxa )
                       - 2.0*beta*xa ) * fraca * fraca;
    vx1a = rha13 * fourthirds * ( ga - xa * gpa );
    vx2a = - gpa / grada;
  }

  if ( rho_dn > 1.e-18 )
  {
    const double& rhb = rho_dn;
    const double rhb13 = cbrt(rhb);
    const double rhb43 = rhb * rhb13;
    const double gradb = sqrt(grad_dn2);
    const double xb = gradb / rhb43;
    const double xb2 = xb*xb;
    const double asinhxb = asinh(xb);
    const double fracb = 1.0 / ( 1.0 + 6.0 * beta * xb * asinhxb );
    const double gb = cx - beta * xb2 * fracb;
    // next line, ex is the energy density, hence rh13
    ex1b = rhb13 * gb;
    const double gpb = ( 6.0*beta*beta*xb2 * ( xb/sqrt(xb2+1.0) - asinhxb )
                       - 2.0*beta*xb ) * fracb * fracb;
    vx1b = rhb13 * fourthirds * ( gb - xb * gpb );
    vx2b = - gpb / gradb;
  }

  *ex_up = ex1a;
  *ex_dn = ex1b;
  *vx1_up = vx1a;
  *vx1_dn = vx1b;
  *vx2_upup = vx2a;
  *vx2_updn = 0.0;
  *vx2_dnup = 0.0;
  *vx2_dndn = vx2b;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::eclyp_sp(double rho_up, double rho_dn,
  double grad_up2, double grad_dn2, double grad_up_grad_dn,
  double *ec_up, double *ec_dn, double *vc1_up, double *vc1_dn,
  double *vc2_upup, double *vc2_dndn, double *vc2_updn, double *vc2_dnup)
{
  *ec_up = 0.0;
  *ec_dn = 0.0;
  *vc1_up = 0.0;
  *vc1_dn = 0.0;
  *vc2_upup = 0.0;
  *vc2_updn = 0.0;
  *vc2_dnup = 0.0;
  *vc2_dndn = 0.0;

  if ( rho_up < 1.e-18 && rho_dn < 1.e-18 ) return;
  if ( rho_up + rho_dn < 1.e-18 ) return;
  if ( rho_up < 0.0 ) rho_up = 0.0;
  if ( rho_dn < 0.0 ) rho_dn = 0.0;

  // LYP constants
  const double a = 0.04918;
  const double b = 0.132;
  const double c = 0.2533;
  const double d = 0.349;
  const double cf = 2.87123400018819; // (3/10)*pow(3*pi*pi,2/3)
  const double ninth = 1.0 / 9.0;
  const double two113 = pow(2.0,11.0/3.0);
  const double fourthirds = 4.0 / 3.0;

  const double& rha = rho_up;
  const double& rhb = rho_dn;

  const double rha13 = cbrt(rha);
  const double rhb13 = cbrt(rhb);
  const double rha43 = rha * rha13;
  const double rhb43 = rhb * rhb13;
  const double rha83 = rha43 * rha43;
  const double rhb83 = rhb43 * rhb43;
  const double rha113 = rha83 * rha;
  const double rhb113 = rhb83 * rhb;

  // LYP correlation
  // Phys. Rev. B 37, 785 (1988).

  const double rho = rha + rhb;
  const double rhoinv = 1.0/rho;
  const double rhab = rha * rhb;
  const double rhabrhm = rhab * rhoinv;
  const double rh13 = cbrt(rho);
  const double rhm13 = 1.0 / rh13;
  const double rhm43 = rhm13 / rho;
  const double rhm113 = rhm43 * rhm43 * rhm43 * rh13;
  const double rhm143 = rhm113 * rhoinv;
  const double e = exp ( - c * rhm13 );
  const double den = 1.0 + d * rhm13;
  const double deninv = 1.0 / den;

  const double w = e * deninv * rhm113;
  const double dw = (1.0/3.0)*(c*d+(c-10.0*d)*rh13-11.0*rh13*rh13)*e*rhm143/
                    ((d+rh13)*(d+rh13));
  const double abw = a * b * w;
  const double f1 = -4.0 * a * deninv * rhabrhm;
  const double f2 = - two113 * cf * abw * rha * rhb  * ( rha83 + rhb83 );

  const double delta = rhm13 * ( c + d * deninv );
  const double ddelta = - (1.0/3.0) * ( c * rhm43
                      + d * rhm13 * rhm13 / ((d+rh13)*(d+rh13)) );
  const double taa = 1.0 - 3.0 * delta + ( 11.0 - delta ) * rha * rhoinv;
  const double tbb = 1.0 - 3.0 * delta + ( 11.0 - delta ) * rhb * rhoinv;

  const double dtaa_drha = -3.0*ddelta - ddelta*rha*rhoinv +
                           (11.0-delta)*rhoinv*(1.0-rha*rhoinv);
  const double dtaa_drhb = -3.0*ddelta - ddelta*rha*rhoinv -
                           (11.0-delta)*rhoinv*rha*rhoinv;
  const double dtbb_drha = -3.0*ddelta - ddelta*rhb*rhoinv -
                           (11.0-delta)*rhoinv*rhb*rhoinv;
  const double dtbb_drhb = -3.0*ddelta - ddelta*rhb*rhoinv +
                           (11.0-delta)*rhoinv*(1.0-rhb*rhoinv);
  const double gaa = - abw * ( rhab * ninth * taa - rhb * rhb );
  const double gbb = - abw * ( rhab * ninth * tbb - rha * rha );
  const double gab = - abw * ( rhab * ninth * ( 47.0 - 7.0 * delta )
                               - fourthirds * rho * rho );

  // next line, ec is the energy density, hence divide the energy by rho
  const double ec1a = ( f1 + f2 + gaa * grad_up2
                        + gab * grad_up_grad_dn
                        + gbb * grad_dn2 ) / rho;
  const double ec1b = ( f1 + f2 + gaa * grad_up2
                        + gab * grad_up_grad_dn
                        + gbb * grad_dn2 ) / rho;

  const double A = -two113*cf*a*b;
  const double df1_drha = -fourthirds*a*d*deninv*deninv*rhm43*rhabrhm
                        - 4.0*a*deninv*rhoinv*(rhb-rhabrhm);
  const double df2_drha = A*dw*(rha113*rhb+rha*rhb113)
                        + A*w*((11.0/3.0)*rha83*rhb+rhb113);

  const double df1_drhb = -fourthirds*a*d*deninv*deninv*rhm43*rhabrhm
                        - 4.0*a*deninv*rhoinv*(rha-rhabrhm);
  const double df2_drhb = A*dw*(rha113*rhb+rha*rhb113)
                        + A*w*(rha113+rha*(11.0/3.0)*rhb83);

  const double dgaa_drha = -a*b*dw*(rhab*ninth*taa-rhb*rhb)
                         - abw*(rhb*ninth*taa+rhab*ninth*dtaa_drha);
  const double dgaa_drhb = -a*b*dw*(rhab*ninth*taa-rhb*rhb)
                         - abw*(rha*ninth*taa+rhab*ninth*dtaa_drhb-2.0*rhb);

  const double dgbb_drha = -a*b*dw*(rhab*ninth*tbb-rha*rha)
                         - abw*(rhb*ninth*tbb+rhab*ninth*dtbb_drha-2.0*rha);
  const double dgbb_drhb = -a*b*dw*(rhab*ninth*tbb-rha*rha)
                         - abw*(rha*ninth*tbb+rhab*ninth*dtbb_drhb);

  const double dgab_drha = -a*b*dw*(rhab*ninth*(47.0-7.0*delta)
                                    -fourthirds*rho*rho)
                           -abw*(rhb*ninth*(47.0-7.0*delta)
                                 -rhab*ninth*7.0*ddelta-2.0*fourthirds*rho);
  const double dgab_drhb = -a*b*dw*(rhab*ninth*(47.0-7.0*delta)
                                    -fourthirds*rho*rho)
                           -abw*(rha*ninth*(47.0-7.0*delta)
                                 -rhab*ninth*7.0*ddelta-2.0*fourthirds*rho);

  const double vc1a = df1_drha + df2_drha
                    + dgaa_drha * grad_up2
                    + dgab_drha * grad_up_grad_dn
                    + dgbb_drha * grad_dn2;
  const double vc1b = df1_drhb + df2_drhb
                    + dgaa_drhb * grad_up2
                    + dgab_drhb * grad_up_grad_dn
                    + dgbb_drhb * grad_dn2;

  *ec_up = ec1a;
  *ec_dn = ec1b;
  *vc1_up = vc1a;
  *vc1_dn = vc1b;
  *vc2_upup = - 2.0 * gaa;
  *vc2_updn = - gab;
  *vc2_dnup = - gab;
  *vc2_dndn = - 2.0 * gbb;
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::exvwn(const double rh, double &ex, double &vx)
{
  x_unpolarized(rh,ex,vx);
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::ecvwn(const double rh, double &ec, double &vc)
{
  c_unpolarized(rh,ec,vc);
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::exvwn_sp(double roe_up, double roe_dn,
  double &ex, double &vx_up, double &vx_dn)
{
  const double fz_prefac = 1.0 / ( cbrt(2.0)*2.0 - 2.0 );
  const double dfz_prefac = (4.0/3.0) * fz_prefac;
  ex = 0.0;
  vx_up = 0.0;
  vx_dn = 0.0;

  if ( roe_up < 0.0 ) roe_up = 0.0;
  if ( roe_dn < 0.0 ) roe_dn = 0.0;
  const double roe = roe_up + roe_dn;

  if ( roe > 0.0 )
  {
    const double zeta = ( roe_up - roe_dn ) / roe;
    const double zp1 = 1.0 + zeta;
    const double zm1 = 1.0 - zeta;
    const double zp1_13 = cbrt(zp1);
    const double zm1_13 = cbrt(zm1);
    const double fz = fz_prefac * ( zp1_13 * zp1 + zm1_13 * zm1 - 2.0 );
    const double dfz = dfz_prefac * ( zp1_13 - zm1_13 );

    double ex_u,vx_u;
    double ex_p,vx_p;
    double a,da;
    x_unpolarized(roe,ex_u,vx_u);
    x_polarized(roe,ex_p,vx_p);
    alpha_c(roe,a,da);

    const double ex_pu = ex_p - ex_u;
    ex = ex_u + fz * ex_pu;
    double vx = vx_u + fz * ( vx_p - vx_u );
    vx_up = vx + ex_pu * ( 1.0 - zeta ) * dfz;
    vx_dn = vx - ex_pu * ( 1.0 + zeta ) * dfz;
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::ecvwn_sp(double roe_up, double roe_dn,
  double &ec, double &vc_up, double &vc_dn)
{
  const double fz_prefac = 1.0 / ( cbrt(2.0)*2.0 - 2.0 );
  const double dfz_prefac = (4.0/3.0) * fz_prefac;
  ec = 0.0;
  vc_up = 0.0;
  vc_dn = 0.0;

  if ( roe_up < 0.0 ) roe_up = 0.0;
  if ( roe_dn < 0.0 ) roe_dn = 0.0;
  const double roe = roe_up + roe_dn;

  if ( roe > 0.0 )
  {
    const double zeta = ( roe_up - roe_dn ) / roe;
    const double zp1 = 1.0 + zeta;
    const double zm1 = 1.0 - zeta;
    const double zp1_13 = cbrt(zp1);
    const double zm1_13 = cbrt(zm1);
    const double fz = fz_prefac * ( zp1_13 * zp1 + zm1_13 * zm1 - 2.0 );
    const double dfz = dfz_prefac * ( zp1_13 - zm1_13 );

    double ec_u,vc_u;
    double ec_p,vc_p;
    double a,da;

    ecvwn(roe,ec_u,vc_u);
    c_polarized(roe,ec_p,vc_p);
    alpha_c(roe,a,da);

    const double zeta3 = zeta*zeta*zeta;
    const double zeta4 = zeta3*zeta;
    a *= (9.0/8.0)*fz_prefac;
    da *= (9.0/8.0)*fz_prefac;
    double ec_pu = ec_p - ec_u - a;

    ec = ec_u + a * fz + ec_pu * fz * zeta4;

    const double vc1 = vc_u + da * fz + ( vc_p - vc_u - da ) * fz * zeta4;
    const double vc2 = a * dfz + ec_pu * zeta3 * ( 4.0*fz + zeta*dfz );

    vc_up = vc1 + ( 1.0 - zeta ) * vc2;
    vc_dn = vc1 - ( 1.0 + zeta ) * vc2;
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::x_unpolarized(const double rh, double &ex, double &vx)
{
  // unpolarized exchange  energy and potential
  // c1 is (3.D0/(4.D0*pi))**third
  const double c1 = 0.6203504908994001;
  // alpha = (4/(9*pi))**third = 0.521061761198
  // c2 = -(3/(4*pi)) / alpha = -0.458165293283
  // c3 = (4/3) * c2 = -0.610887057711
  const double c3 = -0.610887057711;

  ex = 0.0;
  vx = 0.0;

  if ( rh > 0.0 )
  {
    double ro13 = cbrt(rh);
    double rs = c1 / ro13;

    // exchange in Hartree units
    vx = c3 / rs;
    ex = 0.75 * vx;
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::c_unpolarized(const double rh, double &ec, double &vc)
{
  // unpolarized xc energy and potential
  // c1 is (3.D0/(4.D0*pi))**third
  const double c1 = 0.6203504908994001;

  ec = 0.0;
  vc = 0.0;

  if ( rh > 0.0 )
  {
    const double A = 0.0310907;
    const double x0 = -0.10498;
    const double b = 3.72744;
    const double c = 12.9352;
    const double Q = sqrt( 4.0 * c - b * b );
    const double fac1 = 2.0 * b / Q;
    const double fac2 = b * x0 / ( x0 * x0 + b * x0 + c );
    const double fac3 = 2.0 * ( 2.0 * x0 + b ) / Q;

    double ro13 = cbrt(rh);
    double rs = c1 / ro13;

    double sqrtrs = sqrt(rs);
    double X = rs + b * sqrtrs + c;
    double fatan = atan( Q / ( 2.0 * sqrtrs + b ) );

    ec = A * ( log( rs / X ) + fac1 * fatan -
               fac2 * ( log( (sqrtrs-x0)*(sqrtrs-x0) / X ) +
                        fac3 * fatan ));

    double t = sqrtrs - x0;
    vc = ec + ( A / 3.0 ) * ( b * sqrtrs * x0 - c * t ) / ( X * t );
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::x_polarized(const double rh, double &ex, double &vx)
{
  // polarized exchange energy and potential
  // c1 is (3.D0/(4.D0*pi))**third
  const double c1 = 0.6203504908994001;
  // alpha = (4/(9*pi))**third = 0.521061761198
  // c2 = -(3/(4*pi)) / alpha = -0.458165293283
  // c3 = (4/3) * c2 = -0.610887057711
  // c4 = 2**(1/3) * c3
  const double c4 = -0.769669463118;

  ex = 0.0;
  vx = 0.0;

  if ( rh > 0.0 )
  {
    double ro13 = cbrt(rh);
    double rs = c1 / ro13;

    // Next line : exchange part in Hartree units
    vx = c4 / rs;
    ex = 0.75 * vx;
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::c_polarized(const double rh, double &ec, double &vc)
{
  // polarized correlation energy and potential
  // c1 is (3.D0/(4.D0*pi))**third
  const double c1 = 0.6203504908994001;

  ec = 0.0;
  vc = 0.0;

  if ( rh > 0.0 )
  {
    const double A = 0.01554535;
    const double x0 = -0.32500;
    const double b = 7.06042;
    const double c = 18.0578;
    const double Q = sqrt( 4.0 * c - b * b );
    const double fac1 = 2.0 * b / Q;
    const double fac2 = b * x0 / ( x0 * x0 + b * x0 + c );
    const double fac3 = 2.0 * ( 2.0 * x0 + b ) / Q;

    double ro13 = cbrt(rh);
    double rs = c1 / ro13;

    double sqrtrs = sqrt(rs);
    double X = rs + b * sqrtrs + c;
    double fatan = atan( Q / ( 2.0 * sqrtrs + b ) );

    ec = A * ( log( rs / X ) + fac1 * fatan -
               fac2 * ( log( (sqrtrs-x0)*(sqrtrs-x0) / X ) +
                        fac3 * fatan ));

    double t = sqrtrs - x0;
    vc = ec + ( A / 3.0 ) * ( b * sqrtrs * x0 - c * t ) / ( X * t );
  }
}

////////////////////////////////////////////////////////////////////////////////
void B3LYPFunctional::alpha_c(const double rh, double &a, double &da)
{
  // VWN spin stiffness alpha_c(rh)
  // a = spin stiffness
  // da = d(rh * a)/drh
  // c1 is (3.D0/(4.D0*pi))**third
  const double c1 = 0.6203504908994001;

  a = 0.0;
  da = 0.0;

  if ( rh > 0.0 )
  {
    const double A = 1.0/(6.0*M_PI*M_PI);
    const double x0 = -0.0047584;
    const double b = 1.13107;
    const double c = 13.0045;
    const double Q = sqrt( 4.0 * c - b * b );
    const double fac1 = 2.0 * b / Q;
    const double fac2 = b * x0 / ( x0 * x0 + b * x0 + c );
    const double fac3 = 2.0 * ( 2.0 * x0 + b ) / Q;

    double ro13 = cbrt(rh);
    double rs = c1 / ro13;

    double sqrtrs = sqrt(rs);
    double X = rs + b * sqrtrs + c;
    double fatan = atan( Q / ( 2.0 * sqrtrs + b ) );

    a = A * ( log( rs / X ) + fac1 * fatan -
              fac2 * ( log( (sqrtrs-x0)*(sqrtrs-x0) / X ) +
                       fac3 * fatan ));

    double t = sqrtrs - x0;
    da = a + ( A / 3.0 ) * ( b * sqrtrs * x0 - c * t ) / ( X * t );
  }
}
