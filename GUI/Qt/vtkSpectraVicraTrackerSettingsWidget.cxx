/*
Copyright (C) 2011-2012, Northern Digital Inc. All rights reserved.

All Northern Digital Inc. (�NDI�) Media and/or Sample Code and/or Sample Code
Documentation (collectively referred to as �Sample Code�) is licensed and provided "as
is� without warranty of any kind. The licensee, by use of the Sample Code, warrants to
NDI that the Sample Code is fit for the use and purpose for which the licensee intends to
use the Sample Code. NDI makes no warranties, express or implied, that the functions
contained in the Sample Code will meet the licensee�s requirements or that the operation
of the programs contained therein will be error free. This warranty as expressed herein is
exclusive and NDI expressly disclaims any and all express and/or implied, in fact or in
law, warranties, representations, and conditions of every kind pertaining in any way to
the Sample Code licensed and provided by NDI hereunder, including without limitation,
each warranty and/or condition of quality, merchantability, description, operation,
adequacy, suitability, fitness for particular purpose, title, interference with use or
enjoyment, and/or non infringement, whether express or implied by statute, common law,
usage of trade, course of dealing, custom, or otherwise. No NDI dealer, distributor, agent
or employee is authorized to make any modification or addition to this warranty.

In no event shall NDI nor any of its employees be liable for any direct, indirect,
incidental, special, exemplary, or consequential damages, sundry damages or any
damages whatsoever, including, but not limited to, procurement of substitute goods or
services, loss of use, data or profits, or business interruption, however caused. In no
event shall NDI�s liability to the licensee exceed the amount paid by the licensee for the
Sample Code or any NDI products that accompany the Sample Code. The said limitations
and exclusions of liability shall apply whether or not any such damages are construed as
arising from a breach of a representation, warranty, guarantee, covenant, obligation,
condition or fundamental term or on any theory of liability, whether in contract, strict
liability, or tort (including negligence or otherwise) arising in any way out of the use of
the Sample Code even if advised of the possibility of such damage. In no event shall
NDI be liable for any claims, losses, damages, judgments, costs, awards, expenses or
liabilities of any kind whatsoever arising directly or indirectly from any injury to person
or property, arising from the Sample Code or any use thereof.
*/

#include <QtGui>

#include "vtkSpectraVicraTrackerSettingsWidget.h"

vtkSpectraVicraTrackerSettingsWidget::vtkSpectraVicraTrackerSettingsWidget( QWidget *parent, QSettings *settings )
{
  this->m_Parent = parent;
  this->m_Settings = settings;

  // set up the GUI.
  m_Widget.setupUi(this);
  this->CreateActions();

  // initialize the rom file list.
  for(int i=0; i < 12; i++)  // assume 9 wireless tools for now.
  {
    this->m_SpectraVicraSettings.romFiles << "";
  }
  this->m_SpectraVicraSettings.bUseManual = false;
  this->m_SpectraVicraSettings.baudRate = -1;
  this->m_SpectraVicraSettings.commPort = -1;
  this->m_SpectraVicraSettings.logComm = false;
  
  this->m_Widget.baudRateComboBox->addItem("Auto", -1);
  this->m_Widget.baudRateComboBox->addItem("9600", 9600);
  this->m_Widget.baudRateComboBox->addItem("14400", 14400);
  this->m_Widget.baudRateComboBox->addItem("19200", 19200);
  this->m_Widget.baudRateComboBox->addItem("38400", 38400);
  this->m_Widget.baudRateComboBox->addItem("57600", 57600);
  this->m_Widget.baudRateComboBox->addItem("115200", 115200);
  this->m_Widget.baudRateComboBox->addItem("921600", 921600);

  this->m_Widget.commPortComboBox->addItem("Auto", -1);
  this->m_Widget.commPortComboBox->addItem("COM1", 1);
  this->m_Widget.commPortComboBox->addItem("COM2", 2);
  this->m_Widget.commPortComboBox->addItem("COM3", 3);
  this->m_Widget.commPortComboBox->addItem("COM4", 4);
  this->m_Widget.commPortComboBox->addItem("COM5", 5);
  this->m_Widget.commPortComboBox->addItem("COM6", 6);
  this->m_Widget.commPortComboBox->addItem("COM7", 7);
  this->m_Widget.commPortComboBox->addItem("COM8", 8);

  // set validators on port line edits.
  this->m_Widget.updateFrequencyLineEdit->setValidator(new QDoubleValidator(0.1, 60, 1, this) );

  // default to Spectra on set-up.
  this->UseSpectra();
}

