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

#include "Algo/Geometry/localFrame.h"

namespace CGoGN
{

namespace Algo
{

namespace Surface
{

namespace VDPMesh
{
    
template <typename PFP>
VDProgressiveMesh<PFP>::VDProgressiveMesh(
		MAP& map, DartMarker& inactive,
		VertexAttribute<typename PFP::VEC3>& position
	) :
	m_map(map), positionsTable(position), inactiveMarker(inactive), dartSelect(inactiveMarker)
{
	CGoGNout << "  creating approximator .." << CGoGNflush ;

	std::vector<VertexAttribute< typename PFP::VEC3>* > pos_v ;
	pos_v.push_back(&positionsTable) ;
	m_approximators.push_back(new Algo::Surface::Decimation::Approximator_MidEdge<PFP>(m_map, pos_v)) ;

	CGoGNout << "..done" << CGoGNendl ;

	CGoGNout << "  creating selector.." << CGoGNflush ;
			
    m_selector = new Algo::Surface::Decimation::EdgeSelector_Length<PFP>(m_map, positionsTable, m_approximators, dartSelect) ;
	
    CGoGNout << "..done" << CGoGNendl ;

	m_initOk = true ;

	CGoGNout << "  initializing approximators.." << CGoGNflush ;

	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
	{
		if(! (*it)->init())
			m_initOk = false ;
		if((*it)->getApproximatedAttributeName() == "position")
			m_positionApproximator = reinterpret_cast<Algo::Surface::Decimation::Approximator<PFP, VEC3, EDGE>*>(*it) ;
	}

	CGoGNout << "..done" << CGoGNendl ;

	CGoGNout << "  initializing selector.." << CGoGNflush ;

	m_initOk = m_selector->init() ;
	
    CGoGNout << "..done" << CGoGNendl ;
    
    noeud = m_map.template getAttribute<EmbNode, VERTEX>("noeud") ;
	if(!noeud.isValid())
		noeud = m_map.template addAttribute<EmbNode, VERTEX>("noeud") ;
}

template <typename PFP>
VDProgressiveMesh<PFP>::~VDProgressiveMesh()
{
	m_active_nodes.clear();
    if(m_selector)
		delete m_selector ;
	for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		delete (*it) ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::addNodes() {
    TraversorV<MAP> trav(m_map);
    for(Dart d = trav.begin(); d!=trav.end(); d = trav.next()) {
        noeud[d].node = new Node(NULL, true, m_map.template getEmbedding<VERTEX>(d), 0);
        m_active_nodes.push_front(noeud[d].node);
        noeud[d].node->setCurrentPosition(m_active_nodes.begin());
    }
    m_height = 0;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::createPM(unsigned int percentWantedVertices)
{
	unsigned int nbVertices = m_map.template getNbOrbits<VERTEX>() ;
	unsigned int nbWantedVertices = nbVertices * percentWantedVertices / 100 ;
    
    CGoGNout << "  addingNodes.." << CGoGNflush ;
    addNodes();
	CGoGNout << "..done" << CGoGNendl ;
	
    CGoGNout << "  creating PM (" << nbVertices << " vertices).." << /* flush */ CGoGNflush ;
    
	bool finished = false ;
	Dart d ;
	while(!finished)
	{
		if(!m_selector->nextEdge(d))
			break ;

		--nbVertices ;
		Dart d2 = m_map.phi2(m_map.phi_1(d)) ;
		Dart dd2 = m_map.phi2(m_map.phi_1(m_map.phi2(d))) ;
		Dart d1 = m_map.phi2(m_map.phi1(d)) ;
	    Dart dd1 = m_map.phi2(m_map.phi1(m_map.phi2(d))) ;

		VSplit<PFP>* vs = new VSplit<PFP>(m_map, d, dd2, d2, dd1, d1) ;	// create new VSplit node 
        
        Node* n = new Node(vs);   //Création du nouveau noeud de l'arbre

        Node* n_d2 = noeud[d2].node;
        Node* n_dd2 = noeud[dd2].node;

        /*Mise en place de la hiérarchie par rapport au nouveau noeud*/
        n->setLeftChild(n_d2);
        n->setRightChild(n_dd2);
        n_d2->setParent(n);
        n_dd2->setParent(n);

        /*Calcul de la hauteur du plus grand arbre de la forêt*/
        int height;
        if(noeud[d2].node->getHeight()>noeud[dd2].node->getHeight()) {
            height = noeud[d2].node->getHeight();
        }
        else {
            height = noeud[dd2].node->getHeight();
        }

        /*Suppression des deux anciens noeuds du front courant*/
        m_active_nodes.erase(n_d2->getCurrentPosition());
        m_active_nodes.erase(n_dd2->getCurrentPosition());
        n_d2->setActive(false);
        n_dd2->setActive(false);
        
        /*Ajout du nouveau noeud au front courant*/
        n->setActive(true);
        m_active_nodes.push_front(n);
        n->setCurrentPosition(m_active_nodes.begin());
        
        n->setHeight(height+1);
        
        if(m_height<=height) {
            /*Si la hauteur du plus grand arbre est inférieure ou égale à celle de l'arbre actuel*/
            /*NB: La hauteur du plus grand arbre ne sera jamais inférieure à celle de l'arbre courant*/
            ++m_height;
        }

		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
		{
			(*it)->approximate(d) ;					// compute approximated attributes with its associated detail
			(*it)->saveApprox(d) ;
		}

		m_selector->updateBeforeCollapse(d) ;		// update selector

		edgeCollapse(n->getVSplit()) ;							// collapse edge

		unsigned int newV = m_map.template setOrbitEmbeddingOnNewCell<VERTEX>(d2) ;
		unsigned int newE1 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(d2) ;
		unsigned int newE2 = m_map.template setOrbitEmbeddingOnNewCell<EDGE>(dd2) ;
		n->getVSplit()->setApproxV(newV) ;
		n->getVSplit()->setApproxE1(newE1) ;
		n->getVSplit()->setApproxE2(newE2) ;
        
        noeud[d2].node = n; //Affectation du nouveau noeud a l'attribut de sommet
        n->setVertex(m_map.template getEmbedding<VERTEX>(d2));  //Indique le numéro de sommet pointant sur ce noeud
        
		for(typename std::vector<Algo::Surface::Decimation::ApproximatorGen<PFP>*>::iterator it = m_approximators.begin(); it != m_approximators.end(); ++it)
			(*it)->affectApprox(d2);				// affect data to the resulting vertex

		m_selector->updateAfterCollapse(d2, dd2) ;	// update selector

		if(nbVertices <= nbWantedVertices)
			finished = true ;

	}
	delete m_selector ;
	m_selector = NULL ;

	CGoGNout << "..done (" << nbVertices << " vertices)" << CGoGNendl ;
    CGoGNout << m_active_nodes.size() << " active nodes" << CGoGNendl;
    CGoGNout << "Hauteur du plus grand arbre de la forêt : " << m_height << CGoGNendl;
    drawFront();
}

template <typename PFP>
void VDProgressiveMesh<PFP>::edgeCollapse(VSplit<PFP>* vs)
{
	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ;

	inactiveMarker.markOrbit<FACE>(d) ;
	inactiveMarker.markOrbit<FACE>(dd) ;

	m_map.extractTrianglePair(d) ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::vertexSplit(VSplit<PFP>* vs)
{
	Dart d = vs->getEdge() ;
	Dart dd = m_map.phi2(d) ;
	Dart d2 = vs->getLeftEdge() ;
	Dart dd2 = vs->getRightEdge() ;
            
	m_map.insertTrianglePair(d, d2, dd2) ;

	inactiveMarker.unmarkOrbit<FACE>(d) ;
	inactiveMarker.unmarkOrbit<FACE>(dd) ;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::coarsen() {
    for(std::list<Node*>::iterator it=m_active_nodes.begin(); it != m_active_nodes.end(); ++it) {
        coarsen(*it);
    }
}

template <typename PFP>
int VDProgressiveMesh<PFP>::coarsen(Node* n)
{
    int res = 0;
    if(n!=NULL && n->isActive()) {
        //Si n fait partie du front
        if(n->getParent()!=NULL && !n->getParent()->isActive()) {
            /*Si le noeud parent existe et qu'il ne fait pas partie du front*/
            Node* parent = n->getParent();
            Node* child_left = parent->getLeftChild();
            Node* child_right = parent->getRightChild();
            if(child_left!=NULL && child_left->isActive()
            && child_right!=NULL && child_right->isActive()) {
                //Si n a un noeud parent et que celui-ci ne fait pas partie du front
                VSplit<PFP>* vs = parent->getVSplit(); 
                Dart d2 = vs->getLeftEdge();
                Dart dd2 = vs->getRightEdge();
                Dart d1 = vs->getOppositeLeftEdge();
                Dart dd1 = vs->getOppositeRightEdge();

                /*if( inactiveMarker.isMarked(d1)
                ||  inactiveMarker.isMarked(d2)
                ||  inactiveMarker.isMarked(dd1)
                ||  inactiveMarker.isMarked(dd2))
                    return res;*/

                edgeCollapse(vs);

                m_map.template setOrbitEmbedding<VERTEX>(d2, vs->getApproxV());
	            m_map.template setOrbitEmbedding<EDGE>(d2, vs->getApproxE1());
                m_map.template setOrbitEmbedding<EDGE>(dd2, vs->getApproxE2());
        
                //Mise a jour des informations de l'arbre
                m_active_nodes.erase(child_left->getCurrentPosition());
                m_active_nodes.erase(child_right->getCurrentPosition());
                child_left->setActive(false);
                child_right->setActive(false);
                parent->setActive(true);
                m_active_nodes.push_front(parent);
                parent->setCurrentPosition(m_active_nodes.begin());
                ++res;
            }
        }
    }
    return res;
}

template <typename PFP>
void VDProgressiveMesh<PFP>::refine() {
    for(std::list<Node*>::iterator it=m_active_nodes.begin(); it!=m_active_nodes.end(); ++it) {
        refine(*it);
    }
}

template <typename PFP>
int VDProgressiveMesh<PFP>::refine(Node* n)
{
    int res = 0;
    if(n!=NULL && n->isActive()) {
        //Si n fait partie du front
        Node* child_left = n->getLeftChild();
        Node* child_right = n->getRightChild();
        if(child_left!=NULL && child_right!=NULL 
        && !child_left->isActive() && !child_right->isActive()) {
            //Si n a deux fils et que ceux-ci ne font pas partie du front
            VSplit<PFP>* vs = n->getVSplit();

	        Dart d = vs->getEdge();
	        Dart dd = m_map.phi2(d);
	        Dart dd2 = vs->getRightEdge();
	        Dart d2 = vs->getLeftEdge();
	        Dart d1 = vs->getOppositeLeftEdge();
	        Dart dd1 = vs->getOppositeRightEdge();

            if( inactiveMarker.isMarked(d1)
            ||  inactiveMarker.isMarked(d2)
            ||  inactiveMarker.isMarked(dd1)
            ||  inactiveMarker.isMarked(dd2))
                return res;
        
	        unsigned int v1 = m_map.template getEmbedding<VERTEX>(d);				// get the embedding
	        unsigned int v2 = m_map.template getEmbedding<VERTEX>(dd);			// of the new vertices
	        unsigned int e1 = m_map.template getEmbedding<EDGE>(m_map.phi1(d));
	        unsigned int e2 = m_map.template getEmbedding<EDGE>(m_map.phi_1(d));	// and new edges
	        unsigned int e3 = m_map.template getEmbedding<EDGE>(m_map.phi1(dd));
	        unsigned int e4 = m_map.template getEmbedding<EDGE>(m_map.phi_1(dd));
	
            vertexSplit(vs);
            
	        m_map.template setOrbitEmbedding<VERTEX>(d, v1);		// embed the
	        m_map.template setOrbitEmbedding<VERTEX>(dd, v2);	// new vertices
	        m_map.template setOrbitEmbedding<EDGE>(d1, e1);
	        m_map.template setOrbitEmbedding<EDGE>(d2, e2);		// and new edges
	        m_map.template setOrbitEmbedding<EDGE>(dd1, e3);
	        m_map.template setOrbitEmbedding<EDGE>(dd2, e4);

            m_map.template copyDartEmbedding<VERTEX>(m_map.phi_1(d), d1);
            m_map.template copyDartEmbedding<VERTEX>(m_map.phi_1(dd), dd1);

            //Mise a jour des informations de l'arbre
            m_active_nodes.erase(n->getCurrentPosition());
            n->setActive(false);
            child_left->setActive(true);
            child_right->setActive(true);
            m_active_nodes.push_front(child_left);
            child_left->setCurrentPosition(m_active_nodes.begin());
            m_active_nodes.push_front(child_right);
            child_right->setCurrentPosition(m_active_nodes.begin());
            res++;
        }
    }
    return res;
}

/*FONCTIONS DE DEBOGAGE*/
template <typename PFP>
void VDProgressiveMesh<PFP>::drawForest() {
    for(std::list<Node*>::iterator it = m_active_nodes.begin(); it != m_active_nodes.end(); ++it) {
        /*On parcourt l'ensemble des racines de la foret*/
        drawTree(*it);
        CGoGNout << CGoGNendl;
    } 
}

template <typename PFP>
void VDProgressiveMesh<PFP>::drawTree(Node* node) {
    CGoGNout << node->getVertex() << CGoGNflush;
    if(node->getLeftChild()!=NULL) {
        CGoGNout << " G( " << CGoGNflush;
        drawTree(node->getLeftChild());
        CGoGNout << " ) " << CGoGNflush;
    }
    if(node->getRightChild()!=NULL) {
        CGoGNout << " D( " << CGoGNflush;
        drawTree(node->getRightChild());
        CGoGNout << " ) " << CGoGNflush;
    }
}

template <typename PFP>
void VDProgressiveMesh<PFP>::drawFront() {
    CGoGNout << "Front courant : " << CGoGNendl;
    for(std::list<Node*>::iterator it = m_active_nodes.begin(); it != m_active_nodes.end(); ++it) {
        if(it!=--m_active_nodes.end())
            CGoGNout << (*it)->getVertex() << " | " << CGoGNflush;
        else
            CGoGNout << (*it)->getVertex() <<  CGoGNendl;
    }
}

} // namespace VDPMesh
} // namespace Surface
} // namespace Algo
} // namespace CGoGN
