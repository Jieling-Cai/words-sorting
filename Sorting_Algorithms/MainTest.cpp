#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <ctime>
#include <vector>
#include <exception>

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif __has_include(<filesystem>)
#       ifdef _MSC_VER
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>
#               if defined(_HAS_CXX17) && _HAS_CXX17
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif
#       else
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#       include <experimental/filesystem>
     	namespace fs = std::experimental::filesystem;
#   else
#       include <filesystem>
#		if __APPLE__
			namespace fs = std::__fs::filesystem;
#		else
			namespace fs = std::filesystem;
#		endif
#   endif
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions and Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MULTITHREADED_ENABLED 1

enum class ESortType { AlphabeticalAscending, AlphabeticalDescending, LastLetterAscending };
enum class algoType { BubbleSort, RadixSort };

class IStringComparer {
public:
	virtual bool IsFirstAboveSecond(string _first, string _second) = 0;
};

class AlphabeticalAscendingStringComparer : public IStringComparer {
public:
	virtual bool IsFirstAboveSecond(string _first, string _second);
};

class LastLetterAscendingStringComparer : public IStringComparer {
public:
	virtual bool IsFirstAboveSecond(string _first, string _second);
};

void DoSingleThreaded(vector<string> _fileList, ESortType _sortType, string _outputName, algoType _algoType);
void DoMultiThreaded(vector<string> _fileList, ESortType _sortType, string _outputName, algoType _algoType);
vector<string> ReadFile(string _fileName);
void ThreadedReadFile(string _fileName, vector<string>* _listOut);
void BubbleSort(vector<string>* _listToSort, ESortType _sortType);
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken, algoType _algoType);
void swap(string* a, string* b);
vector<string> mergeSort(vector<string> _sortedList1, vector<string> _sortedList2, ESortType _sortType);
size_t getMaxLen(vector<string>* arr, int n);
void countSort(vector<string>* arr, int size, size_t k, size_t max_len, ESortType _sortType);
void radixSort(vector<string>* b, int n, ESortType _sortType);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
	// Enumerate the directory for input files
	vector<string> fileList;
    string inputDirectoryPath = "../InputFiles";
    for (const auto & entry : fs::directory_iterator(inputDirectoryPath)) {
		if (!fs::is_directory(entry)) {
			fileList.push_back(entry.path().string());
		}
	}
	// Do the stuff
	DoSingleThreaded(fileList, ESortType::AlphabeticalAscending, "SingleAscending", algoType::RadixSort);
	DoSingleThreaded(fileList, ESortType::AlphabeticalDescending, "SingleDescending", algoType::RadixSort);
	DoSingleThreaded(fileList, ESortType::LastLetterAscending, "SingleLastLetter", algoType::RadixSort);
	// DoSingleThreaded(fileList, ESortType::AlphabeticalAscending, "SingleAscending", algoType::BubbleSort);
	// DoSingleThreaded(fileList, ESortType::AlphabeticalDescending, "SingleDescending", algoType::BubbleSort);
	// DoSingleThreaded(fileList, ESortType::LastLetterAscending, "SingleLastLetter", algoType::BubbleSort);
#if MULTITHREADED_ENABLED
	DoMultiThreaded(fileList, ESortType::AlphabeticalAscending, "MultiAscending", algoType::RadixSort);
	DoMultiThreaded(fileList, ESortType::AlphabeticalDescending, "MultiDescending", algoType::RadixSort);
	DoMultiThreaded(fileList, ESortType::LastLetterAscending, "MultiLastLetter", algoType::RadixSort);
	// DoMultiThreaded(fileList, ESortType::AlphabeticalAscending, "MultiAscending", algoType::BubbleSort);
	// DoMultiThreaded(fileList, ESortType::AlphabeticalDescending, "MultiDescending", algoType::BubbleSort);
	// DoMultiThreaded(fileList, ESortType::LastLetterAscending, "MultiLastLetter", algoType::BubbleSort);
#endif
	// Wait
	cout << endl << "Finished...";
	getchar();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Stuff
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoSingleThreaded(vector<string> _fileList, ESortType _sortType, string _outputName, algoType _algoType) {
	clock_t startTime = clock();
	vector<string> masterStringList;
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		vector<string> fileStringList = ReadFile(_fileList[i]);
		for (unsigned int j = 0; j < fileStringList.size(); ++j) {
			masterStringList.push_back(fileStringList[j]);
		}
	}
	try
	{
		if(_algoType==algoType::BubbleSort)
			BubbleSort(&masterStringList, _sortType);
		else if(_algoType==algoType::RadixSort)
			radixSort(&masterStringList, masterStringList.size(), _sortType);
	}
	catch (exception& e)
	{
		cout << "Standard exception: " << e.what() << endl;
	}
	clock_t endTime = clock();
	WriteAndPrintResults(masterStringList, _outputName, endTime - startTime, _algoType);
}

