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
#include "vtkMRMLCollaborationConnectorNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLTextNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkXMLUtilities.h>
#include <vtkXMLDataElement.h>
#include <vtkPolyData.h>

// STD includes
#include <sstream>
#include <vtkXMLDataElement.h>
#include <strstream>

// OpenIGTLinkIO include
#include <igtlioPolyDataDevice.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCollaborationConnectorNode);

//----------------------------------------------------------------------------
vtkMRMLCollaborationConnectorNode::vtkMRMLCollaborationConnectorNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLCollaborationConnectorNode::~vtkMRMLCollaborationConnectorNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of,nIndent);

    // vtkMRMLWriteXMLBeginMacro(of);
    // vtkMRMLWriteXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
    // vtkMRMLWriteXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
    // vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::ReadXMLAttributes(const char** atts)
{
    Superclass::ReadXMLAttributes(atts);

    // vtkMRMLReadXMLBeginMacro(atts);
    // vtkMRMLReadXMLEnumMacro(angleMeasurementMode, AngleMeasurementMode);
    // vtkMRMLReadXMLVectorMacro(orientationRotationAxis, OrientationRotationAxis, double, 3);
    // vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
    MRMLNodeModifyBlocker blocker(this);
    Superclass::CopyContent(anode, deepCopy);

    // vtkMRMLCopyBeginMacro(anode);
    // vtkMRMLCopyEnumMacro(AngleMeasurementMode);
    // vtkMRMLCopyVectorMacro(OrientationRotationAxis, double, 3);
    // vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);

    // vtkMRMLPrintBeginMacro(os, indent);
    // vtkMRMLPrintEnumMacro(AngleMeasurementMode);
    // vtkMRMLPrintVectorMacro(OrientationRotationAxis, double, 3);
    // vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLCollaborationConnectorNode::AssignOutGoingNodeToDevice(vtkMRMLNode* node, igtlioDevicePointer device)
{
    std::cout << "AssignOutGoingNodeToDevice" << "\n";
    return Superclass::AssignOutGoingNodeToDevice(node, device);
}

vtkMRMLNode* vtkMRMLCollaborationConnectorNode::CreateNewMRMLNodeForDevice(igtlioDevice* device)
{
    vtkMRMLScene* scene = this->GetScene();
    if (!scene)
    {
        return nullptr;
    }

    const std::string deviceType = device->GetDeviceType();
    std::string deviceName = device->GetDeviceName();
    // Device name is empty, we will not be able to find a node in the scene
    if (device->GetDeviceName().empty())
    {
        deviceName = "OpenIGTLink";
    }
    if (strcmp(device->GetDeviceType().c_str(), "STRING") == 0)
    {
        vtkSmartPointer<vtkMRMLTextNode> textNode =
            vtkMRMLTextNode::SafeDownCast(this->GetScene()->GetFirstNode(deviceName.c_str(), "vtkMRMLTextNode"));
        if (textNode)
        {
            this->RegisterIncomingMRMLNode(textNode, device);
            return textNode;
        }

        igtlioStringDevice* modifiedDevice = reinterpret_cast<igtlioStringDevice*>(device);
        textNode = vtkSmartPointer<vtkMRMLTextNode>::Take(vtkMRMLTextNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLTextNode")));
        textNode->SetEncoding(modifiedDevice->GetContent().encoding);
        textNode->SetText(modifiedDevice->GetContent().string_msg.c_str());
        textNode->SetName(deviceName.c_str());
        textNode->SetDescription("Received by OpenIGTLink");
        // hide in case it contains the display node attributes
        textNode->SetHideFromEditors(1);
        this->GetScene()->AddNode(textNode);
        this->RegisterIncomingMRMLNode(textNode, device);
        return textNode;
    }
    return Superclass::CreateNewMRMLNodeForDevice(device);
}

void vtkMRMLCollaborationConnectorNode::ProcessIncomingDeviceModifiedEvent(vtkObject* caller, unsigned long event, igtlioDevice* modifiedDevice)
{
    vtkMRMLNode* modifiedNode = this->GetMRMLNodeForDevice(modifiedDevice);
    bool isNewNodeCreated = false;
    if (!modifiedNode)
    {
        // Could not find node, create new node

        modifiedNode = this->CreateNewMRMLNodeForDevice(modifiedDevice);
        // new node created, notify other class about the creation of new node at the end of this function when all content are assigned to the new node.
        isNewNodeCreated = true;
    }
    if (!modifiedNode)
    {
        // Could not add node.
        return;
    }

    int wasModifyingNode = modifiedNode->StartModify();

    const std::string deviceType = modifiedDevice->GetDeviceType();
    std::string deviceName = modifiedDevice->GetDeviceName();
    if (deviceName.empty())
    {
        deviceName = "OpenIGTLink";
    }
    if (this->GetNodeTagFromDeviceType(deviceType.c_str()).size() > 0)
    {
        if (strcmp(deviceType.c_str(), "STRING") == 0)
        {
            igtlioStringDevice* stringDevice = reinterpret_cast<igtlioStringDevice*>(modifiedDevice);
            // read text to see if it is the XML of a display node
            std::string text = stringDevice->GetContent().string_msg;
            std::stringstream ss;
            ss << text;
            vtkXMLDataElement* res =
                vtkXMLUtilities::ReadElementFromStream(ss, stringDevice->GetContent().encoding);
            if (res)
            {
                int numberOfAttributes = res->GetNumberOfAttributes();
                // if it is a ModelDisplayNode
                if (strcmp(res->GetAttribute("ClassName"), "vtkMRMLModelDisplayNode") == 0)
                {
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
                    // get the display node from the corresponding model
                    const char* modelName = res->GetAttribute("ModelName");
                    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(modelName));
                    // if the model already exists in the scene, apply the display node
                    if (modelNode)
                    {
                        vtkMRMLModelDisplayNode* currentDisplayNode = modelNode->GetModelDisplayNode();
                        if (currentDisplayNode)
                        {
                            // create display node and apply attributes
                            vtkMRMLModelDisplayNode* newDisplayNode = vtkMRMLModelDisplayNode::New();
                            newDisplayNode->ReadXMLAttributes(atts);
                            // copy display node attributes to the current display node
                            currentDisplayNode->Copy(newDisplayNode);
                            // set name
                            char displayName[] = "DisplayNode";
                            char* displayNodeName = new char[std::strlen(modelName) + std::strlen(displayName) + 1];
                            std::strcpy(displayNodeName, modelName);
                            std::strcat(displayNodeName, displayName);
                            currentDisplayNode->SetName(displayNodeName);
                            currentDisplayNode->Modified();
                            modelNode->Modified();
                        }
                    }
                    else
                    {
                        vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::New();
                        // apply attributes
                        displayNode->ReadXMLAttributes(atts);
                        // set name
                        char displayName[] = "DisplayNode";
                        char* displayNodeName = new char[std::strlen(modelName) + std::strlen(displayName) + 1];
                        std::strcpy(displayNodeName, modelName);
                        std::strcat(displayNodeName, displayName);
                        displayNode->SetName(displayNodeName);
                        // add display node to scene
                        this->GetScene()->AddNode(displayNode);
                        displayNode->Modified();
                    }
                }
            }
            else
            {
                vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(modifiedNode);
                textNode->SetEncoding(stringDevice->GetContent().encoding);
                textNode->SetText(stringDevice->GetContent().string_msg.c_str());
                // make it visible as it does not contain the display node attributes
                textNode->SetHideFromEditors(0);
                textNode->Modified();
            }
        }
        else if (strcmp(deviceType.c_str(), "POLYDATA") == 0)
        {
            igtlioPolyDataDevice* polyDevice = reinterpret_cast<igtlioPolyDataDevice*>(modifiedDevice);
            vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(modifiedNode);
            modelNode->SetAndObservePolyData(polyDevice->GetContent().polydata);
            // see if the display node was already defined
            char* modelNodeName = modelNode->GetName();
            char displayName[] = "DisplayNode";
            char* displayNodeName = new char[std::strlen(modelNodeName) + std::strlen(displayName) + 1];
            std::strcpy(displayNodeName, modelNodeName);
            std::strcat(displayNodeName, displayName);
            vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(displayNodeName));
            if (displayNode)
            {
                vtkMRMLModelDisplayNode* currentDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
                currentDisplayNode->Copy(displayNode);
                displayNode->Modified();
            }
            modelNode->Modified();
        }
    }
    Superclass::ProcessIncomingDeviceModifiedEvent(caller, event, modifiedDevice);
}



