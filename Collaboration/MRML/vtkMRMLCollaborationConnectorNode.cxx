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

#include "vtkMRMLCollaborationConnectorNode.h"

// Slicer MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLTextNode.h"
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsAngleNode.h>
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>

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
void vtkMRMLCollaborationConnectorNode::WriteXML(ostream & of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::CopyContent(vtkMRMLNode * anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::PrintSelf(ostream & os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLCollaborationConnectorNode::AssignOutGoingNodeToDevice(vtkMRMLNode * node, igtlioDevicePointer device)
{
  return Superclass::AssignOutGoingNodeToDevice(node, device);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLCollaborationConnectorNode::CreateNewMRMLNodeForDevice(igtlioDevice * device)
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

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::ProcessIncomingDeviceModifiedEvent(vtkObject * caller, unsigned long event, igtlioDevice * modifiedDevice)
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
        if (strcmp(res->GetAttribute("SuperclassName"), "vtkMRMLDisplayNode") == 0)
        {
          addDisplayNode(res);
        }
        // if it is a markups (non fiducial) node
        else if (strcmp(res->GetAttribute("SuperclassName"), "vtkMRMLMarkupsNode") == 0)
        {
          addMarkupsNode(res);
        }
        else if (strcmp(res->GetAttribute("SuperclassName"), "vtkMRMLTransformNode") == 0)
        {
          orderTransforms(res);
          vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(modifiedNode);
          textNode->SetEncoding(stringDevice->GetContent().encoding);
          textNode->SetText(stringDevice->GetContent().string_msg.c_str());
          // set name
          const char* transformNodeName = res->GetAttribute("TransformName");
          char textName[] = "Text";
          char* transformTextNodeName = new char[std::strlen(transformNodeName) + std::strlen(textName) + 1];
          std::strcpy(transformTextNodeName, transformNodeName);
          std::strcat(transformTextNodeName, textName);
          textNode->SetName(transformTextNodeName);
          // make it visible as it does not contain the display node attributes
          textNode->SetHideFromEditors(0);
          textNode->Modified();
          // if transform already arrived, add reference
          vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(transformNodeName));
          if (transformNode)
          {
            // add node reference to the markups node
            transformNode->AddNodeReferenceRole("TextNode");
            transformNode->AddNodeReferenceID("TextNode", textNode->GetID());
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
    else if (strcmp(deviceType.c_str(), "TRANSFORM") == 0)
    {
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(modifiedNode);
      // see if the text node was already defined and if so, apply it
      char* transformNodeName = transformNode->GetName();
      char textName[] = "Text";
      char* transformTextNodeName = new char[std::strlen(transformNodeName) + std::strlen(textName) + 1];
      std::strcpy(transformTextNodeName, transformNodeName);
      std::strcat(transformTextNodeName, textName);
      vtkMRMLTextNode* transformTextNode = vtkMRMLTextNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(transformTextNodeName));
      if (transformTextNode)
      {
        std::stringstream ss;
        ss << transformTextNode->GetText();
        vtkXMLDataElement* res =
          vtkXMLUtilities::ReadElementFromStream(ss);
        orderTransforms(res);
        // if text node already arrived, add node reference
        transformNode->AddNodeReferenceRole("TextNode");
        transformNode->AddNodeReferenceID("TextNode", transformTextNode->GetID());
      }
    }
  }
  Superclass::ProcessIncomingDeviceModifiedEvent(caller, event, modifiedDevice);
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::orderTransforms(vtkXMLDataElement * res)
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
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(transformName.c_str()));

  // get transformed nodes attribute
  std::string transformedNodesStr = res->GetAttribute("TransformedNodes");

  // get every transformed node
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
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(vect[i].c_str()));
    if (node && transformNode)
    {
      node->SetAndObserveTransformNodeID(transformNode->GetID());
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::addMarkupsNode(vtkXMLDataElement * res)
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

  // get control points attribute
  std::string controlPointsStr = res->GetAttribute("ControlPoints");

  // get every control point, separated by ;
  std::vector<std::string> vectControlPoints;
  std::stringstream ss2(controlPointsStr);
  while (ss2.good())
  {
    std::string substr;
    getline(ss2, substr, ';');
    vectControlPoints.push_back(substr);
  }
  std::vector<vtkMRMLMarkupsNode::ControlPoint*> ControlPointsList;
  for (int controlPointIndex = 0; controlPointIndex < vectControlPoints.size(); controlPointIndex++)
  {
    // get string of control point
    std::string token = vectControlPoints[controlPointIndex];
    // remove brackets
    std::string brackets = "[]";
    for (char c : brackets)
    {
      token.erase(std::remove(token.begin(), token.end(), c), token.end());
    }
    // get every coordinate
    std::stringstream ss(token);
    std::vector<std::string> vect;
    while (ss.good())
    {
      std::string substr;
      getline(ss, substr, ',');
      vect.push_back(substr);
    }
    // add control point to list
    vtkMRMLMarkupsNode::ControlPoint* newControlPoint = new vtkMRMLMarkupsNode::ControlPoint;
    newControlPoint->Position[0] = atof(vect[0].c_str());
    newControlPoint->Position[1] = atof(vect[1].c_str());
    newControlPoint->Position[2] = atof(vect[2].c_str());
    newControlPoint->PositionStatus = true;
    ControlPointsList.push_back(newControlPoint);
  }
  // see if node exists
  const char* nodeName = res->GetAttribute("name");
  std::string className = res->GetAttribute("ClassName");
  vtkMRMLNode* exisitingNode = vtkMRMLNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
  std::string existingNodeClassName = "";
  bool nodeExists = false;
  if (exisitingNode)
  {
    existingNodeClassName = exisitingNode->GetClassName();
  }
  if (strcmp(className.c_str(), existingNodeClassName.c_str()) == 0)
  {
    nodeExists = true;
  }
  if (className == "vtkMRMLMarkupsFiducialNode")
  {
    vtkMRMLMarkupsFiducialNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsFiducialNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsLineNode")
  {
    vtkMRMLMarkupsLineNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsLineNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsLineNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsPlaneNode")
  {
    vtkMRMLMarkupsPlaneNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsPlaneNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsAngleNode")
  {
    vtkMRMLMarkupsAngleNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsAngleNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsAngleNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsCurveNode")
  {
    vtkMRMLMarkupsCurveNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsCurveNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsCurveNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsClosedCurveNode")
  {
    vtkMRMLMarkupsClosedCurveNode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsClosedCurveNode::New();
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }
    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
  else if (className == "vtkMRMLMarkupsROINode")
  {
    vtkMRMLMarkupsROINode* markupsNodeFinal;
    if (nodeExists)
    {
      markupsNodeFinal = vtkMRMLMarkupsROINode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    }
    else
    {
      markupsNodeFinal = vtkMRMLMarkupsROINode::New();
    }

    // get ROI radius attribute
    std::string ROIRadiusStr = res->GetAttribute("ROIRadius");
    // remove brackets
    std::string brackets = "[]";
    for (char c : brackets)
    {
      ROIRadiusStr.erase(std::remove(ROIRadiusStr.begin(), ROIRadiusStr.end(), c), ROIRadiusStr.end());
    }
    // get every coordinate
    std::stringstream ss(ROIRadiusStr);
    std::vector<std::string> vect;
    while (ss.good())
    {
      std::string substr;
      getline(ss, substr, ',');
      vect.push_back(substr);
    }
    // apply attributes
    markupsNodeFinal->ReadXMLAttributes(atts);
    // apply control points
    int numberOfControlPoints = ControlPointsList.size();
    markupsNodeFinal->RemoveAllControlPoints();
    for (int cp = 0; cp < numberOfControlPoints; cp++)
    {
      markupsNodeFinal->AddControlPoint(ControlPointsList[cp], true);
    }

    // apply ROI radius
    double ROIrad[3] = { 0.0 };
    ROIrad[0] = std::stod(vect[0].c_str());
    ROIrad[1] = std::stod(vect[1].c_str());
    ROIrad[2] = std::stod(vect[2].c_str());
    markupsNodeFinal->SetRadiusXYZ(ROIrad);

    this->GetScene()->AddNode(markupsNodeFinal);
    markupsNodeFinal->UpdateAllMeasurements();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLCollaborationConnectorNode::addDisplayNode(vtkXMLDataElement * res)
{
  std::vector<const char*> atts_v;
  int numberOfAttributes = res->GetNumberOfAttributes();
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
  const char* nodeName = res->GetAttribute("NodeName");
  // get node type
  const char* className = res->GetAttribute("ClassName");
  if (strcmp(className, "vtkMRMLModelDisplayNode") == 0)
  {
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
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
        char* displayNodeName = new char[std::strlen(nodeName) + std::strlen(displayName) + 1];
        std::strcpy(displayNodeName, nodeName);
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
      char* displayNodeName = new char[std::strlen(nodeName) + std::strlen(displayName) + 1];
      std::strcpy(displayNodeName, nodeName);
      std::strcat(displayNodeName, displayName);
      displayNode->SetName(displayNodeName);
      // add display node to scene
      this->GetScene()->AddNode(displayNode);
      displayNode->Modified();
    }
  }
  if (strcmp(className, "vtkMRMLMarkupsDisplayNode") == 0)
  {
    vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->GetScene()->GetFirstNodeByName(nodeName));
    // if the model already exists in the scene, apply the display node
    if (markupsNode)
    {
      vtkMRMLMarkupsDisplayNode* currentDisplayNode = markupsNode->GetMarkupsDisplayNode();
      if (currentDisplayNode)
      {
        // create display node and apply attributes
        vtkMRMLMarkupsDisplayNode* newDisplayNode = vtkMRMLMarkupsDisplayNode::New();
        newDisplayNode->ReadXMLAttributes(atts);
        // copy display node attributes to the current display node
        currentDisplayNode->Copy(newDisplayNode);
        // set name
        char displayName[] = "DisplayNode";
        char* displayNodeName = new char[std::strlen(nodeName) + std::strlen(displayName) + 1];
        std::strcpy(displayNodeName, nodeName);
        std::strcat(displayNodeName, displayName);
        currentDisplayNode->SetName(displayNodeName);
        currentDisplayNode->Modified();
        markupsNode->Modified();
      }
    }
    else
    {
      vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::New();
      // apply attributes
      displayNode->ReadXMLAttributes(atts);
      // set name
      char displayName[] = "DisplayNode";
      char* displayNodeName = new char[std::strlen(nodeName) + std::strlen(displayName) + 1];
      std::strcpy(displayNodeName, nodeName);
      std::strcat(displayNodeName, displayName);
      displayNode->SetName(displayNodeName);
      // add display node to scene
      this->GetScene()->AddNode(displayNode);
      displayNode->Modified();
    }
  }
}


