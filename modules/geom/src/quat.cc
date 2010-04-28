//------------------------------------------------------------------------------
// This file is part of the OpenStructure project <www.openstructure.org>
//
// Copyright (C) 2008-2010 by the OpenStructure authors
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 3.0 of the License, or (at your option)
// any later version.
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//------------------------------------------------------------------------------
#include <iostream>
#include <limits>
#include <cassert>
#include <cmath>
#include <algorithm>

#include "quat.hh"

#include "mat3.hh"
#include "vec3.hh"
#include "vecmat3_op.hh"

namespace geom {

namespace {

/*
  helper functions, adapted from the boost quaternion example
  (C) Copyright Hubert Holin 2001.
*/

Vec3 find_invariant_vector(const Mat3& rot)
{
  Real b11 = rot(0,0) - 1.0;
  Real b12 = rot(0,1);
  Real b13 = rot(0,2);
  Real b21 = rot(1,0);
  Real b22 = rot(1,1) - 1.0;
  Real b23 = rot(1,2);
  Real b31 = rot(2,0);
  Real b32 = rot(2,1);
  Real b33 = rot(2,2) -1.0;
        
  Real minors[9] = {
    b11*b22-b12*b21,
    b11*b23-b13*b21,
    b12*b23-b13*b22,
    b11*b32-b12*b31,
    b11*b33-b13*b31,
    b12*b33-b13*b32,
    b21*b32-b22*b31,
    b21*b33-b23*b31,
    b22*b33-b23*b32
  };
        
  Real* where = ::std::max_element(minors, minors+9);
  
  Real det = *where;

  // assume this never happens
  // if (det <= numeric_limits<Real>::epsilon()) 

  Vec3 result;
        
  switch (where-minors) {
  case 0:
    result[2] = 1.0;
    result[0] = (-b13*b22+b12*b23)/det;
    result[1] = (-b11*b23+b13*b21)/det;
    break;
  case 1:
    result[1] = 1.0;
    result[0] = (-b12*b23+b13*b22)/det;
    result[2] = (-b11*b22+b12*b21)/det;
    break;
  case 2:
    result[0] = 1.0;
    result[1] = (-b11*b23+b13*b21)/det;
    result[2] = (-b12*b21+b11*b22)/det;
    break;
  case 3:
    result[2] = 1.0;
    result[0] = (-b13*b32+b12*b33)/det;
    result[1] = (-b11*b33+b13*b31)/det;
    break;
  case 4:
    result[1] = 1.0;
    result[0] = (-b12*b33+b13*b32)/det;
    result[2] = (-b11*b32+b12*b31)/det;
    break;
  case 5:
    result[0] = 1.0;
    result[1] = (-b11*b33+b13*b31)/det;
    result[2] = (-b12*b31+b11*b32)/det;
    break;
  case 6:
    result[2] = 1.0;
    result[0] = (-b23*b32+b22*b33)/det;
    result[1] = (-b21*b33+b23*b31)/det;
    break;
  case 7:
    result[1] = 1.0;
    result[0] = (-b22*b33+b23*b32)/det;
    result[2] = (-b21*b32+b22*b31)/det;
    break;
  case 8:
    result[0] = 1.0;
    result[1] = (-b21*b33+b23*b31)/det;
    result[2] = (-b22*b31+b21*b32)/det;
    break;
  default:
    assert(false); // should never get here
  }
  
  result = Normalize(result);
  return result;
}
    
    
Vec3 find_orthogonal_vector(const Vec3& xyz)
{
  Real vecnormsqr = Length2(xyz); 
  
  // assume this never happens
  //if (vecnormsqr <= numeric_limits<Real>::epsilon())
  
  Real components[3] = {
    fabs(xyz[0]),
    fabs(xyz[1]),
    fabs(xyz[2])
  };
        
  Real*  where = ::std::min_element(components, components+3);

  Vec3 result;
        
  switch (where-components) {
  case 0:
    if (*where <= std::numeric_limits<Real>::epsilon()) {
      result=Vec3(1.0,0.0,0.0);
    } else {
      Real lambda = -xyz[0]/vecnormsqr;
      result = Vec3(1.0 + lambda*xyz[0],lambda*xyz[1],lambda*xyz[2]);
    }
    break;
  case 1:
    if (*where <= std::numeric_limits<Real>::epsilon()) {
      result=Vec3(0.0,1.0,0.0);
    } else {
      Real lambda = -xyz[1]/vecnormsqr;
      result = Vec3(lambda*xyz[0],1.0+lambda*xyz[1],lambda*xyz[2]);
    }
    break;                
  case 2:
    if (*where <= std::numeric_limits<Real>::epsilon()) {
      result=Vec3(0.0,0.0,1.0);
    } else {
      Real lambda = -xyz[2]/vecnormsqr;
      result = Vec3(lambda*xyz[0],lambda*xyz[1],1.0+lambda*xyz[2]);
    }
    break;                
  default:
    assert(false); // should never get here
  }
  result = Normalize(result);
  return result;
}
    
Vec3 find_vector_for_BOD(const Vec3& xyz, const Vec3& uvw)
{
  Vec3 nrvo(+xyz[1]*uvw[2]-xyz[2]*uvw[1],
      -xyz[0]*uvw[2]+xyz[2]*uvw[0],
      +xyz[0]*uvw[1]-xyz[1]*uvw[0]);
  return nrvo;
}

Quat extract_from_rotmat(const Mat3& rot)
{
  if ((std::abs(rot(0,0) - Real(1.0)) <= std::numeric_limits<Real>::epsilon()) &&
      (std::abs(rot(1,1) - Real(1.0)) <= std::numeric_limits<Real>::epsilon()) &&
      (std::abs(rot(2,2) - Real(1.0)) <= std::numeric_limits<Real>::epsilon())) {
    return Quat(1.0,0.0,0.0,0.0);
  }

  Real cos_theta = (rot(0,0)+rot(1,1)+rot(2,2)-Real(1.0))*Real(0.5);
  Real stuff = (cos_theta+Real(1.0))*Real(0.5);
  if(stuff<Real(0.0) || stuff>Real(1.0)) {
    return Quat(1.0,0.0,0.0,0.0);
  }
  Real cos_theta_sur_2 = sqrt(stuff);
  Real sin_theta_sur_2 = sqrt(Real(1.0)-stuff);

  Vec3 xyz = find_invariant_vector(rot);
  Vec3 uvw = find_orthogonal_vector(xyz);
  Vec3 rst = find_vector_for_BOD(xyz,uvw);
  
  Real ru = rot(0,0)*uvw[0]+rot(0,1)*uvw[1]+rot(0,2)*uvw[2];
  Real rv = rot(1,0)*uvw[0]+rot(1,1)*uvw[1]+rot(1,2)*uvw[2];
  Real rw = rot(2,0)*uvw[0]+rot(2,1)*uvw[1]+rot(2,2)*uvw[2];
  
  Real angle_sign_determinator = rst[0]*ru+rst[1]*rv+rst[2]*rw;

  Quat q;
  
  if (angle_sign_determinator > +std::numeric_limits<Real>::epsilon()) {
    q = Quat(cos_theta_sur_2, +xyz[0]*sin_theta_sur_2, +xyz[1]*sin_theta_sur_2, +xyz[2]*sin_theta_sur_2);
  } else if (angle_sign_determinator < -std::numeric_limits<Real>::epsilon()) {
    q = Quat(cos_theta_sur_2, -xyz[0]*sin_theta_sur_2, -xyz[1]*sin_theta_sur_2, -xyz[2]*sin_theta_sur_2);
  } else {
    Real desambiguator = uvw[0]*ru+uvw[1]*rv+uvw[2]*rw;
    if(desambiguator >= 1.0) {
      q = Quat(0.0, +xyz[0], +xyz[1], +xyz[2]);
    } else {
      q = Quat(0.0, -xyz[0], -xyz[1], -xyz[2]);
    }
  }
  
  return q;
}

} // anon ns


Quat::Quat():
  w(1.0), x(0.0),y(0.0),z(0.0)
{}

Quat::Quat(Real w_, Real x_, Real y_, Real z_):
  w(w_), x(x_), y(y_), z(z_)
{}

Quat::Quat(Real angle, const geom::Vec3& axis)
{
  Vec3 na=Normalize(axis);
  w=cos(0.5*angle);
  x=na[0]*sin(0.5*angle);
  y=na[1]*sin(0.5*angle);
  z=na[2]*sin(0.5*angle);
}

