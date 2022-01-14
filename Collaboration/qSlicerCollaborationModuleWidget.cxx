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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerCollaborationModuleWidget.h"
#include "ui_qSlicerCollaborationModuleWidget.h"
#include "qSlicerModuleManager.h"

// Colaboration module includes
#include "vtkMRMLCollaborationNode.h"
#include "vtkMRMLCollaborationConnectorNode.h"

// MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "vtkCollection.h"
#include "qMRMLSubjectHierarchyModel.h";
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLTextNode.h"
#include "vtkMRMLModelNode.h"

// Logic includes
#include "vtkSlicerCollaborationLogic.h"
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsAngleNode.h>
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkXMLUtilities.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <qSlicerApplication.h>
#include <qSlicerAbstractModule.h>
#include <ctkCheckBox.h>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCollaborationModuleWidgetPrivate: public Ui_qSlicerCollaborationModuleWidget
{
public:
  qSlicerCollaborationModuleWidgetPrivate();

};

//-----------------------------------------------------------------------------
// qSlicerCollaborationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCollaborationModuleWidgetPrivate::qSlicerCollaborationModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCollaborationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCollaborationModuleWidget::qSlicerCollaborationModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCollaborationModuleWidgetPrivate )
{
    this->selected_collab_node = "None";
}

//-----------------------------------------------------------------------------
qSlicerCollaborationModuleWidget::~qSlicerCollaborationModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCollaborationModuleWidget::setup()
{
  Q_D(qSlicerCollaborationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Icons
  QIcon rightIcon =
      QApplication::style()->standardIcon(QStyle::SP_ArrowRight);
  d->SynchronizeToolButton->setIcon(rightIcon);

  QIcon leftIcon =
      QApplication::style()->standardIcon(QStyle::SP_ArrowLeft);
  d->UnsynchronizeToolButton->setIcon(leftIcon);

  connect(d->MRMLNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setCollaborationNode(vtkMRMLNode*)));
  connect(d->connectButton, SIGNAL(clicked()), this, SLOT(onConnectButtonClicked()));
  connect(d->connectVRButton, SIGNAL(clicked()), this, SLOT(onConnectVRButtonClicked()));
  connect(d->LoadAvatarsButton, SIGNAL(clicked()), this, SLOT(onLoadAvatarsButtonClicked()));
  // Update connector node values when parameter values are modified in the GUI
  connect(d->serverModeRadioButton, SIGNAL(clicked()), this, SLOT(updateConnectorNode()));
  connect(d->clientModeRadioButton, SIGNAL(clicked()), this, SLOT(updateConnectorNode()));
  connect(d->portLineEdit, SIGNAL(editingFinished()), this, SLOT(updateConnectorNode()));
  connect(d->hostNameLineEdit, SIGNAL(editingFinished()), this, SLOT(updateConnectorNode()));
  // Synchronize nodes
    // Transform nodes connection
  connect(d->SynchronizeToolButton, SIGNAL(clicked()),
      SLOT(synchronizeSelectedNodes()));
  connect(d->UnsynchronizeToolButton, SIGNAL(clicked()),
      SLOT(unsynchronizeSelectedNodes()));
  // Send nodes selected for synchronization
  connect(d->sendButton, SIGNAL(clicked()), this, SLOT(sendNodesForSynchronization()));

  // add the nodes selected for synchronization
  d->SynchronizedTreeView->addNodeAttributeFilter(selected_collab_node);
  d->SynchronizedTreeView->model()->invalidateFilter();
  // exclude the nodes selected for synchronization
  d->AvailableNodesTreeView->addNodeAttributeFilter(selected_collab_node, "true", false);
  d->AvailableNodesTreeView->model()->invalidateFilter();

  // create callback to update text nodes when markups or display nodes are updated
  this->updateTextCallback = vtkCallbackCommand::New();
  this->updateTextCallback->SetClientData(reinterpret_cast<void*>(this));
  this->updateTextCallback->SetCallback(qSlicerCollaborationModuleWidget::nodeUpdated);
}

void qSlicerCollaborationModuleWidget::setCollaborationNode(vtkMRMLNode* node)
{
    Q_D(qSlicerCollaborationModuleWidget);

    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(node);

    // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
    d->MRMLNodeComboBox->setCurrentNode(collabNode);

    if (this->logic() == nullptr)
    {
        return;
    }
    vtkSlicerCollaborationLogic* collaborationLogic = vtkSlicerCollaborationLogic::SafeDownCast(this->logic());
    if (collaborationLogic)
    {
        collaborationLogic->collaborationNodeSelected = collabNode;
    }

    // Each time the node is modified, the qt widgets are updated
    qvtkReconnect(collabNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
    // remove the current attribute filter for the tree view
    d->SynchronizedTreeView->removeNodeAttributeFilter(selected_collab_node, true);
    d->AvailableNodesTreeView->removeNodeAttributeFilter(selected_collab_node, false);
    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));
            //connectorNodeID));

        if (connectorNode) {
            // if the conenctor node is new, get the current parameter values
            if (connectorNode->GetType() == 0) {
                std::string hostName;
                std::string portNumber;
                connectorNode->SetTypeClient("localhost", 18944);
            }
            // Enable Connect button
            d->connectButton->setEnabled(true);
        }
        // update the filter attributed with the selected collaboration node
        selected_collab_node = collabNode->GetID();
    }
    else {
        // Disable Connect button
        d->connectButton->setEnabled(false);
        // set the filter attributed to None
        selected_collab_node = "None";
    }
    // add an attribute filter to the tree view for the selected collaboration node
    d->SynchronizedTreeView->addNodeAttributeFilter(selected_collab_node);
    d->SynchronizedTreeView->model()->invalidateFilter();
    // add an attribute filter to exclude the nodes selectede for synchronization
    d->AvailableNodesTreeView->addNodeAttributeFilter(selected_collab_node, "true", false);
    d->AvailableNodesTreeView->model()->invalidateFilter();

    this->updateWidgetFromMRML();
}

