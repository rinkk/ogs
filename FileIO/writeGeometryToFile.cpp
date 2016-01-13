/**
 *
 * \copyright
 * Copyright (c) 2012-2016, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "writeGeometryToFile.h"

#include "BaseLib/FileTools.h"

#include "FileIO/XmlIO/Boost/BoostXmlGmlInterface.h"
#include "FileIO/Legacy/OGSIOVer4.h"

#include "GeoLib/GEOObjects.h"

namespace FileIO
{
void writeGeometryToFile(std::string const& geo_name,
	GeoLib::GEOObjects& geo_objs, std::string const& fname)
{
	std::string const extension(BaseLib::getFileExtension(fname));
	if (extension == "gml" || extension == "GML") {
		FileIO::BoostXmlGmlInterface xml(geo_objs);
		xml.setNameForExport(geo_name);
		xml.writeToFile(fname);
	} else if (extension == "gli" || extension == "GLI") {
		FileIO::Legacy::writeGLIFileV4(fname, geo_name, geo_objs);
	} else {
		ERR("Writing of geometry failed, since it was not possible to determine"
			" the required format from file extension.");
	}
}
}
