import os
import unittest
import logging
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

#
# Chat
#
class Chat(ScriptedLoadableModule):
  
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Chat"  # TODO: make this more human readable by adding spaces
    self.parent.categories = ["SlicerCollaboration"]  # TODO: set categories (folders where the module shows up in the module selector)
    self.parent.dependencies = []  # TODO: add here list of module names that this module requires
    self.parent.contributors = ["David Garcia-Mato (Ebatinca S.L."]  # TODO: replace with "Firstname Lastname (Organization)"
    # TODO: update with short description of the module and a link to online module documentation
    self.parent.helpText = """
    Text messages using OpenIGTLink communication protocol.
    """
    # TODO: replace with organization, grant and thanks
    self.parent.acknowledgementText = """
    This file was originally developed by David Garcia-Mato, Ebatinca S.L.
    """

    # Additional initialization step after application startup is complete
    slicer.app.connect("startupCompleted()", registerSampleData)

#
# Register sample data sets in Sample Data module
#
def registerSampleData():
  """
  Add data sets to Sample Data module.
  """
  # It is always recommended to provide sample data for users to make it easy to try the module,
  # but if no sample data is available then this method (and associated startupCompeted signal connection) can be removed.

  import SampleData
  iconsPath = os.path.join(os.path.dirname(__file__), 'Resources/Icons')

  # To ensure that the source code repository remains small (can be downloaded and installed quickly)
  # it is recommended to store data sets that are larger than a few MB in a Github release.

  # Chat1
  SampleData.SampleDataLogic.registerCustomSampleDataSource(
    # Category and sample name displayed in Sample Data module
    category='Chat',
    sampleName='Chat1',
    # Thumbnail should have size of approximately 260x280 pixels and stored in Resources/Icons folder.
    # It can be created by Screen Capture module, "Capture all views" option enabled, "Number of images" set to "Single".
    thumbnailFileName=os.path.join(iconsPath, 'Chat1.png'),
    # Download URL and target file name
    uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95",
    fileNames='Chat1.nrrd',
    # Checksum to ensure file integrity. Can be computed by this command:
    #  import hashlib; print(hashlib.sha256(open(filename, "rb").read()).hexdigest())
    checksums = 'SHA256:998cb522173839c78657f4bc0ea907cea09fd04e44601f17c82ea27927937b95',
    # This node name will be used when the data set is loaded
    nodeNames='Chat1'
  )

  # Chat2
  SampleData.SampleDataLogic.registerCustomSampleDataSource(
    # Category and sample name displayed in Sample Data module
    category='Chat',
    sampleName='Chat2',
    thumbnailFileName=os.path.join(iconsPath, 'Chat2.png'),
    # Download URL and target file name
    uris="https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97",
    fileNames='Chat2.nrrd',
    checksums = 'SHA256:1a64f3f422eb3d1c9b093d1a18da354b13bcf307907c66317e2463ee530b7a97',
    # This node name will be used when the data set is loaded
    nodeNames='Chat2'
  )

