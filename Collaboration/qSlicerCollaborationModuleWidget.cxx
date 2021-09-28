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
  d->UnsynchronizedToolButton->setIcon(leftIcon);
}
