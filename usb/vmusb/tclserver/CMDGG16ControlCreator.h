
#ifndef CMDGG16CONTROLCREATOR_H
#define CMDGG16CONTROLCREATOR_H

#include <CModuleCreator.h>
#include <CMDGG16Control.h>
#include <memory>

/**! The creator of CMDGG16Control for the Module command
*
* An instance of this creator object is registered to an
* object of type CModuleCommand.
*/
class CMDGG16ControlCreator : public ::CModuleCreator
{
  public:
   /**! The factory method */
   virtual std::unique_ptr<CControlHardware> operator()(); 
};


#endif
