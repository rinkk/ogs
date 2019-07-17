/**
 *
 * \copyright
 * Copyright (c) 2012-2019, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#pragma once

#include "GeoLib/GEOObjects.h"

#include "VtkAlgorithmProperties.h"
#include <vtkMultiBlockDataSetAlgorithm.h>
#include <vtkSmartPointer.h>

namespace GeoLib {
    class Point;
    class Polyline;
    class Surface;
}

class vtkCellArray;
class vtkIntArray;
class vtkPoints;

class VtkGeometrySource : public vtkMultiBlockDataSetAlgorithm,
                          public VtkAlgorithmProperties
{
public:
    /// Create new objects with New() because of VTKs object reference counting.
    static VtkGeometrySource* New();

    vtkTypeMacro(VtkGeometrySource, vtkMultiBlockDataSetAlgorithm);

    /// Sets the polyline vector.
    void setGeometry(GeoLib::GEOObjects const& geo_objects, std::string const& geo_name);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    void SetUserProperty(QString name, QVariant value) override;

protected:
    VtkGeometrySource();
    ~VtkGeometrySource() override;

    /// Computes the polygonal data object.
    int RequestData(vtkInformation* request,
                    vtkInformationVector** inputVector,
                    vtkInformationVector* outputVector) override;

    int RequestInformation(vtkInformation* request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector) override;

    void createVertices(vtkSmartPointer<vtkCellArray> vertices,
                        vtkSmartPointer<vtkIntArray> point_ids);

    void createLineObjects(vtkSmartPointer<vtkPoints> obj_points,
                           vtkSmartPointer<vtkCellArray> lines,
                           vtkSmartPointer<vtkIntArray> line_ids);

    void createSurfaceObjects(vtkSmartPointer<vtkPoints> obj_points,
                           vtkSmartPointer<vtkCellArray> surfaces,
                           vtkSmartPointer<vtkIntArray> sfc_ids);

    std::vector<GeoLib::Point*> _points;
    std::vector<GeoLib::Polyline*> _lines;
    std::vector<GeoLib::Surface*> _surfaces;

private:
};
