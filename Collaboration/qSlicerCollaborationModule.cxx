/*==============================================================================

  Copyright (c) EBATINCA, S.L.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L., and
  development was supported by "ICEX Espana Exportacion e Inversiones" under
  the program "Inversiones de Empresas Extranjeras en Actividades de I+D
  (Fondo Tecnologico)- Convocatoria 2021", cofunded by the European Regional
  Development Fund (ERDF).

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
  return "The Slicer Collaboration Module allows real-time collaboration between two 3D Slicer instances";
}

//-----------------------------------------------------------------------------
QString qSlicerCollaborationModule::acknowledgementText() const
{
  return "This work was funded by the project 2021/47 ICEX Spain";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollaborationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Monica Garcia-Sevilla (ULPGC), David Garcia-Mato (Ebatinca S.L.), Csaba Pinter (Ebatinca S.L.)");
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
  return QStringList() << "SlicerCollaboration";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollaborationModule::dependencies() const
{
	return QStringList() << "OpenIGTLinkIF";
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
