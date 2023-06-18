#include "part3.h"

#define DIRECTORY_ATTRIBUTE 0x01

void Parameter::dirCommand() {
    const string& directoryPath = getPar1();
    FileSystem fileSystem = getFat();

    cout << "Contents of directory " << directoryPath << ":" << endl;
    cout << "Name\tSize\tLast Modified" << endl;

    // Scan the FAT entries for directory entries
    for (int i = 0; i < 4096; i++) {
        if (fileSystem.getEntriesContent(i, directoryPath.size() + i) == directoryPath) {
            // Read the file/directory name
            string name = fileSystem.getEntriesContent(i, directoryPath.size() + i);

            // Read the file size (0 for directories)
            unsigned int size = fileSystem.getFileSize(); // Simplified. Should actually be read from the entry

            // Read the last modified time
            time_t lastModified = fileSystem.getLastModified(); // Simplified. Should actually be read from the entry

            // Convert the last modified time to string
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&lastModified));

            // Print the details
            cout << name << "\t" << size << "\t" << timeStr << endl;
        }
    }
}

void Parameter::mkdirCommand() {
    const string& directoryName = getPar1();
    FileSystem fileSystem = getFat();
    
    // Check if a directory with the same name already exists
    for (int i = 0; i < 4096; i++) {
        if (fileSystem.getEntriesContent(i, directoryName.size() + i) == directoryName) {
            cerr << "Error: Directory with the same name already exists." << endl;
            return;
        }
    }
    // Find a free FAT entry
    int freeFATEntry = -1;
    for (int i = 0; i < 4096; i++) {
        if (fileSystem.getEntry(i) == 0) {
            freeFATEntry = i;
            break;
        }
    }
    // If no free FAT entry is found, the disk is full
    if (freeFATEntry == -1) {
        cerr << "Error: No free space available on disk." << endl;
        return;
    }
    time_t currentTime;
    time(&currentTime); // Get current time

    fileSystem.setEntriesContent(freeFATEntry, directoryName); // Set directory name
    fileSystem.setFirstBlock(freeFATEntry); // This is a new directory, so it's the first block
    fileSystem.setLastModified(currentTime); // Set the last modified time to the current time
    fileSystem.setFileSize(0); // Directory, so file size is 0

    // Update the FAT entry to indicate that it's used
    fileSystem.setFatEntry(freeFATEntry, 0xFFFF); // 0xFFFF usually indicates end-of-file in FAT-based file systems
    cout << "Directory " << directoryName << " created successfully." << endl;
}

void Parameter::rmdirCommand() {
    const string& directoryPath = getPar1();
    FileSystem fileSystem = getFat();

    int fatIndex = -1;
    bool isEmpty = true;

    // Scan the FAT entries for directory entries
    for (int i = 0; i < 4096; i++) {
        if (fileSystem.getEntriesContent(i, directoryPath.size() + i) == directoryPath) {
            fatIndex = i;

            if (fileSystem.getFileSize() != 0) {
                isEmpty = false;
            }
            break;
        }
    }
    // Check if the directory was found
    if (fatIndex == -1) {
        cerr << "Error: Directory not found." << endl;
        return;
    }
    // Check if directory is empty
    if (!isEmpty) {
        cerr << "Error: Directory is not empty." << endl;
        return;
    }
    fileSystem.removeEntry(fatIndex);

    cout << "Directory " << directoryPath << " has been deleted successfully." << endl;
}

