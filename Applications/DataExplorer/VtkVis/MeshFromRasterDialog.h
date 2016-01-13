/**
 * \file
 * \author Karsten Rink
 * \date   2011-11-17
 * \brief  Definition of the MeshFromRasterDialog class.
 *
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#ifndef MSHFROMRASTERDIALOG_H
#define MSHFROMRASTERDIALOG_H

#include "ui_MeshFromRaster.h"

#include <QDialog>

namespace MeshLib {
	enum class MeshElemType;
	enum class UseIntensityAs;
}


/**
 * \brief A dialog for specifying the parameters to construct a mesh based on a raster
 */
class MeshFromRasterDialog : public QDialog, private Ui_MeshFromRaster
{
	Q_OBJECT

public:
	/// Constructor
	MeshFromRasterDialog(QDialog* parent = 0);

	~MeshFromRasterDialog(void);

private slots:
	/// Instructions if the OK-Button has been pressed.
	void accept();

	/// Instructions if the Cancel-Button has been pressed.
	void reject();

signals:
	void setMeshParameters(QString, MeshLib::MeshElemType, MeshLib::UseIntensityAs);

};

#endif //MSHFROMRASTERDIALOG_H
