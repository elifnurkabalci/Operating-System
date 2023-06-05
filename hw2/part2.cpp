#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <string.h>
#include <string>
#include <math.h>
#include <sys/time.h>
#include <fstream>
#include <pthread.h>

using namespace std;

struct PageTableEntry{
    int referenced; 
    int present; 
    int modified;
    int frameIndex;
    int LRUcounter;
	unsigned long accessTime;
};
typedef PageTableEntry PTE;

struct Statistics{
    int read;
    int write;
    int miss;
    int replacement;
    int diskWrite;
    int diskRead;
    vector<vector<int>> workingSet;
};
typedef Statistics statistic;

struct Parameter{
    int frameSize;
    int numPhysical;
    int numVirtual;
    int pageTablePrintInt;
    char *pageReplacement;
    char *tableType;
    char *diskFileName;
};
typedef Parameter par;

struct helper{ // for global variables
    int LRUcounter = 0;
    int WSCcounter = 0;
    int MemoryAccesscounter = 0;
};
typedef helper help;

vector<vector<int>> physicalM; // memory definitons
vector<vector<int>> virtualM;
vector<int> pageFrame;

// calling structres globally
par user;
statistic stat[6]; // for every thread
PTE* pte; // page table entries
help helper;

// thread declarations
pthread_t threads[6]; // linear, binary, fill, matrix multiplication, vector multiplication, array summation
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Statistic adders
void numOfRead(char* type){ // increase number of read
    if(strcmp(type, "linear") == 0){ 
        stat[0].read++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].read++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].read++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].read++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].read++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].read++;
    }
}
void numOfWrite(char* type){ // increase number of write
    if(strcmp(type, "linear") == 0){ 
        stat[0].write++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].write++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].write++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].write++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].write++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].write++;
    }
}
void numOfMiss(char* type){ // increase number of miss
    if(strcmp(type, "linear") == 0){ 
        stat[0].miss++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].miss++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].miss++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].miss++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].miss++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].miss++;
    }
}
void numOfReplacement(char* type){ // increase number of replacement
    if(strcmp(type, "linear") == 0){ 
        stat[0].replacement++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].replacement++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].replacement++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].replacement++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].replacement++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].replacement++;
    }
}
void numOfdiskWrite(char* type){ // increase number of disk write
    if(strcmp(type, "linear") == 0){
        stat[0].diskWrite++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].diskWrite++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].diskWrite++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].diskWrite++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].diskWrite++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].diskWrite++;
    }
}
void numOfDiskRead(char* type){ // increase number of disk read
    if(strcmp(type, "linear") == 0){ 
        stat[0].diskRead++;
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].diskRead++;
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].diskRead++;
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].diskRead++;
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].diskRead++;
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].diskRead++;
    }
}
void numOfWorkingSet(char* type){ // add working set 
    if(strcmp(type, "linear") == 0){ 
        stat[0].workingSet.push_back(pageFrame);
    }
    else if(strcmp(type, "binary") == 0){
        stat[1].workingSet.push_back(pageFrame);
    }
    else if(strcmp(type, "fill") == 0){ 
        stat[2].workingSet.push_back(pageFrame);
    }
    else if(strcmp(type, "matM") == 0){ 
        stat[3].workingSet.push_back(pageFrame);
    }
    else if(strcmp(type, "vecM") == 0){ 
        stat[4].workingSet.push_back(pageFrame);
    }
    else if(strcmp(type, "sum") == 0){ 
        stat[5].workingSet.push_back(pageFrame);
    }
}
void initials_structs(){
    pte = (PageTableEntry*)malloc(user.numVirtual*sizeof(PageTableEntry));
    // pdf says fill is make from threads, 
    for(int i=0; i<6; ++i){
		stat[i].read = 0;
		stat[i].write = 0;
		stat[i].miss = 0;
		stat[i].replacement = 0;
		stat[i].diskRead = 0;
		stat[i].diskWrite = 0;
	}
    for(int i=0; i<user.numVirtual; ++i){
	    pte[i].referenced = 0;
	    pte[i].present = 0; 
	    pte[i].modified = 0; 
	    pte[i].frameIndex = -1;
	    pte[i].LRUcounter = 0;
		pte[i].accessTime = 0;		
	}
}
void assign_enteries(int argc, char **argv){
    if(argc != 8){
        cerr<<"Number of entries is wrong, Please check your parameter and run again."<<endl;
        exit(EXIT_FAILURE);
    }
    user.frameSize = atoi(argv[1]);
    user.numPhysical = atoi(argv[2]);
	user.numVirtual = atoi(argv[3]);
	user.pageReplacement = argv[4];
	user.tableType = argv[5];
	user.pageTablePrintInt = atoi(argv[6]);
	user.diskFileName = argv[7];

    if(!((strcmp(user.pageReplacement,"SC") == 0) || (strcmp(user.pageReplacement,"LRU") == 0) || (strcmp(user.pageReplacement,"WSClock") == 0))){
        cerr<<"You need to choose SC or LRU or WSClock for page replacement type, Please check your parameter and run again."<<endl;
        exit(EXIT_FAILURE);
    }
    if(user.frameSize<0 || user.numPhysical<0 || user.numVirtual<0 || user.numVirtual<user.numPhysical){
        cerr<<"There is a mistake from frameSize or #of pyhsical or virtual, Please check your parameter and run again."<<endl;
        exit(EXIT_FAILURE);
    }
    user.frameSize = pow(2, user.frameSize); // definitions in hw pdf
	user.numPhysical = pow(2, user.numPhysical); 
	user.numVirtual = pow(2, user.numVirtual);
}
void write_file_memory(){
    ofstream file; // open for writing
    file.open(user.diskFileName); 

    if(!file){
        cerr<<"File cannot open"<<endl;
        exit(EXIT_FAILURE);
    }
    int random, vm_index=0;

    for(int i=0; i<user.numVirtual; i++){
        vector<int> vec_random;
        vector<int> vec_vm;

        for(int j=0; j<user.frameSize; j++){
            random = rand();
            vec_random.push_back(random);
            vec_vm.push_back(vm_index++);
        }
        if(i<user.numPhysical){
            physicalM.push_back(vec_random); 
            pageFrame.push_back(i);
            pte[i].present = 1;
            pte[i].frameIndex = i;
        }
        virtualM.push_back(vec_vm);
        numOfWrite((char*)"fill");
        numOfdiskWrite((char*)"fill");
    }
    file.close();

}
void print_pageTable(){
    cout<<endl<<"Page Table: "<<endl;
    for(int i=0; i<user.numVirtual; i++){
        cout<<"Frame number:          "<<i<<endl;
        cout<<"R bit:                 "<<pte[i].referenced<<endl;
        cout<<"M bit:                 "<<pte[i].modified<<endl;
        cout<<"Present/absent bit:    "<<pte[i].present<<endl;
        cout<<"Frame # in Physical M: "<<pte[i].frameIndex<<endl;
        cout<<"LRU Counter:           "<<pte[i].LRUcounter<<endl;
        cout<<"Access Time:           "<<pte[i].accessTime<<endl;
    }
}
vector<int> getPageUnit(int x, int y){
    int line = x*user.frameSize;
    int index=0;
    ifstream file; // open for read
    string s ="";
    vector<int> pageFrame;

    file.open(user.diskFileName);
    if(!file){
        cerr<<"File cannot open"<<endl;
        exit(EXIT_FAILURE);
    }
    while(index<line && getline(file, s)){
        index++;
    }
    for(int i=0; i<user.frameSize; i++){
        getline(file, s);
        pageFrame.push_back(stoi(s));
    }
    file.close();
    return pageFrame;
}
void SC(vector<int> page, int frame, char *type){
    vector<int> p; // template page
    int f; // template frame

    while(1){
        f = pageFrame[0];
        if(pte[f].referenced == 1){ // this page referenced
            p = physicalM[0];
            f = pageFrame[0];

            physicalM.erase(physicalM.begin()); //removes page 
			physicalM.insert(physicalM.end(), p); //puts it to the end

            pte[f].frameIndex = pageFrame.size()-1;
            pte[f].referenced = 0;
            pte[f].present = 1;

            pageFrame.erase(pageFrame.begin());
            pageFrame.insert(pageFrame.end(), f);

            for(int i=0; i<pageFrame.size()-1; i++){
                f = pageFrame[i];
                pte[f].frameIndex = pte[f].frameIndex -1;
            }
        }
        else{ 
            pte[f].frameIndex = -1; // delete all frame information, return to start
            pte[f].present = 0;
            pte[f].referenced = 0;

            physicalM.erase(physicalM.begin()); //removes page 
			physicalM.insert(physicalM.begin(), page);	//puts the new page 
			
			pte[frame].frameIndex = 0; //update page frame index
			pte[frame].present = 1;			
			pte[frame].referenced = 1;			

			//changes page's frame index in pageFrameArr
			pageFrame.erase(pageFrame.begin());
			pageFrame.insert(pageFrame.begin(), frame);

			break;			
        }
    }
}
void LRU(vector<int> page, int frame, char *type){
    int index;
    int min = 1234567;
    for(int i=0; i<pageFrame.size(); i++){
        if(pte[pageFrame[i]].LRUcounter < min){
            min = pte[pageFrame[i]].LRUcounter;
            index = i;
        }
    }
    pte[pageFrame[index]].frameIndex = index; 
    pte[pageFrame[index]].present = 1;
    pte[pageFrame[index]].referenced = 0;

    pte[frame].frameIndex = index;
    pte[frame].present = 1;
    pte[frame].referenced = 1;

    pageFrame[index] = frame; // update page frame

    physicalM.erase(physicalM.begin() + index); // delete old page
    physicalM.insert(physicalM.begin() + index, page); // insert new page

}
void WSClock(vector<int> page, int frame, char *type){
    int f, index;
	unsigned long currTime;
    struct timeval t;
	while(1){
		f = pageFrame[helper.WSCcounter];

		if(pte[f].referenced == 1){ //if R bit is 1, it changes it with 0
			pte[f].referenced = 0;
            
            gettimeofday(&t, NULL);
			pte[f].accessTime = t.tv_sec/1000000 + t.tv_usec ;
		}
		else{
            gettimeofday(&t, NULL);
			unsigned long currTime = t.tv_sec/1000000 + t.tv_usec ;	

			//checks page age
			if((currTime - pte[f].accessTime) > 100){
				index = helper.WSCcounter;
				break;
			}			
		}	
		helper.WSCcounter++;
		if(helper.WSCcounter == pageFrame.size()) // return to start
			helper.WSCcounter = 0;
	}
	pte[pageFrame[index]].frameIndex = -1; // return to start
	pte[pageFrame[index]].present = 0;	
	pte[pageFrame[index]].referenced = 0;	

	pte[frame].frameIndex = index; //set new page's index and present bit
	pte[frame].present = 1;
	pte[frame].referenced = 1;

	pageFrame[index] = frame; //update page frame in page table

	
	physicalM.erase(physicalM.begin() + index); //delete old page
	physicalM.insert(physicalM.begin() + index, page);	//inserts new page
}
void pageReplace(vector<int> page, int frame, char* type){// used x for frame number
    if(strcmp(user.pageReplacement,"SC") == 0)
		SC(page, frame, type);
	else if(strcmp(user.pageReplacement,"LRU") == 0)
		LRU(page, frame, type);
	else if(strcmp(user.pageReplacement,"WSClock") == 0)
		WSClock(page, frame, type);
}
int hit_miss(int x, int y, int val, char* type){
    if(pte[x].present == 1){ // hit
        x = pte[x].frameIndex;
        val = physicalM[x][y];
        if(pthread_mutex_unlock(&mutex) != 0){
			perror("pthread_mutex_unlock: ");
			exit(EXIT_FAILURE);			
		}
		return val;
    }
    else{ // miss
        vector<int> page = getPageUnit(x,y); // create new disk element for page table
        numOfMiss(type); // add numbers statics
        numOfDiskRead(type);
        numOfReplacement(type);
        numOfWorkingSet(type);

        pageReplace(page, x, type);

        if(pthread_mutex_unlock(&mutex) != 0){
			perror("pthread_mutex_unlock: ");
			exit(EXIT_FAILURE);			
		}
		return page[y];
    }
}
int get(int index, char* type){
    int x = index/user.frameSize;
    int y = index%user.frameSize;
    int val;

    if(helper.MemoryAccesscounter++ == user.pageTablePrintInt){
        helper.MemoryAccesscounter = 0; // restart for clock cycle
        print_pageTable();
    }
    numOfRead(type);

    if(pthread_mutex_lock(&mutex) != 0){
		perror("pthread_mutex_lock: ");
		exit(EXIT_FAILURE);
	}

    pte[x].referenced = 1; 
    pte[x].LRUcounter = helper.LRUcounter++;

    struct timeval t;
    gettimeofday(&t, NULL);
    pte[x].accessTime = t.tv_sec/1000000 + t.tv_usec ;

    return hit_miss(x,y,val,type);
}
void print(){
    int index=0;
    for (int i = 0; i < user.numVirtual; i++){
		for (int j = 0; j < user.frameSize; j++)
			cout<<get(index++, (char*)"")<<"\t";
		cout<<endl;
	}
	cout<<endl;
}
void linearSearch(){
    int arr[6];
    for(int i=0; i<6; i++){
        arr[i] = rand();
    }
    int temp;
    cout<<endl<<"Linear Search"<<endl;
    for(int i=0; i<6; i++){
        cout<<"Search: "<<arr[i]<<endl;
        temp=0;
        for(int j=0; j<user.numVirtual* user.frameSize; j++){
            if(arr[i] == get(j, (char*)"linear")){
                cout<<arr[i]<<" is at address "<<j<<endl;
                temp=1;
            }
        }
        if(temp==0){
            cout<<arr[i]<<" cannot find in the array."<<endl;
        }
    }

}
int binary(int left, int right, int num){
    int mid;

    if (left <= right){
        mid = left + (right-left)/2;
 
        //if number is at mid element
        if(num == get(mid, (char*)"binary"))
            return mid;

        //if number is smaller than mid element
        if(num < get(mid, (char*)"binary"))
            return binary(left, mid-1, num);
 
		//if number is bigger than mid element
        return binary(mid+1, right, num);
    }

    return -1;
}
void binarySearch(){
    long int arr[6];
    for(int i=0; i<6; i++){
        arr[i] = 1234567;
    }
    int temp;
    cout<<endl<<"Binary Search"<<endl;
    for(int i=0; i<6; i++){
        cout<<"Search: "<<arr[i]<<endl;
        temp = binary(0, user.numVirtual*user.frameSize-1, arr[i]); // make binary search for every array unit
        if(temp > -1){
            cout<<arr[i]<<" is at address "<<temp<<endl;
        }
        else{
            cout<<arr[i]<<" cannot find in the array."<<endl;
        }
    }
}
void matrix_product(){ // ı made 2d vector , because of ı use the physical memory and virtual memory
    vector<vector<int> > result;
    size_t m = physicalM.size();
    size_t n = virtualM[0].size();
    size_t p = virtualM.size();

      result.resize(m, std::vector<int>(n, 0));

    if (physicalM[0].size() != p) {
        cerr << "Vector sizes are not compatible for multiplication." << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < p; k++) {
                result[i][j] += physicalM[i][k] * virtualM[k][j];
            }
        }
    }
    cout<<"Result: "<<endl;
    for (int i=0; i<result.size(); i++){
        for (int j=0; j<result[0].size(); j++) {
            cout<<result[i][j]<<" ";
        }
        cout<<endl;
    }
}
void vec_product(){ // ı use the page frame
    vector<int> vec1 = pageFrame; // ı tested with pageframe
    vector<int> vec2 = pageFrame;
    vector<int> result;

    if (vec1.size() != vec2.size()) { // this control for using other vectors 
        cout << "Error: Vector sizes do not match." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < vec1.size(); i++) {
        result.push_back(vec1[i] * vec2[i]);
    }
}
void summation(){
    int temp=0, result = 0;
    int arr1[6], arr2[6];
    for(int i=0; i<6; i++){
        arr1[i] = stat[i].read; // number of reads all threads
        arr2[i] = stat[i].write; // number of writes all threads
    } 
    for(int i=0; i<6; i++){
        result = arr1[i] + arr2[i];
    }
    cout<<"Array summation is: "<<result<<endl;
    
}
void *thread_func(void* id){
    int thread_id = *(int*)id;
    if(thread_id == 0){
        linearSearch();
    }
    else if(thread_id == 1){
        binarySearch();
    }
    else if(thread_id == 2){
        write_file_memory();
    } 
    else if(thread_id == 3){
        matrix_product();
    }
    else if(thread_id == 4){
        vec_product();
    }
    else if(thread_id == 5){
        summation();
    }
    return 0;
}
void thread_op(){
    int thread_id[6];

    for(int i=0; i<6; i++){
        thread_id[i]=i;
        if(pthread_create(&threads[i], NULL, thread_func, (void *)&thread_id[i]) != 0) { 
            perror("pthread_create: ");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0; i<3; i++) { //waits until sorting threads done
        if(pthread_join(threads[i], NULL) == -1) { 
            perror("pthread_join: ");
            exit(EXIT_FAILURE);
        }
    }

    for(int i=3; i<6; i++) { // continue the using threads
        if(pthread_join(threads[i], NULL) == -1) { 
            perror("pthread_join: ");
            exit(EXIT_FAILURE);
        }
    } 	
}
void printStatistic(){
	cout<<"**********Statistics**********"<<endl;
	for(int i=0; i<6; i++){
		if(i == 0)
			cout<<"Linear Search"<<endl;
		else if(i == 1)
			cout<<"\nBinary Searh"<<endl;
		else if(i == 2)
			cout<<"\nFill"<<endl;
		else if(i == 3)
			cout<<"\nMatrix Multiplication"<<endl;
		else if(i == 4)
			cout<<"\nVector Multiplication"<<endl;
		else if(i == 5)
			cout<<"\nSummation"<<endl;

		cout<<"Number of reads:\t\t"<<stat[i].read<<endl;
		cout<<"Number of writes:\t\t"<<stat[i].write<<endl;
		cout<<"Number of page misses:\t\t"<<stat[i].miss<<endl;
		cout<<"Number of page replacements:\t"<<stat[i].replacement<<endl;
		cout<<"Number of disk page writes:\t"<<stat[i].diskWrite<<endl;
		cout<<"Number of disk page reads:\t"<<stat[i].diskRead<<endl;	

		cout<<"Working sets:"<<endl;
		for(int j=0; j<(stat[i].workingSet).size(); ++j){
			cout<<"w"<<j<<":\t";
			for(int k=0; k<(stat[i].workingSet[j]).size(); ++k){
				cout<<stat[i].workingSet[j][k]<<" ";
			}
			cout<<endl;
		}
	}	
}
int main(int argc, char **argv){
    srand(1000); // for random number genarate, always different
    assign_enteries(argc, argv);
    initials_structs();

    cout<<"\nBefore:"<<endl;
    print();

    thread_op();

    cout<<"\nAfter:"<<endl;
    print();

    printStatistic();

    if(pthread_mutex_destroy(&mutex)!=0){
        perror("pthread_mutex_destroy: ");
        exit(EXIT_FAILURE);
    }

    free(pte);
    return 0;
}