void Parameter::writeCommand() {
    FileSystem fileSystem = getFat();
    const string& customFilePath = getPar1();
    const string& linuxFilePath = getPar2();
    // Open the Linux file to read its content
    ifstream linuxFile(linuxFilePath, ios::binary);
    if (!linuxFile.is_open()) {
        cerr << "Error: Unable to open the Linux file " << linuxFilePath << endl;
        return;
    }
    // Read the entire content of the Linux file into a string
    string content((istreambuf_iterator<char>(linuxFile)), istreambuf_iterator<char>());
    linuxFile.close();
    // Check if there is enough free space in the custom file system
    if (content.size() > fileSystem.getFreeBlock() * fileSystem.getBlockSize()) {
        cerr << "Error: Not enough free space in the custom file system." << endl;
        return;
    }
    // Find the first free FAT entry for the new file
    int firstBlock = -1;
    for (int i = 0; i < 4096; i++) {
        if (fileSystem.getEntry(i) == 0) {
            firstBlock = i;
            break;
        }
    }
    fileSystem.setFilename(customFilePath.substr(customFilePath.find_last_of("\\") + 1)); // extracting file name from path
    fileSystem.setFileSize(content.size());
    fileSystem.setFirstBlock(firstBlock);
    fileSystem.setLastModified(time(nullptr));

    // Set file extension
    size_t dotPosition = fileSystem.getFilename().find_last_of(".");
    if (dotPosition != string::npos) {
        fileSystem.setExtension(fileSystem.getFilename().substr(dotPosition + 1));
    } else {
        fileSystem.setExtension("");
    }
    // Set file attributes (in this example, let's say 0 means regular file)
    fileSystem.setAttributes(0);
    fileSystem.addDirectoryEntry(fileSystem);
    fileSystem.writeFileData(firstBlock, content);

    fileSystem.setFatEntry(firstBlock, 0xFFFF); // Mark as end of file

    cout << "File " << customFilePath << " has been created and written to successfully." << endl;
}

void Parameter::readCommand() {
    FileSystem fileSystem = getFat();
    const string& customFilePath = getPar1();
    const string& linuxFilePath = getPar2();

    FileSystem entry = fileSystem.findDirectoryEntryByPath(customFilePath);
    if (entry.getFilename().empty()) {
        cerr << "Error: File not found in the custom file system" << endl;
        return;
    }

    string content = fileSystem.readFileData(entry.getFirstBlock(), entry.getFileSize());

    ofstream linuxFile(linuxFilePath, ios::binary);
    if (!linuxFile.is_open()) {
        cerr << "Error: Unable to open the Linux file for writing" << endl;
        return;
    }
    linuxFile.write(content.c_str(), content.size());
    linuxFile.close();
    cout << "File data has been successfully read from the custom file system and written to the Linux file." << endl;
}
void Parameter::delCommand() {
    const string& filePath = getPar1();
    FileSystem fileSystem = getFat();

    int fatIndex = -1;
    // Scan the FAT entries for file entries
    for (int i = 0; i < 4096; i++) {
        // Check if this entry matches the file path to be deleted
        if (fileSystem.getEntriesContent(i, filePath.size() + i) == filePath) {
            fatIndex = i;
            break;
        }
    }
    // Check if the file was found
    if (fatIndex == -1) {
        cerr << "Error: File not found." << endl;
        return;
    }
    fileSystem.removeEntry(fatIndex);
    // Get the first block of the file from the FAT
    unsigned short firstBlock = fileSystem.getEntry(fatIndex);
    // Iterate through the FAT to free the blocks used by this file
    unsigned short currentBlock = firstBlock;
    while (currentBlock != 0) { // Assuming 0 indicates the end of the file
        unsigned short nextBlock = fileSystem.getEntry(currentBlock);
        fileSystem.setFatEntry(currentBlock, 0); // Mark block as free
        currentBlock = nextBlock;
    }
    cout << "File " << filePath << " has been deleted successfully." << endl;
}

void Parameter::dumpCommand() {
    FileSystem fileSystem = getFat();
    // Printing the Superblock information
    cout << "Super Block Information:" << endl;
    cout << "Block size               : " << fileSystem.getBlockSize() << endl;
    cout << "Data position            : " << fileSystem.getDataPosition() << endl;
    cout << "FAT Position             : " << fileSystem.getFatPosition() << endl;
    cout << "Root Directory Position  : " << fileSystem.getRootDirectoryPosition() << endl;

    // Printing the Directory Entry information
    //cout << "\nRoot Directory Entry Information:" << endl;
    //cout << "File Name                : " << fileSystem.getFilename() << endl;
    //cout << "File Size                : " << fileSystem.getFileSize() << endl;
    //cout << "First Block              : " << fileSystem.getFirstBlock() << endl;

    time_t lastModified = fileSystem.getLastModified();
    char buffer[80];
    //strftime(buffer, 80, "%c", localtime(&lastModified));
    //cout << "Last Modified            : " << buffer << endl;

    // Printing the FAT Entries information
    //cout << "\nFAT Entries:" << endl;
    /*for (int i = 0; i < 4096; i++) {
        cout << "FAT Entry " << i << " : " << fileSystem.getEntry(i) << endl;
    }*/
}

