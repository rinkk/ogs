/**
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "MeshMapping2DDialog.h"
#include "OGSError.h"
#include "StrictDoubleValidator.h"

#include <QSettings>
#include <QFileDialog>

MeshMapping2DDialog::MeshMapping2DDialog(QDialog* parent)
: QDialog(parent)
{
	setupUi(this);

	StrictDoubleValidator* no_data_validator = new StrictDoubleValidator(this);
	noDataValueEdit->setValidator (no_data_validator);
	StrictDoubleValidator* static_value_validator = new StrictDoubleValidator(this);
	staticValueEdit->setValidator (static_value_validator);
}

void MeshMapping2DDialog::on_rasterValueButton_toggled(bool isChecked)
{
	rasterPathEdit->setEnabled(isChecked);
	noDataValueEdit->setEnabled(isChecked);
	rasterSelectButton->setEnabled(isChecked);
	staticValueEdit->setEnabled(!isChecked);
}

void MeshMapping2DDialog::on_rasterSelectButton_pressed()
{
	QSettings settings;
	QString filename = QFileDialog::getOpenFileName(
		this, "Select raster file to open",
		settings.value("lastOpenedRasterFileDirectory").toString(),
		"ASCII raster files (*.asc);;All files (* *.*)");
	rasterPathEdit->setText(filename);
	QFileInfo fi(filename);
	settings.setValue("lastOpenedRasterFileDirectory", fi.absolutePath());
}

void MeshMapping2DDialog::accept()
{
	if (rasterValueButton->isChecked() && rasterPathEdit->text().isEmpty())
	{
		OGSError::box("Please specify path to raster file.");
		return;
	}
	if (rasterValueButton->isChecked() && noDataValueEdit->text().isEmpty())
	{
		OGSError::box("Please specify No Data value.");
		return;
	}
	if (staticValueButton->isChecked() && staticValueEdit->text().isEmpty())
	{
		OGSError::box("Please specify value for mapping.");
		return;
	}
	if (newNameEdit->text().isEmpty())
	{
		OGSError::box("Please specify a name for the resulting mesh.");
		return;
	}

	done(QDialog::Accepted);
}