  // initialize from a rotation matrix
Quat::Quat(const Mat3& rotmat)
{
  (*this) = extract_from_rotmat(rotmat);
}

 // return 3x3 rotation matrix
Mat3 Quat::ToRotationMatrix() const
{
  Real ww = w*w;
  Real wx = w*x;
  Real wy = w*y;
  Real wz = w*z;
  Real xx = x*x;
  Real xy = x*y;
  Real xz = x*z;
  Real yy = y*y;
  Real yz = y*z;
  Real zz = z*z;

  Real norm = ww+xx+yy+zz;
  if(norm<1e-30) {
    return Mat3::Identity();
  }

  Real inorm=1.0/norm;

  Mat3 nrvo=inorm*Mat3((ww+xx-yy-zz), 2.0*(-wz+xy), 2.0*(wy+xz),
           2.0*(wz+xy), (ww-xx+yy-zz), 2.0*(-wx+yz),
           2.0*(-wy+xz), 2.0*(wx+yz),(ww-xx-yy+zz));
  return nrvo;
}

Vec3 Quat::GetAxis() const
{
  Vec3 nrvo=Normalize(Vec3(x,y,z));
  return nrvo;
}

Real Quat::GetAngle() const
{
  Real ww = std::acos(w/Length(Vec3(x,y,z)));
  return ww;
}

Quat Quat::operator-()
{
  Quat nrvo(-w,-x,-y,-z);
  return nrvo;
}

Quat& Quat::operator+=(const Quat& q)
{
  w+=q.w; x+=q.x; y+=q.y; z+=q.z;
  return *this;
}

Quat& Quat::operator-=(const Quat& q)
{
  w-=q.w; x-=q.x; y-=q.y; z-=q.z;
  return *this;
}

Quat& Quat::operator*=(Real s)
{
  w*=s; x*=s; y*=s; z*=s;
  return *this;
}

Quat& Quat::operator*=(const Quat& q)
{
  w = this->w*q.w - this->x*q.x - this->y*q.y - this->z*q.z;
  x = this->w*q.x + this->x*q.w + this->y*q.z - this->z*q.y;
  y = this->w*q.y + this->y*q.w - this->x*q.z + this->z*q.x;
  z = this->w*q.z + this->z*q.w + this->x*q.y - this->y*q.x;
  return *this;
}

Quat& Quat::operator/=(Real s)
{
  w/=s; x/=s; y/=s; z/=s;
  return *this;
}

Quat& Quat::operator/=(const Quat& q)
{
  Real idenom = 1.0/(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
                
  Real w_ = (+w*q.w+x*q.x+y*q.y+z*q.z)*idenom;
  Real x_ = (-w*q.x+x*q.w-y*q.z+z*q.y)*idenom;
  Real y_ = (-w*q.y+x*q.z+y*q.w-z*q.x)*idenom;
  Real z_ = (-w*q.z-x*q.y+y*q.x+z*q.w)*idenom;
  w=w_; x=x_; y=y_; z=z_;
  return(*this);
}

bool Quat::operator==(const Quat& q) const
{
  return (w==q.w && x==q.x && y==q.y && z==q.z);
}

Real Dot(const Quat& q0, const Quat& q1)
{
  return q0.w*q1.w+q0.x*q1.x+q0.y*q1.y+q0.z*q1.z;
}

Quat Conjugate(const Quat& q) 
{
  return Quat(q.w,-q.x,-q.y,-q.z);
}

namespace {


/*
  for a unit quaternion defined as (cos[theta],sin[theta] * [x,y,z])
  the log is given by (0,theta * [x,y,z])
*/
Quat log(const Quat& q) 
{
  Quat nrvo;
  Real sin_theta = std::sqrt(q.x*q.x+q.y*q.y+q.z*q.z);
  if(std::fabs(sin_theta)<1e-30) {
    /*
      for a theta of zero, cos(theta)=1, the log if which is zero,
      and hence an all zero quat is the result, but this may lead to
      trouble...
    */
    nrvo = Quat(0.0,0.0,0.0,0.0);
  } else {

    Real theta = std::atan2(sin_theta,q.w);
    Real f = theta/sin_theta;
    // assume cos(theta) == w, since we are supposed to have a unit quaternion
    nrvo = Quat(0.0,q.x*f,q.y*f,q.z*f);

  }
  return nrvo;
}

Quat exp(const Quat& q)
{
  Quat nrvo;
  Real theta = std::sqrt(q.x*q.x+q.y*q.y+q.z*q.z);
  if(std::fabs(theta)<1e-30) {
    /*
      see log special case above; this will now
      return a 'proper' quaternion
    */
    nrvo = Quat(1.0,0.0,0.0,0.0);
  } else {
    Real f = sin(theta)/theta;
    nrvo = Quat(cos(theta),f*q.x,f*q.y,f*q.z);
  }
  return nrvo;
}

}

/*
  qt = q0 ( q0* q1) ^t
  qt  = q0 Exp[t Log[ Conj[q0] q1]]
*/
Quat Slerp(const Quat& q0, const Quat& q1, Real t)
{
#if 0
  // broken
  Quat nrvo = q0*exp(t*log(conj(q0)*q1));
#else
  Quat nrvo;
  bool shortest=false;
  Real o,so,a,b;
  bool neg_q1 = false;

  Real ca = Dot(q0,q1);
  if (shortest && ca<0) {
    ca = -ca;
    neg_q1 = true;
  }
  o = acos(ca);
  so = sin(o);

  if (fabs(so)<1e-30) {
    nrvo=q0;
  } else {

    a = sin(o*(1.0-t)) / so;
    b = sin(o*t) / so;
    if (neg_q1)
      nrvo = q0*a - q1*b;
    else
      nrvo = q0*a + q1*b;
  }
#endif
  return nrvo;
}

Vec3 Quat::Rotate(const Vec3& vec) const {
  Quat tmp(0.0, vec[0], vec[1], vec[2]);
  Quat conj=Conjugate(*this);
  Quat res=*this*tmp*conj;
  return Vec3(res.x, res.y, res.z);
}

Quat Grassmann(const Quat& lhs, const Quat& rhs)
{
  return Quat(lhs.GetAngle()*rhs.GetAngle()-
              lhs.GetAxis().GetX()*rhs.GetAxis().GetX()-
              lhs.GetAxis().GetY()*rhs.GetAxis().GetY()-
              lhs.GetAxis().GetZ()*rhs.GetAxis().GetZ(),
                    lhs.GetAngle()*rhs.GetAxis().GetX()+
                    lhs.GetAxis().GetX()*rhs.GetAngle()+
                    lhs.GetAxis().GetY()*rhs.GetAxis().GetZ()-
                    lhs.GetAxis().GetZ()*rhs.GetAxis().GetY(),
               lhs.GetAngle()*rhs.GetAxis().GetY()-
               lhs.GetAxis().GetX()*rhs.GetAxis().GetZ()+
               lhs.GetAxis().GetY()*rhs.GetAngle()+
               lhs.GetAxis().GetZ()*rhs.GetAxis().GetX(),
                    lhs.GetAngle()*rhs.GetAxis().GetZ()+
                    lhs.GetAxis().GetX()*rhs.GetAxis().GetY()-
                    lhs.GetAxis().GetY()*rhs.GetAxis().GetX()+
                    lhs.GetAxis().GetZ()*rhs.GetAngle());
}

std::ostream& operator<<(std::ostream& str, const Quat& q)
{
  str << "[" << q.w << "," << q.x << "," << q.y << "," << q.z << "]";
  return str;
}

Quat Normalize (const Quat& q)
{
  Real norm_fact = std::sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
  return Quat(q.w/norm_fact,q.x/norm_fact,q.y/norm_fact,q.z/norm_fact);
}


} // ns