void Parameter::process(){
    string temp = getOp();
    if(temp.compare("mkdir") == 0){
        mkdirCommand();
    }
    else if(temp.compare("rmdir") == 0){
        rmdirCommand();
    }
    else if(temp.compare("write") == 0){
        writeCommand();
    }
    else if(temp.compare("read") == 0){
        readCommand();
    }
    else if(temp.compare("del") == 0){
        delCommand();
    }
    else if(temp.compare("dumpe2fs") == 0){
        dumpCommand();
    }
    else if(temp.compare("dir") == 0){
        dirCommand();
    }
    else {
        cerr <<"Command is not available. "<<endl;
        exit(EXIT_FAILURE);
    }
}
Parameter::Parameter(string filename) : fat(filename){
    this->parameter1 = "";
    this->parameter2 = "";
    this->operation = "";
    FileSystem fat(filename); // take only filename
    this->fat = fat;
    
}
void Parameter::setS(int argc, char* argv[]){
    setop(string(argv[2]));
    string temp = getOp();
    if (argc == 4 && (temp.compare("mkdir") == 0 || temp.compare("rmdir") == 0 || temp.compare("dir") == 0 || temp.compare("del") == 0)){
        setpar1(argv[3]);
    }
    else if (argc == 5 && (temp.compare("write") == 0 || temp.compare("read") == 0)){
        setpar1(argv[3]);
        setpar2(argv[4]);
    }
}
/**********************************************************************************/
void FileSystem::setFatEntry(unsigned int fatIndex, unsigned short fatValue) {
    if (fatIndex >= 4096) {
        cerr << "Error: Invalid FAT index!" << endl;
        exit(EXIT_FAILURE);
    }

    entries[fatIndex] = fatValue;
}

