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

// Colaboration module includes
#include "vtkMRMLCollaborationNode.h"
#include "vtkMRMLCollaborationConnectorNode.h"

// MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "vtkCollection.h"
#include "qMRMLSubjectHierarchyModel.h";

static const char* SYNC = "sync";

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

  d->SynchronizedTreeView->addNodeAttributeFilter(SYNC);

  // OPTION 2
  //QStringList filter;
  //filter << SYNC;
  //d->SynchronizedTreeView->sortFilterProxyModel()->setIncludeNodeAttributeNamesFilter(filter);
  //d->SynchronizedTreeView->show();

}

void qSlicerCollaborationModuleWidget::setCollaborationNode(vtkMRMLNode* node)
{
    Q_D(qSlicerCollaborationModuleWidget);

    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(node);

    // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
    d->MRMLNodeComboBox->setCurrentNode(collabNode);

    // Each time the node is modified, the qt widgets are updated
    qvtkReconnect(collabNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->connectorNodeID));

        if (connectorNode) {
            // if the conenctor node is new, get the current parameter values
            if (connectorNode->GetType() == 0) {
                std::string hostName;
                std::string portNumber;
                bool serverMode = d->serverModeRadioButton->isChecked();
                if (serverMode) {
                    connectorNode->SetTypeServer((d->portLineEdit->text()).toInt());
                }
                else {
                    connectorNode->SetTypeClient((d->hostNameLineEdit->text()).toStdString(), (d->portLineEdit->text()).toInt());
                }
            }
            // Enable Connect button
            d->connectButton->setEnabled(true);
        }
    }
    else {
        // Disable Connect button
        d->connectButton->setEnabled(false);
    }
    
    this->updateWidgetFromMRML();
}

void qSlicerCollaborationModuleWidget::updateWidgetFromMRML()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->connectorNodeID));

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
                d->hostNameLineEdit->setEnabled(false);
                d->hostNameLineEdit->setText("NA");
            }
            // Type Client
            else {
                d->clientModeRadioButton->setChecked(true);
                d->hostNameLineEdit->setText(hostname);
                d->hostNameLineEdit->setEnabled(true);
            }
        }
    }
    
}

void qSlicerCollaborationModuleWidget::onConnectButtonClicked()
{
    Q_D(qSlicerCollaborationModuleWidget);

    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->connectorNodeID));

        if (connectorNode) {
            // Start the connection
            if (d->connectButton->text() == "Connect") {
                connectorNode->Start();
                d->connectButton->setText("Disconnect");
                // disable all buttons
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
                //// enable all buttons
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

    if (d->serverModeRadioButton->isChecked()) {
        d->hostNameLineEdit->setDisabled(true);
        d->hostNameLineEdit->setText("NA");
    }
    else {
        d->hostNameLineEdit->setEnabled(true);
        if (d->hostNameLineEdit->text() == "NA") {
            d->hostNameLineEdit->setText("");
        }
        
    }

    if (collabNode) {
        // Get the connector node associated to the collaboration node
        vtkMRMLCollaborationConnectorNode* connectorNode = vtkMRMLCollaborationConnectorNode::SafeDownCast(this->mrmlScene()->GetNodeByID(collabNode->connectorNodeID));

        connectorNode->DisableModifiedEventOn();

        // Update connector properties
        if (d->serverModeRadioButton->isChecked()) {
            connectorNode->SetType(1);
            d->hostNameLineEdit->setDisabled(true);
        }
        else {
            connectorNode->SetType(2);
            d->hostNameLineEdit->setEnabled(true);
        }
        connectorNode->SetServerHostname(d->hostNameLineEdit->text().toStdString());
        connectorNode->SetServerPort(d->portLineEdit->text().toInt());
        connectorNode->DisableModifiedEventOff();
        connectorNode->InvokePendingModifiedEvent();
    }
}

void qSlicerCollaborationModuleWidget::synchronizeSelectedNodes()
{
    Q_D(qSlicerCollaborationModuleWidget);
    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());
    // Get the selected node to synchronize
    vtkMRMLNode* currentNode = d->AvailableNodesTreeView->currentNode();
    if (!currentNode) {
        qWarning() << Q_FUNC_INFO << ": No node selected";
    }
    else
    {
        currentNode->SetAttribute(SYNC,"true");
        // update tree visiblity
        d->SynchronizedTreeView->model()->invalidateFilter();
    }
}


void qSlicerCollaborationModuleWidget::unsynchronizeSelectedNodes()
{
    Q_D(qSlicerCollaborationModuleWidget);
    // Get the selected collaboration node
    vtkMRMLCollaborationNode* collabNode = vtkMRMLCollaborationNode::SafeDownCast(d->MRMLNodeComboBox->currentNode());
    // Get the selected node to synchronize
    vtkMRMLNode* currentNode = d->SynchronizedTreeView->currentNode();
    if (!currentNode) {
        qWarning() << Q_FUNC_INFO << ": No node selected";
    }
    else
    {
        currentNode->RemoveAttribute(SYNC);
        // update tree visibility
        d->SynchronizedTreeView->model()->invalidateFilter();
    }
}
