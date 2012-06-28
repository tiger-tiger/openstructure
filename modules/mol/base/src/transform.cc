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

#include <ost/config.hh>
#if(OST_INFO_ENABLED)
#include <ost/info/info.hh>
#include <ost/info/geom_info_conversion.hh>
#endif
#include <ost/log.hh>
#include "transform.hh"

namespace ost { 

using namespace geom;

namespace mol {

Transform::Transform():
  rot_(Mat3(1,0,0, 0,1,0, 0,0,1)),
  trans_(Vec3(0,0,0)),
  cen_(Vec3(0,0,0)),
  tm_(),
  ttm_()
{
  update_tm();
}
    
void Transform::SetMatrix(const Mat4& m)
{
  tm_=m;
  ttm_ = Transpose(tm_);
  update_components();
}

void Transform::SetTrans(const Vec3& t) 
{
  trans_=t;
  update_tm();
}
    
Vec3 Transform::GetTrans() const 
{
  return trans_;
}
    
void Transform::SetCenter(const Vec3& c) 
{
  cen_=c;
  update_tm();
}
    
Vec3 Transform::GetCenter() const 
{
  return cen_;
}
    
Mat3 Transform::GetRot() const 
{
  return rot_;
}
    
void Transform::SetRot(const Mat3& r) 
{
  rot_=r;
  update_tm();
}

namespace {

static const float P_180 = M_PI/180.0;

}

geom::Mat3 Transform::GetXAxisRotation(float delta)
{
  return AxisRotation(Vec3(rot_(0,0),rot_(0,1),rot_(0,2)),delta*P_180);
}

geom::Mat3 Transform::GetYAxisRotation(float delta)
{
  return AxisRotation(Vec3(rot_(1,0),rot_(1,1),rot_(1,2)),delta*P_180);
}

geom::Mat3 Transform::GetZAxisRotation(float delta)
{
  return AxisRotation(Vec3(rot_(2,0),rot_(2,1),rot_(2,2)),-delta*P_180);
}

void Transform::ApplyXAxisRotation(float delta)
{
  rot_ = rot_*GetXAxisRotation(delta);
  update_tm();
}

void Transform::ApplyYAxisRotation(float delta)
{
  rot_ = rot_*GetYAxisRotation(delta);
  update_tm();
}
    
void Transform::ApplyZAxisRotation(float delta)
{
  rot_ = rot_*GetZAxisRotation(delta);
  update_tm();
}

void Transform::ApplyAxisRotation(float delta, const Vec3& axis)
{
  rot_=rot_*AxisRotation(rot_*axis, delta*P_180);
  update_tm();
}

void Transform::ApplyXAxisTranslation(float delta)
{
  trans_[0]+=delta;
  update_tm();
}

void Transform::ApplyYAxisTranslation(float delta)
{
  trans_[1]+=delta;
  update_tm();
}

void Transform::ApplyZAxisTranslation(float delta)
{
  trans_[2]+=delta;
  update_tm();
}
    
Vec3 Transform::Apply(const Vec3& v) const
{
  Vec3 nrvo(tm_*Vec4(v));
  return nrvo;
}

Vec4 Transform::Apply(const Vec4& v) const
{
  Vec4 nrvo=tm_*v;
  return nrvo;
}

geom::AlignedCuboid Transform::Apply(const geom::AlignedCuboid& c) const
{
  geom::Vec3 cmin=c.GetMin();
  geom::Vec3 cmax=c.GetMax();
  Vec3 t1 = Apply(Vec3(cmin[0],cmin[1],cmin[2]));
  Vec3 t2 = Apply(Vec3(cmin[0],cmax[1],cmin[2]));
  Vec3 t3 = Apply(Vec3(cmax[0],cmax[1],cmin[2]));
  Vec3 t4 = Apply(Vec3(cmax[0],cmin[1],cmin[2]));
  Vec3 t5 = Apply(Vec3(cmin[0],cmin[1],cmax[2]));
  Vec3 t6 = Apply(Vec3(cmin[0],cmax[1],cmax[2]));
  Vec3 t7 = Apply(Vec3(cmax[0],cmax[1],cmax[2]));
  Vec3 t8 = Apply(Vec3(cmax[0],cmin[1],cmax[2]));
  geom::Vec3 minc = Min(t1,Min(t2,Min(t3,Min(t4,Min(t5,Min(t6,Min(t7,t8)))))));
  geom::Vec3 maxc = Max(t1,Max(t2,Max(t3,Max(t4,Max(t5,Max(t6,Max(t7,t8)))))));
  return geom::AlignedCuboid(minc,maxc);
}

/*
  The order of the transformations given herein is conceptually
  "backward" as they are applied to a vertex, because the left-right
  notation is opposite to the order that the vertex experiences
  the transformations (see Ch 3 in the Red Book for more detail)

    v' = T R S C v

     v' := transformed vector
     T  := translation matrix
     R  := rotation matrix
     S  := scaling matrix
     C  := centering matrix
     v  := original vector

*/

void Transform::update_tm()
{
  tm_ =
    Mat4(1.0,0.0,0.0,trans_[0],
               0.0,1.0,0.0,trans_[1],
               0.0,0.0,1.0,trans_[2],
               0.0,0.0,0.0,1.0) *
    Mat4(rot_) *
    Mat4(1.0,0.0,0.0,-cen_[0],
               0.0,1.0,0.0,-cen_[1],
               0.0,0.0,1.0,-cen_[2],
               0.0,0.0,0.0,1.0);
  ttm_ = Transpose(tm_);
  // TODO: calculate from rot, cen and trans
  try {
    itm_ = Invert(tm_);
  } catch (GeomException& e) {
    LOG_WARNING("caught GeomException in Transform::update_tm: " << e.what());
    itm_=geom::Mat4();
  }
}

void Transform::update_components()
{
  rot_ = tm_.ExtractRotation();
  cen_ = tm_.ExtractTranslation();
  trans_[0] = tm_(3,0);
  trans_[1] = tm_(3,1);
  trans_[2] = tm_(3,2);
}

#if(OST_INFO_ENABLED)
Transform TransformFromInfo(const info::InfoGroup& group)
{
  if (!group.HasItem("center")) {
    throw info::InfoError("Error while loading transform from info: "
                          "Group does not contain a center element");
  }
  if (!group.HasGroup("rotation")) {
    throw info::InfoError("Error while loading transform from info: "
                          "Group does not contain a rotation element");
  }
  if (!group.HasItem("translation")) {
    throw info::InfoError("Error while loading transform from info: "
                          "Group does not contain a translation element");
  } 
  Transform tf;
  tf.SetCenter(group.GetItem("center").AsVector());
  tf.SetTrans(group.GetItem("translation").AsVector());
  tf.SetRot(info::Mat3FromInfo(group.GetGroup("rotation")));
  return tf;
}

void TransformToInfo(const Transform& transform, info::InfoGroup& group)
{
  Transform tf;
  group.CreateItem("center", transform.GetCenter());
  group.CreateItem("translation", transform.GetTrans());
  info::InfoGroup rot=group.CreateGroup("rotation");
  info::Mat3ToInfo(transform.GetRot(), rot);
}
#endif

}} // ns
