//C0F4DFE8B340D81183C208F70F9D2D797908754D
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include "Table.h"
#include "Engine.h"

using namespace std;

void Engine::run(istream &in){

    string command;
    string tableName;
    string tableName2;
    int numRows;
    int numCols;
    string trash;

    //variables for WHERE funtions
    string columnOfInterest;
    string columnOfInterestTable2;
    char operation;

    do {

        std::cout << "% ";

        in >> command;

        if (command == "CREATE"){
            //get table name
            in >> tableName;
            //get number of cols
            in >> numCols;
            //if table already exists
            if(tables.find(tableName) != tables.end()){
                cout << "Error: Cannot create already existing table " << tableName << '\n';
                getline(in, trash);
            }
            else{
                tables.insert({tableName, Table(tableName, numCols, in)});
            }
        }
        else if (command[0] == '#'){
            getline(in, command);
        }
        else if (command == "INSERT"){
            in >> command;

            //take in table name and rows
            in >> command;
            in >> numRows;
            //throw away rest of line
            getline(in, trash);
            //if the table is found in the hash
            if(tables.find(command) != tables.end()){
                tables.find(command)->second.insert(numRows, in);
            }
            //if the table is not found
            else{
                cout << "Error: " << command << " does not name a table in the database\n";
            }

        }
        else if (command == "PRINT"){

            string tableName;

            in >> tableName;
            //get table name
            in >> tableName;
            in >> numCols;

            if(tables.find(tableName) == tables.end()){
                cout << "Error: " << tableName << " does not name a table in the database\n";
                getline(in , trash);
            }
            else{
                tables.find(tableName)->second.printCols(numCols, in, verbose);
            }
            
        }
        else if (command == "REMOVE"){
            in >> command;
            if(tables.find(command) == tables.end()){
                cout << "Error: " << command << " does not name a table in the database\n";
                getline(in , trash);
            }
            else{
                removeTable(command);
            }
        }
        else if (command == "DELETE"){
            in >> command;
            in >> tableName;
            in >> command;
            in >> columnOfInterest;
            in >> operation;

            //if table name exists
            if(tables.find(tableName) != tables.end()){
                tables.find(tableName)->second.deleteRows(columnOfInterest, operation, in);
            }
            else{
                cout << "Error: " << tableName << " does not name a table in the database\n";
                getline(in , trash);
            }
        }
        else if (command == "GENERATE"){
            in >> command;
            in >> tableName;
            string indexType;
            in >> indexType;
            in >> command;
            in >> command;
            in >> columnOfInterest;

            //if table name exists
            if(tables.find(tableName) != tables.end()){
                tables.find(tableName)->second.generateIndices((indexType == "hash" ? true : false), columnOfInterest, true);
            }
            else{
                cout << "Error: " << tableName << " does not name a table in the database\n";
                getline(in , trash);
            }
        }
        else if(command == "JOIN"){
            in >> tableName;
            in >> command;
            in >> tableName2;
            in >> command;
            in >> columnOfInterest;
            in >> operation;
            in >> columnOfInterestTable2;
            in >> command;
            in >> command;
            in >> numCols;
            vector<pair<string, bool>> columnsToPrint;

            string column;
            int whichTable;
            int howManyRowsPrinted = 0;

            for(int i = 0; i < numCols; i++){
                in >> command;
                if(verbose)
                    cout << command << " ";
                in >> whichTable;
                columnsToPrint.push_back({command, (whichTable == 1 ? true : false)});
            }
            if(verbose)
                cout << '\n';

            bool goAhead = true;

            //checking to see if the two tables exist
            if(tables.find(tableName2) != tables.end()){
                if(tables.find(tableName) != tables.end()){
                    //
                    goAhead = join(tableName, tableName2, columnsToPrint, columnOfInterest, columnOfInterestTable2, howManyRowsPrinted);
                    //
                }
                else{
                    cout << "Error: " << tableName << " does not name a table in the database\n";
                    goAhead = false;
                }
            }
            else{
                cout << "Error: " << tableName2 << " does not name a table in the database\n";
                goAhead = false;
            }

            if(goAhead)
                cout << "Printed " << howManyRowsPrinted << " rows from joining " << tableName << " to " << tableName2 << '\n';

            //get rid of newline
            getline(in, trash);
        }
        else if (command == "QUIT"){
            //branch makes sure nothing happens
        }
        else{
            cout << "Error: unrecognized command\n";
            getline(in, trash);
        }

    }
    while (command != "QUIT");

    std::cout << "Thanks for being silly!\n";

}