void qSlicerCollaborationModuleWidget::updateWidgetFromMRML()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));

        if (connectorNode && this->mrmlScene())
        {
            // Update parameter values on GUI with those from the current CollaborationConnector node
            int connectorType = connectorNode->GetType();
            int serverPort = connectorNode->GetServerPort();
            d->portLineEdit->setText(QVariant(serverPort).toString());
            const char* hostname = connectorNode->GetServerHostname();
            // Type Server
            if (connectorType == 1) {
                d->serverModeRadioButton->setChecked(true);
                d->hostNameLineEdit->setText("NA");
                d->hostNameLineEdit->setDisabled(true);
            }
            // Type Client
            else {
                d->clientModeRadioButton->setChecked(true);
                d->hostNameLineEdit->setText(hostname);
                d->hostNameLineEdit->setEnabled(true);
            }
        }
    }
    // update tree visibility
    d->SynchronizedTreeView->model()->invalidateFilter();
    d->AvailableNodesTreeView->model()->invalidateFilter();
}

void qSlicerCollaborationModuleWidget::onConnectButtonClicked()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));

        if (connectorNode) {
            // Start the connection
            if (d->connectButton->text() == "Connect") {
                // send synchronized nodes on connect
                connectorNode->PushOnConnect();
                connectorNode->Start();
                d->connectButton->setText("Disconnect");
                // enable send button
                d->sendButton->setEnabled(true);
                // disable all remaining buttons
                d->MRMLNodeComboBox->setEnabled(false);
                d->serverModeRadioButton->setEnabled(false);
                d->clientModeRadioButton->setEnabled(false);
                d->hostNameLineEdit->setEnabled(false);
                d->portLineEdit->setEnabled(false);
            }
            // Stop the connection
            else {
                connectorNode->Stop();
                d->connectButton->setText("Connect");
                // disable send button
                d->sendButton->setEnabled(false);
                //// enable all remaining buttons
                d->MRMLNodeComboBox->setEnabled(true);
                d->serverModeRadioButton->setEnabled(true);
                d->clientModeRadioButton->setEnabled(true);
                d->portLineEdit->setEnabled(true);
                if (connectorNode->GetType() == 2) {
                    d->hostNameLineEdit->setEnabled(true);
                }
                

            }
        }
    }
}

//------------------------------------------------------------------------------
void qSlicerCollaborationModuleWidget::updateConnectorNode()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));

        if (connectorNode) {
            connectorNode->DisableModifiedEventOn();

            // Update connector properties
            if (d->serverModeRadioButton->isChecked()) {
                connectorNode->SetType(1);
                d->hostNameLineEdit->setText("NA");
                d->hostNameLineEdit->setDisabled(true);
                d->portLineEdit->setEnabled(true);
            }
            else {
                connectorNode->SetType(2);
                d->hostNameLineEdit->setText("localhost");
                d->hostNameLineEdit->setEnabled(true);
                d->portLineEdit->setEnabled(true);
                connectorNode->SetServerHostname(d->hostNameLineEdit->text().toStdString());
            }
            connectorNode->SetServerPort(d->portLineEdit->text().toInt());
            connectorNode->DisableModifiedEventOff();
            connectorNode->InvokePendingModifiedEvent();
        }
    }
}

