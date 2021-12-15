cmake_minimum_required(VERSION 3.13.4)

project(SlicerCollaboration)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/SlicerCollaboration")
set(EXTENSION_CATEGORY "SlicerCollaboration")
set(EXTENSION_CONTRIBUTORS "Csaba Pinter (Ebatinca S.L.), David Garcia-Mato (Ebatinca S.L.), Monica Garcia-Sevilla (ULPGC)")
set(EXTENSION_DESCRIPTION "Extension facilitating real-time collaboration between two 3D Slicer instances")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SlicerCollaboration.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/SlicerCollaboration/Screenshots/1.png")
set(EXTENSION_DEPENDS "SlicerOpenIGTLink") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package( SlicerOpenIGTLink REQUIRED )

set(SlicerOpenIGTLink_ADDITIONAL_MODULE_PATHS
  ${SlicerOpenIGTLink_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
  ${SlicerOpenIGTLink_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Debug
  ${SlicerOpenIGTLink_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Release
  ${SlicerOpenIGTLink_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  )

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Collaboration)
add_subdirectory(Chat)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
