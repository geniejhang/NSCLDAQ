/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <CFatalException.h>
#include <CFilterMain.h>
#include <V12/CFilterAbstraction.h>

#include "CV12SourceCounterFilter.h"

#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

using namespace std;
using namespace DAQ;

struct CmdlineArgs {
  string s_outputFile;
  bool   s_built;
};


/*!
 * \brief printSpecialUsage
 *
 * Print a description of options beyond those provided by the core filter framework.
 */
void printSpecialUsage() {
  cout << "  -O, --output-file            [MANDATORY] The name of the file to write statistics to" << endl;

  cout << "\n  -u, --unbuilt                If present, data is not treated as built data." << endl;
}


/*!
 * \brief Create a vector<string> from argv and argc
 *
 * \param argc  the number of command line args
 * \param argv  the command line args
 *
 * I personally find it easier to work with a vector than a POD-array.
 * For that reason, I am converting.
 *
 * \return a copy of the command line arguments
 */
vector<string> 
cArgsToCppArgs(int argc, char* argv[]) 
{
  vector<string> args(argc);
  for (int i=0; i<argc; ++i) {
    args[i] = string(argv[i]);
  }
  return args;
}

/*!
 * \brief Locate, handle, and remove special arguments
 *
 * \param argv  the command line arguments
 *
 * \return a pair. the first element is the filtered argument list and the
 *         second element contains the state of the special arguments after parsing
 */
pair<vector<string>, CmdlineArgs>
processAndRemoveSpecialArgs(const vector<string>& argv)
{
  CmdlineArgs cmdArgs = {"", true};

  vector<string> filteredArgv;
  for (size_t i=0; i<argv.size(); ++i) {
    string option(argv[i]);
    if (option.find("--output-file") == 0) {
      if (option.size() == 13) {
        cmdArgs.s_outputFile = argv[i+1]; 
        ++i;
      } else {
        cmdArgs.s_outputFile = option.substr(14); 
      }
    } else if (option.find("-O")== 0 ) {
      if (option.size() == 2) {
        cmdArgs.s_outputFile = argv[i+1]; 
        ++i;
      } else {
        cmdArgs.s_outputFile = option.substr(3); 
      }
    } else if (option == "--unbuilt" || option == "-u") {
      cmdArgs.s_built = false;
    } else if (option == "--help" || option == "-h") {
      atexit( printSpecialUsage );
      // the --help or -h needs to be appended to the filtered args in order
      // to cause the core filter framework to output its help information
      filteredArgv.push_back(string(argv.at(i)));
    } else {
      filteredArgv.push_back(string(argv.at(i)));
    } 
  }
  return make_pair(filteredArgv, cmdArgs);
}


/*!
 * \brief Recreate the argv based on a vector<string>
 *
 * \param argV  the list of arguments
 *
 *  This is just the opposite of cArgsToCppArgs
 *
 * \return  a pointer to an array of c-strings
 */
char** createNewCArgV(const vector<string>& argV)
{
  char** pArgV = new char*[argV.size()];
  for (int i=0; i<argV.size(); ++i) {
    pArgV[i] = new char[argV[i].size()+1];
    strcpy(pArgV[i], argV[i].data());
    pArgV[i][argV[i].size()] = 0;

  }

  return pArgV;
}

/**! main function
  Creates a CFilterMain object and 
  executes its operator()() method. 

  \return 0 for normal exit, 
          1 for known fatal error, 
          2 for unknown fatal error
*/
int main(int argc, char* argv[])
{
  int status = 0;

  try {

    auto argV         = cArgsToCppArgs(argc, argv);
    auto parserResult    = processAndRemoveSpecialArgs(argV);
    vector<string> newArgV  = parserResult.first;

    argc = newArgV.size();
    argv = createNewCArgV(newArgV);

    // Create the main
    CFilterMain theApp(argc, argv);

    if (newArgV == argV) {
      cout << "User did not provide an output file. Specify --output-file or -O option" << endl;
      theApp.printUsageString();
      printSpecialUsage();
      return 1;
    }

    V12::CFilterAbstractionPtr pVersion(new V12::CFilterAbstraction);

    auto cmdLineOpts = parserResult.second;
    // Construct filter(s) here.
    auto pSrcCounter = std::make_shared<V12::CSourceCounterFilter>(cmdLineOpts.s_outputFile);
    pSrcCounter->setBuiltData(cmdLineOpts.s_built);

    pVersion->registerFilter(pSrcCounter);

    theApp.setVersionAbstraction(pVersion);

    // Run the main loop
    theApp();

  } catch (CFatalException exc) {
    status = 1;

  } catch (std::exception& exc) {
      cout << "Caught fatal exception : " << exc.what() << endl;
      status = 3;
  } catch (...) {

    cout << "Caught unknown fatal error...!" << endl;
    status = 2;
  }

  return status;
}

