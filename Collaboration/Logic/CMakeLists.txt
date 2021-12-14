project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${SlicerOpenIGTLink_ModuleMRML_INCLUDE_DIRS}
  ${SlicerOpenIGTLink_ModuleLogic_INCLUDE_DIRS}
  ${SlicerCollaborationModule_ModuleMRML_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicerOpenIGTLinkIFModuleMRML
  vtkSlicerOpenIGTLinkIFModuleLogic
  vtkSlicerCollaborationModuleMRML
  vtkSlicerModelsModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

#-----------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Avatars/head.stl
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/head.stl
  COPYONLY)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Avatars/handPoint_L.stl
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/handPoint_L.stl
  COPYONLY)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Avatars/handPoint_R.stl
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/handPoint_R.stl
  COPYONLY)

install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/head.stl
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME} COMPONENT Runtime)

install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/handPoint_L.stl
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME} COMPONENT Runtime)

install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/handPoint_R.stl
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME} COMPONENT Runtime)