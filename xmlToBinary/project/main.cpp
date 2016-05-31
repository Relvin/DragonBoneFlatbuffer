//
//  main.cpp
//  xmlToBinary
//
//  Created by Relvin on 15/12/3.
//  Copyright © 2015年 Relvin. All rights reserved.
//

#include <iostream>
#include "xmlToBinary.hpp"
#include "Platform.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef test
void testRead()
{
    
    std::string filename = "/Users/dengqinghong/Desktop/xml/Ubbie/Ubbie.xml";
    filename += "";
    
    std::string outpath = "/Users/relvin/Desktop/Ubbie/";
    xmlToBinary::getInstance()->readFileAndConverToBinary(filename, outpath);
    
    std::cout << " ===>SUCCESS!<===\n";
}

void testWrite()
{
    
    std::string filename = "/Users/relvin/Desktop/Ubbie/Ubbie.xml";
    filename += "";
    
    std::string outpath = "/Users/relvin/Desktop/Ubbie/";
    xmlToBinary::getInstance()->readFileAndConverToBinary(filename, outpath);
    
    std::cout << " ===>SUCCESS!<===\n";
}
#endif  //end test
int main(int argc, const char * argv[]) {
    // insert code here...
//
#ifdef test
    testWrite();
    testRead();
    
    return 0;
#endif // end test

	if (argc < 3)
	{
        printf("arguments error!");
        return -1;
	}

    std::string filename = argv[1];
    std::string extFilePath = argv[2];
	
    
//    printf("project_path = %s \n",project_path.c_str());

    {
		xmlToBinary::getInstance()->readFileAndConverToBinary(filename, extFilePath);
    }
    
//    std::string filePath = "/Users/dengqinghong/Desktop/1-boss.xml";

    return 0;
}