vtkSpectraVicraTrackerSettingsWidget::vtkSpectraVicraTrackerSettingsWidget( QWidget *parent)
{
  vtkSpectraVicraTrackerSettingsWidget(parent, 0);
}

vtkSpectraVicraTrackerSettingsWidget::~vtkSpectraVicraTrackerSettingsWidget()
{
}

void vtkSpectraVicraTrackerSettingsWidget::CreateActions()
{
  // wired ROMs
  connect(this->m_Widget.port01RomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  connect(this->m_Widget.port02RomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  connect(this->m_Widget.port03RomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  // wireless ROMs
  connect(this->m_Widget.portARomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  connect(this->m_Widget.portBRomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  connect(this->m_Widget.portCRomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  connect(this->m_Widget.portDRomFileBrowseButton, SIGNAL(clicked()), this, SLOT(OnLoadRomFile()) );
  // manual parameters.
  connect(this->m_Widget.useManualParmsCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnUseManual(bool)) );
  connect(this->m_Widget.commPortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCommPortChanged(int)) );
  connect(this->m_Widget.baudRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBaudRateChanged(int)) );
  // other settings
  connect(this->m_Widget.logCommunicationsCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnLogCommunicationChanged(bool)) );
}

void vtkSpectraVicraTrackerSettingsWidget::ReadTrackerSettings()
{
  if( !m_Settings )
    return;

  QString systemType;
  double defaultFrequency;

  switch(this->m_SystemType)
  {
  case NDI_SPECTRA:
    systemType = "Spectra";
    defaultFrequency = 60.0;
    break;
  case NDI_SPECTRA_HYBRID:
    systemType = "SpectraHybrid";
    defaultFrequency = 60.0;
    break;
  case NDI_VICRA:
    systemType = "Vicra";
    defaultFrequency = 20.0;
    break;
  default:
    // todo: add an error pop-up.
    this->m_SystemType = NDI_SPECTRA;
    systemType = "Spectra";
    defaultFrequency = 60.0;
  }

  if( this->m_SystemType == NDI_SPECTRA_HYBRID )
  {
    // Spectra hybrid port data - wired.
    this->m_Widget.port01VirtualRomCheckBox->setChecked(this->m_Settings->value(systemType+"/Port01/UseVirtualRom", false).toBool());
    this->m_SpectraVicraSettings.romFiles[0] = this->m_Settings->value(systemType+"/Port01/RomFile", "").toString();
    this->m_Widget.port02VirtualRomCheckBox->setChecked(this->m_Settings->value(systemType+"/Port02/UseVirtualRom", false).toBool());
    this->m_SpectraVicraSettings.romFiles[1] = this->m_Settings->value(systemType+"/Port02/RomFile", "").toString();
    this->m_Widget.port03VirtualRomCheckBox->setChecked(this->m_Settings->value(systemType+"/Port03/UseVirtualRom", false).toBool());
    this->m_SpectraVicraSettings.romFiles[2] = this->m_Settings->value(systemType+"/Port03/RomFile", "").toString();
  }
  // Spectra port data - wireless.
  this->m_SpectraVicraSettings.romFiles[3] = this->m_Settings->value(systemType+"/PortA/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[4] = this->m_Settings->value(systemType+"/PortB/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[5] = this->m_Settings->value(systemType+"/PortC/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[6] = this->m_Settings->value(systemType+"/PortD/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[7] = this->m_Settings->value(systemType+"/PortE/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[8] = this->m_Settings->value(systemType+"/PortF/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[9] = this->m_Settings->value(systemType+"/PortG/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[10] = this->m_Settings->value(systemType+"/PortH/RomFile", "").toString();
  this->m_SpectraVicraSettings.romFiles[11] = this->m_Settings->value(systemType+"/PortI/RomFile", "").toString();
  // frequency
  this->m_SpectraVicraSettings.updateFrequency = this->m_Settings->value(systemType+"/updateFrequency", defaultFrequency).toDouble();

  // update the GUI - wired.
  this->m_Widget.port01RomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[0]);
  this->m_Widget.port02RomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[1]);
  this->m_Widget.port03RomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[2]);
  // update the GUI - wireless
  this->m_Widget.portARomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[3]);
  this->m_Widget.portBRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[4]);
  this->m_Widget.portCRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[5]);
  this->m_Widget.portDRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[6]);
  /* for future use
  this->m_Widget.portERomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[7]);
  this->m_Widget.portFRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[8]);
  this->m_Widget.portGRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[9]);
  this->m_Widget.portHRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[10]);
  this->m_Widget.portIRomFileLineEdit->setText(this->m_SpectraVicraSettings.romFiles[11]); */

  this->m_Widget.updateFrequencyLineEdit->setText(QString("%1").arg(this->m_SpectraVicraSettings.updateFrequency));

}