void qSlicerCollaborationModuleWidget::synchronizeSelectedNodes()
{
    Q_D(qSlicerCollaborationModuleWidget);
    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());
    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));
        if (connectorNode){
            //Get the selected nodes to synchronize
            vtkMRMLSubjectHierarchyNode* shNode = d->AvailableNodesTreeView->subjectHierarchyNode();
            QList<vtkIdType> currentItemIDs = d->AvailableNodesTreeView->currentItems();
            int numberOfItems = currentItemIDs.size();
            if (numberOfItems > 0) {
                vtkMRMLNode* selectedNode = nullptr;
                for (int nodeIndex = numberOfItems - 1; nodeIndex >= 0; nodeIndex--)
                {
                    selectedNode = shNode->GetItemDataNode(currentItemIDs[nodeIndex]);
                    // set attribute of the collaboration node to the selected node
                    selectedNode->SetAttribute(selected_collab_node, "true");
                    // add node reference to the collaboration node
                    collabNode->AddCollaborationSynchronizedNodeID(selectedNode->GetID());
                    selectedNode->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                    if (!selectedNode->IsA("vtkMRMLMarkupsNode") || selectedNode->IsA("vtkMRMLMarkupsFiducialNode")) {
                        // add as output node of the connector node
                        connectorNode->RegisterOutgoingMRMLNode(selectedNode);
                        connectorNode->PushNode(selectedNode);
                    }
                    // check if it observes a transform node and update it
                    vtkMRMLNode* transformNode = vtkMRMLNode::SafeDownCast(selectedNode->GetNodeReference("transform"));
                    if (transformNode)
                    {
                        updateTransformNodeText(transformNode);
                        selectedNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, updateTextCallback);
                    }

                    // check if it is a model node
                    if (selectedNode->IsA("vtkMRMLModelNode"))
                    {
                        vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(selectedNode);
                        // get the display node
                        vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
                        char* displayClassName = "vtkMRMLModelDisplayNode";
                        // create a text node with the display information
                        vtkMRMLTextNode* textNode = createTextOfDisplayNode(displayNode, modelNode->GetName(), displayClassName);
                        // set attribute of the collaboration node to the selected node
                        textNode->SetAttribute(selected_collab_node, "true");
                        // add node reference to the collaboration node
                        collabNode->AddCollaborationSynchronizedNodeID(textNode->GetID());
                        // add as output node of the connector node
                        textNode->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                        connectorNode->RegisterOutgoingMRMLNode(textNode);
                        // send if the connection is active
                        connectorNode->PushNode(textNode);
                        // add node reference to the display model node
                        displayNode->AddNodeReferenceRole("TextNode");
                        displayNode->AddNodeReferenceID("TextNode", textNode->GetID());
                        // add observer to the display node to update the text node
                        displayNode->AddObserver(vtkCommand::AnyEvent, updateTextCallback);
                        //modelNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, updateTextCallback);
                    }
                    else if (selectedNode->IsA("vtkMRMLMarkupsFiducialNode"))
                    {
                        vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(selectedNode);
                        // get the display node
                        vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
                        char* displayClassName = "vtkMRMLMarkupsDisplayNode";
                        // create a text node with the display information
                        vtkMRMLTextNode* textNodeDisplay = createTextOfDisplayNode(displayNode, markupsNode->GetName(), displayClassName);
                        // set attribute of the collaboration node to the selected node
                        textNodeDisplay->SetAttribute(selected_collab_node, "true");
                        // add node reference to the collaboration node
                        collabNode->AddCollaborationSynchronizedNodeID(textNodeDisplay->GetID());
                        // add as output node of the connector node
                        textNodeDisplay->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                        connectorNode->RegisterOutgoingMRMLNode(textNodeDisplay);
                        // add node reference to the display markups node
                        displayNode->AddNodeReferenceRole("TextNode");
                        displayNode->AddNodeReferenceID("TextNode", textNodeDisplay->GetID());
                        // add observer to the markups node to update the text node
                        displayNode->AddObserver(vtkCommand::ModifiedEvent, updateTextCallback);
                        // send node
                        connectorNode->PushNode(textNodeDisplay);
                        // check if it observes a synched transform node and update it
                        vtkMRMLNode* transformNode = vtkMRMLNode::SafeDownCast(markupsNode->GetNodeReference("transform"));
                    }
                    // check if it is a line markups (non fiducial) node
                    else if (selectedNode->IsA("vtkMRMLMarkupsNode") && !selectedNode->IsA("vtkMRMLMarkupsFiducialNode"))
                    {
                        vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(selectedNode);
                        std::string className = selectedNode->GetClassName();
                        // get control points
                        vtkNew<vtkPoints> controlPoints;
                        markupsNode->GetControlPointPositionsWorld(controlPoints);
                        int numberOfPoints = markupsNode->GetNumberOfControlPoints();
                        std::string controlPointsText = " ControlPoints = \"";
                        for (int p = 0; p < numberOfPoints; p++)
                        {
                            double point[3];
                            controlPoints->GetPoint(p, point);
                            controlPointsText.append("[");
                            controlPointsText.append(std::to_string(point[0]));
                            controlPointsText.append(",");
                            controlPointsText.append(std::to_string(point[1]));
                            controlPointsText.append(",");
                            controlPointsText.append(std::to_string(point[2]));
                            controlPointsText.append("]");
                            if (p < (numberOfPoints - 1))
                            {
                                controlPointsText.append(";");
                            }
                        }
                        controlPointsText.append("\"");

                        // create a text node
                        vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLTextNode"));
                        // hide from Data module
                        textNode->SetHideFromEditors(1);
                        this->mrmlScene()->AddNode(textNode);
                        // write an XML text with the display node attributes
                        std::stringstream ss;
                        ss << "<MRMLNode SuperclassName = \"vtkMRMLMarkupsNode\" ClassName = \"";
                        ss << className;
                        ss << "\" ";
                        ss << controlPointsText;

                        // check if it is a ROI markups node to get ROI radius
                        if (selectedNode->IsA("vtkMRMLMarkupsROINode"))
                        {
                            vtkMRMLMarkupsROINode* markupsROINode = vtkMRMLMarkupsROINode::SafeDownCast(selectedNode);
                            double rad[3];
                            markupsROINode->GetRadiusXYZ(rad);
                            std::string ROIradiusText = " ROIRadius = \"";
                            ROIradiusText.append("[");
                            ROIradiusText.append(std::to_string(rad[0]));
                            ROIradiusText.append(",");
                            ROIradiusText.append(std::to_string(rad[1]));
                            ROIradiusText.append(",");
                            ROIradiusText.append(std::to_string(rad[2]));
                            ROIradiusText.append("]");
                            ROIradiusText.append("\"");
                            ss << ROIradiusText;
                        }
                        
                        markupsNode->WriteXML(ss, 0);
                        ss << " />";
                        // add the XML to the text node
                        textNode->SetText(ss.str());
                        // Set the same name as the model node + Text
                        char* markupsNodeName = markupsNode->GetName();
                        char textName[] = "Text";
                        char* textNodeName = new char[std::strlen(markupsNodeName) + std::strlen(textName) + 1];
                        std::strcpy(textNodeName, markupsNodeName);
                        std::strcat(textNodeName, textName);
                        textNode->SetName(textNodeName);
                        // set attribute of the collaboration node to the selected node
                        textNode->SetAttribute(selected_collab_node, "true");
                        // add node reference to the collaboration node
                        collabNode->AddCollaborationSynchronizedNodeID(textNode->GetID());
                        // add as output node of the connector node
                        textNode->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                        connectorNode->RegisterOutgoingMRMLNode(textNode);
                        // add node reference to the markups node
                        markupsNode->AddNodeReferenceRole("TextNode");
                        markupsNode->AddNodeReferenceID("TextNode", textNode->GetID());
                        // add observer to the markups node to update the text node
                        markupsNode->AddObserver(vtkCommand::AnyEvent, updateTextCallback);

                        // get the display node
                        vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
                        char* displayClassName = "vtkMRMLMarkupsDisplayNode";
                        // create a text node with the display information
                        vtkMRMLTextNode* textNodeDisplay = createTextOfDisplayNode(displayNode, markupsNode->GetName(), displayClassName);
                        // set attribute of the collaboration node to the selected node
                        textNodeDisplay->SetAttribute(selected_collab_node, "true");
                        // add node reference to the collaboration node
                        collabNode->AddCollaborationSynchronizedNodeID(textNodeDisplay->GetID());
                        // add as output node of the connector node
                        textNodeDisplay->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                        connectorNode->RegisterOutgoingMRMLNode(textNodeDisplay);
                        // add node reference to the display markups node
                        displayNode->AddNodeReferenceRole("TextNode");
                        displayNode->AddNodeReferenceID("TextNode", textNodeDisplay->GetID());
                        // add observer to the markups node to update the text node
                        displayNode->AddObserver(vtkCommand::ModifiedEvent, updateTextCallback);
                        // send node
                        connectorNode->PushNode(textNode);
                        connectorNode->PushNode(textNodeDisplay);
                    }
                    else if (selectedNode->IsA("vtkMRMLLinearTransformNode"))
                    {
                        vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(selectedNode);
                        std::string transformNodeID = transformNode->GetID();
                        // create a text node to send the observing and observed nodes
                        vtkMRMLTextNode* transformTextNode = vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLTextNode"));
                        // set attribute of the collaboration node to the selected node
                        transformTextNode->SetAttribute(selected_collab_node, "true");
                        // hide from Data module
                        transformTextNode->SetHideFromEditors(1);
                        this->mrmlScene()->AddNode(transformTextNode);
                        // get transformed nodes
                        vtkStringArray* collection = collabNode->GetCollaborationSynchronizedNodeIDs();
                        std::string transformedNodesText = "";
                        for (int i = 0; i < collection->GetNumberOfTuples(); i++)
                        {
                            std::string ID = collection->GetValue(i);
                            vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(this->mrmlScene()->GetNodeByID(ID));
                            vtkMRMLNode* transformNode = vtkMRMLNode::SafeDownCast(node->GetNodeReference("transform"));
                            if (transformNode)
                            {
                                std::string nodeTranformID = transformNode->GetID();
                                if (transformNodeID == nodeTranformID)
                                {
                                    transformedNodesText.append(node->GetName());
                                    transformedNodesText.append(",");
                                }
                            }
                        }
                        // remove the last comma
                        if (transformedNodesText != "")
                        {
                            transformedNodesText.pop_back();
                        }
                        // write an XML text with the transform node attributes
                        std::stringstream ss;
                        ss << "<MRMLNode SuperclassName = \"vtkMRMLTransformNode\" ClassName = \"vtkMRMLLinearTransformNode\" TransformName = \"";
                        ss << transformNode->GetName();
                        ss << "\" TransformedNodes = \"";
                        ss << transformedNodesText;
                        ss << "\"";
                        ss << " />";
                        // add the XML to the text node
                        transformTextNode->SetText(ss.str());
                        // Set the same name as the model node + Text
                        char* markupsNodeName = transformNode->GetName();
                        char textName[] = "Text";
                        char* textNodeName = new char[std::strlen(markupsNodeName) + std::strlen(textName) + 1];
                        std::strcpy(textNodeName, markupsNodeName);
                        std::strcat(textNodeName, textName);
                        transformTextNode->SetName(textNodeName);
                        // set attribute of the collaboration node to the selected node
                        transformNode->SetAttribute(selected_collab_node, "true");
                        // add node reference to the collaboration node
                        collabNode->AddCollaborationSynchronizedNodeID(transformTextNode->GetID());
                        // add as output node of the connector node
                        transformTextNode->SetAttribute("OpenIGTLinkIF.pushOnConnect", "true");
                        connectorNode->RegisterOutgoingMRMLNode(transformTextNode);
                        // add node reference to the markups node
                        transformNode->AddNodeReferenceRole("TextNode");
                        transformNode->AddNodeReferenceID("TextNode", transformTextNode->GetID());
                        // send node
                        connectorNode->PushNode(transformTextNode);
                    }
                    // update tree visibility
                    d->SynchronizedTreeView->model()->invalidateFilter();
                    d->AvailableNodesTreeView->model()->invalidateFilter();
                }
            }
        }
    }
}


