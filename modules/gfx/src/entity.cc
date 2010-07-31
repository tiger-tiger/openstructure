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
/*
  Authors: Ansgar Philippsen, Marco Biasini
*/

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <sstream>

#include <boost/pointer_cast.hpp>

#include "gl_helper.hh"

#include <ost/log.hh>
#include <ost/dyn_cast.hh>

#include <ost/mol/query.hh>
#include <ost/mol/entity_property_mapper.hh>
#include <ost/mol/view_op.hh>
#include <ost/gfx/impl/cartoon_renderer.hh>
#include <ost/gfx/impl/cpk_renderer.hh>
#include <ost/gfx/impl/custom_renderer.hh>
#include <ost/gfx/impl/debug_renderer.hh>
#include <ost/gfx/impl/simple_renderer.hh>
#include <ost/gfx/impl/sline_renderer.hh>
#include <ost/gfx/impl/trace_renderer.hh>
#include <ost/gfx/impl/line_trace_renderer.hh>

#include <ost/gfx/impl/entity_renderer.hh>

#include "input.hh"
#include "scene.hh"
#include "entity.hh"
#include "vertex_array_helper.hh"
#if OST_SHADER_SUPPORT_ENABLED
#include "shader.hh"
#endif

namespace ost {

using namespace mol;

namespace gfx {
using namespace impl;

Entity::Entity(const String& name,
               const EntityHandle& h,
               const Query& q,
               QueryFlags f):
  GfxObj(name),
  qv_(q, h, f),
  bbox_(geom::Vec3(), geom::Vec3()),
  sel_(),
  sel_update_(),
  trace_(),
  opacity_(1.0),
  blur_(false),
  blurf1_(1.0),
  blurf2_(0.8)
{
  init(RenderMode::SIMPLE);
}

Entity::Entity(const String& name,
               RenderMode::Type m,
               const EntityHandle& h,
               const Query& q,
               QueryFlags f):
  GfxObj(name),
  qv_(q, h, f),
  bbox_(geom::Vec3(), geom::Vec3()),
  sel_(),
  sel_update_(),
  trace_(),
  opacity_(1.0),
  blur_(false),
  blurf1_(1.0),
  blurf2_(0.8)
{
  init(m);
}

Entity::Entity(const String& name,
               const EntityView& v):
  GfxObj(name),
  qv_(v),
  bbox_(geom::Vec3(), geom::Vec3()),
  sel_(),
  sel_update_(),
  trace_(),
  opacity_(1.0),
  blur_(false),
  blurf1_(1.0),
  blurf2_(0.8)
{
  init(RenderMode::SIMPLE);
}

Entity::Entity(const String& name,
               RenderMode::Type m,
               const EntityView& v):
  GfxObj(name),
  qv_(v),
  bbox_(geom::Vec3(), geom::Vec3()),
  sel_(),
  sel_update_(),
  trace_(),
  opacity_(1.0),
  blur_(false),
  blurf1_(1.0),
  blurf2_(0.8)
{
  init(m);
}


impl::EntityRenderer* Entity::GetOrCreateRenderer(RenderMode::Type rm)
{
  RendererMap::iterator rit = renderer_.find(rm);
  if(rit!=renderer_.end()) {
    return rit->second;
  }
  impl::EntityRenderer* r=NULL;
  switch (rm) {
    case RenderMode::SIMPLE:
      r=new impl::SimpleRenderer();
      break;
    case RenderMode::CUSTOM:
      r=new impl::CustomRenderer();
      break;
    case RenderMode::SLINE:
      r=new impl::SlineRenderer(&trace_);
      break;
    case RenderMode::LINE_TRACE:
      r=new impl::LineTraceRenderer(&trace_);
      break;
    case RenderMode::TRACE:
      r=new impl::TraceRenderer(&trace_);
      break;
    case RenderMode::HSC:
      r=new impl::CartoonRenderer(&trace_,false);
      break;
    case RenderMode::CPK:
      r=new impl::CPKRenderer();
      break;
    case RenderMode::TUBE:
      r=new impl::CartoonRenderer(&trace_, true);
      break;
    default:
      return 0;
  }
  renderer_.insert(rm, r);
  return r;
}

void Entity::init(RenderMode::Type rm)
{
  // TODO replace with def mat for this gfx obj type
  SetMatAmb(Color(0,0,0));
  SetMatDiff(Color(1,1,1));
  SetMatSpec(Color(0.7,0.7,0.7));
  SetMatShin(96);

  update_view_=true;
  render_mode_=rm;
  trace_.ResetView(this->GetView());
  sel_=this->GetView().CreateEmptyView();  
  impl::EntityRenderer* r=this->GetOrCreateRenderer(rm);
  if(!r) return;

  r->AddView(this->GetView());
  r->UpdateViews();
  set_static_max_rad();
  
  Rebuild();
}

void Entity::SetBlur(bool f)
{
  blur_=f;
}

void Entity::BlurSnapshot()
{
  for (RendererMap::iterator i=renderer_.begin(),e=renderer_.end(); i!=e; ++i) {
    impl::SimpleRenderer* sr = dynamic_cast<impl::SimpleRenderer*>(i->second);
    if(sr) {
      DoBlurSnapshot(sr->GetBondEntryList());
    }
    impl::LineTraceRenderer* lr = dynamic_cast<impl::LineTraceRenderer*>(i->second);
    if(lr) {
      DoBlurSnapshot(lr->GetBondEntryList());
    }
  }
}

void Entity::SetBlurFactors(float bf1,float bf2)
{
  blurf1_=bf1;
  blurf2_=bf2;
}

void Entity::Rebuild()
{
  geom::Vec3 delta=GetTF().GetTrans()-GetTF().GetCenter();

  if(update_view_) {
    EntityView nv=this->GetView();
    for (RendererMap::iterator i=renderer_.begin(), 
           e=renderer_.end(); i!=e; ++i) {
      i->second->ClearViews();
      i->second->AddView(nv);
      i->second->UpdateViews();
      i->second->PrepareRendering();
    }
  }

  this->ReapplyColorOps();
  FlagRebuild();
  geom::Vec3 center=this->GetCenter();
  Transform tf=this->GetTF();
  tf.SetCenter(center);
  tf.SetTrans(center+delta);
  this->SetTF(tf);  
  Scene::Instance().RequestRedraw();
}

void Entity::UpdatePositions()
{
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* r=i->second;
    r->FlagPositionsDirty();
  }
  FlagRebuild();  
  Scene::Instance().RequestRedraw();
}



geom::AlignedCuboid Entity::GetBoundingBox() const
{
  this->UpdateIfNeeded();
  return bbox_;
}

void Entity::ProcessLimits(geom::Vec3& minc, geom::Vec3& maxc, 
                           const Transform& tf) const
{
  try {
    geom::AlignedCuboid coord_limits=this->GetBoundingBox();
    // update min/max by transforming all 8 corners of the bounding box and 
    // comparing it against the current min/max
    geom::Vec3 mmin=coord_limits.GetMin();
    geom::Vec3 mmax=coord_limits.GetMax();
    geom::Vec3 t1=tf.Apply(geom::Vec3(mmin[0], mmin[1], mmin[2]));
    geom::Vec3 t2=tf.Apply(geom::Vec3(mmin[0], mmax[1], mmin[2]));
    geom::Vec3 t3=tf.Apply(geom::Vec3(mmax[0], mmax[1], mmin[2]));
    geom::Vec3 t4=tf.Apply(geom::Vec3(mmax[0], mmin[1], mmin[2]));
    geom::Vec3 t5=tf.Apply(geom::Vec3(mmin[0], mmin[1], mmax[2]));
    geom::Vec3 t6=tf.Apply(geom::Vec3(mmin[0], mmax[1], mmax[2]));
    geom::Vec3 t7=tf.Apply(geom::Vec3(mmax[0], mmax[1], mmax[2]));
    geom::Vec3 t8=tf.Apply(geom::Vec3(mmax[0], mmin[1], mmax[2]));
    minc = geom::Min(minc, geom::Min(t1, geom::Min(t2, geom::Min(t3, 
                     geom::Min(t4, geom::Min(t5, geom::Min(t6, 
                     geom::Min(t7, t8))))))));
    maxc = geom::Max(maxc, geom::Max(t1, geom::Max(t2, geom::Max(t3, 
                     geom::Max(t4, geom::Max(t5, geom::Max(t6,
                     geom::Max(t7, t8))))))));
  } catch(Error& e) {
    // in case the object is empty...
  }
} 

void Entity::SetColorForAtom(const Color& col,
                             const AtomHandle& atom) 
{
  std::stringstream selection;
  selection << "cname='" << atom.GetResidue().GetChain().GetName() 
            << "' and rindex=" << atom.GetResidue().GetIndex() 
            << " and aname='" << atom.GetName() << "'";
  UniformColorOp cop = UniformColorOp(selection.str(), col);
  this->Apply(cop);
  Scene::Instance().RequestRedraw();
}

void Entity::CacheBoundingBox() const
{
  geom::AlignedCuboid coord_limits=geom::AlignedCuboid(geom::Vec3(), 
                                                       geom::Vec3());
  bool has_data=false;
  for (RendererMap::const_iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    const impl::EntityRenderer* r=i->second;
    if (r->IsEnabled() && r->HasDataToRender()) {
      if (!has_data) {
        coord_limits=r->GetBoundingBox();          
        has_data=true;
      } else {
        coord_limits=geom::Union(coord_limits, r->GetBoundingBox());
      }      
    }
  }
  bbox_=coord_limits;  
}

bool Entity::UpdateIfNeeded() const
{
  bool updated=false;
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    EntityRenderer* renderer =i->second;
    if (renderer->IsDirty()) {
      renderer->PrepareRendering();
      updated=true;
    }
    renderer->VA().SetOpacity(opacity_);
  }
  if (updated) {
    this->CacheBoundingBox();
  }
  return updated;
}