#
# ChatWidget
#
class ChatWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
  
  def __init__(self, parent=None):
    """
    Called when the user opens the module the first time and the widget is initialized.
    """
    ScriptedLoadableModuleWidget.__init__(self, parent)
    VTKObservationMixin.__init__(self)  # needed for parameter node observation
    self.logic = None
    self._parameterNode = None
    self._updatingGUIFromParameterNode = False

    # Variables
    self.connect = True

  def setup(self):
    """
    Called when the user opens the module the first time and the widget is initialized.
    """
    ScriptedLoadableModuleWidget.setup(self)

    # Load widget from .ui file (created by Qt Designer).
    # Additional widgets can be instantiated manually and added to self.layout.
    uiWidget = slicer.util.loadUI(self.resourcePath('UI/Chat.ui'))
    self.layout.addWidget(uiWidget)
    self.ui = slicer.util.childWidgetVariables(uiWidget)

    # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
    # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
    # "setMRMLScene(vtkMRMLScene*)" slot.
    uiWidget.setMRMLScene(slicer.mrmlScene)

    # Create logic class. Logic implements all computations that should be possible to run
    # in batch mode, without a graphical user interface.
    self.logic = ChatLogic()

    self.logic.chat_content = self.ui.ChatEdit

    # Connections
    self.ui.ServerRadioButton.connect('clicked(bool)', self.onTypeRadioButtonChecked)
    self.ui.ClientRadioButton.connect('clicked(bool)', self.onTypeRadioButtonChecked)
    self.ui.ConnectButton.connect('clicked(bool)', self.onConnectButtonClicked)
    self.ui.SendButton.connect('clicked(bool)', self.onSendButtonClicked)

  def onTypeRadioButtonChecked(self):

    if self.ui.ServerRadioButton.isChecked():
      self.ui.Hostname_input.enabled = False
    elif self.ui.ClientRadioButton.isChecked():
      self.ui.Hostname_input.enabled = True

  def onConnectButtonClicked(self):
    """
    Update connection through OpenIGTLink
    """
    
    # Get parameters
    hostname = self.ui.Hostname_input.text
    port = int(self.ui.Port_input.text)
    if self.ui.ServerRadioButton.isChecked():
      typeConnection = 'Server'
    elif self.ui.ClientRadioButton.isChecked():      
      typeConnection = 'Client'
    print('Hostname: ', hostname)
    print('Port: ', port)
    print('Type: ', typeConnection)

    # Update button state
    self.ui.ConnectButton.enabled = False

    # Update connection 
    if self.connect:
      status = self.logic.startConnection(port, hostname, typeConnection) # Start connection
      if status == 1:
        self.connect = False
        self.ui.ConnectButton.setText('DISCONNECT')
        self.ui.ServerRadioButton.enabled = False
        self.ui.ClientRadioButton.enabled = False
        self.ui.Hostname_input.enabled = False
        self.ui.Port_input.enabled = False
      # Prepare text message
      self.logic.setupOIGTLconnection(typeConnection)
      # Status
      print('Status: ', self.logic.checkStatusConnection())
    else:
      self.logic.stopConnection() # Stop connection
      self.connect = True
      self.ui.ConnectButton.setText('CONNECT')
      self.ui.ServerRadioButton.enabled = True
      self.ui.ClientRadioButton.enabled = True
      self.ui.Hostname_input.enabled = True
      self.ui.Port_input.enabled = True
      # Status
      print('Status: ', self.logic.checkStatusConnection())

    # Update button state
    self.ui.ConnectButton.enabled = True

  def onSendButtonClicked(self):
    """
    Send messages through OpenIGTLink
    """
    
    # Get input text
    messageText = self.ui.MessageEdit.plainText
    print('Message: ', messageText)

    # Clear input
    self.ui.MessageEdit.setPlainText('')

    # Add message to chat
    self.logic.AddSentMessageToChat(messageText)

    # Send message through OpenIGTLink
    self.logic.SendMessageThroughOIGTL(messageText)

