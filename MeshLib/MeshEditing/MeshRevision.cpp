/**
 * \file   MeshRevision.cpp
 * \author Karsten Rink
 * \date   2014-02-14
 * \brief  Implementation of the MeshRevision class. 
 *
 * \copyright
 * Copyright (c) 2013, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

// ThirdParty/logog
#include "logog/include/logog.hpp"

#include "MeshRevision.h"

// GeoLib
#include "Grid.h"

// MeshLib
#include "Mesh.h"
#include "Node.h"
#include "Elements/Element.h"
#include "Elements/Line.h"
#include "Elements/Tri.h"
#include "Elements/Quad.h"
#include "Elements/Tet.h"
#include "Elements/Hex.h"
#include "Elements/Pyramid.h"
#include "Elements/Prism.h"


namespace MeshLib {

MeshRevision::MeshRevision(Mesh const*const mesh) :
	_mesh (mesh)
{}


MeshLib::Mesh* MeshRevision::collapseNodes(const std::string &new_mesh_name, double eps)
{
	std::vector<MeshLib::Node*> new_nodes = this->collapseNodeIndeces(eps);
	const std::size_t nElems (this->_mesh->getNElements());
	std::vector<MeshLib::Element*> new_elements (nElems);
	const std::vector<MeshLib::Element*> elements (this->_mesh->getElements());
	for (std::size_t i=0; i<nElems; ++i)
		new_elements[i] = copyElement(elements[i], new_nodes);
	return new MeshLib::Mesh(new_mesh_name, new_nodes, new_elements);
}

MeshLib::Mesh* MeshRevision::simplifyMesh(const std::string &new_mesh_name, double eps, unsigned min_elem_dim)
{
	std::vector<MeshLib::Node*> new_nodes = this->collapseNodeIndeces(eps);
	std::vector<MeshLib::Element*> new_elements;

	const std::size_t nElems (this->_mesh->getNElements());
	const std::vector<MeshLib::Element*> elements (this->_mesh->getElements());
	for (std::size_t i=0; i<nElems; ++i)
	{
		unsigned n_unique_nodes (this->getNUniqueNodes(elements[i]));
		if (n_unique_nodes == elements[i]->getNNodes() && elements[i]->getDimension() >= min_elem_dim)
			new_elements.push_back(copyElement(elements[i], new_nodes));
		else if (n_unique_nodes < elements[i]->getNNodes() && n_unique_nodes>1)
			reduceElement(elements[i], n_unique_nodes, new_nodes, new_elements, min_elem_dim);
		else
			std::cout << "Error: Something is wrong, more unique nodes than actual nodes" << std::endl;

	}
	if (new_elements.size()>0)
		return new MeshLib::Mesh(new_mesh_name, new_nodes, new_elements);
	return nullptr;
}

std::vector<MeshLib::Node*> MeshRevision::collapseNodeIndeces(double eps)
{
	const std::vector<MeshLib::Node*> nodes (_mesh->getNodes());
	const std::size_t nNodes (_mesh->getNNodes());
	std::vector<std::size_t> id_map(nNodes);
	for (std::size_t k=0; k<nNodes; ++k)
		id_map[k]=k;

	GeoLib::Grid<MeshLib::Node>* grid(new GeoLib::Grid<MeshLib::Node>(nodes.begin(), nodes.end(), 64));

	for (size_t k=0; k<nNodes; ++k)
	{
		std::vector<std::vector<MeshLib::Node*> const*> node_vectors;
		MeshLib::Node const*const node(nodes[k]);
		grid->getPntVecsOfGridCellsIntersectingCube(node->getCoords(), eps, node_vectors);

		const size_t nVectors (node_vectors.size());
		for (size_t i=0; i<nVectors; ++i)
		{
			std::vector<MeshLib::Node*> cell_vector (*node_vectors[i]);
			const size_t nGridCellNodes (cell_vector.size());
			for (size_t j=0; j<nGridCellNodes; ++j)
			{
				MeshLib::Node* test_node(cell_vector[j]);
				// are node indices already identical (i.e. nodes will be collapsed)
				if (id_map[node->getID()] == id_map[test_node->getID()])
					continue;

				// if test_node has already been collapsed to another node x, ignore it
				// (if the current node would need to be collapsed with x it would already have happened when x was tested)
				if (test_node->getID() != id_map[test_node->getID()])
					continue;

				// calc distance
				if (MathLib::sqrDist(node->getCoords(), test_node->getCoords()) < eps)
					id_map[test_node->getID()] = node->getID();
			}
		}
	}

	return this->constructNewNodesArray(id_map);
}

std::vector<MeshLib::Node*> MeshRevision::constructNewNodesArray(const std::vector<std::size_t> &id_map)
{
	std::vector<MeshLib::Node*> nodes (_mesh->getNodes());
	const std::size_t nNodes (nodes.size());
	std::vector<MeshLib::Node*> new_nodes;
	for (std::size_t k=0; k<nNodes; ++k)
	{
		// all nodes that have not been collapsed with other nodes are copied into new array
		if (nodes[k]->getID() == id_map[k])
		{
			unsigned id = static_cast<unsigned>(new_nodes.size());
			new_nodes.push_back (new MeshLib::Node( (*nodes[k])[0], (*nodes[k])[1], (*nodes[k])[2], id) );
			nodes[k]->setID(id); // the node in the old array gets the index of the same node in the new array
		}
		// the other nodes are not copied and get the index of the nodes they will have been collapsed with
		else
			nodes[k]->setID(nodes[id_map[k]]->getID());
	}
	return new_nodes;
}

unsigned MeshRevision::getNUniqueNodes(MeshLib::Element const*const element) const
{
	unsigned nNodes (element->getNNodes());
	unsigned count (nNodes);

	for (unsigned i=0; i<nNodes-1; ++i)
		for (unsigned j=i+1; j<nNodes; ++j)
			if (i!=j && element->getNode(i)->getID()==element->getNode(j)->getID())
				count--;
	return count;
}

MeshLib::Element* MeshRevision::copyElement(MeshLib::Element const*const element, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Element* new_elem (nullptr);
	if (element->getGeomType() == MeshElemType::LINE)
		return this->copyLine(element, nodes);
	else if (element->getGeomType() == MeshElemType::TRIANGLE)
		return this->copyTri(element, nodes);
	else if (element->getGeomType() == MeshElemType::QUAD)
		return this->copyQuad(element, nodes);
	else if (element->getGeomType() == MeshElemType::TETRAHEDRON)
		return this->copyTet(element, nodes);
	else if (element->getGeomType() == MeshElemType::HEXAHEDRON)
		return this->copyHex(element, nodes);
	else if (element->getGeomType() == MeshElemType::PYRAMID)
		return this->copyPyramid(element, nodes);
	else if (element->getGeomType() == MeshElemType::PRISM)
		return this->copyPrism(element, nodes);

	ERR ("Error: Unknown element type.");
	return nullptr;
}

void MeshRevision::reduceElement(MeshLib::Element const*const element, 
								 unsigned n_unique_nodes, 
								 const std::vector<MeshLib::Node*> &nodes,
								 std::vector<MeshLib::Element*> &elements,
								 unsigned min_elem_dim) const
{
	/***************
	 * f�r prisms & hexes kann es passieren dass das element in MEHRERE neue elemente zerlegt wird.
	 * dann m�ssen nachbarelemente angepasst werden, um keine "freien" kanten zu erzeugen.
	 */
	MeshLib::Element* new_elem (nullptr);
	if (element->getGeomType() == MeshElemType::TRIANGLE && min_elem_dim == 1)
		elements.push_back (this->constructLine(element, nodes));
	else if (element->getGeomType() == MeshElemType::QUAD)
	{
		if (n_unique_nodes == 3 && min_elem_dim < 3)
			elements.push_back (this->constructTri(element, nodes));
		else if (min_elem_dim == 1)
			elements.push_back (this->constructLine(element, nodes));
	}
	else if (element->getGeomType() == MeshElemType::TETRAHEDRON)
	{
		if (n_unique_nodes == 3 && min_elem_dim<3)
			elements.push_back (this->constructTri(element, nodes));
		else if (min_elem_dim == 1)
			elements.push_back (this->constructLine(element, nodes));
	}
	else if (element->getGeomType() == MeshElemType::HEXAHEDRON)
		this->reduceHex(element, n_unique_nodes, nodes, elements, min_elem_dim);
	else if (element->getGeomType() == MeshElemType::PYRAMID)
		this->reducePyramid(element, n_unique_nodes, nodes, elements, min_elem_dim);
	else if (element->getGeomType() == MeshElemType::PRISM)
		this->reducePrism(element, n_unique_nodes, nodes, elements, min_elem_dim);
	else
		ERR ("Error: Unknown element type.");
	return;
}

