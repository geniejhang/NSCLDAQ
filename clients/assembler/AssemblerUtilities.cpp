////////////////////////////////////////////////////////////////
/*!   Make a size reduced vector of typecount pairs.
     The vector consists of a pair for each non zero  
     element of the array that contains the index and value.
     \param statistics - an array to reduce.
     \param size       - The number of elements in the array.
*/
vector<AssemblerUtilities::typeCountPair>
AssemblerUtilities::makeTypeCountVector(const uint32_t* statistics,
										size_t          size) const
{
  vector<typeCountPair> result;
  for (int i = 0; i < size; i++) {
    if (statistics[i] != 0) {
      result.push_back(typeCountPair(i, statistics[i]));
    }
  }
  return result;
}

///////////////////////////////////////////////////////////
/*!
 * Convert a type/value vector to a Tcl list.
 * The list is dynamically created here and must be deleted
 * by the caller.
 * 
 *  \param  interp  - Interpreter to which the list will be bound.
 *  \param  stats   - vector of type value pairs to build into the list
 *  \return CTCLObject*
 *  \retval A pointer to the stocked list.  The caller must delete this when
 *          done with it.
 */
CTCLObject*
AssemblerUtilities::typeValuePairToList(CTCLInterpreter& interp,
	                            		vector<AssemblerUtilities::typeCountPair>& stats)
{
	CTCLObject* pObject = new CTCLObject;
	pObject->Bind(interp);
	
	for (int i=0; i < stats.size(); i++ ) {
		CTCLObject pair;
		pair.Bind(interp);
		pair += stats[i].first;
		pair += static_cast<int>(stats[i].second);
		
		(*pObject) += pair;
	}
	return pObject;
}