void qSlicerCollaborationModuleWidget::unsynchronizeSelectedNodes()
{
    Q_D(qSlicerCollaborationModuleWidget);
    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());
    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));
        if (connectorNode) {
            //Get the selected nodes to unsynchronize
            vtkMRMLSubjectHierarchyNode* shNode = d->SynchronizedTreeView->subjectHierarchyNode();
            QList<vtkIdType> currentItemIDs = d->SynchronizedTreeView->currentItems();
            int numberOfItems = currentItemIDs.size();
            if (numberOfItems > 0) {
                vtkMRMLNode* selectedNode = nullptr;
                for (int nodeIndex = numberOfItems - 1; nodeIndex >= 0; nodeIndex--)
                {
                    selectedNode = shNode->GetItemDataNode(currentItemIDs[nodeIndex]);
                    // remove the attribute of the collaboration node from the selected node
                    selectedNode->RemoveAttribute(selected_collab_node);
                    // remove node reference from the collaboration node
                    collabNode->RemoveCollaborationSynchronizedNodeID(selectedNode->GetID());
                    // remove as output node of the connector node
                    connectorNode->UnregisterOutgoingMRMLNode(selectedNode);
                    // remove observer to transforms
                    selectedNode->RemoveObserver(updateTextCallback);
                    const char* att_push = selectedNode->GetAttribute("OpenIGTLinkIF.pushOnConnect");
                    if (att_push) {
                        selectedNode->RemoveAttribute("OpenIGTLinkIF.pushOnConnect");
                    }
                    // check if it observes a synched transform node and update it
                    vtkMRMLNode* transformNode = vtkMRMLNode::SafeDownCast(selectedNode->GetNodeReference("transform"));
                    if (transformNode && transformNode->GetAttribute(selected_collab_node))
                    {
                        updateTransformNodeText(transformNode);
                    }
                    // check if it is a model node, to remove the corresponding text node
                    if (selectedNode->IsA("vtkMRMLModelNode"))
                    {
                        vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(selectedNode);
                        // Get the text node
                        const char* textNodeID = modelNode->GetDisplayNode()->GetNthNodeReferenceID("TextNode", 0);
                        vtkSmartPointer<vtkMRMLTextNode> textNode =
                            vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->GetNodeByID(textNodeID));
                        if (textNode)
                        {
                            // remove the attribute of the collaboration node from the selected node
                            textNode->RemoveAttribute(selected_collab_node);
                            // remove node reference from the collaboration node
                            collabNode->RemoveCollaborationSynchronizedNodeID(textNode->GetID());
                            // remove as output node of the connector node
                            const char* att_push_text = selectedNode->GetAttribute("OpenIGTLinkIF.pushOnConnect");
                            if (att_push_text) {
                                textNode->RemoveAttribute("OpenIGTLinkIF.pushOnConnect");
                            }
                            connectorNode->UnregisterOutgoingMRMLNode(textNode);
                            // remove from scene
                            textNode->RemoveAllObservers();
                            this->mrmlScene()->RemoveNode(textNode);
                        }
                    }
                    // check if it is a markups node, to remove the corresponding text nodes
                    if (selectedNode->IsA("vtkMRMLMarkupsNode"))
                    {
                        vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(selectedNode);
                        // Get the corresponding text node
                        const char* textNodeID = markupsNode->GetNthNodeReferenceID("TextNode", 0);
                        vtkSmartPointer<vtkMRMLTextNode> textNode =
                            vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->GetNodeByID(textNodeID));
                        if (textNode)
                        {
                            // remove the attribute of the collaboration node from the selected node
                            textNode->RemoveAttribute(selected_collab_node);
                            // remove node reference from the collaboration node
                            collabNode->RemoveCollaborationSynchronizedNodeID(textNode->GetID());
                            // remove as output node of the connector node
                            const char* att_push_text = selectedNode->GetAttribute("OpenIGTLinkIF.pushOnConnect");
                            if (att_push_text) {
                                textNode->RemoveAttribute("OpenIGTLinkIF.pushOnConnect");
                            }
                            connectorNode->UnregisterOutgoingMRMLNode(textNode);
                            // remove from scene
                            textNode->RemoveAllObservers();
                            this->mrmlScene()->RemoveNode(textNode);
                        }
                        // get the display node and its corresponding text node
                        const char* displayTextNodeID = markupsNode->GetDisplayNode()->GetNthNodeReferenceID("TextNode", 0);
                        vtkSmartPointer<vtkMRMLTextNode> displayTextNode =
                            vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->GetNodeByID(displayTextNodeID));
                        if (displayTextNode)
                        {
                            // remove the attribute of the collaboration node from the selected node
                            displayTextNode->RemoveAttribute(selected_collab_node);
                            // remove node reference from the collaboration node
                            collabNode->RemoveCollaborationSynchronizedNodeID(displayTextNode->GetID());
                            // remove as output node of the connector node
                            const char* att_push_text = selectedNode->GetAttribute("OpenIGTLinkIF.pushOnConnect");
                            if (att_push_text) {
                                displayTextNode->RemoveAttribute("OpenIGTLinkIF.pushOnConnect");
                            }
                            connectorNode->UnregisterOutgoingMRMLNode(displayTextNode);
                            // remove from scene
                            displayTextNode->RemoveAllObservers();
                            this->mrmlScene()->RemoveNode(displayTextNode);
                        }
                    }
                    if (selectedNode->IsA("vtkMRMLLinearTransformNode"))
                    {
                        vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(selectedNode);
                        // Get the corresponding text node
                        const char* textNodeID = transformNode->GetNthNodeReferenceID("TextNode", 0);
                        vtkSmartPointer<vtkMRMLTextNode> textNode =
                            vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->GetNodeByID(textNodeID));
                        if (textNode)
                        {
                            // remove the attribute of the collaboration node from the selected node
                            textNode->RemoveAttribute(selected_collab_node);
                            // remove node reference from the collaboration node
                            collabNode->RemoveCollaborationSynchronizedNodeID(textNode->GetID());
                            // remove as output node of the connector node
                            const char* att_push_text = selectedNode->GetAttribute("OpenIGTLinkIF.pushOnConnect");
                            if (att_push_text) {
                                textNode->RemoveAttribute("OpenIGTLinkIF.pushOnConnect");
                            }
                            connectorNode->UnregisterOutgoingMRMLNode(textNode);
                            // remove from scene
                            textNode->RemoveAllObservers();
                            this->mrmlScene()->RemoveNode(textNode);
                        }
                    }
                    // update tree visibility
                    d->SynchronizedTreeView->model()->invalidateFilter();
                    d->AvailableNodesTreeView->model()->invalidateFilter();
                }
            }
        }
    }
}

