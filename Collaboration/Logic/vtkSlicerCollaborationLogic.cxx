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
#include "vtkSlicerCollaborationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>

// Collaboration module includes
#include "vtkMRMLCollaborationNode.h"
#include "vtkMRMLCollaborationConnectorNode.h"
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTextNode.h>
#include <vtkXMLUtilities.h>
#include "vtkSlicerModelsLogic.h"
#include <vtkMRMLModelHierarchyNode.h>

//----------------------------------------------------------------------------
// Avatars
const char* vtkSlicerCollaborationLogic::AVATAR_HEAD_MODEL_NAME = "head";
const char* vtkSlicerCollaborationLogic::AVATAR_HANDPOINTL_MODEL_NAME = "handPoint_L";
const char* vtkSlicerCollaborationLogic::AVATAR_HANDPOINTR_MODEL_NAME = "handPoint_R";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCollaborationLogic);

//----------------------------------------------------------------------------
vtkSlicerCollaborationLogic::vtkSlicerCollaborationLogic()
{
  vtkSmartPointer<vtkMRMLCollaborationNode> collaborationNode;
  this->collaborationNodeSelected = nullptr;
}

//----------------------------------------------------------------------------
vtkSlicerCollaborationLogic::~vtkSlicerCollaborationLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerCollaborationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerCollaborationLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerCollaborationLogic::RegisterNodes()
{
  //assert(this->GetMRMLScene() != 0);
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene!");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLCollaborationNode"))
  {
      scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCollaborationNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLCollaborationConnectorNode"))
  {
      scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCollaborationConnectorNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerCollaborationLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerCollaborationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
    if (!node || !this->GetMRMLScene())
    {
        vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node!");
        return;
    }
    if (node->IsA("vtkMRMLCollaborationNode"))
    {
        // Check if a ConnectorNode for the new CollaborationNode exists
        vtkMRMLCollaborationNode* collaborationNode = vtkMRMLCollaborationNode::SafeDownCast(node);
        const char* collabconnectorNodeID = collaborationNode->GetCollaborationConnectorNodeID();

        if (!collabconnectorNodeID) {
            // Create a ConnectorNode
            vtkMRMLCollaborationConnectorNode* connectorNode =
                vtkMRMLCollaborationConnectorNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLCollaborationConnectorNode"));
            this->GetMRMLScene()->AddNode(connectorNode);
            // Set the same name as the Collaboration Node + Connector
            char* collaborationNodeName = node->GetName();
            char connectorName[] = "Connector";
            char* connectorNodeName = new char[std::strlen(collaborationNodeName) + std::strlen(connectorName) + 1];
            std::strcpy(connectorNodeName, collaborationNodeName);
            std::strcat(connectorNodeName, connectorName);
            connectorNode->SetName(connectorNodeName);
            const char* connectorNodeID = connectorNode->GetID();
            //collaborationNode->connectorNodeID = connectorNodeID;
            collaborationNode->SetCollaborationConnectorNodeID(connectorNodeID);
            connectorNode->SetType(0);
        }
        this->Modified();
    }
    else if (node->IsA("vtkMRMLModelNode") || node->IsA("vtkMRMLLinearTransformNode") || 
        node->IsA("vtkMRMLMarkupsNode") || node->IsA("vtkMRMLTextNode") || node->IsA("vtkMRMLScalarVolumeNode"))
    {
        // fiducials do not include the description of "Received by OpenIGTLink"
        if (node->IsA("vtkMRMLMarkupsFiducialNode"))
        {
            const char* connectorNodeID = this->collaborationNodeSelected->GetCollaborationConnectorNodeID();
            vtkMRMLCollaborationConnectorNode* connectorNode = 
                vtkMRMLCollaborationConnectorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(connectorNodeID));
            int state = connectorNode->GetState();
            // if connection is active, we are assuming it was received through OpenIGTLink
            if (state == 2)
            {
                node->SetDescription("Received by OpenIGTLink");
            }
        }
        // check if it was received through an OpenIGTLink connection
        const char* nodeDescription = node->GetDescription();
        if (nodeDescription) {
            if (strcmp(nodeDescription, "Received by OpenIGTLink") == 0)
            {
                // set attribute of the collaboration node to the added node
                node->SetAttribute(this->collaborationNodeSelected->GetID(), "true");
                // add node reference to the collaboration node
                if (this->collaborationNodeSelected)
                {
                    this->collaborationNodeSelected->AddCollaborationSynchronizedNodeID(node->GetID());

                }
                // update transforms
                // get transformed nodes
                vtkStringArray* collection = this->collaborationNodeSelected->GetCollaborationSynchronizedNodeIDs();
                std::string transformedNodesText = "";
                for (int i = 0; i < collection->GetNumberOfTuples(); i++)
                {
                    std::string ID = collection->GetValue(i);
                    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(ID));
                    if (node->IsA("vtkMRMLLinearTransformNode"))
                    {
                        const char* textNodeReferenceID = node->GetNthNodeReferenceID("TextNode", 0);
                        vtkMRMLTextNode* transformTextNode = 
                            vtkMRMLTextNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(textNodeReferenceID));
                        if (transformTextNode)
                        {
                            std::stringstream ss;
                            ss << transformTextNode->GetText();
                            vtkXMLDataElement* res =
                                vtkXMLUtilities::ReadElementFromStream(ss);
                            this->orderTransforms(res);
                        }
                    }
                }  
            }
        }
    }
}