void vtkSpectraVicraTrackerSettingsWidget::WriteTrackerSettings()
{
  if( !m_Settings )
    return;

  QString systemType;
  double defaultFrequency;

  switch(this->m_SystemType)
  {
  case NDI_SPECTRA:
    systemType = "Spectra";
    defaultFrequency = 60.0;
    break;
  case NDI_SPECTRA_HYBRID:
    systemType = "SpectraHybrid";
    defaultFrequency = 60.0;
    break;
  case NDI_VICRA:
    systemType = "Vicra";
    defaultFrequency = 20.0;
    break;
  default:
    // todo: add an error pop-up.
    this->m_SystemType = NDI_SPECTRA;
    systemType = "Spectra";
    defaultFrequency = 60.0;
  }

  if( this->m_SystemType == NDI_SPECTRA_HYBRID )
  {
    // write the Spectra data.
    // wired port 01:
    if( this->m_Widget.port01VirtualRomCheckBox->isChecked() )
    {
      this->m_SpectraVicraSettings.romFiles[0] = this->m_Widget.port01RomFileLineEdit->text();
    }
    else
    {
      this->m_SpectraVicraSettings.romFiles[0].clear();
    }
    this->m_Settings->setValue(systemType+"/Port01/UseVirtualRom",this->m_Widget.port01VirtualRomCheckBox->isChecked() );
    this->m_Settings->setValue(systemType+"/Port01/RomFile", this->m_SpectraVicraSettings.romFiles[0]);

    // wired port 02:
    if( this->m_Widget.port02VirtualRomCheckBox->isChecked() )
    {
      this->m_SpectraVicraSettings.romFiles[1] = this->m_Widget.port02RomFileLineEdit->text();
    }
    else
    {
      this->m_SpectraVicraSettings.romFiles[1].clear();
    }
    this->m_Settings->setValue(systemType+"/Port02/UseVirtualRom", this->m_Widget.port02VirtualRomCheckBox->isChecked() );
    this->m_Settings->setValue(systemType+"/Port02/RomFile", this->m_SpectraVicraSettings.romFiles[1]);

    // wired port 03:
    if( this->m_Widget.port03VirtualRomCheckBox->isChecked() )
    {
      this->m_SpectraVicraSettings.romFiles[2] = this->m_Widget.port03RomFileLineEdit->text();
    }
    else
    {
      this->m_SpectraVicraSettings.romFiles[2].clear();
    }
    this->m_Settings->setValue(systemType+"/Port03/UseVirtualRom", this->m_Widget.port03VirtualRomCheckBox->isChecked() );
    this->m_Settings->setValue(systemType+"/Port03/RomFile", this->m_SpectraVicraSettings.romFiles[2]);
  }
  // wireless ports:
  this->m_SpectraVicraSettings.romFiles[3] = this->m_Widget.portARomFileLineEdit->text();
  this->m_Settings->setValue(systemType+"/PortA/RomFile", this->m_SpectraVicraSettings.romFiles[3]);
  this->m_SpectraVicraSettings.romFiles[4] = this->m_Widget.portBRomFileLineEdit->text();
  this->m_Settings->setValue(systemType+"/PortB/RomFile", this->m_SpectraVicraSettings.romFiles[4]);
  this->m_SpectraVicraSettings.romFiles[5] = this->m_Widget.portCRomFileLineEdit->text();
  this->m_Settings->setValue(systemType+"/PortC/RomFile", this->m_SpectraVicraSettings.romFiles[5]);
  this->m_SpectraVicraSettings.romFiles[6] = this->m_Widget.portDRomFileLineEdit->text();
  this->m_Settings->setValue(systemType+"/PortD/RomFile", this->m_SpectraVicraSettings.romFiles[6]);

  this->m_SpectraVicraSettings.updateFrequency = this->m_Widget.updateFrequencyLineEdit->text().toDouble();
  this->m_Settings->setValue(systemType+"/updateFrequency", this->m_SpectraVicraSettings.updateFrequency);
}