void DoMultiThreaded(vector<string> _fileList, ESortType _sortType, string _outputName, algoType _algoType) {
	clock_t startTime = clock();
	vector<vector<string>> masterStringList(_fileList.size());;
	// Get lists (multi-threading)
	vector<thread> workerThreads(_fileList.size());
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		workerThreads[i] = thread(ThreadedReadFile, _fileList[i], &masterStringList[i]);
	}
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		workerThreads[i].join(); 
	}
	// Sort (multi-threading)
	vector<thread> workerThreads2(_fileList.size());
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		try{
			if(_algoType==algoType::BubbleSort)
				workerThreads2[i] = thread(BubbleSort, &masterStringList[i], _sortType);
			else if(_algoType==algoType::RadixSort)
				workerThreads2[i] = thread(radixSort, &masterStringList[i], masterStringList[i].size(), _sortType);
		}
		catch (exception& e)
		{
			cout << "Standard exception: " << e.what() << endl;
		}
	}
	for (unsigned int i = 0; i < _fileList.size(); ++i) {
		workerThreads2[i].join(); 
	}
	// Merge sorted arrays 
	for(unsigned int i=1; i<masterStringList.size(); i++)
		masterStringList[i] = mergeSort(masterStringList[i-1], masterStringList[i], _sortType);
	clock_t endTime = clock();
	WriteAndPrintResults(masterStringList[masterStringList.size()-1], _outputName, endTime - startTime, _algoType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> ReadFile(string _fileName) {
	vector<string> listOut;
	streampos positionInFile = 0;
	bool endOfFile = false;
	ifstream fileIn(_fileName, ifstream::in);
	while (!endOfFile) {
		fileIn.seekg(positionInFile, ios::beg);
		string *tempString = new string();
		getline(fileIn, *tempString);

		endOfFile = fileIn.peek() == EOF; 
		positionInFile = endOfFile? ios::beg : fileIn.tellg();
		if(!endOfFile)
			listOut.push_back(*tempString);
		else{	
			listOut.push_back(*tempString+'\r');
		}
	}
	fileIn.close();
	return listOut; 
}

void ThreadedReadFile(string _fileName, vector<string>* _listOut) {
	*_listOut = ReadFile(_fileName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sorting
////////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> mergeSort(vector<string> _sortedList1, vector<string> _sortedList2, ESortType _sortType) {
	unsigned int i=0, j=0;
	vector<string> masterList;
	while(i<_sortedList1.size() && j<_sortedList2.size()){
		if(_sortType == ESortType::AlphabeticalAscending){
			AlphabeticalAscendingStringComparer* stringSorter = new AlphabeticalAscendingStringComparer();
			if (stringSorter->IsFirstAboveSecond(_sortedList1[i], _sortedList2[j])){
				masterList.push_back(_sortedList1[i]);
				i++;
			}
			else{
				masterList.push_back(_sortedList2[j]);
				j++;
			}
		}
		else if(_sortType == ESortType::AlphabeticalDescending){
			AlphabeticalAscendingStringComparer* stringSorter = new AlphabeticalAscendingStringComparer();
			if (!stringSorter->IsFirstAboveSecond(_sortedList1[i], _sortedList2[j])){
				masterList.push_back(_sortedList1[i]);
				i++;
			}
			else{
				masterList.push_back(_sortedList2[j]);
				j++;
			}
		}
		else if(_sortType == ESortType::LastLetterAscending){
			LastLetterAscendingStringComparer* stringSorter = new LastLetterAscendingStringComparer();
			if (stringSorter->IsFirstAboveSecond(_sortedList1[i], _sortedList2[j])){
				masterList.push_back(_sortedList1[i]);
				i++;
			}
			else{
				masterList.push_back(_sortedList2[j]);
				j++;
			}
		}
	}
	while(i<_sortedList1.size()){
		masterList.push_back(_sortedList1[i]);
		i++;
	}
	while(j<_sortedList2.size()){
		masterList.push_back(_sortedList2[j]);
		j++;
	}
	return masterList;
}

bool AlphabeticalAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
	unsigned int i = 0;
	while (i < _first.length() && i < _second.length()) {
		if (_first[i] < _second[i])
			return true;
		else if (_first[i] > _second[i])
			return false;
		++i;
	}
	return (i == _first.length());
}
	
bool LastLetterAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
	int i, j;
	for(i = _first.length()-1, j = _second.length()-1; i>=0 && j>=0; i--, j--) {
		if (_first[i] < _second[j])
			return true;
		else if (_first[i] > _second[j])
			return false;
	}
	return (i == 0);
}

