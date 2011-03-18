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
#ifndef OST_GFX_IMPL_OCTREE_ISOCONT_HH
#define OST_GFX_IMPL_OCTREE_ISOCONT_HH

/*
  Author: Marco Biasini
 */
#include <boost/unordered_map.hpp>  

#include <ost/img/image_handle.hh>
#include <ost/img/image_state.hh>
#include <ost/gfx/module_config.hh>
#include <ost/gfx/vertex_array.hh>
#include <ost/gfx/impl/map_octree.hh>

namespace ost { namespace gfx { namespace impl {
  
struct EdgeDesc {
  EdgeDesc(uint8_t d, const img::Point& o, 
           uint8_t ac1, uint8_t ac2, bool l):
           dir(d), off(o), c1(ac1), c2(ac2), lv(l)
  { }
  uint32_t GetKey(const img::Point& p, const img::Extent& ext)
  {
    img::Point k=p+off;
    img::Size s=ext.GetSize();
    img::Point p1=k-ext.GetStart();
    return 4*(p1[0]*512*512+p1[1]*512+p1[2])+dir;
  }
  uint8_t      dir;
  img::Point  off;  
  uint8_t      c1;
  uint8_t      c2;
  bool         lv;
};
/// \brief isocontouring of maps using an octree-optimization
/// \internal
/// \sa MapOctree
struct DLLEXPORT_OST_GFX OctreeIsocont {
private:
  static uint16_t EDGE_FLAGS[];
  static img::Point POINT_OFFSETS[];
  static int8_t TRIANGLES[256][16];
  static EdgeDesc EDGE_DESC[12];
public:
  typedef boost::unordered_map<uint32_t, VertexID> EdgeMap;
  OctreeIsocont(IndexedVertexArray& va, float level, bool triangles, 
                const Color& color): 
   va_(va), level_(level), triangles_(triangles), color_(color)
  { }
  bool VisitNode(const impl::OctreeNode& node, uint8_t level, 
                 const img::Extent& ext)
  {
    return (node.GetMin()<level_ && level_<=node.GetMax());
  }
  void VisitLeaf(const MapOctree& octree, img::RealSpatialImageState* map, 
                 const img::Point& point);  
  VertexID GetOrGenVert(const MapOctree& octree, img::RealSpatialImageState* map, 
                        const img::Point& p, EdgeDesc* desc);
  
  const img::Point& GetOffset() const { return offset_; }
private:
  IndexedVertexArray& va_;
  float               level_;
  EdgeMap             edge_map_;
  bool                triangles_;
  Color               color_;
  img::Point          offset_;
  img::Extent         vis_extent_;
};


}}}

#endif

