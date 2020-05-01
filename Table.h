//C0F4DFE8B340D81183C208F70F9D2D797908754D
#pragma once
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <unordered_map>
#include <map>
#include <queue>
#include "TableEntry.h"

using namespace std;

enum DataType{
    INT,
    DOUBLE,
    BOOL,
    STRING
};

//functors

//returns true if the column in the passed row is less than the given val
struct IsRowLessThan{

    private:

    int colOfInterest = 0;
    TableEntry value;

    public:

    //constructor
    IsRowLessThan(int col, TableEntry val) : 
    colOfInterest {col},
    value {val}
    
    {}

    //ask K
    bool operator()(vector<TableEntry> &row) {
        return row[colOfInterest] < value;
    }
};

//returns true if the column in the passed row is greater than the given val
struct IsRowGreaterThan{
    
    private:

    int colOfInterest;
    TableEntry valueToCompareTo;

    public:

    //constructor
    IsRowGreaterThan(int col, TableEntry val) : 
    colOfInterest {col},
    valueToCompareTo {val}
    
    {}

    //ask K
    bool operator()(vector<TableEntry> &row) {
        return row[colOfInterest] > valueToCompareTo;
    }
};

//returns true if the column in the passed row is equal to the given val
struct IsRowEqualTo{
    
    private:

    int colOfInterest;
    TableEntry valueToCompareTo;

    public:

    //constructor
    IsRowEqualTo(int col, TableEntry val) : 
    colOfInterest {col},
    valueToCompareTo {val}
    
    {}

    //ask K
    bool operator()(vector<TableEntry> &row) {
        return row[colOfInterest] == valueToCompareTo;
    }
};

//Table class declaration

class Table{

private:

    //holds name of table
    string tableName;
    //holds data given from insert fucntion (row, col)
    vector<vector<TableEntry>> dataFromInsert;
    //holds meta-data
    vector<string> columnNames;
    //holds what data type each col is
    vector<DataType> columnTypes;
    //will hold index for BST
    map<TableEntry, vector<int>> bst;
    //will hold index for Hash
    unordered_map<TableEntry, vector<int>> hash;
    //index of the column that is indexed
    int positionOfIndexedCol = -1;

    //helper functions - definitions in header for template reasons

    template<class T>
    //deleteRows helper fucntion
    void deleteRowsHelper(T compareVariable, int index, char op){
        TableEntry entry(compareVariable);
        IsRowLessThan less(index, entry);
        IsRowGreaterThan greater(index, entry);
        IsRowEqualTo equal(index, entry);
        switch(op){
            //remove_if rearranges the 'deletable items' at the end and returns an iterator to 'new end',
            //so erase the vector from the returned pointer to the true end of the vector
            case '<':
                dataFromInsert.erase(std::remove_if(dataFromInsert.begin(), dataFromInsert.end(), less), dataFromInsert.end());
                break;
            case '>':
                dataFromInsert.erase(std::remove_if(dataFromInsert.begin(), dataFromInsert.end(), greater), dataFromInsert.end());
                break;
            case '=':
                dataFromInsert.erase(std::remove_if(dataFromInsert.begin(), dataFromInsert.end(), equal), dataFromInsert.end());
        }
    }

    template<class T>
    //another deleteRows helper that deals with with indices
    //returns true if it can use indices to help delete faster
    bool ifColIsIndexed(T compareVariable, int index, char op){
        //if the column at 'index' isnt indexed
        if(index != positionOfIndexedCol){
            return false;
        }
        //true for hash, false for bst
        bool hashOrBst = (bst.empty() ? true : false);
        
        //if index is of type hashmap
        if(hashOrBst){
            //hash tables can only help if op is '='
            if(op == '='){
                if(hash.find(TableEntry(compareVariable)) != hash.end()){
                    int size = (int)hash.find(TableEntry(compareVariable))->second.size();
                    vector<int> erase;
                    for(int i = 0; i < size; i++){
                        erase = hash.find(TableEntry(compareVariable))->second;
                    }
                    std::sort(erase.begin(), erase.end(), std::greater<int>());
                    for(auto i : erase){
                        dataFromInsert.erase(dataFromInsert.begin() + i);
                    }
                }
            }
            else{
                return false;
            }
        }
        //if index is of type bst
        else{
            std::vector<int> eraseIndices;
            switch(op){
                case '<' :
                    for(auto i = bst.begin(); i != bst.lower_bound(TableEntry(compareVariable)); i++){
                        for(int j: i->second){
                            eraseIndices.push_back(j);
                        }
                    }
                    std::sort(eraseIndices.begin(), eraseIndices.end(), std::greater<int>());
                    for(int j: eraseIndices)
                    dataFromInsert.erase(dataFromInsert.begin() + j);
                    break;
                case '>' :
                    for(auto i = bst.upper_bound(TableEntry(compareVariable)); i != bst.end(); i++){
                        for(int j: i->second){
                            eraseIndices.push_back(j);
                        }
                    }
                    std::sort(eraseIndices.begin(), eraseIndices.end(), std::greater<int>());
                    for(int j: eraseIndices)
                    dataFromInsert.erase(dataFromInsert.begin() + j);
                    break;
                case '=' :
                    if(bst.find(TableEntry(compareVariable)) != bst.end()){
                        eraseIndices = bst.find(TableEntry(compareVariable))->second;
                        std::sort(eraseIndices.begin(), eraseIndices.end(), std::greater<int>());
                        for(auto i : eraseIndices){
                            dataFromInsert.erase(dataFromInsert.begin() + i);
                        }
                    }
            }
        }

        return true;
    }

