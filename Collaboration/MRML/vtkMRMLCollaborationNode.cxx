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

// MRML includes
#include "vtkMRMLCollaborationNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
//#include <vtkCollection.h>
// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCollaborationNode);

//----------------------------------------------------------------------------
vtkMRMLCollaborationNode::vtkMRMLCollaborationNode()
{
  this->connectorNodeName = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLCollaborationNode::~vtkMRMLCollaborationNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLCollaborationNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  // vtkMRMLWriteXMLBeginMacro(of);
  // vtkMRMLWriteXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
  // vtkMRMLWriteXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
  // vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  // vtkMRMLReadXMLBeginMacro(atts);
  // vtkMRMLReadXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
  // vtkMRMLReadXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
  // vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  // vtkMRMLCopyBeginMacro(anode);
  // vtkMRMLCopyEnumMacro(AngleMeasurementMode);
  // vtkMRMLCopyVectorMacro(OrientationRotationAxis, double, 3);
  // vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "connectorNodeName:   " << (this->connectorNodeName ? this->connectorNodeName : "nullptr") << "\n";

  // vtkMRMLPrintBeginMacro(os, indent);
  // vtkMRMLPrintEnumMacro(AngleMeasurementMode);
  // vtkMRMLPrintVectorMacro(OrientationRotationAxis, double, 3);
  // vtkMRMLPrintEndMacro();
}