void FileSystem::readfile() {
    ifstream file(filename);
if (!file.is_open()) {
    cerr << "Error: Unable to open the file " << filename << endl;
    exit(EXIT_FAILURE);
}

string line;
while (getline(file, line)) {
    if (line.find("Block size") != string::npos) {
        setBlockSize(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Root Directory Position") != string::npos) {
        setRootDirectoryPosition(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("FAT Position") != string::npos) {
        setFatPosition(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Data position") != string::npos) {
        setDataPosition(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Total Blocks") != string::npos) {
        setTotalBlock(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Free Blocks") != string::npos) {
        setFreeBlock(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("FAT Blocks") != string::npos) {
        setFatBlock(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Number of Files") != string::npos) {
        setNumFiles(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("File Name") != string::npos) {
        setFilename(line.substr(line.find(":") + 1));
    }
    else if (line.find("File Size") != string::npos) {
        setFileSize(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("First Block") != string::npos) {
        setFirstBlock(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("Last Modified") != string::npos) {
        string timeString = line.substr(line.find(":") + 1);
        struct tm tm;
        strptime(timeString.c_str(), " %a %b %d %H:%M:%S %Y", &tm);
        setLastModified(mktime(&tm));
    }
    else if (line.find("Extension") != string::npos) {
        setExtension(line.substr(line.find(":") + 1));
    }
    else if (line.find("Attributes") != string::npos) {
        setAttributes(stoul(line.substr(line.find(":") + 1)));
    }
    else if (line.find("FAT Entry") != string::npos) {
        unsigned int fatIndex = stoul(line.substr(9, line.find(":") - 9));
        unsigned short fatValue = stoul(line.substr(line.find(":") + 1));
        setFatEntry(fatIndex, fatValue);
    }
}

file.close();

}

void FileSystem::setEntriesContent(int index, const std::string& content) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string encodedContent = converter.from_bytes(content);

    // Store the encoded content in the entries array
    entries[index] = static_cast<unsigned short int>(encodedContent[0]);
}
std::string FileSystem::getEntriesContent(int startIndex, int endIndex) const {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::string content;

    for (int i = startIndex; i <= endIndex; ++i) {
        std::u16string encodedContent(1, static_cast<char16_t>(entries[i]));
        std::string entry = converter.to_bytes(encodedContent);
        content += entry;
    }
    return content;
}
FileSystem::FileSystem(string filename){
    this->filename = filename;
    readfile();
}

void FileSystem::removeEntry(unsigned int index) {
    unsigned int entriesSize = sizeof(entries) / sizeof(entries[0]);
    
    if (index >= entriesSize) {
        cerr << "Error: Invalid index!" << endl;
        exit(EXIT_FAILURE);
    }
    for (unsigned int i = index; i < entriesSize - 1; ++i) {
        entries[i] = entries[i + 1];
    }
    entries[entriesSize - 1] = 0; // Set the last element to a default value (assuming 0)
}
void FileSystem::addDirectoryEntry(const FileSystem& entry) {
    fstream file(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Unable to open the file system file " << filename << endl;
        return;
    }
    file.seekp(getRootDirectoryPosition(), ios::beg);

    FileSystem tempEntry; // Declare a temporary variable of type FileSystem

    while (file.read((char*)&tempEntry, sizeof(FileSystem))) {
        if (tempEntry.getFilename().empty()) {
            break; // found an empty slot
        }
    }

    // Write the new directory entry
    file.seekp(-static_cast<streamoff>(sizeof(FileSystem)), ios::cur); // Go back to the start of the empty slot
    file.write((const char*)&entry, sizeof(FileSystem));

    file.close();
}


void FileSystem::writeFileData(unsigned short firstBlock, const string& content) {
    fstream file(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Unable to open the file system file " << filename << endl;
        return;
    }

    // Calculate the position to start writing data
    unsigned int dataPosition = getDataPosition() + (firstBlock * getBlockSize());

    // Navigate to the position
    file.seekp(dataPosition, ios::beg);

    // Write the content
    file.write(content.c_str(), content.size());

    file.close();
}
FileSystem::FileSystem(){  }
FileSystem FileSystem::findDirectoryEntryByPath(const std::string& path) {
    fstream file(filename, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Unable to open the file system file " << filename << endl;
        return FileSystem();
    }
    // Start from the root directory position
    file.seekg(getRootDirectoryPosition(), ios::beg);

    // Split the path into individual directory names
    vector<string> directories;
    stringstream ss(path);
    string directory;
    while (getline(ss, directory, '\\')) {
        if (!directory.empty()) {
            directories.push_back(directory);
        }
    }
    // Traverse the directory structure
    FileSystem entry;
    for (const auto& dir : directories) {
        bool found = false;
        while (file.read((char*)&entry, sizeof(FileSystem))) {
            if (entry.filename == dir) {
                if ((entry.attributes & DIRECTORY_ATTRIBUTE) != 0) {
                    // Directory entry found, move to the next level
                    file.seekg(entry.first_block * getBlockSize(), ios::beg);
                    found = true;
                    break;
                }
                else {
                    // Found an entry with the same name, but it's a file, not a directory
                    cerr << "Error: Path contains a file name instead of a directory" << endl;
                    file.close();
                    return FileSystem();
                }
            }
        }
        if (!found) {
            // Directory not found
            cerr << "Error: Directory not found in the custom file system" << endl;
            file.close();
            return FileSystem();
        }
    }

    file.close();
    return entry;
}
std::string FileSystem::readFileData(unsigned short firstBlock, unsigned int fileSize) {
    std::string content;
    // Open the file system file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open the file system file " << filename << std::endl;
        return content;
    }
    // Calculate the position to start reading data
    unsigned int dataPosition = getDataPosition() + (firstBlock * getBlockSize());

    // Navigate to the position
    file.seekg(dataPosition, std::ios::beg);

    // Read the content from the custom file system
    content.resize(fileSize);
    file.read(reinterpret_cast<char*>(content.data()), fileSize);

    file.close();

    return content;
}