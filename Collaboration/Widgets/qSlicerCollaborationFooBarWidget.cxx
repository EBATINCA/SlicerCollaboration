/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerCollaborationFooBarWidget.h"
#include "ui_qSlicerCollaborationFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Collaboration
class qSlicerCollaborationFooBarWidgetPrivate
  : public Ui_qSlicerCollaborationFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerCollaborationFooBarWidget);
protected:
  qSlicerCollaborationFooBarWidget* const q_ptr;

public:
  qSlicerCollaborationFooBarWidgetPrivate(
    qSlicerCollaborationFooBarWidget& object);
  virtual void setupUi(qSlicerCollaborationFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerCollaborationFooBarWidgetPrivate
::qSlicerCollaborationFooBarWidgetPrivate(
  qSlicerCollaborationFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerCollaborationFooBarWidgetPrivate
::setupUi(qSlicerCollaborationFooBarWidget* widget)
{
  this->Ui_qSlicerCollaborationFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerCollaborationFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerCollaborationFooBarWidget
::qSlicerCollaborationFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCollaborationFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerCollaborationFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerCollaborationFooBarWidget
::~qSlicerCollaborationFooBarWidget()
{
}
