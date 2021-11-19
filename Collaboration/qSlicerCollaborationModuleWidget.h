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

#ifndef __qSlicerCollaborationModuleWidget_h
#define __qSlicerCollaborationModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// MRML includes
#include "vtkMRMLTextNode.h"

#include "qSlicerCollaborationModuleExport.h"

class qSlicerCollaborationModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_COLLABORATION_EXPORT qSlicerCollaborationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCollaborationModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCollaborationModuleWidget();
  const char* selected_collab_node;

public slots:
  /// Update widget GUI from collaboration node
  void updateWidgetFromMRML();

protected slots:
  void setCollaborationNode(vtkMRMLNode*);
  void onConnectButtonClicked();
  /// Internal function to update the CollaborationConnector node based on the property widget
  void updateConnectorNode();

  void synchronizeSelectedNodes();
  void unsynchronizeSelectedNodes();
  void sendNodesForSynchronization();
  vtkMRMLTextNode* createTextOfDisplayNode(vtkMRMLNode* displayNode,char* nodeName, char* className);

protected:
  QScopedPointer<qSlicerCollaborationModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerCollaborationModuleWidget);
  Q_DISABLE_COPY(qSlicerCollaborationModuleWidget);
};

#endif