void Entity::CustomPreRenderGL(bool update)
{
  if (update) {
    this->UpdateIfNeeded();
    RefreshVA();
  }
}

void Entity::RefreshVA()
{
  for (RendererMap::iterator i=renderer_.begin(), 
	 e=renderer_.end(); i!=e; ++i) {
    i->second->Debug(debug_flags_);
  }
}

void Entity::CustomRenderGL(RenderPass pass)
{
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* r=i->second;
    if(r->IsEnabled()) {
      if(pass==STANDARD_RENDER_PASS) {
        r->Render(pass);
        if(outline_flag_) {
          r->VA().SetOutlineMode(outline_mode_);
          r->Render(STANDARD_RENDER_PASS);
          r->VA().SetOutlineMode(0);
        }
      } else if(pass==GLOW_RENDER_PASS) {
        r->Render(GLOW_RENDER_PASS);
      }
      if(blur_) {
        impl::SimpleRenderer* sr = dynamic_cast<impl::SimpleRenderer*>(r);
        if(sr) {
          DoRenderBlur(sr->GetBondEntryList(),blurf1_,blurf2_);
        }
        impl::LineTraceRenderer* lr = dynamic_cast<impl::LineTraceRenderer*>(r);
        if(lr) {
          DoRenderBlur(lr->GetBondEntryList(),blurf1_,blurf2_);
        }
      }
    }
  }
}

