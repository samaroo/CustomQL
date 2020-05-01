//C0F4DFE8B340D81183C208F70F9D2D797908754D
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include "Table.h"


using namespace std; 

//Table implementations

Table::Table(std::string name, int numCols, std::istream &in){

    tableName = name;

    std::string input;

    //loop to read in types
    for(int i = 0; i < numCols; i++){
        in >> input;

        if(input == "int"){
            columnTypes.push_back(DataType::INT);
        }
        else if (input == "double"){
            columnTypes.push_back(DataType::DOUBLE);
        }
        else if (input == "bool"){
            columnTypes.push_back(DataType::BOOL);
        }
        else if (input == "string"){
            columnTypes.push_back(DataType::STRING);
        }
        else{
            cout << "error";
        }

    }

    //loop to read in names
    for(int i = 0; i < numCols; i++){
        in >> input;
        columnNames.push_back(input);
    }

    cout << "New table " << tableName << " with column(s) ";
    for(auto i : columnNames)
        cout << i << " ";
    cout << "created\n";

    //take care of newline
    getline(in, input);
}

void Table::insert(int rows, std::istream &in){

    //if there is not enough room for the insertion
    if((int)dataFromInsert.size() + rows > (int)dataFromInsert.max_size()){
        //dataFromInsert.reserve(dataFromInsert.size() + rows + int(0.2 * dataFromInsert.max_size()));
    }

    string inputS;
    int inputI;
    double inputD;
    bool inputB;
    string trash;
    vector<TableEntry> insertRow;

    //loop inserts each TableEntry into table
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < (int)columnNames.size(); j++){

            if (columnTypes[j] == DataType::STRING){
                in >> inputS;
                insertRow.push_back(TableEntry(inputS));
            }
            else if (columnTypes[j] == DataType::INT){
                in >> inputI;
                insertRow.push_back(TableEntry(inputI));
            }
            else if (columnTypes[j] == DataType::DOUBLE){
                in >> inputD;
                insertRow.push_back(TableEntry(inputD));
            }
            //if datatype for that column is bool
            else{
                in >> inputS;
                if(inputS == "true"){
                    inputB = 1;
                }
                else{
                    inputB = 0;
                }
                insertRow.push_back(TableEntry(inputB));
            }
        }
        //pushback newly created row into dataFromInsert
        dataFromInsert.push_back(insertRow);
        //delete all elm in insertRow
        insertRow.clear();
        //deal with newline char
        getline(in, trash);
    }

    //if you already had an index system and you added new rows, re generate indices
    if(positionOfIndexedCol != -1){
        generateIndices(!hash.empty(), columnNames[positionOfIndexedCol], false);
        
    }

    cout << "Added " << rows <<
     " rows to " << tableName <<
      " from position " << ((int)dataFromInsert.size() - rows) <<
       " to " << ((int)dataFromInsert.size() - 1) << '\n';

}

void Table::printAll(){

    //print column names
    for(auto i : columnNames){
        cout << i << " ";
    }
    cout << '\n';

    //print table data
    for (auto i : dataFromInsert){
        for (auto j : i){
            cout << j << " ";
        }
        cout << '\n';
    }
}

void Table::printCols(int numCols, std::istream &in, bool verbose){

    std::vector<std::string> printCols;
    std::vector<int> printColIndices;
    std::string input;
    int numberOfRowsPrinted = 0;

    //collect all the cols user wants to print fron cin
    for(int i = 0; i < numCols; i++){
        in >> input;
        printCols.push_back(input);
    }

    //stores all the indices that need to be printed
    bool colFound;
    for(auto i : printCols){
        colFound = false;
        for(int j = 0; j < (int)columnNames.size(); j++){
            if (i == columnNames[j]){
                printColIndices.push_back(j);
                colFound = true;
            }
        }
        //if the name from printCols doesnt match a column name
        if(!colFound){
            string error = "One or more of the columns are invalid";
            throw error;
        }
        if(verbose)
            cout << i << " ";
    }
    if(verbose)
        cout << '\n';

    //std::sort(printColIndices.begin(), printColIndices.end());

    in >> input;

    //if ALL, print ever elm in cols
    if(input == "ALL"){

        for (auto i : dataFromInsert){
            for(auto j : printColIndices){
                if(verbose)
                    cout << i[j] << " ";
            }
            if(verbose)
                cout << '\n';
            numberOfRowsPrinted++;
        }
    }

    //if WHERE, take in more info
    else{

        string compareS;
        int compareI;
        double compareD;
        bool compareB;

        string compCol;
        in >> compCol;
        char operation;
        in >> operation;
        int index = -1;

        for(int i = 0; i < (int)columnNames.size(); i++){
            if(columnNames[i] == compCol)
                index = i;
        }

        //if the column name wasnt found
        if(index == -1){
            string error = "Column name doesn't exist";
            throw error;
        }

        if(columnTypes[index] == DataType::STRING){
            in >> compareS;
            if(!printWhereWithIndex(printColIndices, compareS, index, operation, numberOfRowsPrinted, verbose))
                printWhere(printColIndices, compareS, index, operation, numberOfRowsPrinted, verbose);
        }
        else if(columnTypes[index] == DataType::INT){
            in >> compareI;
            if(!printWhereWithIndex(printColIndices, compareI, index, operation, numberOfRowsPrinted, verbose))
                printWhere(printColIndices, compareI, index, operation, numberOfRowsPrinted, verbose);
        }
        else if(columnTypes[index] == DataType::DOUBLE){
            in >> compareD;
            if(!printWhereWithIndex(printColIndices, compareD, index, operation, numberOfRowsPrinted, verbose))
                printWhere(printColIndices, compareD, index, operation, numberOfRowsPrinted, verbose);
        }
        else{
            in >> compareS;
            compareB = (compareS == "true" ? true : false);
            if(!printWhereWithIndex(printColIndices, compareB, index, operation, numberOfRowsPrinted, verbose))
                printWhere(printColIndices, compareB, index, operation, numberOfRowsPrinted,verbose);
        }
    }

    cout << "Printed " << numberOfRowsPrinted << " matching rows from " << tableName << "\n";

    //clears newline
    getline(in, input);

}