#
# ChatLogic
#
class ChatLogic(ScriptedLoadableModuleLogic):
 
  def __init__(self):
    """
    Called when the logic class is instantiated. Can be used for initializing member variables.
    """
    ScriptedLoadableModuleLogic.__init__(self)

    # Chat history
    self.chat_history = ''
    self.chat_content = None

    # OpenIGTLink connector
    self.cnode = None
    self.typeConnection = 'Server'

    # vtkMRMLTextNode
    try:
      self.OIGTL_Chat_Server = slicer.util.getNode('OIGTL_Chat_Server')
    except:
      self.OIGTL_Chat_Server = slicer.vtkMRMLTextNode()
      self.OIGTL_Chat_Server.SetName('OIGTL_Chat_Server')
      slicer.mrmlScene.AddNode(self.OIGTL_Chat_Server)
    try:
      self.OIGTL_Chat_Client = slicer.util.getNode('OIGTL_Chat_Client')
    except:
      self.OIGTL_Chat_Client = slicer.vtkMRMLTextNode()
      self.OIGTL_Chat_Client.SetName('OIGTL_Chat_Client')
      slicer.mrmlScene.AddNode(self.OIGTL_Chat_Client)

  def startConnection(self, port, hostname, typeConnection):
    
    # Open connection
    try:
        self.cnode = slicer.util.getNode('IGTLConnector_Chat')
    except:
        self.cnode = slicer.vtkMRMLIGTLConnectorNode()
        slicer.mrmlScene.AddNode(self.cnode)
        self.cnode.SetName('IGTLConnector_Chat')
    self.typeConnection = typeConnection
    if typeConnection == 'Server':
      status = self.cnode.SetTypeServer(port)
    elif typeConnection == 'Client':
      status = self.cnode.SetTypeClient(hostname, port)
    else:
      return False
    
    # Check connection status
    if status == 1:
      self.cnode.Start()
      logging.debug('Connection Successful')
      
    else:
      print('ERROR: Unable to connect')
      logging.debug('ERROR: Unable to connect')

    return status

  def checkStatusConnection(self):

    output = self.cnode.GetState()
    if output == 2:
      state = 'ON'
    elif output == 1:
      state = 'WAIT'
    elif output == 0:
      state = 'OFF'
    else:
      state = 'UNKNOWN'
    return state

  def stopConnection(self):

    self.cnode.Stop()

  def setupOIGTLconnection(self, typeConnection):

    if typeConnection == 'Server':
      self.cnode.RegisterOutgoingMRMLNode(self.OIGTL_Chat_Server)
      self.cnode.PushNode(self.OIGTL_Chat_Server)
      device = self.cnode.CreateDeviceForOutgoingMRMLNode(self.OIGTL_Chat_Server)
      self.cnode.RegisterIncomingMRMLNode(self.OIGTL_Chat_Client, device)
      self.OIGTL_Chat_Client.RemoveAllObservers()
      self.OIGTL_Chat_Client.AddObserver(slicer.vtkMRMLTextNode.TextModifiedEvent, self.ReceiveMessageThroughOIGTL)
    elif typeConnection == 'Client':
      self.cnode.RegisterOutgoingMRMLNode(self.OIGTL_Chat_Client)
      self.cnode.PushNode(self.OIGTL_Chat_Client)
      device = self.cnode.CreateDeviceForOutgoingMRMLNode(self.OIGTL_Chat_Client)
      self.cnode.RegisterIncomingMRMLNode(self.OIGTL_Chat_Server, device)      
      self.OIGTL_Chat_Server.RemoveAllObservers()
      self.OIGTL_Chat_Server.AddObserver(slicer.vtkMRMLTextNode.TextModifiedEvent, self.ReceiveMessageThroughOIGTL)
    else:
      return False

  def AddSentMessageToChat(self, messageText):
    """
    Add message to chat.
    """
    self.chat_history = self.chat_history + '\n' + '[SENT] ' + messageText
    self.chat_content.setPlainText(self.chat_history)
    return (self.chat_history)

  def AddReceivedMessageToChat(self, messageText):
    """
    Add message to chat.
    """
    self.chat_history = self.chat_history + '\n' + '[RECEIVED] ' + messageText
    self.chat_content.setPlainText(self.chat_history)
    return (self.chat_history)

  def SendMessageThroughOIGTL(self, messageText):

    if self.typeConnection == 'Server':
      self.OIGTL_Chat_Server.SetText(messageText)
      self.cnode.PushNode(self.OIGTL_Chat_Server)
    elif self.typeConnection == 'Client':
      self.OIGTL_Chat_Client.SetText(messageText)
      self.cnode.PushNode(self.OIGTL_Chat_Client)
    else:
      return False

  def ReceiveMessageThroughOIGTL(self, unused1=None, unused2=None):

    # info
    print('Receiving...')

    # Get message
    if self.typeConnection == 'Server':
      messageText = self.OIGTL_Chat_Client.GetText()
    elif self.typeConnection == 'Client':
      messageText = self.OIGTL_Chat_Server.GetText()
    else:
      return False

    # Add message to chat
    self.AddReceivedMessageToChat(messageText)

  