MeshLib::Element* MeshRevision::copyLine(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[2];
	new_nodes[0] = nodes[org_elem->getNode(0)->getID()];
	new_nodes[1] = nodes[org_elem->getNode(1)->getID()];
	return new MeshLib::Line(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyTri(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[3];
	for (unsigned i=0; i<3; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Tri(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyQuad(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[4];
	for (unsigned i=0; i<4; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Quad(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyTet(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[4];
	for (unsigned i=0; i<4; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Tet(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyHex(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[8];
	for (unsigned i=0; i<8; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Hex(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyPyramid(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[5];
	for (unsigned i=0; i<5; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Pyramid(new_nodes, org_elem->getValue());
}

MeshLib::Element* MeshRevision::copyPrism(MeshLib::Element const*const org_elem, const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[6];
	for (unsigned i=0; i<6; ++i)
		new_nodes[i] = nodes[org_elem->getNode(i)->getID()];
	return new MeshLib::Prism(new_nodes, org_elem->getValue());
}

bool MeshRevision::reduceHex(MeshLib::Element const*const org_elem, 
							 unsigned n_unique_nodes, 
							 const std::vector<MeshLib::Node*> &nodes,
							 std::vector<MeshLib::Element*> &new_elements,
							 unsigned min_elem_dim) const
{
	/***** 
	 * return false if more than one element is created
	 */
	if (n_unique_nodes == 7)
	{
		for (unsigned i=0; i<6; ++i)
			for (unsigned j=1; j<7; ++j)
				if (org_elem->getNode(i)->getID() == org_elem->getNode(j)->getID())
				{
					if (i == j-1 || i == j-3) //top or bottom edge, prism is "lying" on a quad side
					{
						unsigned offset = (i<4) ? -4 : 4;
						MeshLib::Node** pyr_nodes = new MeshLib::Node*[5];
						pyr_nodes[0] = nodes[(i+offset-1)%8];//stimmt nicht
						pyr_nodes[1] = nodes[(i+4)%8];
						pyr_nodes[2] = nodes[(j+4)%8];
						pyr_nodes[3] = nodes[(j+1)%8];	// stimmt nicht
						pyr_nodes[4] = nodes[org_elem->getNode(i)->getID()];
						new_elements.push_back (new MeshLib::Tet(tet1_nodes, org_elem->getValue()));

						MeshLib::Node** tet2_nodes = new MeshLib::Node*[4];
						tet2_nodes[0] = nodes[org_elem->getNode(i+offset)->getID()];
						tet2_nodes[1] = nodes[org_elem->getNode(j+offset)->getID()];
						tet2_nodes[2] = nodes[org_elem->getNode(k+offset)->getID()];
						tet2_nodes[3] = nodes[org_elem->getNode(k)->getID()];
						new_elements.push_back (new MeshLib::Tet(tet2_nodes, org_elem->getValue()));
						return false;
					}
					else // vertical edge, prism is "standing upright"
					{
					}
				}
	// n=7 - hex can be reduced to a prism and a pyramid 
	//  find the prism where no node such that the two collapsed nodes are seperate (they will form the top of the pyramid)
		return false;
	}
	if (n_unique_nodes == 6)
	{
	// n=6 - reduce to a prism OR two pyramids OR three tets
	}
	if (n_unique_nodes == 5)
	{
	// n=5 - reduce to a pyramid OR two tets
	}
	if (n_unique_nodes == 4)
	{
		MeshLib::Element* elem (this->constructFourNodeElement(org_elem, nodes, min_elem_dim));
		if (elem)
			new_elements.push_back (elem);
	}
	if (n_unique_nodes == 3 && min_elem_dim < 3)
		new_elements.push_back (this->constructTri(org_elem, nodes));
	else if (min_elem_dim == 1)
		new_elements.push_back (this->constructLine(org_elem, nodes));
	return true;
}

void MeshRevision::reducePyramid(MeshLib::Element const*const org_elem, 
								 unsigned n_unique_nodes, 
								 const std::vector<MeshLib::Node*> &nodes, 
								 std::vector<MeshLib::Element*> &new_elements,
								 unsigned min_elem_dim) const
{
	if (n_unique_nodes == 4)
	{
		MeshLib::Node** new_nodes = new MeshLib::Node*[4];
		for (unsigned i=0; i<4; ++i)
			// top node is collapsed, element is reduced to Quad
			if (org_elem->getNode(i)->getID() == org_elem->getNode(4)->getID() && min_elem_dim < 3)
			{
				for (unsigned j=0; j<4; ++j)
					new_nodes[j] = nodes[org_elem->getNode(j)->getID()];
				new_elements.push_back (new MeshLib::Quad(new_nodes, org_elem->getValue()));
				return;
			}

		// one of the base nodes is collapsed, element is reduced to Tet
		new_nodes[0] = nodes[org_elem->getNode(0)->getID()];
		new_nodes[1] = (org_elem->getNode(0)->getID() == org_elem->getNode(1)->getID()) ?
			           nodes[org_elem->getNode(2)->getID()] : nodes[org_elem->getNode(1)->getID()];
		new_nodes[2] = (org_elem->getNode(2)->getID() == org_elem->getNode(3)->getID()) ?
			           nodes[org_elem->getNode(2)->getID()] : nodes[org_elem->getNode(3)->getID()];
		new_nodes[3] = nodes[org_elem->getNode(4)->getID()];
		new_elements.push_back (new MeshLib::Tet(new_nodes, org_elem->getValue()));
	}
	else if (n_unique_nodes == 3 && min_elem_dim < 3)
		new_elements.push_back (this->constructTri(org_elem, nodes));
	else if (n_unique_nodes == 2 && min_elem_dim == 1)
		new_elements.push_back (this->constructLine(org_elem, nodes));
	return;
}

bool MeshRevision::reducePrism(MeshLib::Element const*const org_elem, 
							   unsigned n_unique_nodes, 
							   const std::vector<MeshLib::Node*> &nodes, 
							   std::vector<MeshLib::Element*> &new_elements,
							   unsigned min_elem_dim) const
{
	/***** 
	 * return false if more than one element is created
	 */
	// if one of the non-triangle edges collapsed, elem can be reduced to a pyramid, otherwise it will be two tets
	if (n_unique_nodes == 5)
	{
		for (unsigned i=0; i<5; ++i)
			for (unsigned j=i+1; j<6; ++j)
				if (i!=j && org_elem->getNode(i)->getID() == org_elem->getNode(j)->getID())
				{
					if (i%3 == j%3)
					{
						// non triangle edge collapsed
						MeshLib::Node** pyramid_nodes = new MeshLib::Node*[5];
						pyramid_nodes[0] = nodes[org_elem->getNode((i+1)%3)->getID()];
						pyramid_nodes[1] = nodes[org_elem->getNode((i+2)%3)->getID()];
						pyramid_nodes[2] = nodes[org_elem->getNode((i+1)%3+3)->getID()];
						pyramid_nodes[3] = nodes[org_elem->getNode((i+2)%3+3)->getID()];
						pyramid_nodes[4] = nodes[org_elem->getNode(i)->getID()];
						new_elements.push_back (new MeshLib::Pyramid(pyramid_nodes, org_elem->getValue()));
						return true;
					}

					// triangle edge collapsed 
					///(can this actually happen? this would mean that at least one of the quad faces were degenerated?!)
					unsigned offset = (i>2) ? -3 : 3;
					unsigned k = (j==i+1) ? ((i>2) ? (i+2)%3+3 : (i+2)%3) : i+1; // sorry!
					MeshLib::Node** tet1_nodes = new MeshLib::Node*[4];
					tet1_nodes[0] = nodes[org_elem->getNode(i+offset)->getID()];
					tet1_nodes[1] = nodes[org_elem->getNode(j+offset)->getID()];
					tet1_nodes[2] = nodes[org_elem->getNode(k)->getID()];
					tet1_nodes[3] = nodes[org_elem->getNode(i)->getID()];
					new_elements.push_back (new MeshLib::Tet(tet1_nodes, org_elem->getValue()));

					MeshLib::Node** tet2_nodes = new MeshLib::Node*[4];
					tet2_nodes[0] = nodes[org_elem->getNode(i+offset)->getID()];
					tet2_nodes[1] = nodes[org_elem->getNode(j+offset)->getID()];
					tet2_nodes[2] = nodes[org_elem->getNode(k+offset)->getID()];
					tet2_nodes[3] = nodes[org_elem->getNode(k)->getID()];
					new_elements.push_back (new MeshLib::Tet(tet2_nodes, org_elem->getValue()));
					return false;
				}
	}
	else if (n_unique_nodes == 4)
	{
		MeshLib::Element* elem (this->constructFourNodeElement(org_elem, nodes, min_elem_dim));
		if (elem)
			new_elements.push_back (elem);
	}
	else if (n_unique_nodes == 3 && min_elem_dim < 3)
		new_elements.push_back (this->constructTri(org_elem, nodes));
	else if (n_unique_nodes == 2 && min_elem_dim == 1)
		new_elements.push_back (this->constructLine(org_elem, nodes));
	return true;
}

MeshLib::Element* MeshRevision::constructLine(MeshLib::Element const*const element, 
											  const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** line_nodes = new MeshLib::Node*[2];
	line_nodes[0] = nodes[element->getNode(0)->getID()];
	for (unsigned i=1; i<element->getNNodes(); ++i)
	{
		if (element->getNode(i)->getID() != element->getNode(0)->getID())
		{
			line_nodes[1] = nodes[element->getNode(i)->getID()];
			break;
		}
	}
	return new MeshLib::Line(line_nodes, element->getValue());
}

MeshLib::Element* MeshRevision::constructTri(MeshLib::Element const*const element, 
											 const std::vector<MeshLib::Node*> &nodes) const
{
	MeshLib::Node** tri_nodes = new MeshLib::Node*[3];
	tri_nodes[0] = nodes[element->getNode(0)->getID()];
	for (unsigned i=1; i<element->getNNodes(); ++i)
		if (element->getNode(i)->getID() != element->getNode(0)->getID())
		{
			tri_nodes[1] = nodes[element->getNode(i)->getID()];
			for (unsigned j=i+1; i<element->getNNodes(); ++j)
				if (element->getNode(i)->getID() != element->getNode(0)->getID() &&
					element->getNode(i)->getID() != element->getNode(i)->getID())
				{
					tri_nodes[2] = nodes[element->getNode(j)->getID()];
					return new MeshLib::Tri(tri_nodes, element->getValue());
				}
		}
	// this should never be reached
	return nullptr;
}

MeshLib::Element* MeshRevision::constructFourNodeElement(MeshLib::Element const*const element, const std::vector<MeshLib::Node*> &nodes, unsigned min_elem_dim) const
{
	MeshLib::Node** new_nodes = new MeshLib::Node*[4];
	unsigned count(0);
	new_nodes[count++] = nodes[element->getNode(0)->getID()];
	for (unsigned i=1; i<element->getNNodes(); ++i)
	{
		bool unique_node (true);
		for (unsigned j=0; i<i; ++j)
		{
			if (element->getNode(i)->getID() == element->getNode(j)->getID())
			{
				unique_node = false;
				break;
			}
			if (unique_node)
				new_nodes[count++] = nodes[element->getNode(i)->getID()];; 
		}
	}
	
	bool isQuad (GeoLib::isCoplanar(*new_nodes[0], *new_nodes[1], *new_nodes[2], *new_nodes[3]));
	if (isQuad && min_elem_dim < 3)
		return new MeshLib::Quad(new_nodes, element->getValue());
	else if (!isQuad)
		return new MeshLib::Tet(new_nodes, element->getValue());
	else // is quad but min elem dim == 3
		return nullptr;
}


} // end namespace MeshLib