void Table::deleteRows(string colOfInterest, char op, istream &in){

    int index = -1;
    string compareS;
    int compareI;
    double compareD;
    bool compareB;
    int previousSize = (int)dataFromInsert.size();
    bool usedIndex;

    for(int i = 0; i < (int)columnNames.size(); i++){
        if(columnNames[i] == colOfInterest)
            index = i;
    }

    //if the column name wasnt found
    if(index == -1){
        string error = "Column name doesn't exist";
        throw error;
    }

    if(columnTypes[index] == DataType::STRING){
        in >> compareS;
        usedIndex = ifColIsIndexed(compareS, index, op);
        if(!usedIndex)
            deleteRowsHelper(compareS, index, op);
    }
    else if(columnTypes[index] == DataType::INT){
        in >> compareI;
        usedIndex = ifColIsIndexed(compareI, index, op);
        if(!usedIndex)
            deleteRowsHelper(compareI, index, op);
    }
    else if(columnTypes[index] == DataType::DOUBLE){
        in >> compareD;
        usedIndex = ifColIsIndexed(compareD, index, op);
        if(!usedIndex)
            deleteRowsHelper(compareD, index, op);
    }
    else{
        in >> compareS;
        compareB = (compareS == "true" ? true : false);
        usedIndex = ifColIsIndexed(compareB, index, op);
        if(!usedIndex)
            deleteRowsHelper(compareB, index, op);
    }

    if(positionOfIndexedCol != -1){
        if(hash.empty()){
            generateIndices(false, columnNames[positionOfIndexedCol], false);
        }
        else{
            generateIndices(true, columnNames[positionOfIndexedCol], false);
        }
    }

    cout << "Deleted " << (previousSize - (int)dataFromInsert.size()) << " rows from " << tableName << '\n';
    
}

void Table::generateIndices(bool hashOrBst, string column, bool verbose){

    hash.clear();
    bst.clear();
    positionOfIndexedCol = -1;

    //look for column in table 
    for(int i = 0; i < (int)columnNames.size(); i++){
        if(columnNames[i] == column)
            positionOfIndexedCol = i;
    }

    //if the column name wasnt found
    if(positionOfIndexedCol == -1){
        string error = "Column name doesn't exist";
        throw error;
    }

    if(hashOrBst){
        for(int i = 0; i < (int)dataFromInsert.size(); i++){
            hash[dataFromInsert[i][positionOfIndexedCol]].push_back(i);
        }
    }
    else{
        for(int i = 0; i < (int)dataFromInsert.size(); i++){
            bst[dataFromInsert[i][positionOfIndexedCol]].push_back(i);
        }
    }
    if(verbose)
        cout << "Created " << (hashOrBst ? "hash " : "bst ") << "index for table " << tableName << " on column " << column << '\n';
}

int Table::findColumnIndex(string col){
    for(int i = 0; i < (int)columnNames.size(); i++){
        if(columnNames[i] == col){
            return i;
        }
    }
    return -1;
}

void Table::hashTable2(unordered_map<TableEntry, vector<int>> &joinHash, int indexOfColumnOfInterestTable2){
    //hash the column of interest
    for(int i = 0; i < (int)dataFromInsert.size(); i++){
        joinHash[dataFromInsert[i][indexOfColumnOfInterestTable2]].push_back(i);
    }
}

void Table::findMatches(unordered_map<TableEntry, vector<int>> &joinHash, vector<pair<int, vector<int>>> &matches, int indexOfColumnOfInterest){
    for(int i = 0; i < (int)dataFromInsert.size(); i++){
        //if the value at column "indexOfColumnOfInterest", row i is found in the hash
        if(joinHash.find(dataFromInsert[i][indexOfColumnOfInterest]) != joinHash.end()){
            matches.push_back({i, joinHash[dataFromInsert[i][indexOfColumnOfInterest]]});
        }
    }
}