void Entity::CustomRenderPov(PovState& pov)
{
  for (RendererMap::iterator it=renderer_.begin(); it!=renderer_.end(); ++it) {
    if(it->second->IsEnabled()){
      it->second->RenderPov(pov,GetName());
    }
  }
}

mol::AtomHandle Entity::PickAtom(const geom::Line3& line, Real line_width)
{
  mol::AtomHandle picked_atom;
  if (!this->IsVisible())
    return picked_atom;  
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* r=i->second;
    if (r->HasDataToRender() && r->IsEnabled()) {
      r->PickAtom(line, line_width, picked_atom);      
    }
  }
  return picked_atom;
}

mol::BondHandle Entity::PickBond(const geom::Line3& line, Real line_width)
{

  mol::BondHandle picked_bond;
  if (!this->IsVisible())
    return picked_bond;  
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* r=i->second;
    if (r->HasDataToRender() && r->IsEnabled()) {
      r->PickBond(line, line_width, picked_bond);      
    }
  }
  return picked_bond;
}

bool Entity::OnSelect(const geom::Line3& line, geom::Vec3& result,
                      float zlim, bool pick_flag)
{
  uint smode=Scene::Instance().GetSelectionMode();
  mol::AtomHandle sel=this->PickAtom(line, 0.5);
  if(smode==1 || smode==2 || smode==3) {
    if(sel.IsValid()) {
      geom::Vec3 ppos = Scene::Instance().Project(sel.GetPos());
      if(ppos[2]>zlim) return false;

      std::ostringstream os;
      os << GetName() << ": " << sel.GetQualifiedName() << " " 
         << sel.GetPos();
      Scene::Instance().StatusMessage(os.str());

      if(!pick_flag) {
        result = sel.GetPos();
        return true;
      }
      if(smode==1) {
        AtomView av = sel_.FindAtom(sel.GetHandle());
        if(av.IsValid()) {
          LOGN_DEBUG("de-selected atom: " << sel);
          sel_.RemoveAtom(av);
          if(av.GetResidue().GetAtomCount()==0){
            av.GetResidue().GetChain().RemoveResidue(av.GetResidue());
          }
        } else {
          LOGN_DEBUG("selected atom: " << sel);
          sel_.AddAtom(sel.GetHandle());
        }
        result = sel.GetPos();
      } else if(smode==2) {
        AtomView av=sel_.FindAtom(sel);
        ResidueView rv=sel_.FindResidue(sel.GetResidue());
        if(av.IsValid() && rv.IsValid()) {
          LOGN_DEBUG("de-selected residue: " << sel.GetResidue());
          sel_.RemoveResidue(rv);
        } else {
          LOGN_DEBUG("selected residue: " << sel.GetResidue());
          sel_.AddResidue(sel.GetResidue(),ViewAddFlag::INCLUDE_ALL);
        }
        AtomHandle ca=sel.GetHandle().GetResidue().GetCentralAtom();
        result = ca.IsValid() ? ca.GetPos() : sel.GetPos();
      } else {
        if (!sel.IsValid()) {
          return false;
        }
        ChainHandle chain=sel.GetHandle().GetResidue().GetChain();
        ChainView cv = sel_.FindChain(chain);
        if(cv.IsValid()) {
          LOGN_DEBUG("de-selected chain: " << chain);
          sel_.RemoveChain(cv);
        } else {
          LOGN_DEBUG("selected chain: " << chain);
          sel_.AddChain(chain, ViewAddFlag::INCLUDE_ALL);
        }
        result = geom::Vec3(); // todo calculate center of mass
      }
      sel_.AddAllInclusiveBonds();
      sel_update_=true;
      GfxObjP o=dyn_cast<GfxObj>(shared_from_this());
      this->UpdateSelection();
      this->FlagRebuild();
      Scene::Instance().SelectionChanged(GetName(), sel_);
      Scene::Instance().RequestRedraw();
      return true;
    }
  }
  if(sel) {
    LOGN_MESSAGE("picked " << sel.GetResidue().GetChain().GetName() << " " 
                 << sel.GetResidue().GetNumber() << " " << sel.GetName());
    return true;
  }
  return false;
}

