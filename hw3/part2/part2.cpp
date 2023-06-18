#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <string>
using namespace std;

struct Superblock {
    unsigned int block_size;
    unsigned int root_directory_position;
    unsigned int fat_position; // fat start block
    unsigned int data_position;

    unsigned int total_block;
    unsigned int free_block;
    unsigned int fat_block; // fat block count
    unsigned int num_files;
};

struct DirectoryEntry {
    string file_name;
    unsigned int file_size;
    time_t last_modified;
    unsigned short first_block;

    string extension;
    unsigned char attributes;
    char reserved[10];
};

struct FAT {
    unsigned short entries[4096];
};

int create_file_system(string file_name, unsigned int block_size_kb) {
    unsigned int block_size = block_size_kb * 1024; // convert block size to bytes

    // Create and open the file
    std::ofstream file(file_name);
    if (!file) {
        std::cerr << "Error opening file." << std::endl;
        return -1;
    }

    // Initialize the superblock
    Superblock superblock;
    superblock.block_size = block_size;
    superblock.root_directory_position = sizeof(Superblock);
    superblock.fat_position = superblock.root_directory_position + sizeof(DirectoryEntry) * 128;
    superblock.data_position = superblock.fat_position + sizeof(FAT);
    superblock.total_block = 4096; // Need to be calculated based on the file system size
    superblock.free_block = 4094; // Initially all blocks are free
    superblock.fat_block = 1; // Assuming one block for FAT
    superblock.num_files = 0; // Initially no files in the file system

    // Write superblock to file
    file << "Super Block" << endl;
    file << "Block size              : " << superblock.block_size << endl;
    file << "Data position           : " << superblock.data_position << endl;
    file << "FAT Position            : " << superblock.fat_position << endl;
    file << "Root Directory Position : " << superblock.root_directory_position << endl;
    file << "Total Blocks            : " << superblock.total_block << endl;
    file << "Free Blocks             : " << superblock.free_block << endl;
    file << "FAT Blocks              : " << superblock.fat_block << endl;
    file << "Number of Files         : " << superblock.num_files << endl << endl;

    // Initialize root directory
    DirectoryEntry root;
    root.file_name = "/";
    root.file_size = 0;
    root.last_modified = std::time(nullptr); // current time
    root.first_block = 0; // root directory should be the first block
    root.extension = "";
    root.attributes = 0; // default attributes
    std::memset(root.reserved, 0, sizeof(root.reserved)); // set reserved bytes to 0
    
    // Write root directory to file
    file << "Root Directory Entry" << endl;
    file << "File Name     : " << root.file_name << endl;
    file << "File Size     : " << root.file_size << endl;
    file << "First Block   : " << root.first_block << endl;
    file << "Last Modified : " << std::ctime(&root.last_modified) << endl;
    file << "Extension     : " << root.extension << endl;
    file << "Attributes    : " << (int)root.attributes << endl << endl;

    // Initialize and write FAT
    FAT fat;
    std::memset(fat.entries, 0x00, sizeof(fat.entries)); // Set all entries to 0
    
    // Write FAT to file
    file << "FAT Entries" << endl;
    for (int i = 0; i < 4096; i++) {
        file << "FAT Entry " << i << " : " << fat.entries[i] << endl;
    }
    
    file.close();
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <block_size> <file_system_file>" << std::endl;
        return 1;
    }
    
    unsigned int block_size = std::stoi(argv[1]);
    
    if(block_size >4){
        cout<<"This blocksize is more than FAT12 system, Enter 4 or less."<<endl;
        exit(EXIT_FAILURE);
    }
    create_file_system(argv[2], block_size);
    return 0;
}
