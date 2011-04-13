//------------------------------------------------------------------------------
// This file is part of the OpenStructure project <www.openstructure.org>
//
// Copyright (C) 2008-2011 by the OpenStructure authors
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

/*
 * Author Niklaus Johner
 */
#include <stdexcept>
//#include <boost/bind.hpp>
#include <ost/base.hh>
#include <ost/geom/vec3.hh>
//#include <ost/mol/alg/svd_superpose.hh>
#include <ost/base.hh>
#include <ost/geom/geom.hh>
#include <ost/mol/entity_view.hh>
#include <ost/mol/coord_group.hh>
#include "trajectory_analysis.hh"


namespace ost { namespace mol { namespace alg {

geom::Vec3List ExtractAtomPosition(const CoordGroupHandle& traj, const AtomHandle& a1, unsigned int stride)
//This function extracts the position of an atom returns it as a vector of geom::Vec3
//Doesn't work in python, because it cannot create the vector of geom::Vec3
{
  traj.CheckValidity();
  geom::Vec3List pos;
  pos.reserve(ceil(traj.GetFrameCount()/float(stride)));
  int i1=a1.GetIndex();
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    pos.push_back(frame->GetAtomPosition(i1));
  }
  return pos;
}

std::vector<Real> ExtractDistance(const CoordGroupHandle& traj, const AtomHandle& a1, const AtomHandle& a2, 
                                  unsigned int stride)
//This function extracts the distance between two atoms from a trajectory and returns it as a vector
{
  traj.CheckValidity();
  std::vector<Real> dist;
  dist.reserve(ceil(traj.GetFrameCount()/float(stride)));
  int i1=a1.GetIndex();
  int i2=a2.GetIndex();
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    dist.push_back((*frame).GetDistance(i1,i2));
  }
  return dist;
} 

std::vector<Real> ExtractAngle(const CoordGroupHandle& traj, const AtomHandle& a1, const AtomHandle& a2, 
                               const AtomHandle& a3, unsigned int stride)
//This function extracts the angle between three atoms from a trajectory and returns it as a vector
{
  traj.CheckValidity();
  std::vector<Real> ang;
  ang.reserve(ceil(traj.GetFrameCount()/float(stride)));
  int i1=a1.GetIndex(),i2=a2.GetIndex(),i3=a3.GetIndex();
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    ang.push_back((*frame).GetAngle(i1,i2,i3));
  }
  return ang;
}

std::vector<Real> ExtractDihedral(const CoordGroupHandle& traj, const AtomHandle& a1, const AtomHandle& a2, 
                                  const AtomHandle& a3, const AtomHandle& a4, unsigned int stride)
//This function extracts the dihedral angle between four atoms from a trajectory and returns it as a vector
{
  traj.CheckValidity();
  std::vector<Real> ang;
  ang.reserve(ceil(traj.GetFrameCount()/float(stride)));
  int i1=a1.GetIndex(),i2=a2.GetIndex(),i3=a3.GetIndex(),i4=a4.GetIndex();
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    ang.push_back((*frame).GetDihedralAngle(i1,i2,i3,i4));
  }
  return ang;
}

geom::Vec3List ExtractCMPosition(const CoordGroupHandle& traj, const EntityView& Sele,unsigned int stride)
//This function extracts the position of the CM of two selection (entity views) from a trajectory 
//and returns it as a vector. 
  {
  traj.CheckValidity();
  geom::Vec3List pos;
  pos.reserve(ceil(traj.GetFrameCount()/float(stride)));
  std::vector<unsigned long> indices;
  std::vector<Real> masses;
  GetIndicesAndMasses(Sele, indices, masses);
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    pos.push_back(frame->GetCMPosition(indices,masses));
  }
  return pos;
}

std::vector<Real> ExtractCMDistance(const CoordGroupHandle& traj, const EntityView& Sele1,
                                    const EntityView& Sele2, unsigned int stride)
//This function extracts the distance between the CM of two selection (entity views) from a trajectory 
//and returns it as a vector. 
  {
  traj.CheckValidity();
  std::vector<Real> dist;
  dist.reserve(ceil(traj.GetFrameCount()/float(stride)));
  std::vector<unsigned long> indices1,indices2;
  std::vector<Real> masses1,masses2;
  GetIndicesAndMasses(Sele1, indices1, masses1);
  GetIndicesAndMasses(Sele2, indices2, masses2);
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    dist.push_back((*frame).GetCMDistance(indices1,masses1,indices2,masses2));
  }
  return dist;
}

std::vector<Real> ExtractRMSD(const CoordGroupHandle& traj, const EntityView& ReferenceView,
                                    const EntityView& SeleView, unsigned int stride)
// This function extracts the rmsd between two entity views and returns it as a vector
// The views don't have to be from the same entity
// If you want to compare to frame i of the trajectory t, first use t.CopyFrame(i) for example:
// eh=io.LoadPDB(...),t=io.LoadCHARMMTraj(eh,...);Sele=eh.Select(...);t.CopyFrame(0);mol.alg.ExtractRMSD(t,Sele,Sele)
  {
  traj.CheckValidity();
  int count_ref=ReferenceView.GetAtomCount();
  int count_sele=SeleView.GetAtomCount();
  if (count_ref!=count_sele){
    throw std::runtime_error("atom counts of the two views are not equal");
  }  
  std::vector<Real> rmsd;
  rmsd.reserve(ceil(traj.GetFrameCount()/float(stride)));
  std::vector<unsigned long> sele_indices;
  std::vector<geom::Vec3> ref_pos;
  GetIndices(ReferenceView, sele_indices);
  GetPositions(SeleView, ref_pos);
  for (size_t i=0; i<traj.GetFrameCount(); i+=stride) {
    CoordFramePtr frame=traj.GetFrame(i);
    rmsd.push_back((*frame).GetRMSD(ref_pos,sele_indices));
  }
  return rmsd;
}
  
}}} //ns