void qSlicerCollaborationModuleWidget::sendNodesForSynchronization()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));

        if (connectorNode) {
            // Start the connection
            if (d->connectButton->text() == "Disconnect") {
                // send synchronized nodes
                vtkCollection* syncNodes = collabNode->GetCollaborationSynchronizedNodes();
                int numNodes = syncNodes->GetNumberOfItems();
                for (int nodeIndex = numNodes - 1; nodeIndex >= 0; nodeIndex--)
                {
                    vtkMRMLNode* syncNode = vtkMRMLNode::SafeDownCast(syncNodes->GetItemAsObject(nodeIndex));
                    connectorNode->PushNode(syncNode);
                }
            }
        }
    }
}

vtkMRMLTextNode* qSlicerCollaborationModuleWidget::createTextOfDisplayNode(vtkMRMLNode* displayNode, char* nodeName, char* className)
{
    // create a text node
    vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLTextNode"));
    // hide from Data module
    textNode->SetHideFromEditors(1);
    this->mrmlScene()->AddNode(textNode);
    // write an XML text with the display node attributes
    std::stringstream ss;
    ss << "<MRMLNode SuperclassName = \"vtkMRMLDisplayNode\" ClassName = \"";
    ss << className;
    ss << "\" NodeName = \"";
    ss << nodeName;
    ss << "\"";
    displayNode->WriteXML(ss, 0);
    ss << " />";
    // add the XML to the text node
    textNode->SetText(ss.str());
    // Set the same name as the model node + Text
    char textName[] = "DisplayText";
    char* textNodeName = new char[std::strlen(nodeName) + std::strlen(textName) + 1];
    std::strcpy(textNodeName, nodeName);
    std::strcat(textNodeName, textName);
    textNode->SetName(textNodeName);
    return textNode;
}

