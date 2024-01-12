// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "pdevunit.h"

#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/BriefTestProgressListener.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <getopt.h>
#include <cstring>



int
pdevunit::run(int argc, char* argv[])
{
    char* tests=strdup("");
    
    try {
        CppUnit::TestFactoryRegistry& registry =
             CppUnit::TestFactoryRegistry::getRegistry();
            
        CppUnit::TestRunner runner;
        runner.addTest(registry.makeTest());

        CppUnit::TestResult controller;

        // Add results collector
        
        CppUnit::TestResultCollector result;
        controller.addListener(&result);
        
         // Run the tests
        
        runner.run(controller, tests);

        // Pass the results to the appropriate outputter.  
        
        CppUnit::CompilerOutputter outputter(&result, std::cerr);
        outputter.write();

       
        return result.wasSuccessful() ? EXIT_SUCCESS : EXIT_FAILURE;
        
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}


// --------------------------------------------------------------------------- 