void vtkSpectraVicraTrackerSettingsWidget::UseSpectraHybrid()
{
  this->m_SystemType = NDI_SPECTRA_HYBRID;

  this->m_Widget.wiredToolsBox->setVisible(true);
  this->ReadTrackerSettings();
}

void vtkSpectraVicraTrackerSettingsWidget::UseSpectra()
{
  this->m_SystemType = NDI_SPECTRA;

  this->m_Widget.wiredToolsBox->setVisible(false);
  this->ReadTrackerSettings();
}

void vtkSpectraVicraTrackerSettingsWidget::UseVicra()
{
  this->m_SystemType = NDI_VICRA;

  this->m_Widget.wiredToolsBox->setVisible(false);
  this->ReadTrackerSettings();
}

void vtkSpectraVicraTrackerSettingsWidget::OnLoadRomFile()
{
  int port;
  QString filename;

  qDebug() << this->sender()->objectName();

  if( this->sender()->objectName().compare("port01RomFileBrowseButton") == 0 )
  {
    port = 0;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port 1"), 
      this->m_Widget.port01RomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.port01RomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("port02RomFileBrowseButton") == 0 )
  {
    port = 1;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port 2"), 
      this->m_Widget.port02RomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.port02RomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("port03RomFileBrowseButton") == 0 )
  {
    port = 2;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port 3"), 
      this->m_Widget.port03RomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.port03RomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("portARomFileBrowseButton") == 0 )
  {
    port = 3;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port A"), 
      this->m_Widget.portARomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.portARomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("portBRomFileBrowseButton") == 0 )
  {
    port = 4;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port B"), 
      this->m_Widget.portBRomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.portBRomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("portCRomFileBrowseButton") == 0 )
  {
    port = 5;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port C"), 
      this->m_Widget.portCRomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.portCRomFileLineEdit->setText(filename);
  }
  else if(this->sender()->objectName().compare("portDRomFileBrowseButton") == 0 )
  {
    port = 6;
    filename = QFileDialog::getOpenFileName(this, tr("Open Spectra ROM File: Port D"), 
      this->m_Widget.portDRomFileLineEdit->text(), tr("NDI ROM File (*.rom)") );
    if( filename.isEmpty() ) return;
    this->m_Widget.portDRomFileLineEdit->setText(filename);
  }
  else
  {
    //TODO: error pop-up.
  }
}

void vtkSpectraVicraTrackerSettingsWidget::OnUseManual(bool bUseManual)
{
  this->m_SpectraVicraSettings.bUseManual = bUseManual;

  if(!bUseManual)
  {
    this->m_Widget.baudRateComboBox->setCurrentIndex(0);
    this->m_Widget.commPortComboBox->setCurrentIndex(0);
  }
  else
  {
    this->m_SpectraVicraSettings.baudRate 
      = this->m_Widget.baudRateComboBox->itemData(this->m_Widget.baudRateComboBox->currentIndex()).toInt();
    this->m_SpectraVicraSettings.commPort 
      = this->m_Widget.commPortComboBox->itemData(this->m_Widget.commPortComboBox->currentIndex()).toInt();
  }
}

void vtkSpectraVicraTrackerSettingsWidget::OnCommPortChanged(int index)
{
  this->m_SpectraVicraSettings.commPort = this->m_Widget.commPortComboBox->itemData(index).toInt();
}

void vtkSpectraVicraTrackerSettingsWidget::OnBaudRateChanged(int index)
{
  this->m_SpectraVicraSettings.baudRate = this->m_Widget.baudRateComboBox->itemData(index).toInt();
}

void vtkSpectraVicraTrackerSettingsWidget::OnLogCommunicationChanged(bool bLogComm)
{
  this->m_SpectraVicraSettings.logComm = bLogComm;
}