void qSlicerCollaborationModuleWidget::nodeUpdated(vtkObject* caller, unsigned long event, void* clientData, void* callData)
{
    qSlicerCollaborationModuleWidget* self = reinterpret_cast<qSlicerCollaborationModuleWidget*>(clientData);
    
    // Get the selected collaboration node
    if (self->logic() == nullptr)
    {
        return;
    }
    vtkSlicerCollaborationLogic* collaborationLogic = vtkSlicerCollaborationLogic::SafeDownCast(self->logic());
    if (collaborationLogic)
    {
        vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(collaborationLogic->collaborationNodeSelected);
        if (collabNode) {
            // Get the connector node associated to the collaboration node
            vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(self->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));
            if (connectorNode) {

                if (caller->IsA("vtkMRMLNode"))
                {
                    vtkMRMLNode* updatedNode = vtkMRMLNode::SafeDownCast(caller);
                    // update all transform texts
                    // get transformed nodes
                    vtkStringArray* collection = collabNode->GetCollaborationSynchronizedNodeIDs();
                    std::string transformedNodesText = "";
                    for (int i = 0; i < collection->GetNumberOfTuples(); i++)
                    {
                        std::string ID = collection->GetValue(i);
                        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(self->mrmlScene()->GetNodeByID(ID));
                        if (node->IsA("vtkMRMLLinearTransformNode"))
                        {
                            self->updateTransformNodeText(node);
                        }
                    }
                }
                
                if (caller->IsA("vtkMRMLDisplayNode"))
                {
                    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(caller);
                    // get the corresponding text node
                    const char* textNodeID = displayNode->GetNthNodeReferenceID("TextNode", 0);
                    vtkMRMLTextNode* displayTextNode = vtkMRMLTextNode::SafeDownCast(self->mrmlScene()->GetNodeByID(textNodeID));
                    if (displayTextNode) {
                        // std::string className = displayNode->GetClassName();
                        char* modelDisplayClass = "vtkMRMLModelDisplayNode";
                        char* markupsDisplayClass = "vtkMRMLMarkupsDisplayNode";
                        // if it is a model display node
                        if (displayNode->IsA("vtkMRMLModelDisplayNode"))
                        {
                            // create a text node with the display information
                            vtkMRMLTextNode* textNode = self->createTextOfDisplayNode(displayNode, displayNode->GetDisplayableNode()->GetName(), modelDisplayClass);
                            // set text
                            displayTextNode->SetText(textNode->GetText());
                            displayTextNode->Modified();
                            connectorNode->PushNode(displayTextNode);
                        }
                        // if it is a markups display node
                        else if (displayNode->IsA("vtkMRMLMarkupsDisplayNode"))
                        {
                            // create a text node with the display information
                            vtkMRMLTextNode* textNode = self->createTextOfDisplayNode(displayNode, displayNode->GetDisplayableNode()->GetName(), markupsDisplayClass);
                            // set text
                            displayTextNode->SetText(textNode->GetText());
                            displayTextNode->Modified();
                            connectorNode->PushNode(displayTextNode);

                            // update also the markups node text
                            // get markups node
                            vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(displayNode->GetDisplayableNode());
                            std::string className = markupsNode->GetClassName();
                            // get control points
                            vtkNew<vtkPoints> controlPoints;
                            markupsNode->GetControlPointPositionsWorld(controlPoints);
                            int numberOfPoints = markupsNode->GetNumberOfControlPoints();
                            std::string controlPointsText = " ControlPoints = \"";
                            for (int p = 0; p < numberOfPoints; p++)
                            {
                                double point[3];
                                controlPoints->GetPoint(p, point);
                                controlPointsText.append("[");
                                controlPointsText.append(std::to_string(point[0]));
                                controlPointsText.append(",");
                                controlPointsText.append(std::to_string(point[1]));
                                controlPointsText.append(",");
                                controlPointsText.append(std::to_string(point[2]));
                                controlPointsText.append("]");
                                if (p < (numberOfPoints - 1))
                                {
                                    controlPointsText.append(";");
                                }
                            }
                            controlPointsText.append("\"");
                            // get the text node
                            const char* textNodeID = markupsNode->GetNthNodeReferenceID("TextNode", 0);
                            vtkMRMLTextNode* textNode2 = vtkMRMLTextNode::SafeDownCast(self->mrmlScene()->GetNodeByID(textNodeID));
                            if (textNode2) {
                                // write an XML text with the display node attributes
                                std::stringstream ss;
                                ss << "<MRMLNode SuperclassName = \"vtkMRMLMarkupsNode\" ClassName = \"";
                                ss << className;
                                ss << "\" ";
                                ss << controlPointsText;
                                markupsNode->WriteXML(ss, 0);
                                ss << " />";
                                // add the XML to the text node
                                textNode2->SetText(ss.str());
                                textNode2->Modified();
                                connectorNode->PushNode(textNode2);
                            }
                        }
                    }
                }
                else if (caller->IsA("vtkMRMLMarkupsNode"))
                {
                    vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(caller);
                    std::string className = markupsNode->GetClassName();
                    // get control points
                    vtkNew<vtkPoints> controlPoints;
                    markupsNode->GetControlPointPositionsWorld(controlPoints);
                    int numberOfPoints = markupsNode->GetNumberOfControlPoints();
                    std::string controlPointsText = " ControlPoints = \"";
                    for (int p = 0; p < numberOfPoints; p++)
                    {
                        double point[3];
                        controlPoints->GetPoint(p, point);
                        controlPointsText.append("[");
                        controlPointsText.append(std::to_string(point[0]));
                        controlPointsText.append(",");
                        controlPointsText.append(std::to_string(point[1]));
                        controlPointsText.append(",");
                        controlPointsText.append(std::to_string(point[2]));
                        controlPointsText.append("]");
                        if (p < (numberOfPoints - 1))
                        {
                            controlPointsText.append(";");
                        }
                    }
                    controlPointsText.append("\"");
                    // get the text node
                    const char* textNodeID = markupsNode->GetNthNodeReferenceID("TextNode", 0);
                    vtkMRMLTextNode* textNode2 = vtkMRMLTextNode::SafeDownCast(self->mrmlScene()->GetNodeByID(textNodeID));
                    if (textNode2) {
                        // write an XML text with the display node attributes
                        std::stringstream ss;
                        ss << "<MRMLNode SuperclassName = \"vtkMRMLMarkupsNode\" ClassName = \"";
                        ss << className;
                        ss << "\" ";
                        ss << controlPointsText;
                        // check if it is a ROI markups node to get ROI radius
                        if (markupsNode->IsA("vtkMRMLMarkupsROINode"))
                        {
                            vtkMRMLMarkupsROINode* markupsROINode = vtkMRMLMarkupsROINode::SafeDownCast(markupsNode);
                            double rad[3];
                            markupsROINode->GetRadiusXYZ(rad);
                            std::string ROIradiusText = " ROIRadius = \"";
                            ROIradiusText.append("[");
                            ROIradiusText.append(std::to_string(rad[0]));
                            ROIradiusText.append(",");
                            ROIradiusText.append(std::to_string(rad[1]));
                            ROIradiusText.append(",");
                            ROIradiusText.append(std::to_string(rad[2]));
                            ROIradiusText.append("]");
                            ROIradiusText.append("\"");
                            ss << ROIradiusText;
                        }
                        markupsNode->WriteXML(ss, 0);
                        ss << " />";
                        // add the XML to the text node
                        textNode2->SetText(ss.str());
                        textNode2->Modified();
                        connectorNode->PushNode(textNode2);
                    }
                }
            }
        }
    }       
}


