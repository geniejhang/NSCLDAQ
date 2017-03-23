#ifndef DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H
#define DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H

#include <CFilterVersionAbstraction.h>

#include <map>

namespace DAQ {

class CFilterVersionAbstractionCreator;

// Some useful typedefs
using CFilterVersionAbstractionCreatorUPtr = std::unique_ptr<CFilterVersionAbstractionCreator>;
using CFilterVersionAbstractionCreatorPtr  = std::shared_ptr<CFilterVersionAbstractionCreator>;

/*!
 * \brief A creator object for the filter version abstractions
 */
class CFilterVersionAbstractionCreator {
public:
  virtual CFilterVersionAbstractionUPtr create() const = 0;
};




/*!
 * \brief An extensible factory for producing filter version abstractions
 */
class CFilterVersionAbstractionFactory {
public:
  using CreatorPtr = CFilterVersionAbstractionCreatorPtr;


    // Initially there are 3 types supported out of the box. The user can
    // add an arbritrary number of new typess and creators using addCreator.
  enum Key { v10 = 0, v11 = 1, v12 = 2 };

private:
  std::map<int, CreatorPtr> m_creators; ///< the map of creators

public:

  /*!
   * \brief Add a new creator type
   *
   * \param type    the type of the object to be created (i.e. key attached to the creator)
   * \param creator the actual creator instance
   *
   * If a creator is already stored for the type specified, it is replaced with
   * the new creator.
   */
  void addCreator(int type, CreatorPtr creator);

  /*!
   * \brief Lookup a creator type by its key
   * \param type    the key associated with the creator
   *
   * \return    pointer to a creator
   * \retval    nullptr if not found
   * \retval    pointer if it is found
   */
  CreatorPtr getCreator(int type);

  /*!
   * \brief Create a new object of a certain type
   *
   * \param type    the type of object to create
   *
   * \return    a new object of type
   *
   * \throws std::out_of_range if no created is stored associated with a key matching type
   */
  CFilterVersionAbstractionUPtr create(int type) const;
};

} // end DAQ

#endif // DAQ_CFILTERVERSIONABSTRACTIONFACTORY_H
