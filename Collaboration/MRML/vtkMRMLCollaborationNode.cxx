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
#include "vtkMRMLCollaborationConnectorNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkStringArray.h>

// STD includes
#include <sstream>

const char* vtkMRMLCollaborationNode::CollaborationConnectorNodeReferenceRole = "CollaborationConnector";
const char* vtkMRMLCollaborationNode::CollaborationConnectorNodeReferenceMRMLAttributeName = "CollaborationConnectorNodeRef";
const char* vtkMRMLCollaborationNode::CollaborationSynchronizedNodesReferenceRole = "SynchronizedNodes";
const char* vtkMRMLCollaborationNode::CollaborationSynchronizedNodesReferenceMRMLAttributeName = "SynchronizedNodesNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCollaborationNode);

//----------------------------------------------------------------------------
vtkMRMLCollaborationNode::vtkMRMLCollaborationNode()
{
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

  // vtkMRMLPrintBeginMacro(os, indent);
  // vtkMRMLPrintEnumMacro(AngleMeasurementMode);
  // vtkMRMLPrintVectorMacro(OrientationRotationAxis, double, 3);
  // vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLCollaborationNode::SetCollaborationConnectorNodeID(const char* CollaborationConnectorNodeID)
{
	this->SetNodeReferenceID(this->GetCollaborationConnectorNodeReferenceRole(), CollaborationConnectorNodeID);
}

//---------------------------------------------------------------------------
const char* vtkMRMLCollaborationNode::GetCollaborationConnectorNodeID()
{
	return this->GetNodeReferenceID(this->GetCollaborationConnectorNodeReferenceRole());
}

//---------------------------------------------------------------------------
vtkMRMLCollaborationConnectorNode *vtkMRMLCollaborationNode::GetCollaborationConnectorNode()
{
	return vtkMRMLCollaborationConnectorNode::SafeDownCast(this->GetNodeReference(this->GetCollaborationConnectorNodeReferenceRole()));
}

//---------------------------------------------------------------------------
const char* vtkMRMLCollaborationNode::GetCollaborationConnectorNodeReferenceRole()
{
	return vtkMRMLCollaborationNode::CollaborationConnectorNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLCollaborationNode::GetCollaborationConnectorNodeReferenceMRMLAttributeName()
{
	return vtkMRMLCollaborationNode::CollaborationConnectorNodeReferenceMRMLAttributeName;
}

//---------------------------------------------------------------------------
void vtkMRMLCollaborationNode::AddCollaborationSynchronizedNodeID(const char* CollaborationSynchronizedNodeID)
{
	this->AddNodeReferenceID(this->GetCollaborationSynchronizedNodeReferenceRole(), CollaborationSynchronizedNodeID);
}

//---------------------------------------------------------------------------
vtkStringArray* vtkMRMLCollaborationNode::GetCollaborationSynchronizedNodeIDs()
{
	vtkStringArray* nodeIDs = vtkStringArray::New();
	int numberOfNodeReferences = this->GetNumberOfNodeReferences(this->GetCollaborationSynchronizedNodeReferenceRole());
	for (int n = 0; n < numberOfNodeReferences; ++n)
	{
		nodeIDs->InsertNextValue(this->GetNthNodeReferenceID(this->GetCollaborationSynchronizedNodeReferenceRole(), n));
	}
	return nodeIDs;
}

//---------------------------------------------------------------------------
vtkCollection* vtkMRMLCollaborationNode::GetCollaborationSynchronizedNodes()
{
	//return vtkMRMLCollaborationConnectorNode::SafeDownCast(this->GetNodeReference(this->GetCollaborationSynchronizedNodeReferenceRole()));
	vtkCollection* nodes = vtkCollection::New();
	int numberOfNodeReferences = this->GetNumberOfNodeReferences(this->GetCollaborationSynchronizedNodeReferenceRole());
	for (int n = 0; n < numberOfNodeReferences; ++n)
	{
		nodes->AddItem(this->GetNthNodeReference(this->GetCollaborationSynchronizedNodeReferenceRole(), n));
	}
	return nodes;
}

//---------------------------------------------------------------------------
const char* vtkMRMLCollaborationNode::GetCollaborationSynchronizedNodeReferenceRole()
{
	return vtkMRMLCollaborationNode::CollaborationSynchronizedNodesReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLCollaborationNode::GetCollaborationSynchronizedNodeReferenceMRMLAttributeName()
{
	return vtkMRMLCollaborationNode::CollaborationSynchronizedNodesReferenceMRMLAttributeName;
}

void vtkMRMLCollaborationNode::RemoveCollaborationSynchronizedNodeID(const char* CollaborationSynchronizedNodeID)
{
	int numberOfNodeReferences = this->GetNumberOfNodeReferences(this->GetCollaborationSynchronizedNodeReferenceRole());
	int removeNodeIndex = -1;
	for (int n = 0; n < numberOfNodeReferences; ++n)
	{
		const char* currentNodeID = (this->GetNthNodeReferenceID(this->GetCollaborationSynchronizedNodeReferenceRole(), n));
		if (std::string(currentNodeID) == std::string(CollaborationSynchronizedNodeID))
		{
			removeNodeIndex = n;
		}
	}
	if (removeNodeIndex != -1)
	{
		this->RemoveNthNodeReferenceID(this->GetCollaborationSynchronizedNodeReferenceRole(), removeNodeIndex);
	}
}

