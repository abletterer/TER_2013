/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* version 0.1                                                                  *
* Copyright (C) 2009-2012, IGG Team, LSIIT, University of Strasbourg           *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef __VSPLIT_H__
#define __VSPLIT_H__

namespace CGoGN
{

namespace Algo
{

namespace Surface
{

namespace VDPMesh
{

template <typename PFP>
class VSplit 
{
public:
	typedef typename PFP::MAP MAP ;
	typedef typename PFP::VEC3 VEC3 ;

private:
	MAP& m_map ;
	Dart m_edge ;
	Dart m_right_edge ;
	Dart m_left_edge ;
    Dart m_opposite_right_edge;
    Dart m_opposite_left_edge;
	unsigned int approxVertexId ;
	unsigned int approxEdgeId1, approxEdgeId2 ;

public:
	VSplit(MAP& m, Dart e, Dart r, Dart l, Dart ro, Dart lo)
		: m_map(m), m_edge(e), m_right_edge(r), m_left_edge(l), m_opposite_right_edge(ro), m_opposite_left_edge(lo), approxVertexId(EMBNULL), approxEdgeId1(EMBNULL), approxEdgeId2(EMBNULL)
	{}
	~VSplit()
	{
		AttributeContainer& cont = m_map.template getAttributeContainer<VERTEX>() ;
		if(approxVertexId != EMBNULL) cont.unrefLine(approxVertexId) ;
	}

	Dart getEdge() { return m_edge ; }
	Dart getLeftEdge() { return m_left_edge ; }
	Dart getRightEdge() { return m_right_edge ; }
	Dart getOppositeRightEdge() { return m_opposite_right_edge ; }
	Dart getOppositeLeftEdge() { return m_opposite_left_edge ; }
	void setEdge(Dart edge) { m_edge = edge ; }
	void setLeftEdge(Dart edge) { m_left_edge = edge ; }
	void setRightEdge(Dart edge) { m_right_edge = edge ; }
	void setOppositeRightEdge(Dart edge) { m_opposite_right_edge = edge ; }
	void setOppositeLeftEdge(Dart edge) { m_opposite_left_edge = edge ; }

	unsigned int getApproxV() { return approxVertexId ; }
	void setApproxV(unsigned int id)
	{
		if(approxVertexId == id) return ;
		AttributeContainer& cont = m_map.template getAttributeContainer<VERTEX>() ;
		if(approxVertexId != EMBNULL)
			cont.unrefLine(approxVertexId) ;
		if(id != EMBNULL) cont.refLine(id) ;
		approxVertexId = id ;
	}

	unsigned int getApproxE1() { return approxEdgeId1 ; }
	void setApproxE1(unsigned int id)
	{
		if(approxEdgeId1 == id) return ;
		AttributeContainer& cont = m_map.template getAttributeContainer<EDGE>() ;
		if(approxEdgeId1 != EMBNULL)
			cont.unrefLine(approxEdgeId1) ;
		if(id != EMBNULL) cont.refLine(id) ;
		approxEdgeId1 = id ;
	}

	unsigned int getApproxE2() { return approxEdgeId2 ; }
	void setApproxE2(unsigned int id)
	{
		if(approxEdgeId2 == id) return ;
		AttributeContainer& cont = m_map.template getAttributeContainer<EDGE>() ;
		if(approxEdgeId2 != EMBNULL)
			cont.unrefLine(approxEdgeId2) ;
		if(id != EMBNULL) cont.refLine(id) ;
		approxEdgeId2 = id ;
	}

    MAP& getMap() { return m_map; }

    bool operator==(const VSplit& vs) 
    {
        return m_edge==vs.m_edge && m_left_edge==vs.m_left_edge && m_right_edge==vs.m_right_edge;
    }
} ;

} //namespace VDPMesh
} //namespace Surface
} //namespace Algo
} //namespace CGoGN

#endif