RenderOptionsPtr Entity::GetOptions(RenderMode::Type render_mode)
{
  EntityRenderer* renderer = this->GetOrCreateRenderer(render_mode);
  if(!renderer) return RenderOptionsPtr();
  RenderOptionsPtr render_options=renderer->GetOptions();
  EntityP e=boost::dynamic_pointer_cast<Entity>(shared_from_this());
  render_options->AddObserver(e);
  return render_options;
}

void Entity::SetOptions(RenderMode::Type render_mode, 
			RenderOptionsPtr& render_options)
{
  if(!render_options) return;
  RendererMap::iterator i=renderer_.find(render_mode);
  if(i!= renderer_.end()) {
    EntityRenderer* entity_renderer =i->second;
    if(entity_renderer->CanSetOptions(render_options)){
      EntityP e=boost::dynamic_pointer_cast<Entity>(shared_from_this());
      RenderOptionsPtr old_render_options = entity_renderer->GetOptions();
      if(old_render_options) {
	old_render_options->RemoveObserver(e);
      }
      entity_renderer->SetOptions(render_options);
      render_options->AddObserver(e);
      FlagRebuild();
      Scene::Instance().RequestRedraw();
    } else{
      throw Error("These render options are not compatible with this render mode.");
    }
  } else {
    throw Error("The render mode does not exist");
  }
}

void Entity::ApplyOptions(RenderMode::Type render_mode,
                                RenderOptionsPtr& render_options)
{
  RendererMap::iterator i=renderer_.find(render_mode);  
  if(i!=renderer_.end()) {
    RenderOptionsPtr options=i->second->GetOptions();
    if(options->CanApplyRenderOptions(render_options)){
      EntityP e=boost::dynamic_pointer_cast<Entity>(shared_from_this());
      options->AddObserver(e);
      options->ApplyRenderOptions(render_options);
    }
    else{
      throw Error("These render options can not be applied to the given render mode.");
    }
  }
  else {
    throw Error("The render mode does not exist");
  }
}

