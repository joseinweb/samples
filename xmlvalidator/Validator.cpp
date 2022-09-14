#include <iostream>
#include <mutex>
#include <tinyxml2.h>

using namespace tinyxml2;

#define VERSION "1.0"
#define DATA_MODEL_FILE "/etc/data-model.xml"
#define _DEBUG

static void *g_dbhandle = NULL;
std::mutex g_db_mutex;

int loadDataModel(const char * fileName)
{

    int dbRet = 0;
    XMLDocument *doc = NULL;

    // Load Document model
    doc = new XMLDocument();
    if(doc != NULL)
    {
        doc->LoadFile(fileName);
        if( doc->ErrorID() == 0 )
        {
            dbRet = 1;
        }
        else
        {
            std::cout <<"Error code is "<<doc->ErrorID() << std::endl;
            std::cout << "Error description " <<doc->ErrorStr() << std::endl;
            
        }
        doc->Clear();
    }
    return dbRet;
}

int main(int argc, char const *argv[])
{
    std::cout<<"Version " << VERSION <<std::endl;
    if(argc >1)
        loadDataModel(argv[1]);
    else
        loadDataModel(DATA_MODEL_FILE);
    return 0;
}
