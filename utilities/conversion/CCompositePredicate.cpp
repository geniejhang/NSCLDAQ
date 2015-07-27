/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/
#include <CCompositePredicate.h>

using namespace std;

void CCompositePredicate::addPredicate(std::unique_ptr<CPredicate> pPred)
{
  m_predicates.push_back(move(pPred));
}


bool CCompositePredicate::operator()()
{
  bool flag = true;

  for (auto& pred : m_predicates) {
    flag &= (*pred)();
  }

  return flag;
}