void vtkSlicerCollaborationLogic::orderTransforms(vtkXMLDataElement* res)
{
    // read attributes
    int numberOfAttributes = res->GetNumberOfAttributes();
    std::vector<const char*> atts_v;
    for (int attributeIndex = 0; attributeIndex < numberOfAttributes; attributeIndex++)
    {
        const char* attName = res->GetAttributeName(attributeIndex);
        const char* attValue = res->GetAttribute(attName);
        atts_v.push_back(attName);
        atts_v.push_back(attValue);
    }
    atts_v.push_back(nullptr);
    const char** atts = (atts_v.data());

    // get transform node
    std::string transformName = res->GetAttribute("TransformName");
    vtkMRMLLinearTransformNode* transformNode = 
        vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(transformName.c_str()));

    // get transformed nodes attribute
    std::string transformedNodesStr = res->GetAttribute("TransformedNodes");

    // get every coordinate
    std::stringstream ss(transformedNodesStr);
    std::vector<std::string> vect;
    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        vect.push_back(substr);
    }
    int numberOfTransformedNodes = vect.size();
    for (int i = 0; i < numberOfTransformedNodes; i++)
    {
        vtkMRMLTransformableNode* node = 
            vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByName(vect[i].c_str()));
        if (node)
        {
            node->SetAndObserveTransformNodeID(transformNode->GetID());
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCollaborationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or input node!");
    return;
  }
  if (node->IsA("vtkMRMLCollaborationNode"))
  {
    vtkMRMLCollaborationNode* collaborationNode = vtkMRMLCollaborationNode::SafeDownCast(node);
    // Get the connector node associated to the collaboration node
    const char* connectorNodeID = collaborationNode->GetCollaborationConnectorNodeID();
    vtkMRMLCollaborationConnectorNode* connectorNode = 
        vtkMRMLCollaborationConnectorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(connectorNodeID));
    this->GetMRMLScene()->RemoveNode(connectorNode);
    collaborationNode->SetCollaborationConnectorNodeID(nullptr);
    this->Modified();
  }
}

void vtkSlicerCollaborationLogic::loadAvatars()
{
    // get directory
    std::string moduleShareDirectory = this->GetModuleShareDirectory();

    // Create a models logic for convenient loading of components
    vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::New();
    modelsLogic->SetMRMLScene(this->GetMRMLScene());

    // Load basic additional device models

    // Head
    std::string headModelFilePath = moduleShareDirectory + "/" + AVATAR_HEAD_MODEL_NAME + ".stl";
    vtkMRMLModelNode* headModelNode = modelsLogic->AddModel(headModelFilePath.c_str());
    
    // Left hand
    std::string handPointLModelFilePath = moduleShareDirectory + "/" + AVATAR_HANDPOINTL_MODEL_NAME + ".stl";
    vtkMRMLModelNode* handPointLModelNode = modelsLogic->AddModel(handPointLModelFilePath.c_str());

    // Right hand
    std::string handPointRModelFilePath = moduleShareDirectory + "/" + AVATAR_HANDPOINTR_MODEL_NAME + ".stl";
    vtkMRMLModelNode* handPointRModelNode = modelsLogic->AddModel(handPointRModelFilePath.c_str());
}