    template<class T>
    //helper function specifically for when user wants to print with a predicate
    void printWhere(vector<int> &printColIndices, T compareVariable, int index, char op, int &numberOfRowsPrinted, bool verbose){
       
        if(op == '<'){
            IsRowLessThan isLess(index, TableEntry(compareVariable));
                for(auto i : dataFromInsert){
                    if(isLess(i)){
                        for(auto j : printColIndices){
                            if(verbose)
                                cout << i[j] << " ";
                        }
                        if(verbose)
                            cout << '\n';
                        numberOfRowsPrinted++;
                    }
                }
        }
        else if(op == '>'){
            IsRowGreaterThan isGreater(index, TableEntry(compareVariable));
                for(auto i : dataFromInsert){
                    if(isGreater(i)){
                        for(auto j : printColIndices){
                            if(verbose)
                                cout << i[j] << " ";
                        }
                        if(verbose)
                            cout << '\n';
                        numberOfRowsPrinted++;
                    }
                }
        }
        else{
            IsRowEqualTo isEqual(index, TableEntry(compareVariable));
                for(auto i : dataFromInsert){
                    if(isEqual(i)){
                        for(auto j : printColIndices){
                            if(verbose)
                                cout << i[j] << " ";
                        }
                        if(verbose)
                            cout << '\n';
                        numberOfRowsPrinted++;
                    }
                }
        }
    }

    template<class T>
    //returns true if it can print more efficiently using indices
    bool printWhereWithIndex(vector<int> &printColIndices, T compareVariable, int index, char op, int &numberOfRowsPrinted, bool verbose){
        //if the column at 'index' isnt indexed
        if(index != positionOfIndexedCol){
            return false;
        }
        //true for hash, false for bst
        bool hashOrBst = (bst.empty() ? true : false);
        
        //if hash map
        if(hashOrBst){
            if(op == '='){
                if(hash.find(TableEntry(compareVariable)) != hash.end()){
                    for(auto i : hash.find(TableEntry(compareVariable))->second){
                        for(auto j : printColIndices){
                            if(verbose)
                                cout << dataFromInsert[i][j] << " ";
                        }
                        if(verbose)
                            cout << '\n';
                        numberOfRowsPrinted++;
                    }
                }
            }
            //hash tables can only help with '='
            else{
                return false;
            }
        }
        //if bst
        else{
            switch(op){
                case '<':
                    //go through every key that is < compareVariable
                    for(auto i = bst.begin(); i != bst.lower_bound(TableEntry(compareVariable)); i++){
                        //go through the rows that have the specific key
                        for(auto j : i->second){
                            //go through each row
                            for(auto k : printColIndices){
                                if(verbose)
                                    cout << dataFromInsert[j][k] << " ";
                            }
                            if(verbose)
                                cout << '\n';
                            numberOfRowsPrinted++;
                        }
                    }
                    break;
                case '>':
                    //go through every key that is < compareVariable
                    for(auto i = bst.upper_bound(TableEntry(compareVariable)); i != bst.end(); i++){
                        //go through the rows that have the specific key
                        for(auto j : i->second){
                            //go through each row
                            for(auto k : printColIndices){
                                if(verbose)
                                    cout << dataFromInsert[j][k] << " ";
                            }
                            if(verbose)
                                cout << '\n';
                            numberOfRowsPrinted++;
                        }
                    }
                    break;
                case '=':
                    if(bst.find(TableEntry(compareVariable)) != bst.end()){
                        for(auto i : bst.find(TableEntry(compareVariable))->second){
                            for(auto j : printColIndices){
                                if(verbose)
                                    cout << dataFromInsert[i][j] << " ";
                            }
                            if(verbose)
                                cout << '\n';
                            numberOfRowsPrinted++;
                        }
                    }
            }
        }

        return true;
    }

public:

    //constructor
    Table(string name, int numCols, istream &in);

    //prints entire table
    void printAll();

    //inserts rows into a given table
    void insert(int rows, istream &in);

    //prints specific columns
    void printCols(int numCols, istream &in, bool verbose);

    //deletes rows given a certain predicate
    void deleteRows(string colOfInterest, char op, istream &in);

    //generates indices for a column in the table
    void generateIndices(bool hashOrBst, string column, bool verbose);

    //find index of a column, returns -1 if not found
    int findColumnIndex(string col);

    //helper function for join
    //hashes a column in table2 onto a given UOmap
    void hashTable2(unordered_map<TableEntry, vector<int>> &joinHash, int indexOfColumnOfInterestTable2);

    //helper function for join
    //finds matches between the two tables and stores them in "matches"
    void findMatches(unordered_map<TableEntry, vector<int>> &joinHash, vector<pair<int, vector<int>>> &matches, int indexOfColumnOfInterest);

    TableEntry returnData(int row, int col){
        return dataFromInsert[row][col];
    }

};