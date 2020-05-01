//C0F4DFE8B340D81183C208F70F9D2D797908754D
#pragma once
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include "Table.h"

using namespace std;

class Engine{

private:

    //holds a hash map of the tables
    unordered_map<string, Table> tables;
    //tells if certain things should be output or not
    bool verbose = true;


public:

    //constructor
    //Engine();

    //will run until user quits 
    void run(istream &in);

    //removes a table from the hash table
    void removeTable(string table);

    //joins tables
    bool join(string tableName, string tableName2, vector<pair<string, bool>> &columnsToPrint, string columnOfInterest, string columnOfInterestTable2, int &howManyRowsPrinted);

    //used to fetch flags from command line
    void get_options(int argc, char **argv);

};