void Entity::OptionsChanged(RenderMode::Type render_mode)
{
  impl::EntityRenderer* entity_renderer=this->GetOrCreateRenderer(render_mode);
  if(!entity_renderer) return;
  entity_renderer->RenderOptionsChanged();
  FlagRebuild();
  Scene::Instance().ObjectChanged(this->GetName());
}

void Entity::SetOpacity(float f)
{
  opacity_=f;
  this->UpdateIfNeeded();
  Scene::Instance().RequestRedraw();
}

void Entity::SetOutlineWidth(float f)
{
  for (RendererMap::iterator it=renderer_.begin(); it!=renderer_.end(); ++it) {
    if(it->second->IsEnabled()){
      it->second->VA().SetOutlineWidth(f);
    }
  }
  Scene::Instance().RequestRedraw();
}

void Entity::SetOutlineExpandFactor(float f)
{
  for (RendererMap::iterator it=renderer_.begin(); it!=renderer_.end(); ++it) {
    if(it->second->IsEnabled()){
      it->second->VA().SetOutlineExpandFactor(f);
    }
  }
  Scene::Instance().RequestRedraw();
}

void Entity::SetOutlineExpandColor(const Color& c)
{
  for (RendererMap::iterator it=renderer_.begin(); it!=renderer_.end(); ++it) {
    if(it->second->IsEnabled()){
      it->second->VA().SetOutlineExpandColor(c);
    }
  }
  Scene::Instance().RequestRedraw();
}

void Entity::OnRenderModeChange()
{
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    i->second->ClearViews();
  }
  impl::EntityRenderer* r=this->GetOrCreateRenderer(render_mode_);
  if(!r) return;

  r->AddView(this->GetView());
  for (RendererMap::iterator i=renderer_.begin(), 
	 e=renderer_.end(); i!=e; ++i) {
    mol::EntityView rv=i->second->GetFullView();
    if (rv.IsValid() && rv.GetAtomCount()>0) {
      i->second->SetSelection(mol::Intersection(sel_, rv));
    }         
    i->second->UpdateViews();
  }  
  this->ReapplyColorOps();
  this->FlagRebuild();
  GfxObj::OnRenderModeChange();
}

const String Entity::GetRenderModeName(RenderMode::Type mode){
  RendererMap::iterator i=renderer_.find(mode);
  String name = "";
  if(i!=renderer_.end()) {
    impl::EntityRenderer* renderer=i->second;
    name = renderer->GetName();
  }
  return name;
}

void Entity::SetEnableRenderMode(RenderMode::Type mode, bool enable){
  RendererMap::iterator i=renderer_.find(mode);
  if(i!=renderer_.end()) {
    impl::EntityRenderer* renderer=i->second;
    if(renderer->IsEnabled() != enable){
      renderer->SetEnabled(enable);
      this->FlagRebuild();
    }
  }
}

bool Entity::IsRenderModeEnabled(RenderMode::Type mode){
  RendererMap::iterator i=renderer_.find(mode);
  if(i!=renderer_.end()) {
    impl::EntityRenderer* renderer=i->second;
    return renderer->IsEnabled();
  }
  return false;
}


RenderModeTypes Entity::GetNotEmptyRenderModes(){
  std::vector<RenderMode::Type> render_modes;
  for (RendererMap::iterator i=renderer_.begin(),
         e=renderer_.end(); i!=e; ++i) {
    if(i->second->HasDataToRender()){
      render_modes.push_back(i->first);
    }
  }
  return render_modes;
}

void Entity::SetRenderMode(RenderMode::Type mode, 
                           const mol::EntityView& view, bool keep)
{
  EntityRenderer* rend = this->GetOrCreateRenderer(mode);
  if(!rend) return;
  rend->AddView(view);

  // substract view from all renderers
  for (RendererMap::iterator i=renderer_.begin(),
       e=renderer_.end(); i!=e; ++i) {
     impl::EntityRenderer* renderer=i->second;
     if (!keep && i->first!=mode) {
       renderer->SubstractView(view);
     }
     mol::EntityView rv=renderer->GetFullView();
     if (rv.IsValid()) {
       renderer->SetSelection(mol::Intersection(sel_, rv));
     }
     renderer->UpdateViews();
  }
  this->ReapplyColorOps(); // done in rebuild?
  this->FlagRebuild();
  Scene::Instance().RenderModeChanged(GetName());
}

