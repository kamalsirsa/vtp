//
// vtNode - encapsulate behavior for a scene graph node
//

#include "vtlib/vtlib.h"

void vtNode::SetEnabled(bool bOn)
{
//	m_pModel->SetActive(bOn);	// TODO
	vtEnabledBase::SetEnabled(bOn);
}

void vtNode::SetSsgEntity(ssgEntity *ent)
{
	m_pEntity = ent;
	if (m_pEntity)
		m_pEntity->setUserData(this);
}

//void vtNode::RemoveFromParent()
//{
//	m_pEntity->getParent(0)->removeKid(m_pEntity);
//}

void vtNode::GetBoundBox(FBox3 &box)
{
	// TODO
//	FBox3 b;
//	m_pTransform->GetBound(&b);
//	box = d2v(b);
}

void vtNode::GetBoundSphere(FSphere &sphere)
{
	sgSphere *s = m_pEntity->getBSphere();
	s2v(s, sphere);
}

vtNode *vtNode::CreateClone()
{
	// TODO
	return new vtNode();
}

void vtNode::SetName2(const char *name)
{
	m_pEntity->setName((char *)name);
}

const char *vtNode::GetName2()
{
	return m_pEntity->getName();
}


//////////////////////////////////////////////////////////////////
// Group

vtGroup::vtGroup(bool suppress) : vtNode()
{
	if (suppress)
		m_pBranch = NULL;
	else
		m_pBranch = new ssgBranch();
	SetSsgEntity(m_pBranch);
}

void vtGroup::AddChild(vtNode *pChild)
{
	if (!pChild || !pChild->GetSsgEntity())
		return;

	// assume that this entity is actually a branch node
	((ssgBranch *)m_pEntity)->addKid(pChild->GetSsgEntity());
}

vtNode *vtGroup::GetChild(int num)
{
	ssgEntity *pEnt = m_pBranch->getKid(num);
	if (!pEnt) return NULL;

	ssgBase *pBase = pEnt->getUserData();
//	return (vtNode *) pEnt;
	return (vtNode *) pBase;
}

int vtGroup::GetNumChildren()
{
	return m_pEntity->getNumKids();
}