void qSlicerCollaborationModuleWidget::updateTransformNodeText(vtkMRMLNode* node)
{
    Q_D(qSlicerCollaborationModuleWidget);

    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
    std::string transformNodeID = transformNode->GetID();
    // get the corresponding text node
    const char* textNodeID = transformNode->GetNthNodeReferenceID("TextNode", 0);
    vtkMRMLTextNode* transformTextNode = vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->GetNodeByID(textNodeID));
    if (transformTextNode)
    {
        vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());
        if (collabNode) {
            // Get the connector node associated to the collaboration node
            vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->GetCollaborationConnectorNodeID()));
            if (connectorNode) {
                // get transformed nodes
                vtkStringArray* collection = collabNode->GetCollaborationSynchronizedNodeIDs();
                std::string transformedNodesText = "";
                for (int i = 0; i < collection->GetNumberOfTuples(); i++)
                {
                    std::string ID = collection->GetValue(i);
                    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(this->mrmlScene()->GetNodeByID(ID));
                    vtkMRMLNode* transformNode = vtkMRMLNode::SafeDownCast(node->GetNodeReference("transform"));
                    if (transformNode)
                    {
                        std::string nodeTranformID = transformNode->GetID();
                        if (transformNodeID == nodeTranformID)
                        {
                            transformedNodesText.append(node->GetName());
                            transformedNodesText.append(",");
                        }
                    }
                }
                // remove the last comma
                if (transformedNodesText != "")
                {
                    transformedNodesText.pop_back();
                }
                // write an XML text with the transform node attributes
                std::stringstream ss;
                ss << "<MRMLNode SuperclassName = \"vtkMRMLTransformNode\" ClassName = \"vtkMRMLLinearTransformNode\" TransformName = \"";
                ss << transformNode->GetName();
                ss << "\" TransformedNodes = \"";
                ss << transformedNodesText;
                ss << "\"";
                ss << " />";
                // add the XML to the text node
                transformTextNode->SetText(ss.str());
                connectorNode->PushNode(transformTextNode);
            }
        }
    }
}

