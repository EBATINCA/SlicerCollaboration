/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Collaboration Logic includes
#include <vtkSlicerCollaborationLogic.h>

// Collaboration includes
#include "qSlicerCollaborationModule.h"
#include "qSlicerCollaborationModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCollaborationModulePrivate
{
public:
  qSlicerCollaborationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCollaborationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCollaborationModulePrivate::qSlicerCollaborationModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCollaborationModule methods

//-----------------------------------------------------------------------------
qSlicerCollaborationModule::qSlicerCollaborationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCollaborationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCollaborationModule::~qSlicerCollaborationModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCollaborationModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerCollaborationModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollaborationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCollaborationModule::icon() const
{
  return QIcon(":/Icons/Collaboration.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollaborationModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollaborationModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerCollaborationModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerCollaborationModule
::createWidgetRepresentation()
{
  return new qSlicerCollaborationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCollaborationModule::createLogic()
{
  return vtkSlicerCollaborationLogic::New();
}