mol::EntityView Entity::GetRenderView(RenderMode::Type mode)
{
  EntityRenderer* rend = this->GetOrCreateRenderer(mode);
  if(!rend) return mol::EntityView();
  return rend->GetFullView();
}

void Entity::SetRenderMode(RenderMode::Type mode)
{
  GfxObj::SetRenderMode(mode);
}

void Entity::SetVisible(const mol::EntityView& view, bool visible){
  for (RendererMap::iterator i=renderer_.begin(),
         e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* renderer=i->second;
    renderer->SetVisible(view, visible);
    renderer->UpdateViews();
  }
  this->ReapplyColorOps(); // done in rebuild?
  this->FlagRebuild();
}

void Entity::SetColor(const Color& col, const String& selection)
{
  UniformColorOp cop = UniformColorOp(selection, col);
  this->Apply(cop);
}

void Entity::SetDetailColor(const Color& col, const String& selection)
{
  UniformColorOp cop = UniformColorOp(selection, col);
  cop.SetMask(DETAIL_COLOR);
  this->Apply(cop);
}

/// \brief set selection
void Entity::SetSelection(const mol::EntityView& view)
{
  if (!view.IsValid()) {
    sel_=this->GetView().CreateEmptyView();
  } else {
    sel_=view;
  }
  this->UpdateSelection();
  this->FlagRebuild();  
  Scene::Instance().SelectionChanged(GetName(), view);  
  Scene::Instance().RequestRedraw();
}

void Entity::UpdateSelection()
{
  for (RendererMap::iterator i=renderer_.begin(), 
       e=renderer_.end(); i!=e; ++i) {
    impl::EntityRenderer* renderer=i->second;
    if (renderer->HasDataToRender()) {
      renderer->SetSelection(mol::Intersection(renderer->GetFullView(), sel_));
      renderer->UpdateViews();
    }
  }  
}

/// \brief get selection
mol::EntityView Entity::GetSelection() const
{
  return sel_;
}


void Entity::ColorByElement()
{
  ByElementColorOp cop = ByElementColorOp();
  this->Apply(cop);
}

void Entity::ColorByChain()
{
  ByChainColorOp cop = ByChainColorOp();
  this->Apply(cop);
}

void Entity::ColorBy(const mol::EntityView& ev,
                     const String& prop,
                     const Gradient& g, float minv, float maxv)
{
  EntityViewColorOp evop = EntityViewColorOp(prop, g,minv,maxv,ev);
  this->Apply(evop);
}

#if OST_IMG_ENABLED
void Entity::ColorBy(const img::MapHandle& mh,
                     const String& prop,
                     const Gradient& g,float minv, float maxv)
{
  MapHandleColorOp mhop = MapHandleColorOp("",prop,g,minv,maxv,mh);
  this->Apply(mhop);
}
#endif //OST_IMG_ENABLED

void Entity::ColorBy(const String& prop,
                     const Gradient& gradient,
                     float minv,float maxv,
                     mol::Prop::Level level)
{
  GradientLevelColorOp glop = GradientLevelColorOp("",prop, gradient,minv,maxv,level);
  this->Apply(glop);
}

void Entity::DetailColorBy(const String& prop,
                           const Gradient& gradient,
                           float minv,float maxv,
                           mol::Prop::Level level)
{
  GradientLevelColorOp glop = GradientLevelColorOp("",prop, gradient,minv,maxv,level);
  glop.SetMask(DETAIL_COLOR);
  this->Apply(glop);
}

void Entity::ColorBy(const String& prop,
                     const Color& c1, const Color& c2,
                     float minv, float maxv,
                     mol::Prop::Level level)
{
  Gradient g;
  g.SetColorAt(0.0,c1);
  g.SetColorAt(1.0,c2);
  this->ColorBy(prop,g,minv,maxv,level);
}

void Entity::ColorBy(const String& prop,
                     const Gradient& gradient,
                     mol::Prop::Level level)
{
  GradientLevelColorOp glop = GradientLevelColorOp("",prop, gradient,level);
  this->Apply(glop);
}