void qSlicerCollaborationModuleWidget::onConnectVRButtonClicked()
{
    Q_D(qSlicerCollaborationModuleWidget);
    
    qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("VirtualReality");
    qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
    if (!moduleWithAction)
    {
        d->connectionTextMessage->setText("Install SlicerVR extension to use this functionality");
        return;
    }
    else {
        // check if ConnectCheckBox from VirtualReality module is checked
        // Get module widget
        qSlicerAbstractModuleWidget* moduleWidget = dynamic_cast<qSlicerAbstractModuleWidget*>(moduleWithAction->widgetRepresentation());
        // Get connection to VR hardware checkbox
        ctkCheckBox* connectCheckBox = moduleWidget->findChild<ctkCheckBox*>("ConnectCheckBox");
        ctkCheckBox* enableRendering = moduleWidget->findChild<ctkCheckBox*>("RenderingEnabledCheckBox");
        ctkCheckBox* ControllerTransformsUpdateCheckBox = moduleWidget->findChild<ctkCheckBox*>("ControllerTransformsUpdateCheckBox");
        // disconnect
        if (connectCheckBox->isChecked())
        {
            connectCheckBox->setChecked(0);
            enableRendering->setChecked(0);
            ControllerTransformsUpdateCheckBox->setChecked(0);
            d->connectVRButton->setText("Connect to VR hardware");
            d->connectionTextMessage->setText("");
        }
        // connect
        else
        {
            connectCheckBox->setChecked(1);
            enableRendering->setChecked(1);
            ControllerTransformsUpdateCheckBox->setChecked(1);
            // show connection status text
            QLabel* connectionStatus = moduleWidget->findChild<QLabel*>("ConnectionStatusLabel");
            d->connectionTextMessage->setText(connectionStatus->text());
            d->LoadAvatarsButton->setEnabled(1);
            if (connectionStatus->text() != "Connection failed")
            {
                d->connectVRButton->setText("Disconnect VR");
            }
        }
    }   
}

void qSlicerCollaborationModuleWidget::onLoadAvatarsButtonClicked()
{
    Q_D(qSlicerCollaborationModuleWidget);
    if (this->logic() == nullptr)
    {
        return;
    }
    vtkSlicerCollaborationLogic* collaborationLogic = vtkSlicerCollaborationLogic::SafeDownCast(this->logic());
    if (collaborationLogic)
    {
        collaborationLogic->loadAvatars();
        d->LoadAvatarsButton->setEnabled(0);

        // hide controllers
        qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("VirtualReality");
        qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
        if (!moduleWithAction)
        {
            d->connectionTextMessage->setText("Install SlicerVR extension to use this functionality");
            return;
        }
        else {
            qSlicerAbstractModuleWidget* moduleWidget = dynamic_cast<qSlicerAbstractModuleWidget*>(moduleWithAction->widgetRepresentation());
            ctkCheckBox* ControllerModelsVisibleCheckBox = moduleWidget->findChild<ctkCheckBox*>("ControllerModelsVisibleCheckBox");
            ControllerModelsVisibleCheckBox->setChecked(0);
        }
    }
}