void BubbleSort(vector<string>* _listToSort, ESortType _sortType) {
	vector<string> sortedList = * _listToSort;
	for (unsigned int i = 0; i < sortedList.size() - 1; ++i) {
		for (unsigned int j = 0; j < sortedList.size() - i - 1; ++j) {
			if(_sortType == ESortType::AlphabeticalAscending){
				AlphabeticalAscendingStringComparer* stringSorter = new AlphabeticalAscendingStringComparer();
				if (!stringSorter->IsFirstAboveSecond(sortedList[j],sortedList[j+1])) 
					swap(&sortedList[j], &sortedList[j+1]);
			}
			else if(_sortType == ESortType::AlphabeticalDescending){
				AlphabeticalAscendingStringComparer* stringSorter = new AlphabeticalAscendingStringComparer();
				if (stringSorter->IsFirstAboveSecond(sortedList[j],sortedList[j+1])) 
					swap(&sortedList[j], &sortedList[j+1]);
			}
			else if(_sortType == ESortType::LastLetterAscending){
				LastLetterAscendingStringComparer* stringSorter = new LastLetterAscendingStringComparer();
				if (!stringSorter->IsFirstAboveSecond(sortedList[j],sortedList[j+1])) 
					swap(&sortedList[j], &sortedList[j+1]);
			}
		}
	}
	* _listToSort = sortedList;
}

void swap(string* a, string* b){
	string tempString = *a;
	*a = *b;
	*b = tempString;
}

size_t getMaxLen(vector<string>* arr, int n){
    vector<string> a = *arr;
    size_t max = a[0].size();
    for (int i = 1; i < n; i++){
        if (a[i].size()>max)
            max = a[i].size();
    }
    return max;
}

void countSort(vector<string>* arr, int size, size_t k, size_t max_len, ESortType _sortType){
    string *b = NULL; int *c = NULL;
    vector<string> a = *arr;
    b = new string[size];
    c = new int[257];

    for (int i = 0; i <257; i++){
        c[i] = 0;
    }

    if(_sortType == ESortType::AlphabeticalAscending){
        for (int j = 0; j <size; j++){  
            int bucket_index =  k < a[j].size() ? (int)(unsigned char)a[j][k] + 1 : 0;
            c[bucket_index]++;          
        }
    }
    else if(_sortType == ESortType::AlphabeticalDescending){
        for (int j = 0; j <size; j++){  
            int bucket_index =  k < a[j].size() ? (int)(unsigned char)a[j][k] + 1 : 0;
            c[256-bucket_index]++;          
        }
    }
    else if(_sortType == ESortType::LastLetterAscending){
        for (int j = 0; j <size; j++){  
            int bucket_index =  (max_len-k) <= a[j].size() ? (int)(unsigned char)a[j][k-max_len+a[j].size()] + 1 : 0;
            c[bucket_index]++;          
        }
    }

    for (int f = 1; f <257; f++){
        c[f] += c[f - 1];
    }

    if(_sortType == ESortType::AlphabeticalAscending){
        for (int r = size - 1; r >= 0; r--){
            int bucket_index = k < a[r].size() ? (int)(unsigned char)a[r][k] + 1 : 0;
            b[--c[bucket_index]] = a[r];
        }
    }
    else if(_sortType == ESortType::AlphabeticalDescending){
        for (int r = size - 1; r >= 0; r--){
            int bucket_index = k < a[r].size() ? (int)(unsigned char)a[r][k] + 1 : 0;
            b[--c[256-bucket_index]] = a[r];
        }
    }
    else if(_sortType == ESortType::LastLetterAscending){
        for (int r = size - 1; r >= 0; r--){
             int bucket_index =  (max_len-k) <= a[r].size() ? (int)(unsigned char)a[r][k-max_len+a[r].size()] + 1 : 0;
            b[--c[bucket_index]] = a[r];
        }
    }

    for (int l = 0; l < size; l++){
        a[l] = b[l];
    }
    *arr = a;
    // avold memory leak
    delete[] b;
    delete[] c;
}


void radixSort(vector<string>* b, int n, ESortType _sortType){
    size_t max = getMaxLen(b, n);
    if(_sortType == ESortType::AlphabeticalAscending){
        for (size_t digit = max; digit > 0; digit--){ 
            countSort(b, n, digit - 1, max, ESortType::AlphabeticalAscending);
        }
    }
    else if(_sortType == ESortType::AlphabeticalDescending){
        for (size_t digit = max; digit > 0; digit--){ 
            countSort(b, n, digit - 1, max, ESortType::AlphabeticalDescending);
        }
    }
    else if(_sortType == ESortType::LastLetterAscending){
        for (size_t digit = 0; digit < max; digit++){ 
            countSort(b, n, digit, max, ESortType::LastLetterAscending);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteAndPrintResults(const vector<string>& _masterStringList, string _outputName, int _clocksTaken, algoType _algoType){
	if(_algoType == algoType::BubbleSort) cout << endl << "Using the sorting algorithm: BubbleSort" << endl;
	else if(_algoType == algoType::RadixSort) cout << endl << "Using the sorting algorithm: RadixSort" << endl;

	cout << endl << _outputName << "\t- Clocks Taken: " << _clocksTaken << endl;
	ofstream fileOut(_outputName + ".txt", ofstream::trunc);
	for (unsigned int i = 0; i < _masterStringList.size(); ++i) {
		fileOut << _masterStringList[i] << endl;
	}
	fileOut.close();
}