mol::EntityView Entity::GetView() const
{
  if (update_view_) {
    update_view_=false;    
    cached_view_=qv_.GetEntityView();
  }
  return cached_view_;
}

void Entity::SetQuery(const mol::Query& q)
{
  qv_.SetQuery(q);
  update_view_=true;
  Rebuild();
}

void Entity::ColorBy(const String& prop,
                     const Color& c1, const Color& c2,
                     mol::Prop::Level level)
{
  std::pair<float,float> minmax = this->GetView().GetMinMax(prop);
  this->ColorBy(prop,c1,c2,minmax.first, minmax.second,level);
}

void Entity::RadiusBy(const String& prop,
                      float rmin, float rmax,
                      float minv, float maxv,
                      mol::Prop::Level hint)
{
  set_static_max_rad();
  EntityPropertyMapper epm(prop, hint);
#if 0  
  for(NodeEntryListList::iterator ll_it=node_list_list_->begin();ll_it!=node_list_list_->end();++ll_it) {
    for(NodeEntryList::iterator it=(*ll_it).begin();it!=(*ll_it).end();++it) {
      try {
        it->rad=clamp(normalize(epm.Get(it->atom), minv, maxv),0.0,1.0)*(rmax-rmin)+rmin;
        max_rad_=std::max(max_rad_,it->rad);
      } catch (std::exception&) {
        LOGN_DEBUG("property " << prop << " not found");
      }
    }
  }
#endif
  FlagRebuild();
}

void Entity::RadiusBy(const String& prop,
                      float rmin, float rmax,
                      mol::Prop::Level hint)
{
  std::pair<float,float> minmax = this->GetView().GetMinMax(prop);
  RadiusBy(prop,rmin,rmax,minmax.first,minmax.second,hint);
}

void Entity::ResetRadiusBy()
{
#if 0  
  for(NodeEntryListList::iterator ll_it=node_list_list_->begin();ll_it!=node_list_list_->end();++ll_it) {
    for(NodeEntryList::iterator it=(*ll_it).begin();it!=(*ll_it).end();++it) {
      it->rad=1.0;
    }
  }
#endif
  FlagRebuild();
}


//////////////////////////////////////////
// protected and private methods


void Entity::set_static_max_rad()
{
  //TODO implement method
}

bool Entity::HasSelection() const
{
  return (sel_.IsValid() && sel_.GetAtomCount()>0);
}

namespace {

template <typename I, typename O>
void apply_color_op_to_renderer_list(I begin, I end, const O& o)
{
  for (I i=begin; i!=end; ++i) {
    i->second->Apply(o);
  }
}

}

void Entity::Apply(const gfx::UniformColorOp& op, bool store)
{
  if(store){
    UniformColorOp* op_ptr = new UniformColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}

void Entity::Apply(const gfx::GradientLevelColorOp& op, bool store)
{

  if(store){
    GradientLevelColorOp* op_ptr = new GradientLevelColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  if(op.GetCalculateMinMax()){
    std::pair<float,float> minmax = this->GetView().GetMinMax(op.GetProperty(), op.GetLevel());
    op.SetMinV(minmax.first);
    op.SetMaxV(minmax.second);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}

void Entity::Apply(const gfx::ByElementColorOp& op, bool store)
{
  if(store){
    ByElementColorOp* op_ptr = new ByElementColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}

void Entity::Apply(const gfx::ByChainColorOp& op, bool store)
{
  if(store){
    ByChainColorOp* op_ptr = new ByChainColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}

void Entity::Apply(const gfx::EntityViewColorOp& op, bool store)
{
  if(store){
    EntityViewColorOp* op_ptr = new EntityViewColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}

#if OST_IMG_ENABLED
void Entity::Apply(const gfx::MapHandleColorOp& op, bool store)
{
  if(store){
    MapHandleColorOp* op_ptr = new MapHandleColorOp(op);
    this->AppendColorOp(op_ptr);
  }
  apply_color_op_to_renderer_list(renderer_.begin(), renderer_.end(), op);
  FlagRebuild();
}
#endif

void Entity::CleanColorOps()
{
  GfxObj::CleanColorOps();
}

void Entity::ReapplyColorOps()
{
  GfxObj::ReapplyColorOps();
}

void Entity::UpdateView() 
{
  update_view_=true; 
  Rebuild();
  UpdatePositions();
}

}} // ns
