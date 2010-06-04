#------------------------------------------------------------------------------
# This file is part of the OpenStructure project <www.openstructure.org>
#
# Copyright (C) 2008-2010 by the OpenStructure authors
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 3.0 of the License, or (at your option)
# any later version.
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#------------------------------------------------------------------------------
# -*- coding: utf-8 -*-
import math
from ost import gui
from ost import gfx
try: 
  from ost import img
  _img_present=True
except ImportError:
  _img_present=False
  pass
from PyQt4 import QtCore, QtGui

#Map Level Widget
class MapLevelWidget(QtGui.QWidget):
  def __init__(self, parent=None):
    QtGui.QWidget.__init__(self, parent)
    
    #Create Ui elements
    map_level_label = QtGui.QLabel("Map Contour Level")
    font = map_level_label.font()
    font.setBold(True)
    
    self.level_preview_ = LevelPreview()
    
    self.level_spinbox_ = QtGui.QDoubleSpinBox()
    self.level_spinbox_.setDecimals(3)
    self.level_spinbox_.setSingleStep(0.05)
    
    grid = QtGui.QGridLayout()
    grid.setContentsMargins(0,5,0,0)
    grid.addWidget(self.level_preview_, 0, 0, 1, 4)
    grid.addWidget(map_level_label, 1, 0, 1, 3)
    grid.addWidget(self.level_spinbox_,1,3,1,1)
    grid.setRowStretch(3, 1)
    self.setLayout(grid)
    
    QtCore.QObject.connect(self.level_preview_, QtCore.SIGNAL("levelUpdated"), self.UpdateLevel)
    QtCore.QObject.connect(self.level_preview_, QtCore.SIGNAL("levelModified"), self.ModifySpinBox)
    QtCore.QObject.connect(self.level_spinbox_, QtCore.SIGNAL("valueChanged(double)"), self.UpdateLevel)

    self.setMinimumSize(250,300)
        
  def Update(self):
    scene_selection = gui.SceneSelection.Instance()
    if(scene_selection.GetActiveNodeCount()==1):
      node = scene_selection.GetActiveNode(0)
      if _img_present and isinstance(node, gfx.MapIso):
        self.level_preview_.SetBins(node.GetHistogram())
        self.level_preview_.SetMinimum(node.GetMinLevel())
        self.level_spinbox_.setMinimum(node.GetMinLevel())
        self.level_preview_.SetMaximum(node.GetMaxLevel())
        self.level_spinbox_.setMaximum(node.GetMaxLevel())
        self.level_preview_.SetLevel(node.GetLevel())
        self.setEnabled(True)
      else:
        self.setEnabled(False)
    else:
      self.setEnabled(False)
        
  def UpdateLevel(self, level):
    scene_selection = gui.SceneSelection.Instance()
    if(scene_selection.GetActiveNodeCount()==1):
      node = scene_selection.GetActiveNode(0)
      node.SetLevel(level)
      
  def ModifySpinBox(self, level):
    QtCore.QObject.disconnect(self.level_spinbox_, QtCore.SIGNAL("valueChanged(double)"), self.UpdateLevel)
    self.level_spinbox_.setValue(level)
    QtCore.QObject.connect(self.level_spinbox_, QtCore.SIGNAL("valueChanged(double)"), self.UpdateLevel)
        
#Level Preview
class LevelPreview(QtGui.QWidget):
  def __init__(self, parent=None):
    QtGui.QWidget.__init__(self, parent)
    
    #Defaults
    self.border_offset_ = 3
    self.preview_height_ = 100
    QtGui.QWidget.__init__(self, parent)
    
    #Ui
    self.setMinimumSize(0, self.preview_height_ + 4)

    self.bins_ = None
    self.level_ = 0
    self.minimum_ = 0
    self.maximum_ = 0
    
    self.paint_mouse_=False
    
  def SetBins(self, bins):
    self.bins_ = bins
    self.update()

  def SetMaximum(self, max):
    self.maximum_ = max

  def SetMinimum(self, min):
    self.minimum_ = min
    
  def SetLevel(self, level):
    self.level_ = level

  def GetLevel(self):
    return self.level_
    
  def paintEvent(self, event):   
    if self.isEnabled() and self.bins_ is not None:
      painter = QtGui.QPainter()
      if painter.begin(self):
        self.PaintBackground(painter)
        self.PaintBins(painter)
        self.PaintLevel(painter)
        if(self.paint_mouse_):
          self.PaintMouse(painter)
      painter.end()

  def PaintBackground(self,painter):
    size = self.size()
    painter.setBrush(QtCore.Qt.white)
    painter.setPen(QtCore.Qt.white)
    painter.drawRect(self.border_offset_,
                   self.border_offset_,
                   size.width() - 2 * self.border_offset_,
                   self.preview_height_)

  def PaintBins(self,painter):
    size = self.size()
    bin_cnt = len(self.bins_)
    bin_width = (size.width()-2* self.border_offset_) / float(bin_cnt)
    max=0
    for b in self.bins_:
      if(b>max):
        max = b
    max = math.log(max)
    if(max > 0):
      painter.setBrush(QtCore.Qt.black)
      painter.setPen(QtCore.Qt.black)
      for i in range(0,bin_cnt):
        bin_height = self.bins_[i]
        if(bin_height>0):
          bin_height = math.floor((math.log(bin_height)/max)*(self.preview_height_-2*(self.border_offset_)))
          painter.drawRect(self.border_offset_ + (i*bin_width),
                         self.preview_height_ - bin_height,
                         bin_width,
                         bin_height)

  def PaintLevel(self,painter):
    size = self.size()
    width = size.width()-(2* self.border_offset_)
    tot_len = self.maximum_-self.minimum_
    if(tot_len>0):
      cur_len = self.level_-self.minimum_
      painter.setBrush(QtCore.Qt.red)
      painter.setPen(QtCore.Qt.red)
      painter.drawRect((width / tot_len) * cur_len,
                     self.border_offset_,
                     1,
                     self.preview_height_)

  def PaintMouse(self,painter):
    size = self.size()
    width = size.width()-(2* self.border_offset_)
    painter.setBrush(QtCore.Qt.gray)
    painter.setPen(QtCore.Qt.gray)
    pos=self.mapFromGlobal(QtGui.QCursor.pos())
    painter.drawRect(pos.x(),
                   self.border_offset_,
                   1,
                   self.preview_height_)

  def mouseReleaseEvent(self, event):
    self.paint_mouse_=False
    size = self.size()
    width = size.width()-(2* self.border_offset_)
    tot_len = self.maximum_-self.minimum_
    self.level_ = self.minimum_ + float(event.x())/width * tot_len
    self.update()
    self.emit(QtCore.SIGNAL("levelUpdated"),(self.level_))
  
  def mousePressEvent(self,event):
    self.paint_mouse_=True
    
  def mouseMoveEvent(self, event):
    size = self.size()
    width = size.width()-(2* self.border_offset_)
    tot_len = self.maximum_-self.minimum_
    level = self.minimum_ + float(event.x())/width * tot_len
    self.emit(QtCore.SIGNAL("levelModified"),(level))
    self.update()