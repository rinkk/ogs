/**
 *
 * \copyright
 * Copyright (c) 2012-2019, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

// ** INCLUDES **
#include "VtkGeometrySource.h"

#include <logog/include/logog.hpp>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "GeoLib/Point.h"
#include "GeoLib/Polyline.h"
#include "GeoLib/Surface.h"

vtkStandardNewMacro(VtkGeometrySource);

VtkGeometrySource::VtkGeometrySource()
{
    this->SetNumberOfInputPorts(0);
}

VtkGeometrySource::~VtkGeometrySource() = default;

void VtkGeometrySource::setGeometry(GeoLib::GEOObjects const& geo_objects,
                                    std::string const& geo_name)
{
    _points = *geo_objects.getPointVec(geo_name);
    _lines = *geo_objects.getPolylineVec(geo_name);
    _surfaces = *geo_objects.getSurfaceVec(geo_name);
}

void VtkGeometrySource::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

int VtkGeometrySource::RequestData(vtkInformation* request,
                                   vtkInformationVector** inputVector,
                                   vtkInformationVector* outputVector)
{
    (void)request;
    (void)inputVector;

    if (_points.empty())
    {
        ERR("VtkPolylineSource::RequestData(): No data to process.");
        return 0;
    }
    
    //vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
    //vtkSmartPointer<vtkCellArray> newLines = vtkSmartPointer<vtkCellArray>::New();

    vtkSmartPointer<vtkPoints> obj_points = vtkSmartPointer<vtkPoints>::New();
    obj_points->SetNumberOfPoints(_points.size());
    unsigned i = 0;
    for (auto point : _points)
    {
        double const coords[3] = {(*point)[0], (*point)[1], (*point)[2]};
        obj_points->SetPoint(i++, coords);
    }

    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> point_ids = vtkSmartPointer<vtkIntArray>::New();
    createVertices(vertices, point_ids);
    vtkSmartPointer<vtkPolyData> output_points = vtkPolyData::New();
    output_points->SetPoints(obj_points);
    output_points->SetVerts(vertices);
    output_points->GetCellData()->AddArray(point_ids);
    output_points->GetCellData()->SetActiveAttribute("PointIDs", vtkDataSetAttributes::SCALARS);

    vtkSmartPointer<vtkCellArray> lines =  vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIntArray> line_ids = vtkSmartPointer<vtkIntArray>::New();
    createLineObjects(obj_points, lines, line_ids);
    vtkSmartPointer<vtkPolyData> output_lines = vtkPolyData::New();
    output_lines->SetPoints(obj_points);
    output_lines->SetLines(lines);
    output_lines->GetCellData()->AddArray(line_ids);
    output_lines->GetCellData()->SetActiveAttribute("PolylineIDs", vtkDataSetAttributes::SCALARS);
    output_lines->Squeeze();

    //vtkSmartPointer<vtkPolyData> output_surfaces = vtkPolyData::New();

    vtkSmartPointer<vtkInformation> out_info_obj = outputVector->GetInformationObject(0);
    vtkSmartPointer<vtkMultiBlockDataSet> output =
        vtkMultiBlockDataSet::SafeDownCast(out_info_obj->Get(vtkDataObject::DATA_OBJECT()));
    if (out_info_obj->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
        return 1;

    output->SetBlock(0, output_points);
    output->SetBlock(1, output_lines);
    //output->SetBlock(2, output_surfaces);

    return 1;
}

void VtkGeometrySource::createVertices(
    vtkSmartPointer<vtkCellArray> vertices,
    vtkSmartPointer<vtkIntArray> point_ids)
{
    std::size_t n_points (_points.size());
    vertices->Allocate(n_points);
    point_ids->SetNumberOfComponents(1);
    point_ids->SetNumberOfValues(n_points);
    point_ids->SetName("PointIDs");
    for (std::size_t i = 0; i < n_points; ++i)
    {
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(i);
        point_ids->SetValue(i, _points[i]->getID());
    }
}

void VtkGeometrySource::createLineObjects(
    vtkSmartPointer<vtkPoints> obj_points,
    vtkSmartPointer<vtkCellArray> lines,
    vtkSmartPointer<vtkIntArray> line_ids)
{
    std::size_t n_lines(_lines.size());
    lines->Allocate(n_lines);
    line_ids->Allocate(n_lines);
    line_ids->SetNumberOfComponents(1);
    line_ids->SetName("PolylineIDs");

    int count = 0;
    for (auto line : _lines)
    {
        int const n_points = line->getNumberOfPoints();
        lines->InsertNextCell(n_points);
        line_ids->InsertNextValue(count++);
        for (int i = 0; i < n_points; i++)
        {
            std::size_t const point_id = line->getPointID(i);
            lines->InsertCellPoint(point_id);
        }
    }
}



int VtkGeometrySource::RequestInformation(
    vtkInformation* /*request*/,
    vtkInformationVector** /*inputVector*/,
    vtkInformationVector* /*outputVector*/)
{
    return 1;
}

void VtkGeometrySource::SetUserProperty(QString name, QVariant value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}
