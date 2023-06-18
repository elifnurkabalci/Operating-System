#ifndef PART3_H
#define PART3_H 

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <set>
#include <ctime>
#include <cstdlib>
#include <filesystem>

using namespace std;

class FileSystem{
private:
    // Superblock attributes
    unsigned int block_size;
    unsigned int root_directory_position;
    unsigned int fat_position;
    unsigned int data_position;
    unsigned int total_block;
    unsigned int free_block;
    unsigned int fat_block;
    unsigned int num_files;

    // DirectoryEntry attributes
    string filename;
    unsigned int file_size;
    time_t last_modified;
    unsigned short first_block;
    string extension;
    unsigned char attributes;
    char reserved[10];

    // FAT attributes
    unsigned short entries[4096];

public:
    // Getters and Setters for Superblock attributes
    unsigned int getBlockSize() const { return block_size; }
    void setBlockSize(unsigned int blockSize) { block_size = blockSize; }
    
    unsigned int getRootDirectoryPosition() const { return root_directory_position; }
    void setRootDirectoryPosition(unsigned int position) { root_directory_position = position; }
    
    unsigned int getFatPosition() const { return fat_position; }
    void setFatPosition(unsigned int position) { fat_position = position; }
    
    unsigned int getDataPosition() const { return data_position; }
    void setDataPosition(unsigned int position) { data_position = position; }
    
    unsigned int getTotalBlock() const { return total_block; }
    void setTotalBlock(unsigned int totalBlock) { total_block = totalBlock; }
    
    unsigned int getFreeBlock() const { return free_block; }
    void setFreeBlock(unsigned int freeBlock) { free_block = freeBlock; }
    
    unsigned int getFatBlock() const { return fat_block; }
    void setFatBlock(unsigned int fatBlock) { fat_block = fatBlock; }
    
    unsigned int getNumFiles() const { return num_files; }
    void setNumFiles(unsigned int numFiles) { num_files = numFiles; }

    // Getters and Setters for DirectoryEntry attributes
    string getFilename() const { return filename; }
    void setFilename(const string &filename) { this->filename = filename; }
    
    unsigned int getFileSize() const { return file_size; }
    void setFileSize(unsigned int fileSize) { file_size = fileSize; }
    
    time_t getLastModified() const { return last_modified; }
    void setLastModified(time_t lastModified) { last_modified = lastModified; }
    
    unsigned short getFirstBlock() const { return first_block; }
    void setFirstBlock(unsigned short firstBlock) { first_block = firstBlock; }
    
    string getExtension() const { return extension; }
    void setExtension(const string &extension) { this->extension = extension; }
    
    unsigned char getAttributes() const { return attributes; }
    void setAttributes(unsigned char attributes) { this->attributes = attributes; }
    
    const char* getReserved() const { return reserved; }
    void setReserved(const char reserved[10]) {
        for (int i = 0; i < 10; ++i) {
            this->reserved[i] = reserved[i];
        }
    }
    
    // FAT related methods
    const unsigned short* getEntries() const { return entries; }
    unsigned short getEntry(int index) { return entries[index]; }
    void setEntries(const unsigned short* newEntries) {
        for (int i = 0; i < 4096; ++i) {
            entries[i] = newEntries[i];
        }
    }
    void setFatEntry(unsigned int fatIndex, unsigned short fatValue);

    // Other methods
    FileSystem(string filename);
    FileSystem();
    void readfile();
    void setEntriesContent(int index, const std::string& content);
    vector<std::string> split(const std::string& str, char delimiter);
    void removeEntry(unsigned int index);
    string getEntriesContent(int startIndex, int endIndex) const;
    void addDirectoryEntry(const FileSystem& entry);
    void writeFileData(unsigned short firstBlock, const string& content);
    FileSystem findDirectoryEntryByPath(const std::string& path);
    std::string readFileData(unsigned short firstBlock, unsigned int fileSize);
};

class Parameter{
private:
    string parameter1;
    string parameter2;
    string operation;
    FileSystem fat;

public:
    string getPar1(){ return parameter1;}
    string getPar2(){ return parameter2;}
    string getOp() const{ return operation;}
    
    FileSystem getFat(){ return fat; }

    void setpar1(string par){ this->parameter1 = par; }
    void setpar2(string par){ this->parameter2 = par; }
    void setop(const string& op){ this->operation = op; }
    void setFat(FileSystem fat){this->fat = fat;}
    Parameter(string filename);
    void process();
    void setS(int argc, char* argv[]);
    
    void dirCommand();
    void mkdirCommand();
    void rmdirCommand();
    void writeCommand();
    void readCommand();
    void delCommand();
    void dumpCommand();
};
#endif // PART3_H