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

#ifndef __vtkMRMLCollaborationNode_h
#define __vtkMRMLCollaborationNode_h

// MRML includes
#include "vtkMRMLNode.h"
#include <vtkStringArray.h>
#include <vtkCollection.h>

//#include "vtkMRMLCollaborationConnectorNode.h"
class vtkMRMLCollaborationConnectorNode;
// Collaboration includes
#include "vtkSlicerCollaborationModuleMRMLExport.h"

/// \brief TODO
/// TODO
class VTK_SLICER_COLLABORATION_MODULE_MRML_EXPORT vtkMRMLCollaborationNode : public vtkMRMLNode
{
public:
  static vtkMRMLCollaborationNode *New();
  vtkTypeMacro(vtkMRMLCollaborationNode, vtkMRMLNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "CollaborationNode";}

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLCollaborationNode);

  void SetCollaborationConnectorNodeID(const char* CollaborationConnectorNodeID);
  const char* GetCollaborationConnectorNodeID();
  vtkMRMLCollaborationConnectorNode* GetCollaborationConnectorNode();
  const char* GetCollaborationConnectorNodeReferenceRole(); // virtual
  // const char* GetCollaborationConnectorNodeReferenceMRMLAttributeName();

  void AddCollaborationSynchronizedNodeID(const char* CollaborationSynchronizedNodeID);
  vtkStringArray* GetCollaborationSynchronizedNodeIDs();
  vtkCollection* GetCollaborationSynchronizedNodes();
  const char* GetCollaborationSynchronizedNodeReferenceRole(); // virtual
  void RemoveCollaborationSynchronizedNodeID(const char* CollaborationSynchronizedNodeID);

protected:
  vtkMRMLCollaborationNode();
  ~vtkMRMLCollaborationNode() override;
  vtkMRMLCollaborationNode(const vtkMRMLCollaborationNode&);
  void operator=(const vtkMRMLCollaborationNode&);

  virtual const char* GetCollaborationConnectorNodeReferenceMRMLAttributeName();
  virtual const char* GetCollaborationSynchronizedNodeReferenceMRMLAttributeName();

  static const char* CollaborationConnectorNodeReferenceRole;
  static const char* CollaborationConnectorNodeReferenceMRMLAttributeName;
  static const char* CollaborationSynchronizedNodesReferenceRole;
  static const char* CollaborationSynchronizedNodesReferenceMRMLAttributeName;
};

#endif
