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
#include <utility>
#include <vector>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <ctime>

#include <QDir>
#include <QTimer>
#include <ost/gui/module_config.hh>
#include <ost/log.hh>
#include <ost/platform.hh>
#include <ost/gui/python_shell/python_shell.hh>
#include <ost/gui/python_shell/python_interpreter.hh>
#include "gl_win.hh"
#include "widget_state_saver.hh"
#include "gosty.hh"
#include "change_process_name.hh"
// must come last
#include <QApplication>
#include <QResource>
#include <QFileInfo>

using namespace ost;
using namespace ost::gui;

namespace ost { namespace gui{ namespace detail {

DelayedScriptExecutor::DelayedScriptExecutor()
{
  // the trick here is, that timers won't get executed until we call app.exec()
  QTimer::singleShot(0, this, SLOT(Exec()));
}
void DelayedScriptExecutor::Exec()
{
  PythonInterpreter& interp=PythonInterpreter::Instance();  
  interp.Start();
}

}}}

namespace {

static void sigint_handler(int) 
{ 
  std::cerr << "shutting down..." << std::endl; 
  QApplication::exit(-1);
} 

static void reclaim_signals() 
{ 
#ifndef _MSC_VER
  struct sigaction sa; 
  sa.sa_sigaction=0; // first assign this in case sigaction is union 
  sa.sa_handler = sigint_handler; 
  sa.sa_mask = sigset_t(); 
  sa.sa_flags = 0; 
  sigaction(SIGINT, &sa, 0);
#endif
}

String get_ost_root()
{
  QDir dir(QApplication::applicationDirPath());

  #ifdef _MSC_VER
    dir.cdUp();
    dir.cdUp();
  #else
    dir.cdUp();
  #endif

  return dir.path().toStdString();
}

void setup_python_search_path(const String& root, PythonInterpreter& pi)
{
#ifdef _MSC_VER
  pi.AppendModulePath(QString::fromStdString(root+"\\lib\\openstructure"));
#else  
#  if (defined(__ppc64__) || defined(__x86_64__)) && !defined(__APPLE__)
  pi.AppendModulePath(QString::fromStdString(root+"/lib64/openstructure"));
#  else
  pi.AppendModulePath(QString::fromStdString(root+"/lib/openstructure"));
#  endif
#endif
  pi.AppendModulePath(".");  
}
  
int setup_resources(QApplication& app) 
{
  QResource qr(":/images");
   if(!qr.isValid()) {
     LOGN_ERROR("no valid /image Qt resource found");
     return -1;
   }
   int sizes[]={512,256,128,32,16, 0};
   int i=0;
   QIcon icon;
   while (sizes[i]>0) {
    icon.addFile(QString(":images/icon")+QString::number(sizes[i]), 
                 QSize(sizes[i], sizes[i]));
    ++i; 
   }
   app.setWindowIcon(icon);
   return 0;
}

int init_python_interpreter()
{
  // the order of these two calls is important!
  PythonInterpreter::Instance();
  reclaim_signals();
  //
  PythonInterpreter& py=PythonInterpreter::Instance();
  String root =get_ost_root();
  ost::SetPrefixPath(root);
  if(root == "") {
    return -1;
  }
  setup_python_search_path(root, py);
  py.RunCommand("from ost import *");  
  return 0;
}

void prepare_scripts(int argc, char** argv, PythonInterpreter& py)
{
  QTextStream stdin_stream(stdin);
  QString stdin_line;
  QString script_qstring("");
  stdin_line=stdin_stream.readLine();
  while (!stdin_line.isNull())
  {
    stdin_line.append("\n");
    script_qstring += stdin_line;
    stdin_line=stdin_stream.readLine();
  }
  for (int param_iter=1; param_iter<argc; ++param_iter) {
    py.AppendCommandlineArgument(QString(argv[param_iter]));
  }
  py.RunCommand(script_qstring);
}


}

// initialise gosty - the graphical open structure interpreter
int main(int argc, char** argv)
{
  int dummy_argc=1;
  QApplication app(dummy_argc,argv);
  QCoreApplication::setOrganizationName("OpenStructure");
  QCoreApplication::setOrganizationDomain("openstructure.org");
  QCoreApplication::setApplicationName(argv[0]);
  if (int rv=setup_resources(app)<0) {
    return rv;
  }
  if (int r=init_python_interpreter()<0) {
    return r;
  }
  PythonInterpreter& py_int=PythonInterpreter::Instance();
  // todo remove RunInitRC and replace with general call to run script (with dngrc as argument)
  //py_int.RunInitRC();
  prepare_scripts(argc,argv,py_int);
  //  delay all execution of python scripts after app.exec() has been called.
  ost::gui::detail::DelayedScriptExecutor delayed_executor;
  return app.exec();
}