void Engine::removeTable(string table){
    //if the table exists erase it
    if(tables.find(table) != tables.end()){
        tables.erase(tables.find(table));
    }
    cout << "Table " << table << " deleted" << '\n';
    
}

bool Engine::join(string tableName, string tableName2, vector<pair<string, bool>> &columnsToPrint, string columnOfInterest, string columnOfInterestTable2, int &howManyRowsPrinted){

    //temporary hashmap used for joining (will be filled bu a function)
    unordered_map<TableEntry, vector<int>> joinHash;
    //holds that matches that are made during the joining process
    vector<pair<int, vector<int>>> matches;
    //vector that holds the indices for each column in columnToPrint (indices are in their respective tables)
    vector<int> indicesOfColumnsToPrint;
    int indexOfColumnOfInterest = tables.find(tableName)->second.findColumnIndex(columnOfInterest);
    int indexOfColumnOfInterestTable2 = tables.find(tableName2)->second.findColumnIndex(columnOfInterestTable2);

    if(indexOfColumnOfInterest == -1){
        cout << "Error: " << columnOfInterest << " does not name a column in " << tableName << '\n';
        return false;
    }
    else if (indexOfColumnOfInterestTable2 == -1){
        cout << "Error: " << columnOfInterestTable2 << " does not name a column in " << tableName2 << '\n';
        return false;
    }

    //find indices
    for(auto i : columnsToPrint){
        //if from table 2
        if(!i.second){
            //if column exists in table 1
            int indx = tables.find(tableName2)->second.findColumnIndex(i.first);
            if(indx != -1){
                indicesOfColumnsToPrint.push_back(indx);
            }
            //if columns was not found
            else{
                cout << "Error: " << columnOfInterestTable2 << " does not name a column in " << tableName2 << '\n';
                return false;
            }
        }
        //if from table 1
        else{
            int indx = tables.find(tableName)->second.findColumnIndex(i.first);
            if(indx != -1){
                indicesOfColumnsToPrint.push_back(indx);
            }
            else{
                cout << "Error: " << columnOfInterest << " does not name a column in " << tableName << '\n';
                return false;
            }
        }
    }

    //store hashing of columnOfInterest2 in joinHash
    tables.find(tableName2)->second.hashTable2(joinHash, indexOfColumnOfInterestTable2);
    //find all the matches and store the indices in "matches"
    tables.find(tableName)->second.findMatches(joinHash, matches, indexOfColumnOfInterest);

    if(!verbose){
        for(auto i : matches){
            howManyRowsPrinted += (int)i.second.size();
        }
    }
    
    else{
    for(auto i : matches){
        for(auto j : i.second){
            for(int k = 0; k < (int)indicesOfColumnsToPrint.size(); k++){
                //if table1
                if(columnsToPrint[k].second){
                    if(verbose)
                        std::cout << tables.find(tableName)->second.returnData(i.first, indicesOfColumnsToPrint[k]) << " ";
                }
                //if table2
                else{
                    if(verbose)
                        std::cout << tables.find(tableName2)->second.returnData(j, indicesOfColumnsToPrint[k]) << " ";
                }
            }
            if(verbose)
                std::cout << '\n';
            howManyRowsPrinted++;
        }
    }
    }
    return true;

}

void Engine::get_options(int argc, char **argv) {
    int option_index = 0, option = 0;

    // display getopt error messages about options
    opterr = true;

    // use getopt to find command line options
    struct option longOpts[] = {{"quiet", no_argument, nullptr, 'q'},
                                {"help", no_argument, nullptr, 'h'},
                                {nullptr, 0, nullptr, '\0'}};

    while ((option = getopt_long(argc, argv, "qh", longOpts,
                                 &option_index)) != -1) {
      switch (option) {
        case 'q':
          verbose = false;
          break;

        case 'h':
          cout << "You actually thought you'd get help, come back when u get a brain";
          exit(0);

        default:
          string error = "Invalid flag";
          throw error;
      }
    }
  }

int main(int argc, char **argv) {

    ios::sync_with_stdio(false);

    cout <<std::boolalpha;

    Engine e;

    e.get_options(argc, argv);

    e.run(cin);

}