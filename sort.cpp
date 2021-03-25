#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

struct block {
    int eid = 0;            // 4 bytes
    char ename[40] = "0";   // 40 bytes
    int age = 0;            // 4 bytes
    double salary = 0;      // 8 bytes
};

void print_block(struct block* print_me) {
    cout << "===================================================================" << endl;
    cout << " eid: " << print_me->eid << " | ename: " << print_me->ename << " | age: "<< print_me->age << " | salary: " << int(print_me->salary) << endl;
    cout << "===================================================================" << endl;

}

// Referenced
// https://www.tutorialspoint.com/parsing-a-comma-delimited-std-string-in-cplusplus
void parse_csv_to_memory(string my_str, struct block* memory_slot) {
    stringstream s_stream(my_str); //create string stream from the string
    for(int i = 0; i < 4; i++) {
        string substr;
        getline(s_stream, substr, ','); //get first string delimited by comma

        switch(i) {
            case 0:
                memory_slot->eid = stoi(substr);
                break;
            case 1:
                substr.copy(memory_slot->ename, sizeof(char[40]));
                break;
            case 2:
                memory_slot->age = stoi(substr);
                break;
            case 3:
                memory_slot->salary = stod(substr);
                break;
        }

    }
}

string create_run_name(int run_number, int run_index) {
    stringstream run_file_name;
    run_file_name << "run-" << run_number << "." << run_index;
    
    return run_file_name.str();
}

void copy_memory_block(struct block* destination, struct block* source) {
    destination->eid = source->eid;
    strcpy(destination->ename, source->ename);
    destination->age = source->age;
    destination->salary = source->salary;
}

// A utility function to swap two elements 
void swap_memory_blocks(struct block* a, struct block* b) { 
    struct block t;
    copy_memory_block(&t, a);
    copy_memory_block(a, b);
    copy_memory_block(b, &t);
} 

void free_memory_blocks(int m, struct block** memory) {
    for(int i = 0; i < m; i++) {
        delete memory[i];
    }
    delete memory;    
}

// Referenced
// https://www.geeksforgeeks.org/quick-sort/
int partition (struct block* arr[], int low, int high) 
{ 
    int pivot = arr[high]->eid;
    int i = (low - 1);
 
    for (int j = low; j <= high - 1; j++) 
    { 
        if (arr[j]->eid < pivot) 
        { 
            i++;
            swap_memory_blocks(arr[i], arr[j]);
        } 
    } 
    swap_memory_blocks(arr[i+1], arr[high]);
    return (i + 1); 
} 

