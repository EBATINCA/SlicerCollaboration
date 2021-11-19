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

#ifndef __vtkMRMLCollaborationConnectorNode_h
#define __vtkMRMLCollaborationConnectorNode_h

// OpenIGTLink includes
#include "vtkMRMLIGTLConnectorNode.h"

// std includes
#include <vtkXMLDataElement.h>

// Collaboration includes
#include "vtkSlicerCollaborationModuleMRMLExport.h"

/// \brief TODO
/// TODO
class VTK_SLICER_COLLABORATION_MODULE_MRML_EXPORT vtkMRMLCollaborationConnectorNode : public vtkMRMLIGTLConnectorNode
{
public:
  static vtkMRMLCollaborationConnectorNode *New();
  vtkTypeMacro(vtkMRMLCollaborationConnectorNode, vtkMRMLIGTLConnectorNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "CollaborationConnector";}

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLCollaborationConnectorNode);

protected:
  vtkMRMLCollaborationConnectorNode();
  ~vtkMRMLCollaborationConnectorNode() override;
  vtkMRMLCollaborationConnectorNode(const vtkMRMLCollaborationConnectorNode&);
  void operator=(const vtkMRMLCollaborationConnectorNode&);

  unsigned int AssignOutGoingNodeToDevice(vtkMRMLNode* node, igtlioDevicePointer device) override;
  vtkMRMLNode* CreateNewMRMLNodeForDevice(igtlioDevice* device) override;
  void ProcessIncomingDeviceModifiedEvent(vtkObject* caller, unsigned long event, igtlioDevice* modifiedDevice) override;
  void addMarkupsNode(vtkXMLDataElement* res);
  void addDisplayNode(vtkXMLDataElement* res);
};

#endif
