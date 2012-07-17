//------------------------------------------------------------------------------
// This file is part of the OpenStructure project <www.openstructure.org>
//
// Copyright (C) 2008-2011 by the OpenStructure authors
// Copyright (C) 2003-2010 by the IPLT authors
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
  Authors: Ansgar Philippsen, Andreas Schenk
*/

#ifndef OST_GUI_ARGAND_H
#define OST_GUI_ARGAND_H


#include <ost/base.hh>
#include <ost/img/data.hh>
#include <ost/img/extent.hh>

#include <ost/gui/module_config.hh>

#include <QWidget>
#include <QPixmap>

namespace ost { namespace img { namespace gui {

class DLLEXPORT_OST_GUI Argand: public QWidget
{
  Q_OBJECT;
public:
  Argand(QWidget* p=0);
  ~Argand();


 public slots:
  void SetCurrentPixel(const QPointF& p,Complex val );
  void SetExtent(const Extent& e, const Data& d);
  void ClearExtent();
protected:
  virtual void paintEvent(QPaintEvent* event);
  Real scalefactor_;
  Extent extent_;
  Point current_pos_;
  Complex current_val_;

private:
  QPixmap buffer_;

};

}}}  //ns

#endif