void quickSort(struct block* arr[], int low, int high) 
{ 
    if (low < high) 
    { 
        int pi = partition(arr, low, high); 
 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 

// Appending to the file_name
void write_out_one_block(string file_name, struct block output_block) {
    ofstream write_out_stream(file_name, fstream::app);

    stringstream current_block;

    current_block << output_block.eid << "," << output_block.ename << "," << output_block.age << "," << int(output_block.salary);     
    
    write_out_stream << current_block.str() << endl;

    write_out_stream.close();
}

void output_run(int run_number, int run_index, struct block** buffer, int blocks_to_write) {

    ofstream write_out_stream(create_run_name(run_number, run_index));

    for(int i = 0; i < blocks_to_write; i++) {
        stringstream current_block;
        current_block << buffer[i]->eid << "," << buffer[i]->ename << "," << buffer[i]->age << "," << int(buffer[i]->salary);     
        
        write_out_stream << current_block.str() << endl;  
    }

    write_out_stream.close();

}

// Returns how many lines read to buffer
int read_m_blocks_from_file_stream(int m, fstream& file_stream, struct block** memory_pointer) {

    for(int i = 0; i < m; i++) {

        string line_buffer;

        // If no more lines in file before memory full
        if(!getline(file_stream, line_buffer)) {
            return i;
        }

        memory_pointer[i] = new struct block;

        parse_csv_to_memory(line_buffer, memory_pointer[i]);
        
    }

    return m;
}

void do_initial_run(int m, fstream& file) {

    int blocks_last_read = m, index = 0;

    // While there are still blocks in csv
    for (int i = 0; blocks_last_read == m; i++) {
        struct block** memory_buffer = new struct block*[m]();

        blocks_last_read = read_m_blocks_from_file_stream(m, file, memory_buffer);        

        quickSort(memory_buffer, 0, blocks_last_read-1);

        output_run(0, i, memory_buffer, blocks_last_read);
        
        free_memory_blocks(blocks_last_read, memory_buffer);
    }
}

// Reads one block from m files and puts it into memory, if no more lines in file will set block to NULL
struct block** read_one_block_from_m(int m, fstream** m_files_streams) {
    int block_read_from_fs = 0;
    struct block** memory_blocks = new struct block*[m]();
    struct block** new_block = new struct block*[1]();


    for(int i = 0; i < m; i++) {

        // Allocates the memory for the new block
        // Will overwrite the first block if passing memory_blocks
        int read_block_check = read_m_blocks_from_file_stream(1, *m_files_streams[i], new_block);

        memory_blocks[i] = new_block[0];

        block_read_from_fs += read_block_check;

        if(read_block_check != 1) {
            memory_blocks[i] = NULL;
        }

    }

    delete new_block;

    return memory_blocks;
}

// Returns the amount of newly written runs
int do_subsequent_runs(int m, int run_number) {
    bool more_blocks = true;

    int run_index = 0;
    int starting_run_index = 0;

    int written_runs = 0;

    // For each m-1 files in run-i
    for(int run_index=0; more_blocks; run_index++) {

        string run_output_name = create_run_name(run_number, run_index);

        fstream** run_i_files = new fstream*[m-1];

        int runs_open = 0;

        for(int i = 0; i < (m-1); i++) {

            run_i_files[i] = new fstream; 
            run_i_files[i]->open(create_run_name(run_number-1, i+starting_run_index));
            
            if(!run_i_files[i]->is_open()) {
                more_blocks = false;
                runs_open = i;

                break;
            } else {
                runs_open = i+1;
            }

        }

        starting_run_index += runs_open;
        
        struct block** memory_blocks;
        if(runs_open > 0) {
            memory_blocks = read_one_block_from_m(runs_open, run_i_files);
        }

        bool blocks_to_write = true;

        while(blocks_to_write && runs_open != 0) {
            
            struct block* smallest_block = NULL;
            int smallest_index = 0;
            
            // write out the smallest block and replace with a new block
            for(int i = 0; i < runs_open; i++) {
                if(memory_blocks[i] != NULL) {

                    if(smallest_block == NULL) {
                    
                        smallest_block = memory_blocks[i];
                        smallest_index = i;
                    
                    } else {
                    
                        // cout << "Checking: " << memory_blocks[i]->eid << " < " << smallest_block->eid << endl;
                    
                        if(memory_blocks[i]->eid < smallest_block->eid) {
                            // cout << "Found: " << memory_blocks[i]->eid << " < " << smallest_block->eid << endl;

                            smallest_block = memory_blocks[i];
                            smallest_index = i;
                        }
                    
                    }

                    
                }

            }

            if(smallest_block == NULL) {
                blocks_to_write = false;
            } else {
                // Writeout index to run_run_number.run_index
                write_out_one_block(run_output_name, *smallest_block);

                delete smallest_block;

                smallest_block = NULL;

                // Need to create a fake block to copy
                struct block** new_block = new struct block*[1]();

                // Replace the block
                int read_block_check = read_m_blocks_from_file_stream(1, *run_i_files[smallest_index], new_block);

                if(read_block_check != 1) {
                    memory_blocks[smallest_index] = NULL;
                } else {
    
                    memory_blocks[smallest_index] = new_block[0];

                }

                delete new_block;
            }
            
        }

        // Close streams
        for(int i = 0; i < runs_open; i++) {
            run_i_files[i]->close();
        }

        free_memory_blocks(runs_open, memory_blocks);

        written_runs += 1;
    }

    return written_runs;

}

void rename_final_file(string last_run_name) {
    string final_name = "EmpSorted.csv";

    if(remove(final_name.c_str()) == 0) {
        cout << "Removed existing EmpSorted.csv" << endl;
    }

    fstream run_file(last_run_name);
    ofstream write_out_stream(final_name, fstream::app);
    
    string line_buffer;

    while(getline(run_file, line_buffer)) {
        write_out_stream << line_buffer << endl;
    }

    write_out_stream.close();
    run_file.close();

}

void read_csv_to_initial_run(int m, string file_location) {
    fstream csv_file(file_location);
    // Catch bad file path
    if(!csv_file.is_open()) {
        cout << "Error " << file_location << " could not be opened!" << endl;
        exit(1);
    }

    do_initial_run(m, csv_file);

    csv_file.close();
}

int main(int argc, char* argv[]) {

    int m = 22;

    read_csv_to_initial_run(m, "Emp.csv");

    int new_runs = 0;
    int run_number = 1;
    do {
        new_runs = do_subsequent_runs(m, run_number);

        if(new_runs == 1) {
            rename_final_file(create_run_name(run_number, 0));
        } else {
            run_number++;
        }
    } while (new_runs != 1);

    bool existing_run = true;
    bool existing_index = true;

    for(int i = 0; existing_run; i++) {
        existing_index = true;
        existing_run = (remove(create_run_name(i, 0).c_str()) == 0);

        if(!existing_run) {
            break;
        }

        for(int j = 1; existing_index; j++) {
            existing_index = (remove(create_run_name(i, j).c_str()) == 0);
        }

    }

    return 0;
}
